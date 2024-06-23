/*
MIT License

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

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#include "debug.h"
#include "applebus/buffers.h"

#ifdef FEATURE_TEST
bool __no_inline_not_in_flash_func(get_bootsel_button)(void)
{
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = (sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
            GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
            IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}
#endif

void debug_init()
{
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
}

#ifdef FEATURE_TEST
void __no_inline_not_in_flash_func(debug_check_bootsel)()
{
    if(0 == get_bootsel_button())
    {
        gpio_put(LED_PIN, 0);
        reset_usb_boot(0,0);
    }
}
#endif

void __time_critical_func(printXY)(uint32_t x, uint32_t line, const char* pMsg, TPrintMode PrintMode)
{
    char* pScreenArea = ((char*) text_p1) + (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40)) + x;
    for (uint32_t i=0;pMsg[i];i++)
    {
        char c = pMsg[i];
        switch(PrintMode)
        {
            case PRINTMODE_FLASH:
                c |= 0x40;
                break;
            case PRINTMODE_INVERSE:
                if ((c>='A')&&(c<='Z'))
                    c -= 'A'-1;
                break;
            case PRINTMODE_RAW:
                break;
            default:
            case PRINTMODE_NORMAL:
                c |= 0x80;
                break;
        }
        pScreenArea[i] = c;
    }
}

void __time_critical_func(clearTextScreen)(void)
{
    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
        ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
    }
}
