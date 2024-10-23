/*
MIT License

Copyright (c) 2021 Mark Aikens
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

#include <string.h>
#include <hardware/pio.h>
#include "abus.h"
#include "abus_setup.h"
#include "abus_pin_config.h"
#include "buffers.h"
#include "config/config.h"
#include "config/device_regs.h"
#include "fonts/textfont.h"

#define VIDEX_ABUS
#include "videx/videx_vterm.h"

// reset vector for Apple II AUTOSTART ROMs (and all IIe ROMs)
#define APPLEII_AUTOSTART_ROM_RESET 0xFA62

// reset vector for original Apple II ROM
#define APPLEII_ORIGINAL_ROM_RESET  0xFA59

// address of the "HOME" routine (common to all Apple II ROMs)
// (This is executed _after_ the beep, so with a little delay.
//  Also, this is exeucted just prior to clearing the text area.)
#define APPLEII_ROM_HOME            0xFC58

static uint8_t  romx_unlocked;
static uint8_t  romx_textbank;
static bool     ramworks_active;

typedef void (*a2busfunc)(uint32_t value);

static inline void __time_critical_func(machine_auto_detection)(uint32_t address)
{
    if(current_machine == MACHINE_AUTO)
    {
        char Char1 = apple_memory[0x0413];
        char Char2 = apple_memory[0x0415];
        if (Char1 == 0xA0) // ' '
        {
            if (((uint16_t*)apple_memory)[0x07D0>>1] == 0x60AA)
            {
                detected_machine = MACHINE_II;  // "*(CURSOR)" = Apple II without Autostart
                set_machine(MACHINE_II);
            }
        }
        else
        if (Char1 == 0xC5) // 'E'
        {
            if (Char2 == 0xDD) // ']': "APPLE ]["
            {
                detected_machine = MACHINE_II; // Apple II/Plus/J-Plus with Autostart
                set_machine(MACHINE_II);
            }
        }
        else
        if (Char1 == 0xE5) // 'e'
        {
            if (Char2 == 0xDD) // ']': "Apple ]["
            {
                detected_machine = MACHINE_IIE; // Apple IIe unenhanced
                set_machine(MACHINE_IIE);
            }
            else
            if (Char2 == 0xAF) // '/': "Apple //e"
            {
                detected_machine = MACHINE_IIE_ENH;  // Apple //e enhanced
                set_machine(MACHINE_IIE_ENH);
            }
            else
            if (Char2 == 0xFA) // 'z': "Pravetz"
            {
                detected_machine = MACHINE_PRAVETZ;
                set_machine(MACHINE_PRAVETZ);
            }
#if 0
            else
            if (Char2 == 0xC9) // 'I': "Apple IIgs"
            {
                detected_machine = MACHINE_IIGS;
                set_machine(MACHINE_IIGS);
            }
#endif
        }
        else
        if (Char1 == 0xD8) // 'X'
        {
            if (Char2 == 0xC8) // 'H': ROMX ("ROM EXCHANGE")
            {
                detected_machine = MACHINE_II;
                set_machine(MACHINE_II);
            }
        }
        else
        if (Char1 == 0xee) // 'n' "ROMXe: theRomExchange.com"
        {
            if (Char2 == 0xE5) // ''e'
            {
                detected_machine = MACHINE_IIE_ENH;
                set_machine(MACHINE_IIE_ENH);
            }
        }
    }
}

// Control sequences used by ROMX: CACA CACA CAFE
static inline void romx_cxxx_check_read(uint_fast16_t address)
{
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
            reload_charsets  |= 1;
            romx_unlocked     = 0;
        }
    }
}

// Control sequences used by ROMXe
static inline void romxe_faxx_check_read(uint_fast16_t address)
{
    // only supported on IIe
    if (!IS_IFLAG(IFLAGS_IIE_REGS))
        return;

    // Trigger on read sequence FACA FACA FAFE
    if((address >> 8) == 0xFA)
    {
        if (address == 0xFACA)
            romx_unlocked = (romx_unlocked == 1) ? 2 : 1;
        else
        if (address == 0xFAFE)
            romx_unlocked = (romx_unlocked == 2) ? 3 : 0;
        else
        if(romx_unlocked != 3)
            romx_unlocked = 0;
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
            reload_charsets  |= 1;
            romx_unlocked     = 0;
        }
    }
}

static inline void __time_critical_func(apple2_softswitches)(bool is_write, uint32_t address, uint32_t value)
{
    switch(address & 0x7f)
    {
    case 0x00: // 80STOREOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_80STORE;
        }
        break;
    case 0x01: // 80STOREON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_80STORE;
        }
        break;
    case 0x02: // RAMRDOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_AUX_READ;
        }
        break;
    case 0x03: // RAMRDON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_AUX_READ;
        }
        break;
    case 0x04: // RAMWRTOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_AUX_WRITE;
        }
        break;
    case 0x05: // RAMWRTON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_AUX_WRITE;
        }
        break;
    case 0x06: // INTCXROMOFF: slot ROMs mapped to CXXX range
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_INTCXROM;
        }
        break;
    case 0x07: // INTCXROMON: internal ROM mapped to CXXX range
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_INTCXROM;
        }
        break;
    case 0x08: // ALTZPOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_AUXZP;
        }
        break;
    case 0x09: // ALTZPON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_AUXZP;
        }
        break;
    case 0x0a: // SLOTC3ROMOFF: 80 column ROM mapped to C3xx range
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_SLOT3ROM;
        }
        break;
    case 0x0b: // SLOTC3ROMON: slot 3's ROM mapped to C3xx range
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_SLOT3ROM;
        }
        break;
    case 0x0c: // 80COLOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_80COL;
        }
        break;
    case 0x0d: // 80COLON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_80COL;
        }
        break;
    case 0x0e: // ALTCHARSETOFF
        if (is_write) // on IIE+IIGS only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_ALTCHAR;
        }
        break;
    case 0x0f: // ALTCHARSETON
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_ALTCHAR;
        }
        break;
    case 0x19: // VBLANK
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (!is_write))
            vblank_counter += 1;
        break;
    case 0x21: // COLOR/MONO
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            uint_fast8_t  data = DATA_BUS(value);
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
        if (IS_IFLAG(IFLAGS_IIGS_REGS) && (is_write))
        {
            apple_tbcolor = DATA_BUS(value);
        }
        break;
    case 0x29:
        if (IS_IFLAG(IFLAGS_IIGS_REGS) && (is_write))
        {
            soft_switches = (soft_switches & ~(SOFTSW_NEWVID_MASK << SOFTSW_NEWVID_SHIFT)) | ((data & SOFTSW_NEWVID_MASK) << SOFTSW_NEWVID_SHIFT);
        }
        break;
    case 0x34:
        if (IS_IFLAG(IFLAGS_IIGS_REGS) && (is_write))
        {
            apple_border = DATA_BUS(value);
        }
        break;
    case 0x35:
        if (IS_IFLAG(IFLAGS_IIGS_REGS) && (is_write))
        {
            uint_fast8_t data = DATA_BUS(value);
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
    case 0x58: // VIDEX80COLUMN: OFF
        soft_switches &= ~SOFTSW_VIDEX_80COL;
        break;
    case 0x59: // VIDEX80COLUMN: ON
        if (videx_enabled)
        {
            soft_switches |= SOFTSW_VIDEX_80COL;
        }
        break;
    case 0x5e: // DGRON
        if (IS_IFLAG(IFLAGS_IIE_REGS))
        {
            soft_switches |= SOFTSW_DGR;
        }
        break;
    case 0x5f: // DGROFF
        // Video 7 shift register
        if (IS_SOFTSWITCH(SOFTSW_DGR))
        {
            soft_switches = ( soft_switches & (~SOFTSW_V7_MODE3)) |
                            ((soft_switches &   SOFTSW_V7_MODE1)<<1) |
                            ((soft_switches & SOFTSW_80COL) ? SOFTSW_V7_MODE1 : 0);
        }

        if (IS_IFLAG(IFLAGS_IIE_REGS))
        {
            soft_switches &= ~SOFTSW_DGR;
        }
        break;
    case 0x71: // fall-through
    case 0x75: // fall-through
    case 0x77: // fall-through
    case 0x73: // RAMWorks ($C073 is the official address, but 71,75,77 were also supported)
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            if (IS_IFLAG(IFLAGS_RAMWORKS))
            {
                uint_fast8_t data = DATA_BUS(value);
                // we don't care which page is active, just check if page is 1..127
                ramworks_active = (data) && ((data & 0x80)==0);
            }
        }
        break;
    case 0x7e: // IOUDISON: disable IOU
        if (IS_IFLAG(IFLAGS_IIE_REGS) && (is_write))
        {
            soft_switches |= SOFTSW_IOUDIS;
        }
        break;
    case 0x7f: // IOUDISOFF: enable IOU
        if (is_write) // on IIE only (but doesn't matter for clearing the register)
        {
            soft_switches &= ~SOFTSW_IOUDIS;
        }
        break;
    }
}

// access to card's DEVSEL register area
static void bus_card_selected(uint32_t value)
{
    uint_fast16_t address = ADDRESS_BUS(value);

    // check if address in register range (DEVSEL)
    if ((address>>8) < (0xc100>>8))
    {
        if (ACCESS_WRITE(value))
        {
            device_write(address & 0xf, DATA_BUS(value));
        }

        // remember the slot number
        cardslot = (address >> 4) & 0x7;

        devicereg_counter++;
    }
}

// handle address ranges we're not really interested in
void __time_critical_func(bus_func_ignore)(uint32_t value)
{
    // do nothing for this address area
}

// Shadow screen area of the Apple's memory by observing the bus write cycles
void __time_critical_func(bus_func_screen_write)(uint32_t value)
{
    uint_fast16_t address = ADDRESS_BUS(value);
    if ((address > 0x200)&&(address < MAX_ADDRESS))
    {
        uint_fast8_t data = DATA_BUS(value);

        // Mirror Video Memory from MAIN & AUX banks
        if ((soft_switches & SOFTSW_80STORE)&&
            (((address >= 0x400) && (address < 0x800))||
            ((soft_switches & SOFTSW_HIRES_MODE) && (address >= 0x2000) && (address < 0x4000))))
        {
            // 80STORE is on AND address is within an active display page
            if(soft_switches & SOFTSW_PAGE_2)
            {
                if (!ramworks_active)
                    aux_memory[address] = data;
            }
            else
            if (!IS_SOFTSWITCH(SOFTSW_MENU_ENABLE))
                apple_memory[address] = data;
            // nothing else to do
        }
        else
        {
            if(soft_switches & SOFTSW_AUX_WRITE)
            {
                if (!ramworks_active)
                    aux_memory[address] = data;
            }
            else
            if (!IS_SOFTSWITCH(SOFTSW_MENU_ENABLE))
            {
                apple_memory[address] = data;
                if (address < 0x800)
                {
                    machine_auto_detection(address);
                }
            }
        }
    }
}

void __time_critical_func(bus_func_cxxx_read)(uint32_t value)
{
    uint_fast16_t address = ADDRESS_BUS(value);

    // Shadow the soft-switches by observing all read & write bus cycles
    if((address&0xFF80) == 0xc000)
    {
        return apple2_softswitches(false, address, value);
    }

    if (videx_enabled)
    {
        if ((address & 0xFFF0) == 0xC0B0) // slot #3 register area ($C0B0-$C0BF)
            videx_reg_read(address);
        else
        if ((address & 0xFF00) == 0xC300)
        {
            videx_vterm_mem_selected = true;
        }
        else
        if ((address & 0xF800) == 0xC800)
        {
            if (videx_vterm_mem_selected)
                videx_c8xx_read(address);
        }
    }

    romx_cxxx_check_read(address);
}

void __time_critical_func(bus_func_cxxx_write)(uint32_t value)
{
    uint_fast16_t address = ADDRESS_BUS(value);

    // Shadow the soft-switches by observing all read & write bus cycles
    if((address&0xFF80) == 0xc000)
    {
        return apple2_softswitches(true, address, value);
    }

    if (videx_enabled)
    {
        if ((address & 0xFFF0) == 0xC0B0) // slot #3 register area ($C0B0-$C0BF)
            videx_reg_write(address, DATA_BUS(value));
        else
        if ((address >> 8) == (0xC300>>8))
        {
            videx_vterm_mem_selected = true;
        }
        else
        if ((address & 0xF800) == 0xC800)
        {
            if (videx_vterm_mem_selected)
                videx_c8xx_write(address, DATA_BUS(value));
        }
    }
}

void __time_critical_func(bus_func_fxxx_read)(uint32_t value)
{
    uint_fast16_t address = ADDRESS_BUS(value);

    if ((last_read_address == 0xFFFCFFFD)&&  // 6502 reset vector (0xFFFC+0xFFFD)
        ((address == APPLEII_AUTOSTART_ROM_RESET)||
        (address == APPLEII_ORIGINAL_ROM_RESET)) // Apple II reset routine
       )
    {
        soft_switches = SOFTSW_TEXT_MODE | SOFTSW_V7_MODE3;
        romx_unlocked = 0;
        dev_config_lock = 0;
#ifndef FEATURE_ABUS_DEBUG
        bus_overflow_counter = 0;
#endif
        videx_vterm_mem_selected = false;
        ramworks_active = false;
        reset_counter++;
    }
    if (address == APPLEII_ROM_HOME+1) // HOME, executed after the BELL, about 100ms after RESET
    {
        if (((uint16_t) last_read_address) == APPLEII_ROM_HOME)
            soft_switches &= ~ SOFTSW_SHOW_SPLASH;
    }
    else
    {
        romxe_faxx_check_read(address);
    }
}

a2busfunc bus_functions[16*2] =
{
    /*$0xxx READ */ bus_func_ignore,
    /*$1xxx READ */ bus_func_ignore,
    /*$2xxx READ */ bus_func_ignore,
    /*$3xxx READ */ bus_func_ignore,
    /*$4xxx READ */ bus_func_ignore,
    /*$5xxx READ */ bus_func_ignore,
    /*$6xxx READ */ bus_func_ignore,
    /*$7xxx READ */ bus_func_ignore,
    /*$8xxx READ */ bus_func_ignore,
    /*$9xxx READ */ bus_func_ignore,
    /*$Axxx READ */ bus_func_ignore,
    /*$Bxxx READ */ bus_func_ignore,
    /*$Cxxx READ */ bus_func_cxxx_read,
    /*$Dxxx READ */ bus_func_ignore,
    /*$Exxx READ */ bus_func_ignore,
    /*$Fxxx READ */ bus_func_fxxx_read,

    /*$0xxx WRITE*/ bus_func_screen_write,
    /*$1xxx WRITE*/ bus_func_ignore,
    /*$2xxx WRITE*/ bus_func_screen_write,
    /*$3xxx WRITE*/ bus_func_screen_write,
    /*$4xxx WRITE*/ bus_func_screen_write,
    /*$5xxx WRITE*/ bus_func_screen_write,
    /*$6xxx WRITE*/ bus_func_ignore,
    /*$7xxx WRITE*/ bus_func_ignore,
    /*$8xxx WRITE*/ bus_func_ignore,
    /*$9xxx WRITE*/ bus_func_ignore,
    /*$Axxx WRITE*/ bus_func_ignore,
    /*$Bxxx WRITE*/ bus_func_ignore,
    /*$Cxxx WRITE*/ bus_func_cxxx_write,
    /*$Dxxx WRITE*/ bus_func_ignore,
    /*$Exxx WRITE*/ bus_func_ignore,
    /*$Fxxx WRITE*/ bus_func_ignore
};

#ifdef FEATURE_TEST
void __time_critical_func(abus_interface)(uint32_t value)
#else
static inline void abus_interface(uint32_t value)
#endif
{
    if (CARD_SELECT(value))
    {
        bus_card_selected(value);
    }
    else
    {
        // get highest nibble of address
        uint_fast8_t adr = ADDRESS_BUS_HI_NIBBLE(value);
        // check read/write access
        if (ACCESS_WRITE(value))
            adr += 0x10;
        // call handler for this memory area/access type
        bus_functions[adr](value);
    }

    // keep track of 6502 activity (debug monitor)
    {
        uint_fast16_t address = ADDRESS_BUS(value);

        if (address < 0x100)
            last_address_zp = address;
        else
        if (address < 0x200)
            last_address_stack = address;
        else
        if (ACCESS_READ(value))
        {
            if (address == ((uint16_t)last_read_address)+1)
                last_address_pc = address;
            // remember addresses of two most recent read cycles
            last_read_address <<= 16;
            last_read_address |= address;
        }
    }
}

void __time_critical_func(abus_clear_fifo)(void)
{
    while (abus_pio_fifo_level())
    {
        (void) abus_pio_blocking_read();
    }
}

void __time_critical_func(abus_init)()
{
#ifdef APPLE_MODEL_IIPLUS
    videx_vterm_init();
#endif
    abus_pio_setup();
}

void __time_critical_func(abus_loop)()
{
    // initialize the Apple II bus interface
    abus_init();

    uint32_t value = 0;
#if FEATURE_ABUS_DEBUG
    uint32_t idle_count = 0;
    bus_overflow_counter = 0xffff;
#endif
    while(1)
    {
        if (abus_pio_is_full())
        {
            bus_overflow_counter++;
            value = abus_pio_read();
        }
        else
        {
#ifdef FEATURE_ABUS_DEBUG
            idle_count = 0;
            while (abus_pio_is_empty()) idle_count++;
            value = abus_pio_read();
#else
            value = abus_pio_blocking_read();
#endif
        }

        abus_interface(value);

#ifdef FEATURE_ABUS_DEBUG
        if (idle_count < bus_overflow_counter)
            bus_overflow_counter = idle_count;
#endif
        bus_cycle_counter++;
    }
}
