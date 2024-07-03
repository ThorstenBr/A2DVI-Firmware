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
#include "abus_pin_config.h"
#include "abus_setup.h"
#include "abus.pio.h"

#if CONFIG_PIN_APPLEBUS_PHI0 != PHI0_GPIO
#error CONFIG_PIN_APPLEBUS_PHI0 and PHI0_GPIO must be set to the same pin
#endif

void abus_pio_setup(void)
{
    PIO pio = CONFIG_ABUS_PIO;
    const uint sm = ABUS_MAIN_SM;

    uint program_offset = pio_add_program(pio, &abus_program);
    pio_sm_claim(pio, sm);

    pio_sm_config c = abus_program_get_default_config(program_offset);

    // set the bus R/W pin as the jump pin
    sm_config_set_jmp_pin(&c, CONFIG_PIN_APPLEBUS_RW);

    // map the IN pin group to the data signals
    sm_config_set_in_pins(&c, CONFIG_PIN_APPLEBUS_DATA_BASE);

    // map the SET pin group to the bus transceiver enable signals
    sm_config_set_set_pins(&c, CONFIG_PIN_APPLEBUS_CONTROL_BASE, 3);

    // configure left shift into ISR & autopush every 27 bits
    sm_config_set_in_shift(&c, false, true, 27);

    pio_sm_init(pio, sm, program_offset, &c);

    // configure the GPIOs
    // Ensure all transceivers will start disabled
    pio_sm_set_pins_with_mask(
        pio, sm, (uint32_t)0x7 << CONFIG_PIN_APPLEBUS_CONTROL_BASE, (uint32_t)0x7 << CONFIG_PIN_APPLEBUS_CONTROL_BASE);
    pio_sm_set_pindirs_with_mask(pio, sm, (0x7 << CONFIG_PIN_APPLEBUS_CONTROL_BASE),
        (1 << CONFIG_PIN_APPLEBUS_PHI0) | (0x7 << CONFIG_PIN_APPLEBUS_CONTROL_BASE) | (0x3ff << CONFIG_PIN_APPLEBUS_DATA_BASE));

    // Disable input synchronization on input pins that are sampled at known stable times
    // to shave off two clock cycles of input latency
    pio->input_sync_bypass |= (0x3ff << CONFIG_PIN_APPLEBUS_DATA_BASE);

    pio_gpio_init(pio, CONFIG_PIN_APPLEBUS_PHI0);
    gpio_set_pulls(CONFIG_PIN_APPLEBUS_PHI0, false, false);

    for(int pin = CONFIG_PIN_APPLEBUS_CONTROL_BASE; pin < CONFIG_PIN_APPLEBUS_CONTROL_BASE + 3; pin++)
    {
        pio_gpio_init(pio, pin);
    }

    // initialize GPIO on all 8 data pins + DEVSEL + RW + LANGUAGE_SW = 11
    for(int pin = CONFIG_PIN_APPLEBUS_DATA_BASE; pin < CONFIG_PIN_APPLEBUS_DATA_BASE + 11; pin++)
    {
        pio_gpio_init(pio, pin);
        gpio_set_pulls(pin, false, false);
    }

    pio_enable_sm_mask_in_sync(pio, (1 << ABUS_MAIN_SM));
}
