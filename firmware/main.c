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

#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/vreg.h"

#include "dvi/a2dvi.h"
#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "util/dmacopy.h"
#include "config/config.h"
#include "menu/menu.h"
#include "debug/debug.h"

#include "fonts/textfont.h"

#ifdef FEATURE_TEST
    #include "test/tests.h"
    #define FUNCTION_PROFILER
#endif

#include "debug/profiler.h"

#define VREG_VSEL         VREG_VOLTAGE_1_20

int main()
{
    // slightly rise the core voltage, preparation for overclocking
    vreg_set_voltage(VREG_VSEL);

    PROFILER_INIT(boot_time);

    // enable LED etc
    debug_init();

    // check hardware
    a2dvi_check_hardware();

    // load config settings
    config_load();

#ifdef FEATURE_TEST
    // start testsuite, simulating some 6502 activity and
    // cycle through the test cases
    multicore_launch_core1(test_loop);
#else
    // process the Apple II bus interface on core 1
    multicore_launch_core1(abus_loop);
#endif
    boot_time = to_us_since_boot(get_absolute_time());

    // Finish copying remaining data and code from flash to RAM
    memcpy32(__ram_delayed_copy_start__, __ram_delayed_copy_source__, ((uint32_t)__ram_delayed_copy_end__) - (uint32_t) __ram_delayed_copy_start__);

    // when testing: release flash, so we can access the BOOTSEL button
    debug_flash_release();

    // DVI processing on core 0
    a2dvi_loop();
}
