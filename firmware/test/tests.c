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

#include "pico/time.h"
#include "debug/debug.h"
#include "applebus/buffers.h"
#include "applebus/businterface.h"
#include "config/config.h"

#ifdef FEATURE_TEST

void sleep(int Milliseconds)
{
    while (Milliseconds--)
    {
        debug_check_bootsel();
        sleep_ms(1);
    }
}

void test80columns()
{
    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
        ((uint32_t*)text_p3)[i] = 0xA0A0A0A0; // initialize with blanks
    }

    businterface(0xc00d << 10); // enable 80column mode
    sleep(5000);
    businterface(0xc00c << 10); // disable 80column mode
}


void test40columns()
{
    businterface(0xc00c << 10); // disable 80column mode
    sleep(5000);
}

void test_loop()
{
    sleep(1000*10);
    while (1)
    {
        current_machine = MACHINE_IIE;
        internal_flags |= IFLAGS_IIE_REGS;

        test40columns();
        test80columns();
    }
}


#endif // FEATURE_TEST
