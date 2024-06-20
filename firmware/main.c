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

#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "dvi/a2dvi.h"

const char BootMsg[] = " A2DVI: WAITING FOR 6502 ";

int main()
{
    // basic CPU configuration required for DVI
    a2dvi_init();

    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
        ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
    }
    char* pScreenArea = (char*) (text_p1+0x1a8+7);
    for (uint32_t i=0;BootMsg[i];i++)
        pScreenArea[i] = BootMsg[i]|0x80;

    // initialize the Apple II bus interface
    abus_init();

    // process the Apple II bus interface on core 1
    multicore_launch_core1(abus_loop);

    // enable LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    // process DVI on core 0
    a2dvi_loop();
}
