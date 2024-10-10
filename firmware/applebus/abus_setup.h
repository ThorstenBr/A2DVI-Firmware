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

#pragma once

#include <hardware/pio.h>

// use PIO 0 (PIO1 is used for VGA/DVI)
#define CONFIG_ABUS_PIO pio0

// statemachines (0-3)
#define ABUS_MAIN_SM    0

#define abus_pio_fifo_level()    (pio_sm_get_rx_fifo_level(CONFIG_ABUS_PIO, ABUS_MAIN_SM))
#define abus_pio_is_full()       (pio_sm_is_rx_fifo_full(CONFIG_ABUS_PIO, ABUS_MAIN_SM))
#define abus_pio_is_empty()      (pio_sm_is_rx_fifo_empty(CONFIG_ABUS_PIO, ABUS_MAIN_SM))

#define abus_pio_blocking_read() (pio_sm_get_blocking(CONFIG_ABUS_PIO, ABUS_MAIN_SM))

void abus_pio_setup(void);
