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

#include "abus_pin_config.h"

void abus_init();
void abus_loop();

#define ACCESS_WRITE(value)    ((value & (1u << (CONFIG_PIN_APPLEBUS_RW     - CONFIG_PIN_APPLEBUS_DATA_BASE))) == 0)
#define ACCESS_READ(value)     ((value & (1u << (CONFIG_PIN_APPLEBUS_RW     - CONFIG_PIN_APPLEBUS_DATA_BASE))) != 0)
#define CARD_DEVSEL(value)     ((value & (1u << (CONFIG_PIN_APPLEBUS_DEVSEL - CONFIG_PIN_APPLEBUS_DATA_BASE))) == 0)
#define LANGUAGE_SWITCH(value) ((value & (1u << (CONFIG_PIN_LANGUAGE_SW     - CONFIG_PIN_APPLEBUS_DATA_BASE))) == 0)
