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
#include "dvi/render.h"
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

void test40columns()
{
    clearTextScreen();
    printXY(0,0, "A2DVI Test: 40 columns", PRINTMODE_NORMAL);
    for (uint i=0;i<255;i++)
    {
        char s[3];
        s[0] = i;
        s[1] = 0;
        printXY(i&0xf, 2+(i>>4), s, PRINTMODE_RAW);
    }
    businterface(0xc00c << 10); // disable 80column mode
    sleep(5000);
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


void setLoresPixel(uint16_t x, uint8_t y, bool page2, uint8_t color)
{
     if (page2)
        x = x | 1024;

     color &= 0xF;
     uint8_t mask = 0xF0; // even rows in low nibble

     if ((y & 1) == 1)
     {
         // odd rows in high nibble
         mask = 0x0F;
         color <<= 4;
     }

     uint line = y >> 1;
     uint16_t address = 0x400+((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40)+x;
     apple_memory[address] = (apple_memory[address] & mask) | color;
}


void testLores()
{
    businterface(0xc050 << 10); // enable LORES graphics

    for (uint y=0;y<48;y++)
    {
        for (uint x=0;x<40;x++)
        {
            setLoresPixel(x, y, false, (x+y)&0xf);
        #if 0
            uint8_t color = 0;
            if ((x==0)||(x==39))
                color = 15;
            if ((y==0)||(y==47))
                color = 15;
            setLoresPixel(x,y, false, color);
        #endif
        }
    }
    sleep(5000);

    soft_switches |= SOFTSW_MONOCHROME;
    sleep(5000);

    soft_switches &= ~SOFTSW_MONOCHROME;
    businterface(0xc051 << 10); // enable text mode
}
void test_loop()
{
    sleep(1000*3);
    while (1)
    {
        current_machine = MACHINE_IIE;
        internal_flags |= IFLAGS_IIE_REGS;

        test40columns();
        test80columns();
        testLores();
    }
}


#endif // FEATURE_TEST
