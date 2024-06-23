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
#include "debug/debug.h"
#include "dvi/a2dvi.h"

const char BootMsg[] = "A2DVI: WAITING FOR 6502";

typedef enum {
    PRINTMODE_NORMAL  = 0,
    PRINTMODE_INVERSE = 1,
    PRINTMODE_FLASH   = 2
} TPrintMode;

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
            default:
            case PRINTMODE_NORMAL:
                c |= 0x80;
                break;
        }
        pScreenArea[i] = c;
    }
}

int main()
{
    // basic CPU configuration required for DVI
    a2dvi_init();

    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
        ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
    }
    printXY(20-3 , 1,  "A2DVI", PRINTMODE_NORMAL);
    printXY(20-7 , 3,  "FIRMWARE V" FW_VERSION, PRINTMODE_NORMAL);

#ifdef FEATURE_DEBUG_NO6502
    printXY(20-3 , 5,  "DEBUG", PRINTMODE_INVERSE);
#endif

    printXY(20-8 , 11, "NO 6502 ACTIVITY", PRINTMODE_FLASH);

    printXY(20-9 , 19, "APPLE II FOREVER!", PRINTMODE_NORMAL);
    printXY(20-15, 21, "RELEASED UNDER THE MIT LICENSE", PRINTMODE_NORMAL);
    printXY(20-20, 23, "(C) 2024 RALLE PALAVEEV, THORSTEN BREHM", PRINTMODE_NORMAL);

    // initialize the Apple II bus interface
    abus_init();

    // process the Apple II bus interface on core 1
    multicore_launch_core1(abus_loop);

    // enable LED, configure BOOTSEL button etc
    debug_init();

    // process DVI on core 0
    a2dvi_loop();
}
