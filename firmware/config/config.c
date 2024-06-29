#include <string.h>
#include "config.h"
#include "applebus/buffers.h"

volatile compat_t cfg_machine = MACHINE_AUTO;
volatile compat_t current_machine = MACHINE_AUTO;
volatile bool language_switch = false;
volatile uint8_t color_mode = 1;
