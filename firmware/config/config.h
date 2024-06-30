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

#include <stdint.h>
#include <pico/stdlib.h>

typedef enum
{
    MACHINE_II      = 0,
    MACHINE_IIE     = 1,
    MACHINE_PRAVETZ = 2,
    MACHINE_AGAT7   = 3,
    MACHINE_BASIS   = 4,
    MACHINE_AGAT9   = 5,
    MACHINE_MAX_CFG = MACHINE_AGAT9, // valid maximum option for config, otherwise "AUTO" is assumed
    MACHINE_IIGS    = 6,             // currently not supported
    MACHINE_INVALID = 0xfe,
    MACHINE_AUTO    = 0xff
} compat_t;

extern volatile compat_t cfg_machine;
extern volatile compat_t current_machine;

extern uint8_t cfg_local_charset;
extern uint8_t cfg_alt_charset;

extern volatile bool language_switch_enabled;
extern volatile bool language_switch;

enum {
    COLOR_MODE_BW    = 0,
    COLOR_MODE_GREEN = 1,
    COLOR_MODE_AMBER = 2
} color_mode_t;

extern volatile uint8_t color_mode;

#if 0
  #define DELAYED_COPY_CODE(n) __noinline __attribute__((section(".delayed_code."))) n
#else
  #define DELAYED_COPY_CODE(n) __noinline __time_critical_func(n)
#endif

#if 0
  #define DELAYED_COPY_DATA(n) __attribute__((section(".delayed_data."))) n
#else
  #define DELAYED_COPY_DATA(n) n
#endif

#define IS_IFLAG(FLAGS)             ((internal_flags & FLAGS)==FLAGS)
#define SET_IFLAG(condition, FLAGS) { if (condition) internal_flags |= FLAGS;else internal_flags &= ~FLAGS; }

extern void config_load();
extern void config_load_defaults();
extern void config_save();
