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
#include <malloc.h>
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
volatile bool debug_flash_released;

// ATTENTION: This temporarily DISABLES flash access. Not a good idea,
// since we need the flash at run-time, to read/write config data,
// to read/write fonts etc.
// So we only use this function for convenience in test mode.
bool __no_inline_not_in_flash_func(get_bootsel_button)(void)
{
#if !PICO_RP2040
    // PICO2: access to bootsel button not working/not implemented yet
    const uint button_state = 1;
#else
    // PICO1
    const uint CS_PIN_INDEX = 1;

    // do not access the button (which disables flash), unless we have the
    // go (main init code is complete).
    if (!debug_flash_released)
        return 1;

  #if 0
    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();
  #endif

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

  #if 0
    restore_interrupts(flags);
  #endif
#endif

    return button_state;
}
#endif

#ifdef FEATURE_TEST
static void debug_flashmode()
{
    gpio_put(LED_PIN, 0);
    reset_usb_boot(0,0);
}
#endif

void debug_init()
{
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
}

void debug_blink(uint8_t count, uint32_t delay0, uint32_t delay1, uint32_t delay2)
{
    gpio_put(LED_PIN, 0);
    sleep_ms(delay0);
    for (int i=0;i<count;i++)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay1);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay2);
    }
}

void debug_sos(void)
{
    debug_blink(3, 1000, 200, 200);
    debug_blink(3,  500, 500, 200);
    debug_blink(3,  500, 200, 200);

#ifdef FEATURE_TEST
    debug_flashmode();
#endif
}

#ifdef FEATURE_TEST
void debug_error(uint32_t error_code)
{
    gpio_put(LED_PIN, 0);
    sleep_ms(2000);

    for (int i=0;i<error_code;i++)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }

    debug_flashmode();
}
#endif

#ifdef FEATURE_TEST
void __no_inline_not_in_flash_func(debug_check_bootsel)()
{
    if(0 == get_bootsel_button())
    {
        debug_flashmode();
    }
}
#endif

// total size of heap
uint32_t getTotalHeap(void)
{
   extern char __StackLimit, __bss_end__;
   return &__StackLimit  - &__bss_end__;
}

// available heap size
uint32_t getFreeHeap(void)
{
   struct mallinfo m = mallinfo();
   return getTotalHeap() - m.uordblks;
}
