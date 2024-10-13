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

#pragma once

#include "dvi/tmds.h"

extern uint32_t show_subtitle_cycles;
extern bool mono_rendering;

extern bool color_support;  // flag indicating whether current display mode supports color

extern void render_init();
extern void render_splash();
extern void render_loop();

extern void update_text_flasher();
extern void render_text();
extern void render_mixed_text();
extern void render_text40_line(const uint8_t *page, unsigned int line, uint8_t color_mode);
extern void render_color_text40_line(unsigned int line);

extern void render_lores();
extern void render_mixed_lores();

extern void render_hires();
extern void render_mixed_hires();

extern void render_dhgr();
extern void render_mixed_dhgr();

extern void render_dgr();
extern void render_mixed_dgr();

extern void render_videx_text();

extern void render_debug(bool IsVidexMode, bool top);
extern void copy_str(uint8_t* dest, const char* pMsg);
extern void int2hex(uint8_t* pStrBuf, uint32_t value, uint32_t digits);

//#define FEATURE_TEST_TMDS

#ifdef FEATURE_TEST_TMDS
extern void render_tmds_test();
#endif
