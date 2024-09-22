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

#pragma once

#include <stdint.h>
#include "applebus/buffers.h"

#define MAX_FONT_COUNT    32
#define CUSTOM_FONT_COUNT 16   /* custom fonts are part of MAX_FONT_COUNT */

#define DEFAULT_LOCAL_CHARSET       0
#define DEFAULT_ALT_CHARSET         0

// for automatic font selection depending on machine type
#define FONT_MIN_IIE      0
#define FONT_MAX_IIE     10
#define FONT_MIN_II      11
#define FONT_MAX_II      15
#define FONT_PRAVETZ      8
#define FONT_MIN_CUSTOM  16

extern uint8_t __font_roms_start[];

#define CUSTOM_FONT_ROM(i) (&__font_roms_start[(i)*CHARACTER_ROM_SIZE])

extern const uint8_t* character_roms[MAX_FONT_COUNT];
#define VIDEX_FONT_COUNT 10
extern const uint8_t* character_roms_videx[VIDEX_FONT_COUNT];

extern const uint8_t textfont_iie_us_enhanced[256 * 8];
extern const uint8_t textfont_iie_us_unenhanced[256 * 8];
extern const uint8_t textfont_iie_us_reactive[256 * 8];
extern const uint8_t textfont_iie_uk_enhanced[256 * 8];
extern const uint8_t textfont_iie_fr_ca_enhanced[256 * 8];
extern const uint8_t textfont_iie_de_enhanced[256 * 8];
extern const uint8_t textfont_iie_spanish_enhanced[256 * 8];
extern const uint8_t textfont_iie_it_enhanced[256 * 8];
extern const uint8_t textfont_iie_hebrew_enhanced[256 * 8];
extern const uint8_t textfont_iie_se_fi_enhanced[256 * 8];

extern const uint8_t textfont_iiplus_us[256 * 8];
extern const uint8_t textfont_iiplus_videx_lowercase1[256 * 8];
extern const uint8_t textfont_iiplus_videx_lowercase2[256 * 8];
extern const uint8_t textfont_iiplus_pigfont[256 * 8];
extern const uint8_t textfont_iiplus_jp_katakana[256 * 8];

extern const uint8_t textfont_clone_pravetz_cyrillic[256 * 8];

/////////////////////////////////////////////////

extern const uint8_t videx_apl[128 * 16];
extern const uint8_t videx_french[128 * 16];
extern const uint8_t videx_inverse[128 * 16];
extern const uint8_t videx_normal[128 * 16];
extern const uint8_t videx_super_sub[128 * 16];
extern const uint8_t videx_uppercase[128 * 16];
extern const uint8_t videx_epson[128 * 16];
extern const uint8_t videx_german[128 * 16];
extern const uint8_t videx_katakana[128 * 16];
extern const uint8_t videx_spanish[128 * 16];
extern const uint8_t videx_symbol[128 * 16];
