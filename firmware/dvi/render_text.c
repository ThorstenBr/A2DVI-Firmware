/*
MIT License

Copyright (c) 2021 Mark Aikens
Copyright (c) 2023 David Kuder
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
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "../applebus/buffers.h"
#include "../config/config.h"
#include "../fonts/A2e_US_Enhanced.h"

#include "render.h"

#define TMDS_SYMBOL_0_0     0x7fd00 // actually 00/01
#define TMDS_SYMBOL_255_0   0x7fe00 // actually FF/01
#define TMDS_SYMBOL_0_255   0x801ff // actually 01/FF
#define TMDS_SYMBOL_255_255 0xbfe00 // actually FF/FE

#define character_rom A2E_US_ENHANCED_BIN

uint32_t text_fore[3] = {TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_255, TMDS_SYMBOL_0_0};
uint32_t text_back[3] = {TMDS_SYMBOL_0_0, TMDS_SYMBOL_0_0,     TMDS_SYMBOL_0_0};

volatile uint_fast32_t text_flasher_mask = 0;
static uint64_t next_flash_tick = 0;

void DELAYED_COPY_CODE(update_text_flasher)()
{
    uint64_t now = time_us_64();
    if(now > next_flash_tick)
    {
        text_flasher_mask ^= 0xff;

        switch(current_machine)
        {
        default:
        case MACHINE_II:
        case MACHINE_PRAVETZ:
            next_flash_tick = now + 125000u;
            break;
        case MACHINE_IIE:
        case MACHINE_IIGS:
            next_flash_tick = now + 250000u;
            break;
        }
    }
}

static inline uint_fast8_t char_text_bits(uint_fast8_t ch, uint_fast8_t glyph_line)
{
    uint_fast8_t bits, invert;

    if((soft_switches & SOFTSW_ALTCHAR) || (ch & 0x80))
    {
        // normal / mousetext character
        invert = 0x00;
    } else {
        // flashing character or inverse character
        invert = (ch & 0x40) ? text_flasher_mask : 0x7f;
        ch = (ch & 0x3f) | 0x80;
    }

    bits = character_rom[((uint_fast16_t)ch << 3) | glyph_line];

    return (bits ^ invert) & 0x7f;
}

void DELAYED_COPY_CODE(render_text40_line)(const uint8_t *page, unsigned int line)
{
    const uint8_t *line_buf = (const uint8_t *)(page + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    for(uint glyph_line=0; glyph_line < 8; glyph_line++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

        for(uint col=0; col < 40; )
        {
            // Grab 14 pixels from the next two characters
            uint32_t bits;
            bits  = char_text_bits(line_buf[col++], glyph_line);
            bits |= char_text_bits(line_buf[col++], glyph_line) << 7;

            // Translate bits into a pair of pixels
            for(int i=0; i < 14; i++)
            {
                if ((bits & 1) == 0)
                {
                    *(tmdsbuf_blue++)  = text_fore[2];
                    *(tmdsbuf_green++) = text_fore[1];
                    *(tmdsbuf_red++)   = text_fore[0];
                }
                else
                {
                    *(tmdsbuf_blue++)  = text_back[2];
                    *(tmdsbuf_green++) = text_back[1];
                    *(tmdsbuf_red++)   = text_back[0];
                }
                bits >>= 1;
            }
        }
        dvi_send_scanline(tmdsbuf);
    }
}

void DELAYED_COPY_CODE(render_text)()
{
    const bool page2 = soft_switches & SOFTSW_PAGE_2;
    const uint8_t *page = (const uint8_t *)(page2 ? text_p2 : text_p1);

    for (uint y=0;y<24;y++)
    {
        render_text40_line(page, y);
    }

#if 0
	// fill with dummy lines
	for (uint y=0;y<192;y++)
	{
		uint32_t *tmdsbuf;
		queue_remove_blocking_u32(&dvi0.q_tmds_free, &tmdsbuf);
		tmdsbuf[0] = TMDS_SYMBOL_255_255;
		tmdsbuf[5] = TMDS_SYMBOL_255_255;
		tmdsbuf[10] = TMDS_SYMBOL_255_255;
		tmdsbuf[15] = TMDS_SYMBOL_255_255;
		queue_add_blocking_u32(&dvi0.q_tmds_valid, &tmdsbuf);
	}
#endif
}
