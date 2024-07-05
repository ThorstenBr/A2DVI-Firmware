#include "buffers.h"

volatile uint32_t soft_switches = SOFTSW_TEXT_MODE;
volatile uint32_t internal_flags = IFLAGS_V7_MODE3;

volatile uint8_t  cardslot;
// Set SlotROM area to invalid address, so decoder does not trigger before the actual cardslot is determined.
volatile uint16_t card_rom_address = 0x00ff;

volatile uint8_t reset_state = 0;

uint8_t __attribute__((section (".appledata."))) apple_memory[MAX_ADDRESS];
uint8_t __attribute__((section (".appledata."))) private_memory[MAX_ADDRESS];
uint8_t __attribute__((section (".appledata."))) slot_memory[256];

volatile uint8_t *text_p1 = apple_memory   + 0x0400;
volatile uint8_t *text_p2 = apple_memory   + 0x0800;
volatile uint8_t *text_p3 = private_memory + 0x0400;
volatile uint8_t *text_p4 = private_memory + 0x0800;
volatile uint8_t *hgr_p1  = apple_memory   + 0x2000;
volatile uint8_t *hgr_p2  = apple_memory   + 0x4000;
volatile uint8_t *hgr_p3  = private_memory + 0x2000;
volatile uint8_t *hgr_p4  = private_memory + 0x4000;

// The currently programmed character generator ROMs for text mode (US + local char set)
uint8_t __attribute__((section (".videodata."))) character_rom[2* 256 * 8];
