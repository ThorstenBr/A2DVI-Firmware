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
#include <stdbool.h>

// VIDEX was hardcoded to use slot 3.
// Only for internal testing, we can switch to slot 1.
#define VIDEX_SLOT 3
#define VIDEX_ROM_ADDR (0xC000 | (VIDEX_SLOT<<8))
#define VIDEX_REG_ADDR (0xC080 | (VIDEX_SLOT<<4))

extern uint8_t videx_vterm_mem_selected;
extern uint8_t videx_vram[2048];
extern uint_fast16_t videx_bankofs;  // selected videx memory bank offset
extern uint8_t videx_crtc_regs[18];

#define VIDEXFUNC extern

VIDEXFUNC void videx_reg_read  (uint_fast16_t address);
VIDEXFUNC void videx_reg_write (uint_fast16_t address, uint_fast8_t data);
VIDEXFUNC void videx_c8xx_read (uint_fast16_t address);
VIDEXFUNC void videx_c8xx_write(uint_fast16_t address, uint_fast8_t data);

#undef VIDEXFUNC
#define VIDEXFUNC
