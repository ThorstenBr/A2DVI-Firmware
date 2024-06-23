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
#include "../debug/debug.h"
#include "businterface.h"
//#include "colors.h"
//#include "device_regs.h"
#ifdef APPLE_MODEL_IIPLUS
#include "videx_vterm.h"
#endif

#if 0
static int reset_detect_state = 0;

typedef void (*shadow_handler)(bool is_write, uint_fast16_t address, uint_fast8_t data);

static shadow_handler softsw_handlers[256];

static void shadow_softsw_00(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_80store = false;
}

static void shadow_softsw_01(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_80store = true;
}

static void shadow_softsw_04(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_ramwrt = false;
}

static void shadow_softsw_05(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_ramwrt = true;
}

static void shadow_softsw_0c(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_80col = false;
}

static void shadow_softsw_0d(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_80col = true;
}

static void shadow_softsw_0e(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_altcharset = false;
}

static void shadow_softsw_0f(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write)
        soft_altcharset = true;
}

static void shadow_softsw_21(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(is_write) {
        if(data & 0x80) {
            soft_monochrom = true;
        } else {
            soft_monochrom = false;
        }
    }
}

static void shadow_softsw_50(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches &= ~((uint32_t)SOFTSW_TEXT_MODE);
}

static void shadow_softsw_51(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches |= SOFTSW_TEXT_MODE;
}

static void shadow_softsw_52(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches &= ~((uint32_t)SOFTSW_MIX_MODE);
}

static void shadow_softsw_53(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches |= SOFTSW_MIX_MODE;
}

static void shadow_softsw_54(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches &= ~((uint32_t)SOFTSW_PAGE_2);
}

static void shadow_softsw_55(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches |= SOFTSW_PAGE_2;
}

static void shadow_softsw_56(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches &= ~((uint32_t)SOFTSW_HIRES_MODE);
}

static void shadow_softsw_57(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_switches |= SOFTSW_HIRES_MODE;
}

static void shadow_softsw_58(bool is_write, uint_fast16_t address, uint_fast8_t data) {
#ifdef APPLE_MODEL_IIPLUS
    videx_vterm_80col_enabled = false;
#endif
}

static void shadow_softsw_59(bool is_write, uint_fast16_t address, uint_fast8_t data) {
#ifdef APPLE_MODEL_IIPLUS
    videx_vterm_80col_enabled = true;
#endif
}

static void shadow_softsw_5e(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    soft_dhires = true;
}

static void shadow_softsw_5f(bool is_write, uint_fast16_t address, uint_fast8_t data) {
    if(soft_dhires) {
        // This is the VIDEO7 Magic (Not documented by apple but by a patent US4631692)
        // Apple II has softswitches and also a special 2bit shift register (two flipflops basically)
        // controlled with Softwitch 80COL and AN3. AN3 is the Clock so when AN3 goes from clear to set
        // it shifts the content of 80COL into the 2 switches.
        // this is VIDEO7 Mode
        soft_video7_mode = ((soft_video7_mode & 0x01) << 1) | (soft_80col ? 0 : 1);
    }

    soft_dhires = false;
}
#endif

void abus_init()
{
    // Init states
    soft_switches = SOFTSW_TEXT_MODE;

#ifdef APPLE_MODEL_IIPLUS
    videx_vterm_init();
#endif

#if 0
    // Setup soft-switch handlers for the Apple model
    softsw_handlers[0x21] = shadow_softsw_21;
    softsw_handlers[0x50] = shadow_softsw_50;
    softsw_handlers[0x51] = shadow_softsw_51;
    softsw_handlers[0x52] = shadow_softsw_52;
    softsw_handlers[0x53] = shadow_softsw_53;
    softsw_handlers[0x54] = shadow_softsw_54;
    softsw_handlers[0x55] = shadow_softsw_55;
    softsw_handlers[0x56] = shadow_softsw_56;
    softsw_handlers[0x57] = shadow_softsw_57;
    softsw_handlers[0x58] = shadow_softsw_58;
    softsw_handlers[0x59] = shadow_softsw_59;
#ifdef APPLE_MODEL_IIE
    softsw_handlers[0x00] = shadow_softsw_00;
    softsw_handlers[0x01] = shadow_softsw_01;
    softsw_handlers[0x04] = shadow_softsw_04;
    softsw_handlers[0x05] = shadow_softsw_05;
    softsw_handlers[0x0c] = shadow_softsw_0c;
    softsw_handlers[0x0d] = shadow_softsw_0d;
    softsw_handlers[0x0e] = shadow_softsw_0e;
    softsw_handlers[0x0f] = shadow_softsw_0f;
    softsw_handlers[0x5e] = shadow_softsw_5e;
    softsw_handlers[0x5f] = shadow_softsw_5f;
#endif
#ifdef APPLE_MODEL_IIPLUS
    // slot 3 device registers
    for(uint i = 0xb0; i < 0xc0; i++) {
        softsw_handlers[i] = videx_vterm_shadow_register;
    }
#endif
#endif
    abus_pio_setup();
}

#if 0
// Shadow parts of the Apple's memory by observing the bus write cycles
static void shadow_memory(bool is_write, uint_fast16_t address, uint32_t value)
{
    uint8_t *bank;

    switch((address & 0xfc00) >> 10)
    {
    case 0x0400 >> 10:
        // text page 1
        reset_detect_state = 0;
        if(!is_write)
            break;

        // Refer to "Inside the Apple IIe" p.295 for how Aux memory addressing is done
        if(soft_switches & SOFTSW_80STORE) {
            // 80STORE takes precedence - bank is then controlled by the PAGE2 switch
            bank = (soft_switches & SOFTSW_PAGE_2) ? private_memory : apple_memory;
        } else if(soft_switches & SOFTSW_AUX_WRITE) {
            bank = private_memory;
        } else {
            bank = apple_memory;
        }
        bank[address] = value & 0xff;
        break;

    case 0x0800 >> 10:
        // text page 2
        reset_detect_state = 0;
        if(!is_write)
            break;

        bank = soft_ramwrt ? aux_memory : main_memory;
        bank[address] = value & 0xff;
        break;

    case 0x2000 >> 10 ... 0x3c00 >> 10:
        // hires page 1
        reset_detect_state = 0;
        if(!is_write)
            break;

        if((soft_switches & SOFTSW_80STORE)  && (soft_switches & SOFTSW_HIRES_MODE)) {
            // 80STORE takes precedence - bank is then controlled by the PAGE2 switch
            bank = (soft_switches & SOFTSW_PAGE_2) ? private_memory : apple_memory;
        } else if(soft_ramwrt) {
            bank = private_memory;
        } else {
            bank = apple_memory;
        }
        bank[address] = value & 0xff;
        break;

    case 0x4000 >> 10 ... 0x5c00 >> 10:
        // hires page 2
        reset_detect_state = 0;
        if(!is_write)
            break;

        bank = soft_ramwrt ? private_memory : apple_memory;
        bank[address] = value & 0xff;
        break;

    case 0xc000 >> 10:
        reset_detect_state = 0;

        // Handle shadowing of the soft switches and I/O in the range $C000 - $C0FF
        if(address < 0xc100) {
            shadow_handler h = softsw_handlers[address & 0xff];
            if(h) {
                h(is_write, address, value & 0xff);
            }
        }
#ifdef APPLE_MODEL_IIPLUS
        else if((address >= 0xc300) && (address < 0xc400)) {
            // slot 3 access
            videx_vterm_mem_selected = true;
        }
#endif
        break;

    case 0xc800 >> 10:
    case 0xcc00 >> 10:
        // expansion slot memory space $C800-$CFFF
        reset_detect_state = 0;
#ifdef APPLE_MODEL_IIPLUS
        videx_vterm_shadow_c8xx(is_write, address, value);
#endif
        break;

    case 0x0000 >> 10:
    case 0xfc00 >> 10:
        // reset detection - the IOU chip on the Apple IIe cleverly detects a CPU reset by monitoring bus activity
        // for a sequence usually only seen when the CPU resets: three page 1 accesses followed by an access to
        // address $FFFC (see "Understanding the Apple IIe" pages 4-14 and 5-29).
        if((address >> 8) == 0x01) {
            // count the consecutive page 1 accesses, up to 3
            if(reset_detect_state < 3) {
                reset_detect_state++;
            } else if(reset_detect_state > 3) {
                reset_detect_state = 1;
            }
        } else if((reset_detect_state == 3) && (address == 0xfffc)) {
            reset_detect_state++;
        } else if((reset_detect_state == 4) && (address == 0xfffd)) {
            // reset soft-switches
            soft_switches = SOFTSW_TEXT_MODE;
            soft_video7_mode = VIDEO7_MODE_140x192;
            soft_dhires = false;
            soft_80col = false;
            soft_80store = false;
            soft_altcharset = false;
            soft_ramwrt = false;
#ifdef APPLE_MODEL_IIPLUS
            videx_vterm_80col_enabled = false;
#endif

            reset_detect_state = 0;
        } else {
            reset_detect_state = 0;
        }
        break;

    default:
        reset_detect_state = 0;
        break;
    }
}
#endif

void __time_critical_func(abus_loop)()
{
    uint32_t count = 100;
    while(1)
    {
#ifdef FEATURE_DEBUG_NO6502
        debug_check_bootsel();
#else
        uint32_t value = abus_pio_blocking_read();
        businterface(value);
        if (--count == 0)
        {
            gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
            count = 100*1000;
        }
#endif
    }
}
