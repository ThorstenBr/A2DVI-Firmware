/*
MIT License

Copyright (c) 2021 Mark Aikens
Copyright (c) 2023 David Kuder
Copyright (c) 2024 Thorsten Brehm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <pico/stdlib.h>
#include <string.h>
#include "abus.h"
#include "businterface.h"
#include "buffers.h"
#include "config/config.h"
#include "config/device_regs.h"
#include "fonts/textfont.h"

static uint8_t romx_unlocked;
static uint8_t romx_textbank;

typedef enum
{
    WriteMem = 0,
    ReadMem  = 1,
    WriteDev = 2,
    ReadDev  = 3,
} TAccessMode;

static inline void __time_critical_func(machine_auto_detection)(uint32_t address)
{
    if(current_machine == MACHINE_AUTO)
    {
        if((apple_memory[0x0403] == 0xD8) && (apple_memory[0x404] == 0xE5)) {        // "Xe" = ROMXe
            detected_machine = MACHINE_IIE;
            set_machine(MACHINE_IIE);
        } else if((apple_memory[0x0412] == 0xC5) && (apple_memory[0x0413] == 0xD8)) {// "EX" = ROMX
            detected_machine = MACHINE_II;
            set_machine(MACHINE_II);
#if 0
        } else if((apple_memory[0x416] == 0xC9) && (apple_memory[0x0417] == 0xE7)) { // "Ig" = Apple IIgs
            detected_machine = MACHINE_IIGS;
            set_machine(MACHINE_IIGS);
#endif
        } else if((apple_memory[0x416] == 0xAF) && (apple_memory[0x0417] == 0xE5)) { // "/e" = Apple //e Enhanced
            detected_machine = MACHINE_IIE;
            set_machine(MACHINE_IIE);
        } else if((apple_memory[0x413] == 0xE5) && (apple_memory[0x0415] == 0xDD)) { // "e ]" = Apple //e Unenhanced
            detected_machine = MACHINE_IIE;
            set_machine(MACHINE_IIE);
        } else if(apple_memory[0x0410] == 0xD0) { // "P" = Apple II/Plus/J-Plus with Autostart
            detected_machine = MACHINE_II;
            set_machine(MACHINE_II);
        } else if((apple_memory[0x07D0] == 0xAA)&&(apple_memory[0x07D1] == 0x60)) { // "*(CURSOR)" = Apple II without Autostart
            detected_machine = MACHINE_II;
            set_machine(MACHINE_II);
        } else if(apple_memory[0x0410] == 0xF2) { // "r" = Pravetz!
            detected_machine = MACHINE_PRAVETZ;
            set_machine(MACHINE_PRAVETZ);
        }
    }
}

// Control sequences used by ROMX and ROMXe
static inline void __time_critical_func(check_romx_read)(uint32_t address)
{
    switch(current_machine)
    {
        case MACHINE_IIE:
            // Trigger on read sequence FACA FACA FAFE
            if((address >> 8) == 0xFA)
            {
                switch(address & 0xFF)
                {
                    case 0xCA:
                        romx_unlocked = (romx_unlocked == 1) ? 2 : 1;
                        break;
                    case 0xFE:
                        romx_unlocked = (romx_unlocked == 2) ? 3 : 0;
                        break;
                    default:
                        if(romx_unlocked != 3)
                            romx_unlocked = 0;
                        break;
                }
            }
            else
            if(romx_unlocked == 3)
            {
                if((address >> 4) == 0xF81)
                {
                    romx_textbank = (MAX_FONT_COUNT-CUSTOM_FONT_COUNT) + (address & 0xF);
                }
                else
                if(address == 0xF851)
                {
                    cfg_local_charset = romx_textbank;
                    reload_charsets   = 1;
                    romx_unlocked     = 0;
                }
            }
            break;
        case MACHINE_II:
            // Trigger on read sequence CACA CACA CAFE
            if((address >> 8) == 0xCA)
            {
                switch(address & 0xFF)
                {
                    case 0xCA:
                        romx_unlocked = (romx_unlocked == 1) ? 2 : 1;
                        break;
                    case 0xFE:
                        romx_unlocked = (romx_unlocked == 2) ? 3 : 0;
                        break;
                    default:
                        if(romx_unlocked != 3)
                            romx_unlocked = 0;
                        break;
                }
            }
            else
            if(romx_unlocked == 3)
            {
                if((address >> 4) == 0xCFD)
                {
                    romx_textbank = (MAX_FONT_COUNT-CUSTOM_FONT_COUNT) + (address & 0xF);
                }
                if((address >> 4) == 0xCFE)
                {
                    cfg_local_charset = romx_textbank;
                    reload_charsets   = 1;
                    romx_unlocked     = 0;
                }
            }
            break;
        default:
            break;
    }
}

static inline void __time_critical_func(apple2_softswitches)(TAccessMode AccessMode, uint32_t address, uint8_t data)
{
    switch(address & 0x7f)
    {
    case 0x00: // 80STOREOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_80STORE;
        }
        break;
    case 0x01: // 80STOREON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_80STORE;
        }
        break;
    case 0x02: // RAMRDOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_AUX_READ;
        }
        break;
    case 0x03: // RAMRDON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_AUX_READ;
        }
        break;
    case 0x04: // RAMWRTOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_AUX_WRITE;
        }
        break;
    case 0x05: // RAMWRTON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_AUX_WRITE;
        }
        break;
    case 0x06: // INTCXROMOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_CXROM;
        }
        break;
    case 0x07: // INTCXROMON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_CXROM;
        }
        break;
    case 0x08: // ALTZPOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_AUXZP;
        }
        break;
    case 0x09: // ALTZPON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_AUXZP;
        }
        break;
    case 0x0a: // SLOTC3ROMOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_SLOT3ROM;
        }
        break;
    case 0x0b: // SLOTC3ROMOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_SLOT3ROM;
        }
        break;
    case 0x0c: // 80COLOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_80COL;
        }
        break;
    case 0x0d: // 80COLON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_80COL;
        }
        break;
    case 0x0e: // ALTCHARSETOFF
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_ALTCHAR;
        }
        break;
    case 0x0f: // ALTCHARSETON
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_ALTCHAR;
        }
        break;
    case 0x19: // VBLANK
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == ReadMem))
            vblank_counter += 1;
        break;
    case 0x21: // COLOR/MONO
        if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
        {
            if(data & 0x80)
            {
                soft_switches |= SOFTSW_MONOCHROME;
            }
            else
            {
                soft_switches &= ~SOFTSW_MONOCHROME;
            }
        }
        break;
#ifdef APPLEIIGS
    case 0x22:
        if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
        {
            apple_tbcolor = data;
        }
        break;
    case 0x29:
        if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
        {
            soft_switches = (soft_switches & ~(SOFTSW_NEWVID_MASK << SOFTSW_NEWVID_SHIFT)) | ((data & SOFTSW_NEWVID_MASK) << SOFTSW_NEWVID_SHIFT);
        }
        break;
    case 0x34:
        if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
        {
            apple_border = data;
        }
        break;
    case 0x35:
        if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
        {
            soft_switches = (soft_switches & ~(SOFTSW_SHADOW_MASK << SOFTSW_SHADOW_SHIFT)) | ((data & SOFTSW_SHADOW_MASK) << SOFTSW_SHADOW_SHIFT);
        }
        break;
#endif
    case 0x50: // TEXTOFF
        soft_switches &= ~SOFTSW_TEXT_MODE;
        break;
    case 0x51: // TEXTON
        soft_switches |= SOFTSW_TEXT_MODE;
        break;
    case 0x52: // MIXEDOFF
        soft_switches &= ~SOFTSW_MIX_MODE;
        break;
    case 0x53: // MIXEDON
        soft_switches |= SOFTSW_MIX_MODE;
        break;
    case 0x54: // PAGE2OFF
        soft_switches &= ~SOFTSW_PAGE_2;
        break;
    case 0x55: // PAGE2ON
        soft_switches |= SOFTSW_PAGE_2;
        break;
    case 0x56: // HIRESOFF
        soft_switches &= ~SOFTSW_HIRES_MODE;
        break;
    case 0x57: // HIRESON
        soft_switches |= SOFTSW_HIRES_MODE;
        break;
    case 0x5e: // DGRON
        if(internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS))
        {
            soft_switches |= SOFTSW_DGR;
        }
        break;
    case 0x5f: // DGROFF
        // Video 7 shift register
        if(soft_switches & SOFTSW_DGR)
        {
            internal_flags = (internal_flags & 0xfffffffc) | ((internal_flags & 0x1) << 1) | ((soft_switches & SOFTSW_80COL) ? 1 : 0);
        }

        if(internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS))
        {
            soft_switches &= ~SOFTSW_DGR;
        }
        break;
    case 0x7e: // IOUDISOFF
        if((internal_flags & IFLAGS_IIE_REGS) && (AccessMode == WriteMem))
        {
            soft_switches |= SOFTSW_IOUDIS;
        }
        break;
    case 0x7f: // IOUDISON
        if((internal_flags & IFLAGS_IIE_REGS) && (AccessMode == WriteMem))
        {
            soft_switches &= ~SOFTSW_IOUDIS;
        }
        break;
    }
}

static void __time_critical_func(apple2emulation)(TAccessMode AccessMode, uint32_t address, uint8_t data)
{
    if (address < 0x100)
        last_address_zp = address;
    else
    if (address < 0x200)
        last_address_stack = address;
    else
    if (address == last_address+1)
        last_address_pc = address;
    last_address = address;

    // Shadow parts of the Apple's memory by observing the bus write cycles
    if(AccessMode == WriteMem)
    {
        if(address < MAX_ADDRESS)
        {
            // Mirror Video Memory from MAIN & AUX banks
            if ((soft_switches & SOFTSW_80STORE)&&
                (((address >= 0x400) && (address < 0x800))||
                ((soft_switches & SOFTSW_HIRES_MODE) && (address >= 0x2000) && (address < 0x4000))))
            {
                // 80STORE is on AND address is within an active display page
                if(soft_switches & SOFTSW_PAGE_2)
                    private_memory[address] = data;
                else
                    apple_memory[address] = data;
                // nothing else to do
                return;
            }

            if (address >= 0x200)
            {
                if(soft_switches & SOFTSW_AUX_WRITE)
                {
                    private_memory[address] = data;
                    return;
                }
                else
                if((internal_flags & IFLAGS_MENU_ENABLE)==0)
                {
                    apple_memory[address] = data;
                    if (address < 0x800)
                    {
                        machine_auto_detection(address);
                    }
                }

                // Nothing left to do for RAM write.
                return;
            }
        }
        else
        if ((address & 0xFF00) == card_rom_address)
        {
            // access to card's ROM area
            devicerom_counter++;
            return;
        }
    }
    else
    if(AccessMode == ReadMem)
    {
        // Control sequences used by ROMX and ROMXe
        check_romx_read(address);
    }

    // nothing to do addresses outside register area
    if ((address & 0xF800) != 0xc000)
        return;

    // Shadow the soft-switches by observing all read & write bus cycles
    if(address < 0xc080)
    {
        return apple2_softswitches(AccessMode, address, data);
    }

    // Card Registers
    if ((AccessMode == WriteDev)||
        (AccessMode == ReadDev))
    {
        // remember the slot number
        cardslot = (address >> 4) & 0x7;
        if (cardslot)
        {
            // remember address range of card's ROM area ($Cs00, s=1..7)
            card_rom_address = 0xC000 | (cardslot << 8);
        }
        devicereg_counter++;
    }

    if (AccessMode == WriteDev)
    {
        device_write(address & 0xF, data);
    }
}

void __time_critical_func(businterface)(uint32_t value)
{
    uint32_t access_mode = ACCESS_WRITE(value) ? 0 : 1;
    if (CARD_DEVSEL(value))
        access_mode |= 2;
    uint32_t address = ADDRESS_BUS(value);

    apple2emulation(access_mode, address, value & 0xff);

    // Apple II reset detection: monitor addresses
    if(access_mode != ReadMem)
        reset_state = 0;
    else
    switch(reset_state)
    {
        case 0:
            if (address == 0xFFFC) // reset vector, low byte
                reset_state++;
            break;
        case 1:
            if (address == 0xFFFD) // reset vector, high byte
                reset_state++;
            else
                reset_state = 0;
            break;
        case 2:
            if (address == 0xFA62) // Apple II reset vector address
            {
                soft_switches   = SOFTSW_TEXT_MODE;
                internal_flags &= ~(IFLAGS_MENU_ENABLE);
                internal_flags |= IFLAGS_V7_MODE3;
                romx_unlocked = 0;
                // clear dev register lock
                dev_config_lock = 0;
                reset_counter++;
                bus_overflow_counter = 0;
            }
            // fall-through
        default:
            reset_state = 0;
            break;
    }
}
