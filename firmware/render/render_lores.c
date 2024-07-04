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

#include <pico/stdlib.h>
#include "applebus/buffers.h"
#include "config/config.h"
#include "render.h"

// monochrome dot pattern: 14 dots (bits) per word
uint16_t DELAYED_COPY_DATA(lores_dot_pattern)[16] =
{
    0x0000,
    0x1111,
    0x2222,
    0x3333,
    0x0444,
    0x1555,
    0x2666,
    0x3777,
    0x0888,
    0x1999,
    0x2aaa,
    0x3bbb,
    0x0ccc,
    0x1ddd,
    0x2eee,
    0x3fff
};

static void render_lores_line(bool p2, uint line);

#define PAGE2SEL ((soft_switches & (SOFTSW_80STORE | SOFTSW_PAGE_2)) == SOFTSW_PAGE_2)

void DELAYED_COPY_CODE(render_lores)()
{
    for(uint line=0; line < 24; line++)
    {
        render_lores_line(PAGE2SEL, line);
    }
}


void DELAYED_COPY_CODE(render_mixed_lores)()
{
    for(uint line=0; line < 20; line++)
    {
        render_lores_line(PAGE2SEL, line);
    }

    render_mixed_text();
}


static void DELAYED_COPY_CODE(render_lores_line)(bool p2, uint line)
{
    // Construct two scanlines for the two different colored cells at the same time
    dvi_get_scanline(tmdsbuf1);
    dvi_scanline_rgb(tmdsbuf1, tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue);

    dvi_get_scanline(tmdsbuf2);
    dvi_scanline_rgb(tmdsbuf2, tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue);

    const uint8_t *line_buf = (const uint8_t *)((p2 ? text_p2 : text_p1) + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    if(mono_rendering)
    {
        uint8_t color_offset = color_mode*12;
        for(uint i = 0; i < 40; i+=2)
        {
            uint32_t pattern1  = lores_dot_pattern[line_buf[i] & 0xf];
            pattern1 |= lores_dot_pattern[line_buf[i+1] & 0xf] << 14;

            uint32_t pattern2  = lores_dot_pattern[(line_buf[i] >> 4) & 0xf];
            pattern2 |= lores_dot_pattern[(line_buf[i+1] >> 4) & 0xf] << 14;

            for(uint j = 0; j < 14; j++)
            {
                uint32_t offset = color_offset + (pattern1 & 0x3);
                *(tmdsbuf1_red++)   = tmds_mono_pixel_pair[offset+0];
                *(tmdsbuf1_green++) = tmds_mono_pixel_pair[offset+4];
                *(tmdsbuf1_blue++)  = tmds_mono_pixel_pair[offset+8];
                pattern1 >>= 2;

                offset = color_offset + (pattern2 & 0x3);
                *(tmdsbuf2_red++)   = tmds_mono_pixel_pair[offset+0];
                *(tmdsbuf2_green++) = tmds_mono_pixel_pair[offset+4];
                *(tmdsbuf2_blue++)  = tmds_mono_pixel_pair[offset+8];
                pattern2 >>= 2;
            }
        }
    }
    else
    {
        for(uint i = 0; i < 40; i++)
        {
            uint32_t color1 = line_buf[i] & 0xf;
            uint32_t color2 = (line_buf[i] >> 4) & 0xf;

            // Each lores pixel is 7 hires pixels, or 14 VGA pixels wide
            uint32_t* pTmds = &tmds_lorescolor[color1*3];
            uint32_t r = pTmds[0];
            uint32_t g = pTmds[1];
            uint32_t b = pTmds[2];
            for (uint j = 0; j < 7; j++)
            {
                *(tmdsbuf1_red++)   = r;
                *(tmdsbuf1_green++) = g;
                *(tmdsbuf1_blue++)  = b;
            }

            pTmds = &tmds_lorescolor[color2*3];
            r = pTmds[0];
            g = pTmds[1];
            b = pTmds[2];
            for (uint j = 0; j < 7; j++)
            {
                *(tmdsbuf2_red++)   = r;
                *(tmdsbuf2_green++) = g;
                *(tmdsbuf2_blue++)  = b;
            }
        }
    }

    // repeat this line 3 more times (4x in total)
    for (uint yrepeat=0;yrepeat<3;yrepeat++)
    {
        dvi_get_scanline(tmdsbufRepeat);
        dvi_copy_scanline(tmdsbufRepeat, tmdsbuf1);
        // send copied buffer
        dvi_send_scanline(tmdsbufRepeat);
    }
    // send original buffer
    dvi_send_scanline(tmdsbuf1);

    // repeat this line 3 more times (4x in total)
    for (uint yrepeat=0;yrepeat<3;yrepeat++)
    {
        dvi_get_scanline(tmdsbufRepeat);
        dvi_copy_scanline(tmdsbufRepeat, tmdsbuf2);
        // send copied buffer
        dvi_send_scanline(tmdsbufRepeat);
    }

    // send original buffer
    dvi_send_scanline(tmdsbuf2);
}
