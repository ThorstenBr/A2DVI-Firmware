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

#include "config/config.h"

#ifdef FEATURE_TEST
extern bool PrintMode80Column; // for 40/80 column printing (only in test mode)
extern bool PrintModePage2;    // for page1/2 printing (only in test mode)
#endif

typedef enum {
    PRINTMODE_NORMAL  = 0,
    PRINTMODE_INVERSE = 1,
    PRINTMODE_FLASH   = 2,
    PRINTMODE_RAW     = 3
} TPrintMode;

extern bool        IgnoreNextKeypress;
extern const char* TitleGitHub[2];
extern const char* MenuColorMode[COLOR_MODE_AMBER+1];

void printXY(uint32_t x, uint32_t line, const char* pMsg, TPrintMode PrintMode);
void centerY(uint32_t y, const char* pMsg, TPrintMode PrintMode);
void int2str(uint32_t value, char* pStrBuf, uint32_t digits);
void clearTextScreen(void);
void showTitle(TPrintMode PrintMode);
void menuShow(char key);
void menuShowSaved(void);

#ifdef FEATURE_TEST
void menuShowDebug();
#endif
