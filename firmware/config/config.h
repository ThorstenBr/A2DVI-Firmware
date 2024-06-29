#pragma once

#include <stdint.h>
#include <pico/stdlib.h>

typedef enum {
    MACHINE_II = 0,
    MACHINE_IIE = 1,
    MACHINE_IIGS = 2,
    MACHINE_PRAVETZ = 6,
    MACHINE_AGAT7 = 7,
    MACHINE_BASIS = 8,
    MACHINE_AGAT9 = 9,
    MACHINE_INVALID = 0xfe,
    MACHINE_AUTO = 0xff
} compat_t;

extern volatile compat_t cfg_machine;
extern volatile compat_t current_machine;
extern volatile bool language_switch;
extern volatile uint8_t color_mode;

void dmacopy32(uint32_t *start, uint32_t *end, uint32_t *source);

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
