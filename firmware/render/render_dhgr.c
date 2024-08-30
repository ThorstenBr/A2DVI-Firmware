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

// map DHGR values to the LORES palette (also multiply by 3, as we need an index to the RGB TMDS table, with 3 values per color)
uint8_t DELAYED_COPY_DATA(tmds_dhgr_lores_mapping)[16] =
{
    0*3 /*0:BLACK*/,    2*3 /*2:DGREEN*/,  4*3 /*4:BROWN*/,     6*3 /*6:HGREEN*/,
    8*3 /*8:MAGENTA*/, 10*3 /*10:DGRAY*/, 12*3 /*12:HORANGE*/, 14*3 /*14:YELLOW*/,
    1*3 /*1:DBLUE*/,    3*3 /*3:HBLUE*/,   5*3 /*5:LGRAY*/,     7*3 /*7:AQUA*/,
    9*3 /*9:HVIOLET*/, 11*3 /*11:LBLUE*/, 13*3 /*13:PINK*/,    15*3 /*15:WHITE*/
};

#define PAGE2SEL ((soft_switches & (SOFTSW_80STORE | SOFTSW_PAGE_2)) == SOFTSW_PAGE_2)


static inline uint dhgr_line_to_mem_offset(uint line)
{
    return ((line & 0x07) << 10) | ((line & 0x38) << 4) | (((line & 0xc0) >> 6) * 40);
}

static void DELAYED_COPY_CODE(render_dhgr_line)(bool p2, uint line, bool mono)
{
     // Construct scanline
    dvi_get_scanline(tmdsbuf);
    dvi_scanline_rgb(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

    const uint8_t *line_mema = (const uint8_t *)((p2 ? hgr_p2 : hgr_p1) + dhgr_line_to_mem_offset(line));
    const uint8_t *line_memb = (const uint8_t *)((p2 ? hgr_p4 : hgr_p3) + dhgr_line_to_mem_offset(line));

    // DHGR is weird. Video-7 just makes it weirder. Nuff said.
    uint32_t dots = 0;
    uint_fast8_t dotc = 0;
    uint i = 0;

    if(mono)
    {
        uint8_t color_offset = color_mode*12;
        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc < 28) && (i < 40))
            {
                dots |= (line_memb[i] & 0x7f) << dotc;
                dotc += 7;
                dots |= (line_mema[i] & 0x7f) << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while(dotc)
            {
                uint32_t offset = color_offset + (dots & 0x3);
                *(tmdsbuf_red++)   = tmds_mono_pixel_pair[offset+0];
                *(tmdsbuf_green++) = tmds_mono_pixel_pair[offset+4];
                *(tmdsbuf_blue++)  = tmds_mono_pixel_pair[offset+8];
                dots >>= 2;

                dotc -= 2;
            }
        }
    }
#if 0
    else
    if((internal_flags & IFLAGS_VIDEO7) && ((soft_switches & (SOFTSW_80STORE | SOFTSW_80COL)) == (SOFTSW_80STORE)))
    {
        int j;

        // Video 7 F/B HiRes
        while(i < 40)
        {
            dots = (line_mema[i] & 0x7f);
            color1 = lores_palette[(line_memb[i] >> 4) & 0xF];
            color2 = lores_palette[(line_memb[i] >> 0) & 0xF];
            i++;

            dots |= (line_mema[i] & 0x7f) << 7;
            color3 = lores_palette[(line_memb[i] >> 4) & 0xF];
            color4 = lores_palette[(line_memb[i] >> 0) & 0xF];
            i++;

            for(j = 0; j < 3; j++)
            {
                pixeldata = ((dots & 1) ? (color1) : (color2)) | THEN_EXTEND_1;
                dots >>= 1;
                pixeldata |= (((dots & 1) ? (color1) : (color2)) | THEN_EXTEND_1) << 16;
                dots >>= 1;
                sl->data[sl_pos++] = pixeldata;
            }

            pixeldata = ((dots & 1) ? (color1) : (color2)) | THEN_EXTEND_1;
            dots >>= 1;
            pixeldata |= (((dots & 1) ? (color3) : (color4)) | THEN_EXTEND_1) << 16;
            dots >>= 1;
            sl->data[sl_pos++] = pixeldata;

            for(j = 0; j < 3; j++)
            {
                pixeldata = ((dots & 1) ? (color3) : (color4)) | THEN_EXTEND_1;
                dots >>= 1;
                pixeldata |= (((dots & 1) ? (color3) : (color4)) | THEN_EXTEND_1) << 16;
                dots >>= 1;
                sl->data[sl_pos++] = pixeldata;
            }
        }
    }
    else
    if((internal_flags & IFLAGS_VIDEO7) && ((internal_flags & IFLAGS_V7_MODE3) == IFLAGS_V7_MODE2))
    {
        // 160x192 Video-7
        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc <= 18) && (i < 40))
            {
                dots |= (line_memb[i] & 0xff) << dotc;
                dotc += 8;
                dots |= (line_mema[i] & 0xff) << dotc;
                dotc += 8;
                i++;
            }

            // Consume pixels
            while(dotc >= 8)
            {
                pixeldata = (lores_palette[dots & 0xf] | THEN_EXTEND_3);
                dots >>= 4;
                pixeldata |= (lores_palette[dots & 0xf] | THEN_EXTEND_3) << 16;
                dots >>= 4;
                sl->data[sl_pos++] = pixeldata;
                dotc -= 8;
            }
        }
    }
    else
    if((internal_flags & (IFLAGS_VIDEO7 | IFLAGS_V7_MODE3)) == (IFLAGS_VIDEO7 | IFLAGS_V7_MODE1))
    {
        // Video-7 Mixed B&W/RGB
        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc <= 18) && (i < 40)) {
                dots |= (line_memb[i] & 0x7f) << dotc;
                pixelmode |= ((line_memb[i] & 0x80) ? 0x7f : 0x00) << dotc;
                dotc += 7;
                dots |= (line_mema[i] & 0x7f) << dotc;
                pixelmode |= ((line_mema[i] & 0x80) ? 0x7f : 0x00) << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while(dotc >= 4) {
                if(pixelmode) {
                    pixeldata = (dhgr_palette[dots & 0xf] | THEN_EXTEND_1);
                    pixeldata |= pixeldata << 16;
                    dots >>= 4;
                    pixelmode >>= 4;
                    sl->data[sl_pos++] = pixeldata;
                    dotc -= 4;
                } else {
                    pixeldata = ((dots & 1) ? (text_fore) : (text_back));
                    dots >>= 1;
                    pixelmode >>= 1;
                    pixeldata |= (((dots & 1) ? (text_fore) : (text_back))) << 16;
                    dots >>= 1;
                    pixelmode >>= 1;
                    sl->data[sl_pos++] = pixeldata;
                    dotc -= 2;
                    pixeldata = ((dots & 1) ? (text_fore) : (text_back));
                    dots >>= 1;
                    pixelmode >>= 1;
                    pixeldata |= (((dots & 1) ? (text_fore) : (text_back))) << 16;
                    dots >>= 1;
                    pixelmode >>= 1;
                    sl->data[sl_pos++] = pixeldata;
                    dotc -= 2;
                }
            }
        }
    }
    else
    if((internal_flags & (IFLAGS_INTERP | IFLAGS_GRILL)) == (IFLAGS_INTERP | IFLAGS_GRILL))
    {
        // Preload black into the sliding window
        dots = 0;
        dotc = 4;

        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc <= 18) && (i < 40))
            {
                dots |= (line_memb[i] & 0x7f) << dotc;
                dotc += 7;
                dots |= (line_mema[i] & 0x7f) << dotc;
                dotc += 7;
                i++;
            }

            while((dotc >= 8) || ((dotc > 0) && (i == 40)))
            {
                dots &= 0xfffffffe;
                dots |= (dots >> 4) & 1;
                pixeldata = half_palette[dots & 0xf];
                dots &= 0xfffffffc;
                dots |= (dots >> 4) & 3;
                pixeldata |= dhgr_palette[dots & 0xf] << 16;
                sl->data[sl_pos++] = pixeldata;

                dots &= 0xfffffff8;
                dots |= (dots >> 4) & 7;
                pixeldata = half_palette[dots & 0xf];
                dots >>= 4;
                pixeldata |= dhgr_palette[dots & 0xf] << 16;
                sl->data[sl_pos++] = pixeldata;

                dotc -= 4;
            }
        }
    }
#endif
    else
    if(internal_flags & IFLAGS_INTERP_DHGR)
    {
        // Preload black into the sliding window
        dots = 0;
        dotc = 4;

        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc <= 18) && (i < 40))
            {
                dots |= (line_memb[i] & 0x7f) << dotc;
                dotc += 7;
                dots |= (line_mema[i] & 0x7f) << dotc;
                dotc += 7;
                i++;
            }

            while((dotc >= 8) || ((dotc > 0) && (i == 40)))
            {
                dots &= 0xfffffffe;
                dots |= (dots >> 4) & 1;
                uint8_t dhgr_index = dots & 0xf; // index for first pixel
                dots &= 0xfffffffc;
                dots |= (dots >> 4) & 3;
                dhgr_index |= (dots & 0xf)<<4;   // index for second pixel

                // add 2 pixels
                *(tmdsbuf_red++)   = tmds_dhgr_red[dhgr_index];
                *(tmdsbuf_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf_blue++)  = tmds_dhgr_blue[dhgr_index];

                dots &= 0xfffffff8;
                dots |= (dots >> 4) & 7;
                dhgr_index = dots & 0xf;         // index for third pixel
                dots >>= 4;
                dhgr_index |= (dots & 0xf)<<4;   // index for fourth pixel

                // add 2 pixels
                *(tmdsbuf_red++)   = tmds_dhgr_red[dhgr_index];
                *(tmdsbuf_green++) = tmds_dhgr_green[dhgr_index];
                *(tmdsbuf_blue++)  = tmds_dhgr_blue[dhgr_index];

                dotc -= 4;
            }
        }
    }
    else
    {
        while(i < 40)
        {
            // Load in as many subpixels as possible
            while((dotc <= 18) && (i < 40))
            {
                dots |= (line_memb[i] & 0x7f) << dotc;
                dotc += 7;
                dots |= (line_mema[i] & 0x7f) << dotc;
                dotc += 7;
                i++;
            }

            // Consume pixels
            while(dotc >= 8)
            {
                // map HGR dot values to 16 color (RGB LORES) palette
                uint32_t* pTmds = &tmds_lorescolor[tmds_dhgr_lores_mapping[dots&0xf]];
                uint32_t r = pTmds[0];
                uint32_t g = pTmds[1];
                uint32_t b = pTmds[2];

                // add 4 pixels (two double pixels)
                *(tmdsbuf_red++)   = r;
                *(tmdsbuf_red++)   = r;

                *(tmdsbuf_green++) = g;
                *(tmdsbuf_green++) = g;

                *(tmdsbuf_blue++)  = b;
                *(tmdsbuf_blue++)  = b;
                dots >>= 4;

                // map HGR dot values to 16 color (LORES) palette
                pTmds = &tmds_lorescolor[tmds_dhgr_lores_mapping[dots&0xf]];
                dots >>= 4;

                r = pTmds[0];
                g = pTmds[1];
                b = pTmds[2];

                // add 4 pixels (two double pixels)
                *(tmdsbuf_red++)   = r;
                *(tmdsbuf_red++)   = r;

                *(tmdsbuf_green++) = g;
                *(tmdsbuf_green++) = g;

                *(tmdsbuf_blue++)  = b;
                *(tmdsbuf_blue++)  = b;

                dotc -= 8;
            }
        }
    }

    // send buffer
    dvi_send_scanline(tmdsbuf);
}

void DELAYED_COPY_CODE(render_dhgr)()
{
    bool mono = mono_rendering;
    if((internal_flags & IFLAGS_VIDEO7) && ((internal_flags & IFLAGS_V7_MODE3) == IFLAGS_V7_MODE0)) {
        mono = true;
    }
    for(uint line=0; line < 192; line++) {
        render_dhgr_line(PAGE2SEL, line, mono);
    }
}

void DELAYED_COPY_CODE(render_mixed_dhgr)()
{
    bool mono = mono_rendering;
    if((internal_flags & IFLAGS_VIDEO7) && ((internal_flags & IFLAGS_V7_MODE3) == IFLAGS_V7_MODE0)) {
        mono = true;
    }
    for(uint line=0; line < 160; line++) {
        render_dhgr_line(PAGE2SEL, line, mono);
    }

    render_mixed_text();
}
