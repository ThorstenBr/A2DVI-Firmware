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
#include "dvi/render.h"

#define PAGE2SEL ((soft_switches & (SOFTSW_80STORE | SOFTSW_PAGE_2)) == SOFTSW_PAGE_2)

uint8_t DELAYED_COPY_DATA(dgr_dot_pattern)[32] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F,
    0x00, 0x44, 0x08, 0x4C, 0x11, 0x55, 0x19, 0x5D,
    0x22, 0x66, 0x2A, 0x6E, 0x33, 0x77, 0x3B, 0x7F,
};

extern uint32_t DELAYED_COPY_DATA(tmds_lores_mono)[4*3];

// TMDS symbols for LORES RGB colors - using the "double pixel" trick
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
extern uint32_t DELAYED_COPY_DATA(tmds_lorescolor)[3*16];

static void render_dgr_line(bool p2, uint line);

void DELAYED_COPY_CODE(render_dgr)() {
    for(uint line=0; line < 24; line++) {
        render_dgr_line(PAGE2SEL, line);
    }
}


void DELAYED_COPY_CODE(render_mixed_dgr)() {
    for(uint line=0; line < 20; line++) {
        render_dgr_line(PAGE2SEL, line);
    }

    render_mixed_text();
}


static void DELAYED_COPY_CODE(render_dgr_line)(bool p2, uint line)
{
    // Construct two scanlines for the two different colored cells at the same time
    dvi_get_scanline(tmdsbuf1);
    dvi_scanline_rgb(tmdsbuf1, tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue);

    dvi_get_scanline(tmdsbuf2);
    dvi_scanline_rgb(tmdsbuf2, tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue);

    const uint8_t *line_bufa = (const uint8_t *)((p2 ? text_p2 : text_p1) + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    const uint8_t *line_bufb = (const uint8_t *)((p2 ? text_p4 : text_p3) + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    uint i = 0;
    uint_fast8_t dotc = 0;

#if 0
    if(mono_rendering)
#endif
    {
        uint32_t pattern1=0, pattern2=0;

        while(i < 40)
        {
            while((dotc <= 14) && (i < 40))
            {
                pattern1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufb[i] & 0xf)] << dotc;
                pattern2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufb[i] >> 4) & 0xf)] << dotc;
                dotc += 7;
                pattern1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufa[i] & 0xf)] << dotc;
                pattern2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufa[i] >> 4) & 0xf)] << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while(dotc >= 2)
            {
                uint32_t offset = (pattern1 & 3);
                *(tmdsbuf1_red++)   = tmds_lores_mono[offset+0];
                *(tmdsbuf1_green++) = tmds_lores_mono[offset+4];
                *(tmdsbuf1_blue++)  = tmds_lores_mono[offset+8];

                offset = (pattern2 & 3);
                *(tmdsbuf2_red++)   = tmds_lores_mono[offset+0];
                *(tmdsbuf2_green++) = tmds_lores_mono[offset+4];
                *(tmdsbuf2_blue++)  = tmds_lores_mono[offset+8];
                pattern1 >>= 2;
                pattern2 >>= 2;
                dotc -= 2;
            }
        }
    }
#if 0
    else
    {
        uint32_t color1 = 0, color2 = 0;
        while(i < 40)
        {
            while((dotc <= 14) && (i < 40))
            {
                color1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufb[i] & 0xf)] << dotc;
                color2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufb[i] >> 4) & 0xf)] << dotc;
                dotc += 7;
                color1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufa[i] & 0xf)] << dotc;
                color2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufa[i] >> 4) & 0xf)] << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while((dotc >= 8) || ((dotc > 0) && (i == 40)))
            {
                color1 &= 0xfffffffe;
                color1 |= (color1 >> 4) & 1;
                pixeldata = dhgr_palette[color1 & 0xf];

                color1 &= 0xfffffffc;
                color1 |= (color1 >> 4) & 3;
                pixeldata |= dhgr_palette[color1 & 0xf] << 16;
                sl1->data[sl_pos] = pixeldata;

                color2 &= 0xfffffffe;
                color2 |= (color2 >> 4) & 1;
                pixeldata = dhgr_palette[color2 & 0xf];
                color2 &= 0xfffffffc;
                color2 |= (color2 >> 4) & 3;
                pixeldata |= dhgr_palette[color2 & 0xf] << 16;
                sl2->data[sl_pos] = pixeldata;

                sl_pos++;

                color1 &= 0xfffffff8;
                color1 |= (color1 >> 4) & 7;
                pixeldata = dhgr_palette[color1 & 0xf];
                color1 >>= 4;
                pixeldata |= dhgr_palette[color1 & 0xf] << 16;
                sl1->data[sl_pos] = pixeldata;

                color2 &= 0xfffffff8;
                color2 |= (color2 >> 4) & 7;
                pixeldata = dhgr_palette[color2 & 0xf];
                color2 >>= 4;
                pixeldata |= dhgr_palette[color2 & 0xf] << 16;
                sl2->data[sl_pos] = pixeldata;

                sl_pos++;
                dotc -= 4;
            }
        }
    }
#endif

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
