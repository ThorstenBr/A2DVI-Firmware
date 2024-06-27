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

#include "dvi/a2dvi.h"
#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "debug/debug.h"

#ifdef FEATURE_TEST
    #include "test/tests.h"
#endif

int main()
{
    // basic CPU configuration required for DVI
    a2dvi_init();

    // initialize the screen buffer area
    clearTextScreen();
    printXY(20-3 , 1,  "A2DVI", PRINTMODE_NORMAL);
    printXY(20-7 , 3,  "FIRMWARE V" FW_VERSION, PRINTMODE_NORMAL);

#ifdef FEATURE_DEBUG_NO6502
    printXY(20-3 , 5,  "DEBUG", PRINTMODE_INVERSE);
#endif

    printXY(20-8 , 11, "NO 6502 ACTIVITY", PRINTMODE_FLASH);

    printXY(20-8 n , 19, "APPLE II FOREVER!", PRINTMODE_NORMAL);
    printXY(20-15, 21, "RELEASED UNDER THE MIT LICENSE", PRINTMODE_NORMAL);
    printXY(20-20, 23, "(C) 2024 RALLE PALAVEEV, THORSTEN BREHM", PRINTMODE_NORMAL);

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
