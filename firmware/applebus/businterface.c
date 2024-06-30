#include <pico/stdlib.h>
#include <string.h>
#include "abus.h"
#include "businterface.h"
#include "buffers.h"
#include "config/config.h"
#include "config/device_regs.h"

//volatile uint8_t *terminal_page = terminal_memory;

typedef enum
{
    WriteMem = 0,
    ReadMem  = 1,
    WriteDev = 2,
    ReadDev  = 3,
} TAccessMode;

static inline void machine_auto_detection(uint32_t address)
{
    if(current_machine == MACHINE_AUTO)
    {
        if((apple_memory[0x0403] == 0xD8) && (apple_memory[0x404] == 0xE5)) {        // "Xe" = ROMXe
            current_machine = MACHINE_IIE;
            internal_flags |= IFLAGS_IIE_REGS;
            internal_flags &= ~IFLAGS_IIGS_REGS;
        } else if((apple_memory[0x0412] == 0xC5) && (apple_memory[0x0413] == 0xD8)) {// "EX" = ROMX
            current_machine = MACHINE_II;
            internal_flags &= ~(IFLAGS_IIE_REGS | IFLAGS_IIGS_REGS);
#if 0
        } else if((apple_memory[0x416] == 0xC9) && (apple_memory[0x0417] == 0xE7)) { // "Ig" = Apple IIgs
            current_machine = MACHINE_IIGS;
            internal_flags &= ~IFLAGS_IIE_REGS;
            internal_flags |= IFLAGS_IIGS_REGS;
#endif
        } else if((apple_memory[0x416] == 0xAF) && (apple_memory[0x0417] == 0xE5)) { // "/e" = Apple //e Enhanced
            current_machine = MACHINE_IIE;
            internal_flags |= IFLAGS_IIE_REGS;
            internal_flags &= ~IFLAGS_IIGS_REGS;
        } else if((apple_memory[0x413] == 0xE5) && (apple_memory[0x0415] == 0xDD)) { // "e ]" = Apple //e Unenhanced
            current_machine = MACHINE_IIE;
            internal_flags |= IFLAGS_IIE_REGS;
            internal_flags &= ~IFLAGS_IIGS_REGS;
        } else if(apple_memory[0x0410] == 0xD0) { // "P" = Apple II/Plus/J-Plus with Autostart
            current_machine = MACHINE_II;
            internal_flags &= ~(IFLAGS_IIE_REGS | IFLAGS_IIGS_REGS);
        } else if((apple_memory[0x07D0] == 0xAA)&&(apple_memory[0x07D1] == 0x60)) { // "*(CURSOR)" = Apple II without Autostart
            current_machine = MACHINE_II;
            internal_flags &= ~(IFLAGS_IIE_REGS | IFLAGS_IIGS_REGS);
        } else if(apple_memory[0x0410] == 0xF2) { // "r" = Pravetz!
            current_machine = MACHINE_PRAVETZ;
            internal_flags &= ~(IFLAGS_IIE_REGS | IFLAGS_IIGS_REGS);
        }
    }
}

static void apple2emulation(TAccessMode AccessMode, uint32_t address, uint8_t data)
{
    // Shadow parts of the Apple's memory by observing the bus write cycles
    if(AccessMode == WriteMem)
    {
        // Mirror Video Memory from MAIN & AUX banks
        if(soft_switches & SOFTSW_80STORE)
        {
            if(soft_switches & SOFTSW_PAGE_2)
            {
                if((address >= 0x400) && (address < 0x800))
                {
                    private_memory[address] = data;
                    return;
                }
                if((soft_switches & SOFTSW_HIRES_MODE) && (address >= 0x2000) && (address < 0x4000))
                {
                    private_memory[address] = data;
                    return;
                }
            }
        }
        else
        if(soft_switches & SOFTSW_AUX_WRITE)
        {
            if((address >= 0x200) && (address < 0xC000))
            {
                private_memory[address] = data;
                return;
            }
        }

        if((address >= 0x200) && (address < 0xC000))
        {
            apple_memory[address] = data;
            if (address < 0x800)
            {
                machine_auto_detection(address);
            }
            return;
        }

        // Nothing left to do for RAM accesses.
        if(address < 0xc000)
            return;

        // Shadow the soft-switches by observing all read & write bus cycles
        if(address < 0xc080)
        {
            switch(address & 0x7f)
            {
            case 0x00:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_80STORE;
                }
                break;
            case 0x01:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_80STORE;
                }
                break;
            case 0x04:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_AUX_WRITE;
                }
                break;
            case 0x05:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_AUX_WRITE;
                }
                break;
            case 0x08:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_AUXZP;
                }
                break;
            case 0x09:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_AUXZP;
                }
                break;
            case 0x0c:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_80COL;
                }
                break;
            case 0x0d:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_80COL;
                }
                break;
            case 0x0e:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_ALTCHAR;
                }
                break;
            case 0x0f:
                if((internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS)) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_ALTCHAR;
                }
                break;
            case 0x21:
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
    #if 0
            case 0x22:
                if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
                {
                    apple_tbcolor = data;
                }
                break;
    #endif
            case 0x29:
                if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
                {
                    soft_switches = (soft_switches & ~(SOFTSW_NEWVID_MASK << SOFTSW_NEWVID_SHIFT)) | ((data & SOFTSW_NEWVID_MASK) << SOFTSW_NEWVID_SHIFT);
                }
                break;
    #if 0
            case 0x34:
                if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
                {
                    apple_border = data;
                }
                break;
    #endif
            case 0x35:
                if((internal_flags & IFLAGS_IIGS_REGS) && (AccessMode == WriteMem))
                {
                    soft_switches = (soft_switches & ~(SOFTSW_SHADOW_MASK << SOFTSW_SHADOW_SHIFT)) | ((data & SOFTSW_SHADOW_MASK) << SOFTSW_SHADOW_SHIFT);
                }
                break;
            case 0x50:
                soft_switches &= ~SOFTSW_TEXT_MODE;
                break;
            case 0x51:
                soft_switches |= SOFTSW_TEXT_MODE;
                break;
            case 0x52:
                soft_switches &= ~SOFTSW_MIX_MODE;
                break;
            case 0x53:
                soft_switches |= SOFTSW_MIX_MODE;
                break;
            case 0x54:
                soft_switches &= ~SOFTSW_PAGE_2;
                break;
            case 0x55:
                soft_switches |= SOFTSW_PAGE_2;
                break;
            case 0x56:
                soft_switches &= ~SOFTSW_HIRES_MODE;
                break;
            case 0x57:
                soft_switches |= SOFTSW_HIRES_MODE;
                break;
            case 0x5e:
                if(internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS))
                {
                    soft_switches |= SOFTSW_DGR;
                }
                break;
            case 0x5f:
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
            case 0x7e:
                if((internal_flags & IFLAGS_IIE_REGS) && (AccessMode == WriteMem))
                {
                    soft_switches |= SOFTSW_IOUDIS;
                }
                break;
            case 0x7f:
                if((internal_flags & IFLAGS_IIE_REGS) && (AccessMode == WriteMem))
                {
                    soft_switches &= ~SOFTSW_IOUDIS;
                }
                break;
            }
            return;
        }
    }

#if 0
    // Control sequences used by ROMX and ROMXe
    switch(current_machine)
    {
        case MACHINE_IIE:
            // Trigger on read sequence FACA FACA FAFE
            if(AccessMode == ReadMem)
            {
                if((address >> 8) == 0xFA)
                {
                    switch(address & 0xFF) {
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
                } else if(romx_unlocked == 3) {
                    if((address >> 4) == 0xF81) {
                        romx_textbank = address & 0xF;
                    }
                    if(address == 0xF851) {
                        romx_changed = 1;
                        romx_unlocked = 0;
                    }
                }
            }
            break;
        case MACHINE_II:
            // Trigger on read sequence CACA CACA CAFE
            if(AccessMode == ReadMem)
            {
                if((address >> 8) == 0xCA)
                {
                    switch(address & 0xFF) {
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
                } else if(romx_unlocked == 3) {
                    if((address >> 4) == 0xCFD) {
                        romx_textbank = address & 0xF;
                    }
                    if((address >> 4) == 0xCFE) {
                        romx_changed = 1;
                        romx_unlocked = 0;
                    }
                }
            }
            break;
    }
#endif

    // Card Registers
    if (AccessMode == WriteDev)
    {
        device_write(address & 0x0F, data);
        // remember the slot number
        cardslot = (address >> 4) & 0x7;
    }
    else
    if (AccessMode == ReadDev)
    {
        // remember the slot number
        cardslot = (address >> 4) & 0x7;
#if 0
        if((address & 0x0F) == 0x0A)
        {
            //apple_memory[address] = (terminal_fifo_rdptr - terminal_fifo_wrptr);
        }
#endif
    }
}

void __time_critical_func(businterface)(uint32_t value)
{
    uint32_t access_mode = ACCESS_WRITE(value) ? 0 : 1;
    if (CARD_DEVSEL(value))
        access_mode |= 2;
    uint32_t address = (value >> 10) & 0xffff;

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
                //internal_flags &= ~(IFLAGS_TERMINAL | IFLAGS_TEST);
                internal_flags |= IFLAGS_V7_MODE3;
            }
            // fall-through
        default:
            reset_state = 0;
            break;
    }
}
