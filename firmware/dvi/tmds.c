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

#include "tmds.h"
#include "config/config.h"

// TMDS data for RGB channels for a double pixel (a perfectly bit balanced pixel)
uint32_t DELAYED_COPY_DATA(tmds_mono_double_pixel)[3*5] =
{
    /* R                 G                    B               */
    TMDS_SYMBOL_255_255, TMDS_SYMBOL_255_255, TMDS_SYMBOL_255_255, /* white */
    TMDS_SYMBOL_0_0,     TMDS_SYMBOL_255_255, TMDS_SYMBOL_0_0,     /* green */
    TMDS_SYMBOL_255_255, TMDS_SYMBOL_128_128, TMDS_SYMBOL_0_0,     /* amber */

    TMDS_SYMBOL_0_0,     TMDS_SYMBOL_0_0,     TMDS_SYMBOL_0_0,     /* black */
    TMDS_SYMBOL_255_255, TMDS_SYMBOL_0_0,     TMDS_SYMBOL_0_0      /* red */
};

// TMDS data for RGB channels for a pattern of two pixels (a perfectly bit balanced pixel pair)
uint32_t DELAYED_COPY_DATA(tmds_mono_pixel_pair)[4*3*3] =
{
    // white
    /*R*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_0, TMDS_SYMBOL_0_255, TMDS_SYMBOL_255_255,
    /*G*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_0, TMDS_SYMBOL_0_255, TMDS_SYMBOL_255_255,
    /*B*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_0, TMDS_SYMBOL_0_255, TMDS_SYMBOL_255_255,

    // green
    /*R*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0,
    /*G*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_0, TMDS_SYMBOL_0_255, TMDS_SYMBOL_255_255,
    /*B*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0,

    // amber
    /*R*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_255_0, TMDS_SYMBOL_0_255, TMDS_SYMBOL_255_255,
    /*G*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_128_0, TMDS_SYMBOL_0_128, TMDS_SYMBOL_128_128,
    /*B*/ TMDS_SYMBOL_0_0, TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0,   TMDS_SYMBOL_0_0
};

void DELAYED_COPY_CODE(tmds_color_load)(void)
{
    reload_colors = false;
    tmds_color_load_lores(cfg_color_style);
    tmds_color_load_dhgr(cfg_color_style);
}
