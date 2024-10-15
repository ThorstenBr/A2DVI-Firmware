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
#include "dvi.h"

extern struct dvi_inst dvi0;

#define DVI_X_RESOLUTION       640
#define DVI_WORDS_PER_CHANNEL (DVI_X_RESOLUTION/2)
#define DVI_APPLE2_XOFS_560   ((DVI_X_RESOLUTION/2-560/2)/2)

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

#define TMDS_SYMBOL_128_0   0x7f980
#define TMDS_SYMBOL_0_128   0xdfd00
#define TMDS_SYMBOL_128_128 0x5fd80

#define dvi_get_scanline(tmdsbuf)  \
    uint32_t* tmdsbuf;\
    queue_remove_blocking_u32(&dvi0.q_tmds_free, &tmdsbuf);

// get scanline rgb pointers for 640pixel/line rendering
#define dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue) \
        uint32_t *tmdsbuf_blue  = tmdsbuf; \
        uint32_t* tmdsbuf_green = tmdsbuf_blue  + DVI_WORDS_PER_CHANNEL; \
        uint32_t* tmdsbuf_red   = tmdsbuf_green + DVI_WORDS_PER_CHANNEL;

// get scanline rgb pointers for 560pixel/line rendering: this automatically fills the left/right border (40 pixels each)
#define dvi_scanline_rgb560(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue) \
        dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue); \
        for (uint32_t i=0;i<DVI_APPLE2_XOFS_560;i++) \
        {\
            *(tmdsbuf_red+(600/2))   = TMDS_SYMBOL_0_0; \
            *(tmdsbuf_green+(600/2)) = TMDS_SYMBOL_0_0; \
            *(tmdsbuf_blue+(600/2))  = TMDS_SYMBOL_0_0; \
            *(tmdsbuf_red++)   = TMDS_SYMBOL_0_0; \
            *(tmdsbuf_green++) = TMDS_SYMBOL_0_0; \
            *(tmdsbuf_blue++)  = TMDS_SYMBOL_0_0; \
        }

#define dvi_copy_scanline(destbuf, srcbuf) \
    for (uint32_t i=0;i<DVI_WORDS_PER_CHANNEL;i++) \
    { \
        destbuf[i                        ] = srcbuf[i                        ]; \
        destbuf[i+  DVI_WORDS_PER_CHANNEL] = srcbuf[i+  DVI_WORDS_PER_CHANNEL]; \
        destbuf[i+2*DVI_WORDS_PER_CHANNEL] = srcbuf[i+2*DVI_WORDS_PER_CHANNEL]; \
    }

#define dvi_send_scanline(tmdsbuf) \
    queue_add_blocking_u32(&dvi0.q_tmds_valid, &tmdsbuf);

// TMDS data for a duplicated monochrome pixel (a "bit balanced" double pixel).
extern uint32_t tmds_mono_double_pixel[3*5];

// TMDS data for two separate monochrome pixels (a "bit balanced" pixel pair).
extern uint32_t tmds_mono_pixel_pair[4*3*3];

// TMDS data for a duplicated color pixel ("bit balanced" double pixels).
// 16 entries, matching the LORES color palette
extern uint32_t tmds_lorescolor[3*16];

extern uint32_t tmds_hires_color_patterns_red[2*256];
extern uint32_t tmds_hires_color_patterns_green[2*256];
extern uint32_t tmds_hires_color_patterns_blue[2*256];

extern uint32_t tmds_dhgr_red[16*16];
extern uint32_t tmds_dhgr_green[16*16];
extern uint32_t tmds_dhgr_blue[16*16];

extern void tmds_color_load(void);
extern void tmds_color_load_lores(uint color_style);
extern void tmds_color_load_dhgr(uint color_style);
