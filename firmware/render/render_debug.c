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

uint32_t show_subtitle_cycles;

void DELAYED_COPY_CODE(int2hex)(uint8_t* pStrBuf, uint32_t value, uint32_t digits)
{
    for (int32_t i=0;i<digits;i++)
    {
        pStrBuf[digits-i-1] = (value & 0xF) < 10 ? ((0x80|'0')+(value & 0xf)) : (0x80|('A'-10))+(value & 0xf);
        value >>= 4;
    }
}

void DELAYED_COPY_CODE(copy_str)(uint8_t* dest, const char* pMsg)
{
    while (*pMsg)
    {
        *(dest++) = *(pMsg++)|0x80;
    }
}

void DELAYED_COPY_CODE(update_debug_monitor)(void)
{
    if ((frame_counter & 3) == 0) // do not update too fast, so data remains readable
    {
        uint8_t* line1 = status_line;
        uint8_t* line2 = &status_line[40];

        // clear status lines
        for (uint i=0;i<sizeof(status_line)/4/2;i++)
        {
            ((uint32_t*)status_line)[i] = 0xA0A0A0A0;
        }

        // show graphics/text mode
        switch(soft_switches & SOFTSW_MODE_MASK)
        {
            case 0:
                copy_str(&line1[0], (IS_SOFTSWITCH(SOFTSW_DGR)) ? "DGR" : "GR");
                break;
            case SOFTSW_MIX_MODE:
                copy_str(&line1[0], ((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR)) ? "DGR" : "DGR MIX");
                break;
            case SOFTSW_HIRES_MODE:
                copy_str(&line1[0], (IS_SOFTSWITCH(SOFTSW_DGR)) ? "DHGR" : "HGR");
                break;
            case SOFTSW_HIRES_MODE|SOFTSW_MIX_MODE:
                copy_str(&line1[0], ((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR)) ? "DHGR MIX" : "HGR");
                break;
            default:
                copy_str(&line1[0], "TEXT");
                break;
        }

        // show state of other soft-switches
        copy_str(&line1[ 9], (IS_SOFTSWITCH(SOFTSW_80COL))  ? "80" : "40");
        copy_str(&line1[12], (IS_SOFTSWITCH(SOFTSW_PAGE_2)) ? "P2" : "P1");

        if (IS_IFLAG(IFLAGS_TEST) && vblank_counter)
        {
            copy_str(&line1[28], "VSYNC");
            vblank_counter = 0;
        }

        if (IS_IFLAG(IFLAGS_VIDEO7))
        {
            copy_str(&line1[34], "V7:");
            line1[37] = 0x80|'0'|(internal_flags&0x3);
        }

        // Apple IIe specific registers
        if(internal_flags & (IFLAGS_IIGS_REGS | IFLAGS_IIE_REGS))
        {
            if (IS_SOFTSWITCH(SOFTSW_MONOCHROME))
                copy_str(&line1[15], "MONOCHROME");

            if (IS_SOFTSWITCH(SOFTSW_ALTCHAR))
                copy_str(&line1[26], "ALTCHAR");

            if (IS_SOFTSWITCH(SOFTSW_80STORE))
                copy_str(&line2[3], "80STR");

            if (IS_SOFTSWITCH(SOFTSW_AUX_READ))
                copy_str(&line2[9], "AUXR");

            if (IS_SOFTSWITCH(SOFTSW_AUX_WRITE))
                copy_str(&line2[14], "AUXW");

            if (IS_SOFTSWITCH(SOFTSW_AUXZP))
                copy_str(&line2[19], "AUXZ");

            if (IS_SOFTSWITCH(SOFTSW_SLOT3ROM))
                copy_str(&line2[24], "C3ROM");

            if (IS_SOFTSWITCH(SOFTSW_INTCXROM))
                 copy_str(&line2[30], "CXROM");

            if (IS_SOFTSWITCH(SOFTSW_IOUDIS))
                copy_str(&line2[36], "IOUD");
        }
    }

    if (((frame_counter & 0xF) == 0)&& // do not update too fast, so data remains readable
        (show_subtitle_cycles == 0))
    {
        /*0123456789012345678901234567890123456789
         *PC:1234 S:123 ZP:12              OV:1234
         *   80STR AUXR AUXW AUXZ C3ROM CXROM IOUD
         */
        uint8_t* line1 = &status_line[80];
        uint8_t* line2 = &status_line[120];

        // clear status line
        for (uint i=0;i<sizeof(status_line)/4/2;i++)
        {
            ((uint32_t*)line1)[i] = 0xA0A0A0A0;
        }

        // program counter
        copy_str(&line2[0], "PC:");
        int2hex(&line2[3], last_address_pc, 4);

        // recent stack access
        copy_str(&line2[8], "S:");
        int2hex(&line2[10], last_address_stack, 3);

        // recent zero-page access
        copy_str(&line2[14], "ZP:");
        int2hex(&line2[17], last_address_zp, 2);

        if (IS_IFLAG(IFLAGS_TEST))
        {
            copy_str(&line2[33], "OV:");
            int2hex(&line2[36], bus_overflow_counter, 4);
        }

#ifdef FEATURE_DEBUG_COUNTER
#warning DEBUG FEATURE IS ENABLED! ************************************
        int2hex(&line2[20], dbg_counter1, 8);
        int2hex(&line2[29], dbg_counter2, 8);
#endif
    }
}

void DELAYED_COPY_CODE(render_debug)(bool IsVidexMode, bool top)
{
    if (!IS_IFLAG(IFLAGS_DEBUG_LINES))
    {
        if ((top)||(show_subtitle_cycles==0))
        {
            // videx needs 216 lines/screen instead of 192 as usual, we can only render one status line above/below the screen area)
            uint row_count = 16;
            if (IsVidexMode)
            {
                row_count = (top) ? 0 : 8;
            }
            for (uint row=0;row<row_count;row++)
            {
                dvi_get_scanline(tmdsbuf);
                dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);
                for (uint32_t x=0;x<320;x++)
                {
                    *(tmdsbuf_red++)   = TMDS_SYMBOL_0_0;
                    *(tmdsbuf_green++) = TMDS_SYMBOL_0_0;
                    *(tmdsbuf_blue++)  = TMDS_SYMBOL_0_0;
                }
                dvi_send_scanline(tmdsbuf);
            }
            return;
        }
    }
    else
    if (top)
    {
        update_debug_monitor();
    }

    if (top)
    {
        // render two debug monitor lines above the screen area
        uint8_t* line1 = status_line;
        uint8_t* line2 = &status_line[40];
        if (!IsVidexMode)
        {
            render_text40_line(line1, 0, 4);
            render_text40_line(line2, 0, 4);
        }
    }
    else
    {
        // render two debug monitor lines below the screen area
        uint8_t* line1 = &status_line[80];
        uint8_t* line2 = &status_line[120];
        if (!IsVidexMode)
            render_text40_line(line1, 0, 4);
        render_text40_line(line2, 0, 4);

        if (show_subtitle_cycles)
            show_subtitle_cycles--;
    }
}
