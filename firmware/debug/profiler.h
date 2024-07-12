/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Thorsten Brehm
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "hardware/structs/systick.h"

#ifdef FUNCTION_PROFILER

	// enable systick timer, use CPU clock source, keep timer exception disabled
	#define PROFILER_INIT(MaxTime) \
	{\
		systick_hw->csr = 0x5;\
		systick_hw->rvr = 0x00FFFFFF;\
		systick_hw->cvr = 0x00FFFFFF;\
		MaxTime         = 0x00FFFFFF;\
	}

	// start time measurement (count-down timer)
	#define PROFILER_START(){ systick_hw->cvr = 0x00FFFFFF;}

	// stop time measurement and calculate maximum
	// elapsed time is actually: 0x00FFFFFF-MaxTime
	#define PROFILER_STOP(Time) { \
		uint32_t t = systick_hw->cvr; \
		Time = 0x00FFFFFF-t;\
	}

#else
	#define PROFILER_INIT(x) {}
	#define PROFILER_START() {}
	#define PROFILER_STOP(x) {}
#endif
