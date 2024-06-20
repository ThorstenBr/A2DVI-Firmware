#include "buffers.h"

volatile uint32_t soft_switches = SOFTSW_TEXT_MODE;
volatile uint32_t internal_flags = IFLAGS_OLDCOLOR | IFLAGS_INTERP | IFLAGS_V7_MODE3;

volatile uint8_t reset_state = 0;
volatile uint8_t cardslot = 0;

volatile uint32_t busactive = 0;

uint8_t __attribute__((section (".appledata."))) apple_memory[MAX_ADDRESS];
uint8_t __attribute__((section (".appledata."))) private_memory[MAX_ADDRESS];

volatile uint8_t jumpers = 0;

volatile uint8_t *text_p1 = apple_memory + 0x0400;
volatile uint8_t *text_p2 = apple_memory + 0x0800;
volatile uint8_t *text_p3 = private_memory + 0x0400;
volatile uint8_t *text_p4 = private_memory + 0x0800;
volatile uint8_t *hgr_p1  = apple_memory + 0x2000;
volatile uint8_t *hgr_p2  = apple_memory + 0x4000;
volatile uint8_t *hgr_p3  = private_memory + 0x2000;
volatile uint8_t *hgr_p4  = private_memory + 0x4000;
