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

#include "applebus/buffers.h"
#include "config/config.h"
#include "render.h"

#define PAGE2SEL ((soft_switches & (SOFTSW_80STORE | SOFTSW_PAGE_2)) == SOFTSW_PAGE_2)

uint8_t DELAYED_COPY_DATA(dgr_dot_pattern)[32] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F,

    0x00, 0x44, 0x08, 0x4C, 0x11, 0x55, 0x19, 0x5D,
    0x22, 0x66, 0x2A, 0x6E, 0x33, 0x77, 0x3B, 0x7F,
};

// double lores to dhgr palette mapping (for odd columns, with "non-rotated" nibbles)
static uint8_t DELAYED_COPY_DATA(dlores_dhgr_map)[16] = {
    0x00,0x08,0x01,0x09,0x02,0x0A,0x03,0x0B,
    0x04,0x0C,0x05,0x0D,0x06,0x0E,0x07,0x0F
};

// add pixels to TMDS line
#define ADD_TMDS_DGR_PIXELS(tmds_red, tmds_green, tmds_blue, dhgr_index, count) \
{\
    uint32_t r = tmds_dhgr_red  [dhgr_index];\
    uint32_t g = tmds_dhgr_green[dhgr_index];\
    uint32_t b = tmds_dhgr_blue [dhgr_index];\
    for (uint x=0;x<count;x++)\
    {\
        *(tmds_red++)   = r;\
        *(tmds_green++) = g;\
        *(tmds_blue++)  = b;\
    }\
}

static void render_dgr_line(bool p2, uint line);

static void DELAYED_COPY_CODE(render_dgr_line)(bool p2, uint line)
{
    // Construct two scanlines for the two different colored cells at the same time
    dvi_get_scanline(tmdsbuf1);
    dvi_scanline_rgb560(tmdsbuf1, tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue);

    dvi_get_scanline(tmdsbuf2);
    dvi_scanline_rgb560(tmdsbuf2, tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue);

    const uint8_t *line_bufa = (const uint8_t *)((p2 ? text_p2 : text_p1) + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    const uint8_t *line_bufb = (const uint8_t *)((p2 ? text_p4 : text_p3) + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));

    uint i = 0;
    uint_fast8_t dotc = 0;

    if(mono_rendering)
    {
        uint32_t pattern1=0, pattern2=0;
        uint8_t color_offset = color_mode*12;

        while(i < 40)
        {
            while((dotc <= 14) && (i < 40))
            {
                pattern1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufb[i]          & 0xf)    ] << dotc;
                pattern2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufb[i] >> 4) /*& 0xf*/)] << dotc;
                dotc += 7;
                pattern1 |= dgr_dot_pattern[((i & 1) << 4) | (line_bufa[i]          & 0xf)    ] << dotc;
                pattern2 |= dgr_dot_pattern[((i & 1) << 4) | ((line_bufa[i] >> 4) /*& 0xf*/)] << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while(dotc >= 2)
            {
                uint32_t offset = color_offset + (pattern1 & 3);
                *(tmdsbuf1_red++)   = tmds_mono_pixel_pair[offset+0];
                *(tmdsbuf1_green++) = tmds_mono_pixel_pair[offset+4];
                *(tmdsbuf1_blue++)  = tmds_mono_pixel_pair[offset+8];

                offset = color_offset + (pattern2 & 3);
                *(tmdsbuf2_red++)   = tmds_mono_pixel_pair[offset+0];
                *(tmdsbuf2_green++) = tmds_mono_pixel_pair[offset+4];
                *(tmdsbuf2_blue++)  = tmds_mono_pixel_pair[offset+8];
                pattern1 >>= 2;
                pattern2 >>= 2;
                dotc -= 2;
            }
        }
    }
    else
    if(internal_flags & IFLAGS_INTERP_DGR)
    {
        // based David's DGR renderer - with artifacts
        uint32_t color1 = 0, color2 = 0;
        uint8_t dhgr_index;
        while(i < 40)
        {
            while((dotc <= 14) && (i < 40))
            {
                uint8_t iofs = (i & 1) << 4;
                color1 |= dgr_dot_pattern[iofs | ( line_bufb[i]         & 0xf)  ] << dotc;
                color2 |= dgr_dot_pattern[iofs | ((line_bufb[i] >> 4) /*& 0xf*/)] << dotc;
                dotc += 7;
                color1 |= dgr_dot_pattern[iofs | ( line_bufa[i]         & 0xf)  ] << dotc;
                color2 |= dgr_dot_pattern[iofs | ((line_bufa[i] >> 4) /*& 0xf*/)] << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while((dotc >= 8) || ((i == 40) && (dotc > 0)))
            {
                color1 &= 0xfffffffe;
                color1 |= (color1 >> 4) & 1;
                dhgr_index = color1 & 0xf; // index for first pixel

                color1 &= 0xfffffffc;
                color1 |= (color1 >> 4) & 3;
                dhgr_index |= (color1 & 0xf)<<4;   // index for second pixel

                // add 2 pixels
                *(tmdsbuf1_red++)   = tmds_dhgr_red  [dhgr_index];
                *(tmdsbuf1_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf1_blue++)  = tmds_dhgr_blue [dhgr_index];

                color2 &= 0xfffffffe;
                color2 |= (color2 >> 4) & 1;
                dhgr_index = color2 & 0xf;         // index for first pixel
                color2 &= 0xfffffffc;
                color2 |= (color2 >> 4) & 3;
                dhgr_index |= (color2 & 0xf)<<4;   // index for second pixel

                // add 2 pixels
                *(tmdsbuf2_red++)   = tmds_dhgr_red  [dhgr_index];
                *(tmdsbuf2_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf2_blue++)  = tmds_dhgr_blue [dhgr_index];

                color1 &= 0xfffffff8;
                color1 |= (color1 >> 4) & 7;
                dhgr_index = color1 & 0xf;         // index for first pixel
                color1 >>= 4;
                dhgr_index |= (color1 & 0xf)<<4;   // index for second pixel

                // add 2 pixels
                *(tmdsbuf1_red++)   = tmds_dhgr_red  [dhgr_index];
                *(tmdsbuf1_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf1_blue++)  = tmds_dhgr_blue [dhgr_index];

                color2 &= 0xfffffff8;
                color2 |= (color2 >> 4) & 7;
                dhgr_index = color2 & 0xff;        // index for first+second pixel
                color2 >>= 4;

                // add 2 pixels
                *(tmdsbuf2_red++)   = tmds_dhgr_red  [dhgr_index];
                *(tmdsbuf2_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf2_blue++)  = tmds_dhgr_blue [dhgr_index];

                dotc -= 4;
            }
        }
    }
    else
    {
        // plain DGR rendering - no artifacts
        for(int i = 0; i < 40; i++)
        {
            // First pixel data is from aux memory
            // Colors in even columns/aux mem are rotated to the right by 1 bit, which is how our DHGR palette is organized -
            // so we do NOT need to rotate the nibbles.
            uint8_t color1_l1 = line_bufb[i]          & 0xf;
            uint8_t color1_l2 = (line_bufb[i] >> 4) /*& 0xf*/;

            // Next pixel data is from main memory
            // Colors in odd columns/main mem are in natural encoding/not rotated.
            // However, our DHGR palette expects rotated nibbles - so we have to use a mapping.
            uint8_t color2_l1 = dlores_dhgr_map[  line_bufa[i]         & 0xf] ;
            uint8_t color2_l2 = dlores_dhgr_map[ (line_bufa[i] >> 4) /*& 0xf*/];

            // line 1: 14 pixels
            {
                // add 3 double pixels to line 1
                uint8_t index = color1_l1 | (color1_l1<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue, index, 3);

                // add 1 double pixel to line 2
                index = color1_l1 | (color2_l1<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue, index, 1);

                // add 3 double pixels to line 1
                index = color2_l1 | (color2_l1<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf1_red, tmdsbuf1_green, tmdsbuf1_blue, index, 3);
            }

            // line2: 14 pixels
            {
                // add 3 double pixels to line 2
                uint8_t index = color1_l2 | (color1_l2<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue, index, 3);

                // add 1 double pixel to line 2
                index = color1_l2 | (color2_l2<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue, index, 1);

                // add 3 double pixels to line 2
                index = color2_l2 | (color2_l2<<4);
                ADD_TMDS_DGR_PIXELS(tmdsbuf2_red, tmdsbuf2_green, tmdsbuf2_blue, index, 3);
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

void DELAYED_COPY_CODE(render_dgr)()
{
    for(uint line=0; line < 24; line++)
    {
        render_dgr_line(PAGE2SEL, line);
    }
}

void DELAYED_COPY_CODE(render_mixed_dgr)()
{
    for(uint line=0; line < 20; line++)
    {
        render_dgr_line(PAGE2SEL, line);
    }

    render_mixed_text();
}
