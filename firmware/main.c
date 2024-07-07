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
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "dvi/a2dvi.h"
#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "util/dmacopy.h"
#include "config/config.h"
#include "menu/menu.h"
#include "debug/debug.h"

#include "fonts/textfont.h"

#ifdef FEATURE_TEST
    #include "test/tests.h"
#endif

int main()
{
    // basic CPU configuration required for DVI
    a2dvi_init();

    // load config settings
    config_load();

    // initialize the screen buffer area
    showTitle(PRINTMODE_NORMAL);
    centerY(11, "NO 6502 BUS ACTIVITY", PRINTMODE_FLASH);

    // initialize the Apple II bus interface
    abus_init();

#ifndef FEATURE_TEST
    // process the Apple II bus interface on core 1
    multicore_launch_core1(abus_loop);
#else
    // start testsuite on core1, simulating some 6502 activity and
    // cycle through the test cases
    multicore_launch_core1(test_loop);
#endif

    // enable LED etc
    debug_init();

    // DVI processing on core 0
    a2dvi_loop();
}
