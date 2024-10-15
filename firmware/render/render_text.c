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

#include "applebus/buffers.h"
#include "config/config.h"

#include "render.h"

#define PAGE2SEL ((soft_switches & (SOFTSW_80STORE | SOFTSW_PAGE_2)) == SOFTSW_PAGE_2)

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
            case MACHINE_IIE_ENH:
#ifdef MACHINE_IIGS
            case MACHINE_IIGS:
#endif
                next_flash_tick = now + 250000u;
                break;
        }
    }
}

static inline uint_fast8_t char_text_bits(uint_fast8_t ch, uint_fast8_t glyph_line)
{
    uint_fast8_t bits, invert;

    if((ch & 0x80) || (soft_switches & SOFTSW_ALTCHAR))
    {
        // normal / mousetext character
        invert = 0x00;
    } else {
        // flashing character or inverse character
        invert = (ch & 0x40) ? text_flasher_mask : 0x7f;
        ch = (ch & 0x3f) | 0x80;
    }

    uint_fast16_t LanguageOffset = (language_switch) ? 0x800 : 0x0;
    bits = character_rom[LanguageOffset | ((uint_fast16_t)ch << 3) | glyph_line];

    return (bits ^ invert) & 0x7f;
}

void DELAYED_COPY_CODE(render_text40_line)(const uint8_t *page, unsigned int line, uint8_t color_mode)
{
    const uint8_t *line_buf = (const uint8_t *)(page + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    for(uint glyph_line=0; glyph_line < 8; glyph_line++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb560(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

        for(uint col=0; col < 40; )
        {
            // Grab 14 pixels from the next two characters
            uint32_t bits;
            bits  = char_text_bits(line_buf[col++], glyph_line);
            bits |= char_text_bits(line_buf[col++], glyph_line) << 7;

            // Translate bits into a pair of pixels
            for(int i=0; i < 14; i++)
            {
                uint8_t color_offset = color_mode*3;
                if ((bits & 1)==0)
                    color_offset = 3*3;
                *(tmdsbuf_blue++)  = tmds_mono_double_pixel[color_offset + 2];
                *(tmdsbuf_green++) = tmds_mono_double_pixel[color_offset + 1];
                *(tmdsbuf_red++)   = tmds_mono_double_pixel[color_offset + 0];
                bits >>= 1;
            }
        }
        dvi_send_scanline(tmdsbuf);
    }
}

#define ADD_LORES_PIXEL(color) { \
    uint32_t* pTmds = &tmds_lorescolor[color*3]; \
    *(tmdsbuf_red++)   = pTmds[0]; \
    *(tmdsbuf_green++) = pTmds[1]; \
    *(tmdsbuf_blue++)  = pTmds[2]; \
}

void DELAYED_COPY_CODE(render_color_text40_line)(unsigned int line)
{
    const uint8_t *line_buf = (const uint8_t *)(text_p1 + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    const uint8_t *color_buf = (const uint8_t *)(text_p3 + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    for(uint glyph_line=0; glyph_line < 8; glyph_line++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb560(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

        for(uint col=0; col < 40; col++)
        {
            // Grab 7 pixels from the next character
            uint32_t bits  = char_text_bits(line_buf[col], glyph_line);
            uint8_t foreground_color = (color_buf[col] >> 4) & 0xf;
            uint8_t background_color = (color_buf[col]     ) & 0xf;

            // Translate each pair of bits into a pair of pixels
            for(int i=0; i < 7; i++)
            {
                if (bits & 0x1)
                {
                    ADD_LORES_PIXEL(foreground_color);
                }
                else
                {
                    ADD_LORES_PIXEL(background_color);
                }
                bits >>= 1;
            }
        }

        dvi_send_scanline(tmdsbuf);
    }
}

void DELAYED_COPY_CODE(render_text80_line)(const uint8_t *page_a, const uint8_t *page_b, unsigned int line, uint8_t color_mode)
{
    uint line_offset = ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40);
    const uint8_t *line_buf_a = (const uint8_t *) (page_a + line_offset);
    const uint8_t *line_buf_b = (const uint8_t *) (page_b + line_offset);

    uint8_t color_offset = color_mode*12;

    for(uint glyph_line=0; glyph_line < 8; glyph_line++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb560(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

        for(uint col=0; col < 40;)
        {
            // Grab 14 pixels from the next two characters
            uint32_t bits;
            bits  = char_text_bits(line_buf_a[col], glyph_line) << 7;
            bits |= char_text_bits(line_buf_b[col], glyph_line);
            col++;

            // Translate each pair of bits into a pair of pixels
            for(int i=0; i < 7; i++)
            {
                char symbol = (bits&3) + color_offset;
                *(tmdsbuf_blue++)  = tmds_mono_pixel_pair[symbol+8];
                *(tmdsbuf_green++) = tmds_mono_pixel_pair[symbol+4];
                *(tmdsbuf_red++)   = tmds_mono_pixel_pair[symbol+0];
                bits >>= 2;
            }
        }
        dvi_send_scanline(tmdsbuf);
    }
}

void DELAYED_COPY_CODE(render_mixed_text)()
{
    if((internal_flags & IFLAGS_VIDEO7) && ((soft_switches & (SOFTSW_80STORE | SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80STORE | SOFTSW_DGR)))
    {
        if (!mono_rendering)
        {
            for(uint line=20; line < 24; line++)
            {
                render_color_text40_line(line);
            }
            return;
        }
    }

    // monochrome rendering
    {
        const bool page2 = PAGE2SEL;
        const uint8_t *pageA = (const uint8_t *)(page2 ? text_p2 : text_p1);
        uint8_t cmode = (mono_rendering) ? color_mode : 0; /* white */

        if(soft_switches & SOFTSW_80COL)
        {
            // 80 column mode rendering
            const uint8_t *pageB = (const uint8_t *)(page2 ? text_p4 : text_p3);
            for(uint line=20; line < 24; line++)
            {
                render_text80_line(pageA, pageB, line, cmode);
            }
        }
        else
        {
            // 40 column mode rendering
            for(uint line=20; line < 24; line++)
            {
                render_text40_line(pageA, line, cmode);
            }
        }
    }
}

void DELAYED_COPY_CODE(render_text)()
{
    if((internal_flags & IFLAGS_VIDEO7) && ((soft_switches & (SOFTSW_80STORE | SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80STORE | SOFTSW_DGR)))
    {
        if (!mono_rendering)
        {
            for(uint line=0; line < 24; line++)
            {
                render_color_text40_line(line);
            }
            return;
        }
    }
    else
    {
        color_support = false;
    }

    // monochrome rendering
    {
        const bool page2 = PAGE2SEL;
        const uint8_t *pageA = (const uint8_t *)(page2 ? text_p2 : text_p1);

        if(soft_switches & SOFTSW_80COL)
        {
            // 80 column mode rendering
            const uint8_t *pageB = (const uint8_t *)(page2 ? text_p4 : text_p3);
            for(uint line=0; line < 24; line++)
            {
                render_text80_line(pageA, pageB, line, color_mode);
            }
        }
        else
        {
            // 40 column mode rendering
            for(uint line=0; line < 24; line++)
            {
                render_text40_line(pageA, line, color_mode);
            }
        }
    }
}
