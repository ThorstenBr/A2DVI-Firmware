/*
MIT License

Copyright (c) 2021 Mark Aikens
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

#include "textfont.h"
#include "config/config.h"

const uint8_t* DELAYED_COPY_DATA(character_roms)[32] =
{
    textfont_iie_us_enhanced,         //  0
    textfont_iie_uk_enhanced,         //  1
    textfont_iie_fr_ca_enhanced,      //  2
    textfont_iie_de_enhanced,         //  3
    textfont_iie_spanish_enhanced,    //  4
    textfont_iie_it_enhanced,         //  5
    textfont_iie_se_fi_enhanced,      //  6
    textfont_iie_hebrew_enhanced,     //  7
    textfont_clone_pravetz_cyrillic,  //  8
    textfont_iie_us_reactive,         //  9
    textfont_iie_us_unenhanced,       // 10
    textfont_iiplus_us,               // 11
    textfont_iiplus_videx_lowercase1, // 12
    textfont_iiplus_videx_lowercase2, // 13
    textfont_iiplus_pigfont,          // 14
    textfont_iiplus_jp_katakana,      // 15

    CUSTOM_FONT_ROM(0),
    CUSTOM_FONT_ROM(1),
    CUSTOM_FONT_ROM(2),
    CUSTOM_FONT_ROM(3),
    CUSTOM_FONT_ROM(4),
    CUSTOM_FONT_ROM(5),
    CUSTOM_FONT_ROM(6),
    CUSTOM_FONT_ROM(7),
    CUSTOM_FONT_ROM(8),
    CUSTOM_FONT_ROM(9),
    CUSTOM_FONT_ROM(10),
    CUSTOM_FONT_ROM(11),
    CUSTOM_FONT_ROM(12),
    CUSTOM_FONT_ROM(13),
    CUSTOM_FONT_ROM(14),
    CUSTOM_FONT_ROM(15)
};

const uint8_t* DELAYED_COPY_DATA(character_roms_videx)[VIDEX_FONT_COUNT] =
{
    videx_normal,
    videx_uppercase,
    videx_german,
    videx_french,
    videx_spanish,
    videx_katakana,
    videx_apl,
    videx_super_sub,
    videx_epson,
    videx_symbol
};
