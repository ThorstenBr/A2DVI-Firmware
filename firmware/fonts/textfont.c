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

#define DEFAULT_ALT_CHARACTER_ROM textfont_iie_us_enhanced

const uint8_t* character_roms[32] =
{
    textfont_iie_us_enhanced,
    textfont_iie_uk_enhanced,
    textfont_iie_fr_ca_enhanced,
    textfont_iie_de_enhanced,
    textfont_iie_spanish_enhanced,
    textfont_iie_it_enhanced,
    textfont_iie_se_fi_enhanced,
    textfont_iie_hebrew_enhanced,
    textfont_clone_pravetz_cyrillic,
    textfont_iie_us_reactive,
    textfont_iie_us_unenhanced,
    textfont_iiplus_us,
    textfont_iiplus_videx_lowercase1,
    textfont_iiplus_videx_lowercase2,
    textfont_iiplus_pigfont,
    textfont_iiplus_jp_katakana,

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
