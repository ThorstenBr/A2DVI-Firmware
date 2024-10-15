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
#include "util/dmacopy.h"

// TMDS symbols for LORES RGB colors - using the "double pixel" trick
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
uint32_t __attribute__((section (".appledata."))) tmds_lorescolor[3*16];

// default: initial A2DVI color palette...
// gray1 != gray2
const uint32_t __in_flash("chr_rom") tmds_lores_default[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0x886de, 0x7e107, 0xa2277, // magenta
    0x465e6, 0x465e6, 0x826f6, // darkblue
    0x85ee8, 0x44dec, 0xbfe00, // purple
    0x7fd00, 0x5fd80, 0x7fd00, // darkgreen
    0x5fd80, 0x5fd80, 0x5fd80, // gray1 (dark gray)
    0x411fb, 0x5819f, 0xbfe00, // mediumblue
    0xb3233, 0xb3233, 0xbfe00, // lightblue
    0x73133, 0x73133, 0x7fd00, // brown
    0x812fb, 0x9829f, 0x7fd00, // orange
    0x6fd40, 0x6fd40, 0x6fd40, // gray2 (light gray)
    0xbfe00, 0x5e187, 0x826f6, // pink
    0x45de8, 0x84eec, 0x7fd00, // green
    0x866e6, 0x866e6, 0x425f6, // yellow
    0x485de, 0xbe207, 0x62177, // aqua
    0xbfe00, 0xbfe00, 0xbfe00, // white
};

// Apple IIe original
// based on a color analysis by Linards Ticmanis (TeaRex)
// gray1==gray2
const uint32_t __in_flash("chr_rom") tmds_lores_original[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0xb7e20, 0x7d909, 0x71d38, // magenta
    0x71d38, 0x445ee, 0xbce0c, // darkblue
    0x862e7, 0x4f9c1, 0xbfe00, // purple
    0x421f7, 0x9e686, 0x70d3c, // darkgreen
    0x6017f, 0x6017f, 0x6017f, // gray1 (dark gray)
    0xae247, 0x5c58e, 0xbfe00, // mediumblue
    0xb0a3d, 0x882df, 0xbfe00, // lightblue
    0x4e1c7, 0x77123, 0x7fd00, // brown
    0xb8a1d, 0x9ee84, 0x7fd00, // orange
    0x6017f, 0x6017f, 0x6017f, // gray2 (light gray)
    0x80efc, 0x8bed0, 0x8fac1, // pink
    0xafa41, 0xbba11, 0x7fd00, // green
    0x8fac1, 0x862e7, 0x7fd00, // yellow
    0xa3a71, 0x822f7, 0xb0a3d, // aqua
    0x816fa, 0x816fa, 0x816fa, // white
};

// improved: Apple IIe colors of an Apple IIgs in IIe emulation mode
// based on a color analysis by Linards Ticmanis (TeaRex)
// gray1!=gray2
const uint32_t __in_flash("chr_rom") tmds_lores_improved[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0xb8e1c, 0x97ea0, 0x7053e, // magenta
    0x7ed04, 0x7c10f, 0x58d9c, // darkblue
    0x87ae1, 0x4f9c1, 0x6815f, // purple
    0x42df4, 0x6057e, 0x441ef, // darkgreen
    0x77921, 0x77921, 0x77921, // gray1 (dark gray)
    0x7a117, 0x906be, 0xbfe00, // mediumblue
    0x9f283, 0x8eec4, 0xbfe00, // lightblue
    0x5c18f, 0x48ddc, 0x7fd00, // brown
    0xbf203, 0xa1e78, 0x7fd00, // orange
    0x8eec4, 0x8eec4, 0x8eec4, // gray2 (light gray)
    0x80efc, 0x67163, 0x63971, // pink
    0x4fdc0, 0xb821f, 0x7fd00, // green
    0x802ff, 0x802ff, 0x7fd00, // yellow
    0x9de88, 0xbfa01, 0x67961, // aqua
    0xbfe00, 0xbfe00, 0xbfe00, // white
};

void DELAYED_COPY_CODE(tmds_color_load_lores)(uint color_style)
{
    const uint32_t* pSource = tmds_lores_default;

    switch(color_style)
    {
        case 1:
            pSource = tmds_lores_original;
            break;
        case 2:
            pSource = tmds_lores_improved;
            break;
        case 0: // fall-through
        default:
            break;
    }
    memcpy32(tmds_lorescolor, pSource, sizeof(tmds_lorescolor));
}
