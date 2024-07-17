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

void int2hex(uint8_t* pStrBuf, uint32_t value, uint32_t digits)
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

void DELAYED_COPY_CODE(render_status)(uint8_t line)
{
	if (!IS_IFLAG(IFLAGS_STATUSLINES))
	{
		for (uint line=0;line<8;line++)
		{
			dvi_get_scanline(tmdsbuf);
			dvi_scanline_rgb(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);
			for (uint32_t x=0;x<280;x++)
			{
				*(tmdsbuf_red++)   = TMDS_SYMBOL_0_0;
				*(tmdsbuf_green++) = TMDS_SYMBOL_0_0;
				*(tmdsbuf_blue++)  = TMDS_SYMBOL_0_0;
			}
			dvi_send_scanline(tmdsbuf);
		}
		return;
	}

	if (line == 0)
	{
		// clear status line
		for (uint i=0;i<sizeof(status_line)/4/2;i++)
		{
			((uint32_t*)status_line)[i] = 0xA0A0A0A0;
		}

		// show graphics/text mode
		switch(soft_switches & SOFTSW_MODE_MASK)
 		{
			case 0:
				copy_str(&status_line[0], (IS_IFLAG(SOFTSW_DGR)) ? "DGR" : "GR");
				break;
			case SOFTSW_MIX_MODE:
				copy_str(&status_line[0], ((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR)) ? "DGR" : "DGR MIX");
				break;
			case SOFTSW_HIRES_MODE:
				copy_str(&status_line[0], (IS_IFLAG(SOFTSW_DGR)) ? "DHGR" : "HGR");
				break;
			case SOFTSW_HIRES_MODE|SOFTSW_MIX_MODE:
				copy_str(&status_line[0], ((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR)) ? "DHGR MIX" : "HGR");
				break;
			default:
				copy_str(&status_line[0], "TEXT");
				break;
		}

		// show state of other soft-switches
		copy_str(&status_line[9],   (IS_IFLAG(SOFTSW_PAGE_2)) ? "P2" : "P1");
		copy_str(&status_line[9+3], (IS_IFLAG(SOFTSW_80COL))  ? "80" : "40");

		if (IS_SOFTSWITCH(SOFTSW_MONOCHROME))
		{
			copy_str(&status_line[9+3+3], "MONO");
		}

		if (IS_SOFTSWITCH(SOFTSW_ALTCHAR))
		{
			copy_str(&status_line[9+3+3+5], "ALT");
		}

		if (IS_SOFTSWITCH(SOFTSW_80STORE))
			copy_str(&status_line[9+3+3+5+4], "80S");

		if (IS_SOFTSWITCH(SOFTSW_AUX_READ))
			copy_str(&status_line[9+3+3+5+4+4], "XR");

		if (IS_SOFTSWITCH(SOFTSW_AUX_WRITE))
			copy_str(&status_line[9+3+3+5+4+4+3], "XW");

		if (IS_SOFTSWITCH(SOFTSW_AUXZP))
			copy_str(&status_line[9+3+3+5+4+4+3+3], "XZ");

		if (IS_SOFTSWITCH(SOFTSW_IOUDIS))
		{
			copy_str(&status_line[9+3+3+5+4+4+3+3+3], "IOU");
		}

		// render status line
		render_text40_line(status_line, 0, 4);
	}
	else
	{
		if ((frame_counter & 7) == 0) // do not update too fast, so data remains readable
		{
			// clear status line
			for (uint i=0;i<sizeof(status_line)/4/2;i++)
			{
				((uint32_t*)status_line)[40/4+i] = 0xA0A0A0A0;
			}

			// program counter
			copy_str(&status_line[40], "PC:");
			int2hex(&status_line[43], last_address_pc, 4);

			// recent stack access
			copy_str(&status_line[43+5], "S:");
			int2hex(&status_line[43+5+2], last_address_stack, 3);

			// recent zero-page access
			copy_str(&status_line[43+5+2+4], "ZP:");
			int2hex(&status_line[43+5+2+4+3], last_address_zp, 2);
		}

		// render status line
		render_text40_line(&status_line[40], 0, 4);
	}
}
