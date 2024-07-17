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

#pragma once

#include "pico/stdlib.h"

extern struct dvi_inst dvi0;

#define DVI_WORDS_PER_CHANNEL (640/2)
#define DVI_APPLE2_XOFS       ((640/2-560/2)/2)

#define dvi_get_scanline(tmdsbuf)  \
    uint32_t* tmdsbuf;\
    queue_remove_blocking_u32(&dvi0.q_tmds_free, &tmdsbuf);

#define dvi_scanline_rgb(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue) \
        uint32_t *tmdsbuf_blue  = tmdsbuf+DVI_APPLE2_XOFS; \
        uint32_t* tmdsbuf_green = tmdsbuf_blue  + DVI_WORDS_PER_CHANNEL; \
        uint32_t* tmdsbuf_red   = tmdsbuf_green + DVI_WORDS_PER_CHANNEL;

#define dvi_copy_scanline(destbuf, srcbuf) \
    for (uint32_t i=0;i<DVI_WORDS_PER_CHANNEL-DVI_APPLE2_XOFS;i++) \
    { \
        destbuf[i                        ] = srcbuf[i                        ]; \
        destbuf[i+  DVI_WORDS_PER_CHANNEL] = srcbuf[i+  DVI_WORDS_PER_CHANNEL]; \
        destbuf[i+2*DVI_WORDS_PER_CHANNEL] = srcbuf[i+2*DVI_WORDS_PER_CHANNEL]; \
    }

#define dvi_send_scanline(tmdsbuf) \
    queue_add_blocking_u32(&dvi0.q_tmds_valid, &tmdsbuf);

// DVI TMDS encoding data (Transition-Minimized Differential Signaling)
// each TMDS symbol needs to cover two pixels (2x10bit) and the pair
// must be perfectly 'bit balanced' according to the TMDS.
// The perfect balance means we can simply copy and concatenate the
// TMDS symbols to the output stream. Any unbalanced pair would require
// the encoding of the following symbol to be adapted - which would
// require calculation overhead and could not be done 'on the fly' on a
// single CPU core. So we're restricted to "perfectly bit balanced"
// symbols to significantly simplify the production of a valid TMDS
// encoded bit stream.

#define TMDS_SYMBOL_0_0     0x7fd00 // actually 00/01
#define TMDS_SYMBOL_255_0   0x402ff // actually FE/00
#define TMDS_SYMBOL_0_255   0xbfd00 // actually 00/FE
#define TMDS_SYMBOL_255_255 0xbfe00 // actually FF/FE
//#define TMDS_SYMBOL_255_0   0x7fe00 // actually 255/1
//#define TMDS_SYMBOL_0_255   0x801ff // actually 1/255

#define TMDS_SYMBOL_128_0   0x7f980
#define TMDS_SYMBOL_0_128   0xdfd00
#define TMDS_SYMBOL_128_128 0x5fd80

// TMDS data for a duplicated monochrome pixel (a "bit balanced" double pixel).
extern uint32_t tmds_mono_double_pixel[4*4];

// TMDS data for two separate monochrome pixels (a "bit balanced" pixel pair).
extern uint32_t tmds_mono_pixel_pair[4*3*3];

// TMDS data for a duplicated color pixel ("bit balanced" double pixels).
// 16 entries, matching the LORES color palette
extern uint32_t tmds_lorescolor[3*16];

extern uint32_t tmds_hires_color_patterns_red[2*256];
extern uint32_t tmds_hires_color_patterns_green[2*256];
extern uint32_t tmds_hires_color_patterns_blue[2*256];
