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
#include "businterface.h"
#include "config/config.h"

#ifdef APPLE_MODEL_IIPLUS
    #include "videx_vterm.h"
#endif

//#define FEATURE_DEBUG_INTERPOLATION

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

    uint32_t count = 100;
    while(1)
    {
        if (--count == 0)
        {
            gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
            count = 100*1000;
        }

        if (abus_pio_is_full())
            bus_overflow_counter++;

        uint32_t value = abus_pio_blocking_read();

        input_switch_state = LANGUAGE_SWITCH(value);

        bus_counter++;

        businterface(value);
    }
}
