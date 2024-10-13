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
#include "applebus/buffers.h"
#include "config/config.h"

#include "render.h"
#include "menu/menu.h"
#include "dvi/a2dvi.h"

/*
 *
 0123456789012345678901234567890123456789
0           A2DVI FIRMWARE X.Y
1              GITHUB.COM
2
3    AAAA    2222   DDDDD   V    V  I
4   A    A  2   22  D    D  V    V  I
5   A    A     22   D    D  V    V  I
6   AAAAAA   22     D    D   V  V   I
7   A    A  22      D    D   V  V   I
8   A    A  222222  DDDDD     VV    I
9
10       APPLE II DIGITAL VIDEO
11
12           COPYRIGHT 2024
13   THORSTEN BREHM & RALLE PALAVEEV
14
15
16           6502 STATUS
17     BUS CYCLES : 00000000000
18     ADDRESS BUS: FFA0
19     6502 PC    : FFA0
20        NO 6502 BUS ACTIVITY
21
22             GITHUB.COM
23            GITHUB.COM
*/

const char* DELAYED_COPY_DATA(pLogo) =
    "    AAAA    2222   DDDDD   V    V  I\n"
    "   A    A  2   22  D    D  V    V  I\n"
    "   A    A     22   D    D  V    V  I\n"
    "   AAAAAA   22     D    D   V  V   I\n"
    "   A    A  22      D    D   V  V   I\n"
    "   A    A  222222  DDDDD     VV    I\n";

const char* DELAYED_COPY_DATA(pSplashMsg)[3] = {
    "APPLE II DIGITAL VIDEO",
    "COPYRIGHT 2024",
    "THORSTEN BREHM & RALLE PALAVEEV"
};

const char* DELAYED_COPY_DATA(pStatus6502)[6] = {
    "6502 STATUS",
    "BUS CYCLES :",
    "ADDRESS BUS: ?",
    "PC         : ?",
    "NO 6502 BUS ACTIVITY",
    "WAITING FOR APPLE II ROM RESET ROUTINE"
};

#define LINE_A2DVI     (2)
#define LINE_A2DIGITAL (LINE_A2DVI+7)
#define LINE_COPYRIGHT (12)
#define LINE_6502      (16)
#define LINE_ACTIVITY  (20)

#define BGCOLOR (2)

uint8_t DELAYED_COPY_DATA(logoColors)[6] = {0xc, 0xd, 9, 1, 3, 6};

#define TEXT_OFFSET(line) ((((line) & 0x7) << 7) + ((((line) >> 3) & 0x3) * 40))

void DELAYED_COPY_CODE(render_splash)()
{
    // save original pointers to text page
    void* p1 = (void*) text_p1;
    void* p3 = (void*) text_p3;

    // use local text buffer for the "splash screen"
    uint8_t text_page[1024];
    uint8_t color_page[1024];
    // redirect pointers to local buffer
    text_p1 = text_page;
    text_p3 = color_page;

    // initialize the splash screen
    showTitle(PRINTMODE_NORMAL);
    centerY(LINE_A2DIGITAL,   pSplashMsg[0], PRINTMODE_NORMAL);
    centerY(LINE_COPYRIGHT,   pSplashMsg[1], PRINTMODE_NORMAL);
    centerY(LINE_COPYRIGHT+1, pSplashMsg[2], PRINTMODE_NORMAL);
    centerY(LINE_6502,        pStatus6502[0], PRINTMODE_INVERSE);
    if (bus_cycle_counter < 3*1000)
        centerY(LINE_ACTIVITY,    pStatus6502[4], PRINTMODE_FLASH);
    centerY(22, TitleGitHub[1], PRINTMODE_NORMAL);

    // set colors
    for (uint8_t line=0;line<24;line++)
    {
        uint8_t color = 0xF0|BGCOLOR;
        switch(line)
        {
            case 0: // fall-through
            case 22:// fall-through
            case 23:color = 0xF4;break; // white, dark-green
            case LINE_COPYRIGHT:// fall-through
            case LINE_COPYRIGHT+1:color = 0xD0|BGCOLOR;break; // yellow, black
            case LINE_6502...LINE_6502+4:color = 0x90|BGCOLOR;break; // orange, black
        }

        for (uint8_t i=0;i<40;i++)
        {
            color_page[TEXT_OFFSET(line)+i] = color;
        }
    }

    // show A2DVI "logo"
    {
        uint32_t ofs = TEXT_OFFSET(LINE_A2DVI);
        uint8_t line = 0;
        uint8_t color;
        for (uint32_t i=0;(pLogo[i]!=0);i++)
        {
            color = logoColors[line];
            char c = pLogo[i];
            if (c =='\n')
            {
                line++;
                ofs = TEXT_OFFSET(LINE_A2DVI+line);
            }
            else
            {
                color_page[ofs++] = (c == ' ') ? BGCOLOR : color;
            }
        }
    }

    // show 6502 status
    const uint X1=10;
    const uint X2=23;
    printXY(X1, LINE_6502+1, "BUS CYCLES :",   PRINTMODE_NORMAL);
    printXY(X1, LINE_6502+2, "ADDRESS BUS: ?", PRINTMODE_NORMAL);
    printXY(X1, LINE_6502+3, "PC         : ?", PRINTMODE_NORMAL);

    // enable DVI output now
    a2dvi_dvi_enable();

    dvi0.scanline_emulation = false;

    // render splash screen, show 6502 status
    uint32_t splash_frames = 0;
    while ((bus_cycle_counter < 6*1000*1000)&&(reset_counter == 0)) // until reset vector detected, or bus activity for >= 6 seconds
    {
        if ((frame_counter & 0x7)==2) // don't update text in the very first frame, since the PIOs expect the TMDS stream to begin *immediately*
        {
            // show number of bus cycles
            int2hex(text_page+TEXT_OFFSET(LINE_6502+1)+X2, bus_cycle_counter, 8);

            if (bus_cycle_counter>0)
            {
                // show address bus and PC (if the clock is alive)
                int2hex(text_page+TEXT_OFFSET(LINE_6502+2)+X2, (uint16_t) last_read_address, 4);
                int2hex(text_page+TEXT_OFFSET(LINE_6502+3)+X2, last_address_pc, 4);
            }
        }

        render_debug(false, true);
        for (uint line=0;line<24;line++)
        {
            render_color_text40_line(line);
        }
        render_debug(false, false);
        update_text_flasher();
        frame_counter++;
        splash_frames++;

        if (bus_cycle_counter > 3*1000)
        {
            centerY(LINE_ACTIVITY, pStatus6502[5], PRINTMODE_FLASH);
        }
    }

    // restore pointers to text page
    text_p1 = (volatile uint8_t*) p1;
    text_p3 = (volatile uint8_t*) p3;
}
