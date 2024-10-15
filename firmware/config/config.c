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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/flash.h>

#include "config.h"
#include "applebus/buffers.h"
#include "util/dmacopy.h"
#include "fonts/textfont.h"
#include "videx/videx_vterm.h"

volatile compat_t  detected_machine = MACHINE_AUTO;
volatile compat_t  cfg_machine = MACHINE_AUTO;
volatile compat_t  current_machine = MACHINE_AUTO;
volatile bool      language_switch = false; // false: main/local char set, true: alternate char set (normally fixed to US default)
volatile bool      unenhance_font; // switch to explicitly "unenhance" a font (by removing the mouse text characters)
volatile uint8_t   reload_charsets = 4;
volatile bool      reload_colors;

bool               videx_enabled;
uint8_t            cfg_videx_selection = 0; //0:DISABLED
uint8_t            cfg_local_charset = 0;
uint8_t            cfg_alt_charset   = 0;
uint32_t           invalid_fonts = 0xffffffff;
uint8_t            cfg_color_style;
volatile uint8_t   color_mode = 1;
rendering_fx_t     cfg_rendering_fx = FX_ENABLED;
ToggleSwitchMode_t input_switch_mode = ModeSwitchCycleVideo;

// A block of flash is reserved for storing configuration persistently across power cycles
// and firmware updates.
//
// The memory is divided as:
//  * 4K for a 'config' structure
//  * the remaining is reserved for future use

// DVI2
#define CFG_MAGIC_WORD_VALUE 0x32495644

// packed config structure (packed = do not padd to a multiple of 4 bytes)
struct __attribute__((__packed__)) config_t
{
    // magic word determines if the stored configuration is valid
    uint32_t magic_word;

    // the real size of the stored structure
    uint16_t size;
    uint8_t  scanline_emulation;
    uint8_t  forced_monochrome;

    uint8_t  color_mode;
    uint8_t  machine_type;
    uint8_t  local_charset; // selection for local language video ROM
    uint8_t  alt_charset;   // selection for alternate video ROM (usually fixed to US charset)

    uint8_t  input_switch_mode;
    uint8_t  OBSOLETE_enhanced_font_enabled; // no longer used (replaced by MACHINE_IIE_ENH)
    uint8_t  video7_enabled;
    uint8_t  debug_lines_enabled;

    uint8_t  test_mode_enabled;
    rendering_fx_t rendering_fx;
    uint8_t  videx_selection;
    uint8_t  color_style;

    // Add new fields after here. When reading the config use the IS_STORED_IN_CONFIG macro
    // to determine if the field you're looking for is actually present in the stored config.
};

// 'FONT'
#define FONT_MAGIC_WORD_VALUE 0x544e4f46

struct __attribute__((__packed__))  fontdir_t
{
    uint32_t magic_word;
    uint32_t invalid_fonts;   // bit mask identifying invalid fonts (1:invalid, 0:valid)
};

// This is a compile-time check to ensure the size of the config struct fits within one flash erase sector
typedef char config_struct_size_check[(sizeof(struct config_t) <= FLASH_SECTOR_SIZE) - 1];

#define IS_STORED_IN_CONFIG(cfg, field) ((offsetof(struct config_t, field) + sizeof((cfg)->field)) <= (cfg)->size)


extern uint8_t __config_data_start[];
static struct config_t *cfg = (struct config_t *)__config_data_start;

extern uint8_t __font_dir_start[];
static struct fontdir_t *font_directory = (struct fontdir_t *)__font_dir_start;

#if FLASH_SECTOR_SIZE != 2*CHARACTER_ROM_SIZE
    #error Platform with unsupported flash segmentation. Needs adaption.
#endif

/* Check if font matches the current machine type.
 * Otherwise switch the font selection. */
void __time_critical_func(check_machine_font)()
{
    // custom fonts: no check
    if ((cfg_local_charset >= FONT_MIN_CUSTOM)||
        (cfg_local_charset == FONT_PRAVETZ))
    {
        return;
    }

    if (internal_flags & IFLAGS_IIE_REGS)
    {
        // Apple IIe detected
        if ((cfg_local_charset >= FONT_MIN_IIE)&&
            (cfg_local_charset <= FONT_MAX_IIE))
        {
            // an Apple IIe font is already selected
            return;
        }
        // switch to default Apple IIE font instead
        cfg_local_charset = FONT_MIN_IIE;
    }
    else
    {
        // Apple II
        if ((cfg_local_charset >= FONT_MIN_II)&&
            (cfg_local_charset <= FONT_MAX_II))
        {
            // an Apple II font is already selected
            return;
        }
        // switch to default Apple II font instead
        cfg_local_charset = (current_machine == MACHINE_PRAVETZ) ? FONT_PRAVETZ : FONT_MIN_II;
    }
    reload_charsets |= 1;
}

void __time_critical_func(set_machine)(compat_t machine)
{
    switch(machine)
    {
        case MACHINE_IIE:
        case MACHINE_IIE_ENH:
            internal_flags &= ~IFLAGS_IIGS_REGS;
            internal_flags |=  IFLAGS_IIE_REGS;
        #if VIDEX_SLOT != 1
            videx_enabled = false;
        #endif
            break;
#ifdef MACHINE_IIGS
        case MACHINE_IIGS:
            internal_flags &= ~IFLAGS_IIE_REGS;
            internal_flags |=  IFLAGS_IIGS_REGS;
            videx_enabled = false;
            break;
#endif
        case MACHINE_AUTO:
        case MACHINE_AGAT7:
        case MACHINE_AGAT9:
        case MACHINE_BASIS:
        case MACHINE_PRAVETZ:
        case MACHINE_II:
        default:
            internal_flags &= ~(IFLAGS_IIGS_REGS|IFLAGS_IIE_REGS);
            break;
    }

    if (machine == MACHINE_IIE)
    {
        // we need to "unenhance" the fonts for the IIe (unenhanced)
        if (!unenhance_font)
        {
            unenhance_font = true;
            reload_charsets |= 3;
        }
    }
    else
    {
        // all other machines, including "//e enhanced": leave fonts unchanged
        if (unenhance_font)
        {
            unenhance_font = false;
            reload_charsets |= 3;
        }
    }

    if ((current_machine == MACHINE_AUTO)&&
        (machine != MACHINE_AUTO))
    {
        static bool done = false;
        if (!done)
        {
            done = true;
            current_machine = machine;
            // machine auto detection: also adjust font to machine type
            check_machine_font();
        }
    }
    current_machine = machine;
}

bool DELAYED_COPY_CODE(config_flash_write)(void* flash_address, uint8_t* data, uint32_t size)
{
    if (size > FLASH_SECTOR_SIZE)
        return false;

    const uint32_t flash_offset = ((uint32_t) flash_address) - XIP_BASE;

    flash_range_erase(flash_offset, FLASH_SECTOR_SIZE);
    flash_range_program(flash_offset, data, size);

    return true;
}

void DELAYED_COPY_CODE(config_font_update)(void)
{
    // We could use the "directory" to store the name of each custom font.
    // But for now we save the effort - and just remember which font was
    // uploaded and is valid.
    const uint32_t flash_offset = ((uint32_t) font_directory) - XIP_BASE;

    struct fontdir_t new_dir;
    new_dir.magic_word = FONT_MAGIC_WORD_VALUE;
    new_dir.invalid_fonts = invalid_fonts;
    flash_range_erase(flash_offset, FLASH_SECTOR_SIZE);
    flash_range_program(flash_offset, (uint8_t*) &new_dir, sizeof(new_dir));
}

static uint8_t check_valid_font(uint32_t font_nr)
{
    // hardcoded fonts are always ok
    if (font_nr < (MAX_FONT_COUNT-CUSTOM_FONT_COUNT))
        return font_nr;

    // custom fonts: only valid when programmed
    uint8_t custom_font = font_nr - (MAX_FONT_COUNT-CUSTOM_FONT_COUNT);

    // custom font is loaded/valid: use the custom font
    if ((invalid_fonts & (1 << custom_font)) == 0)
        return font_nr;

    // custom font is not valid/not programmed: use a font from a
    // built-in font slot
    return custom_font;
}

void DELAYED_COPY_CODE(config_load_charsets)(void)
{
    if (reload_charsets & 1)
    {
        // local font
        memcpy32(character_rom, character_roms[check_valid_font(cfg_local_charset)], CHARACTER_ROM_SIZE);

        if (unenhance_font)
        {
            // unenhance the font, by overwriting the mousetext characters
            memcpy32(&character_rom[0x40*8], &character_rom[0], 0x20*8);
        }
    }

    if (reload_charsets & 2)
    {
        // alternate fixed US font (with language switch)
        memcpy32(&character_rom[0x800], character_roms[check_valid_font(cfg_alt_charset)], CHARACTER_ROM_SIZE);

        if (unenhance_font)
        {
            // unenhance the font, by overwriting the mousetext characters
            memcpy32(&character_rom[0x800+0x40*8], &character_rom[0x800], 0x20*8);
        }
    }

    if ((reload_charsets & 4)&&(cfg_videx_selection>0))
    {
        // videx character sets
        memcpy32(character_rom_videx_normal,  character_roms_videx[cfg_videx_selection-1], CHARACTER_ROM_SIZE);
        memcpy32(character_rom_videx_inverse, videx_inverse,                               CHARACTER_ROM_SIZE);
    }

    reload_charsets = 0;
}

void config_setflags(void)
{
    SET_IFLAG(((cfg_rendering_fx==FX_ENABLED)||(cfg_rendering_fx == FX_DGR_ONLY)), IFLAGS_INTERP_DGR);
    SET_IFLAG(((cfg_rendering_fx==FX_ENABLED)||(cfg_rendering_fx == FX_DHGR_ONLY)),IFLAGS_INTERP_DHGR);

    videx_enabled = (cfg_videx_selection > 0);
}

void config_load(void)
{
    if (font_directory->magic_word == FONT_MAGIC_WORD_VALUE)
    {
        invalid_fonts = font_directory->invalid_fonts;
    }

    if((cfg->magic_word != CFG_MAGIC_WORD_VALUE) || (cfg->size > FLASH_SECTOR_SIZE))
    {
        config_load_defaults();
        return;
    }

    cfg_machine = (cfg->machine_type <= MACHINE_MAX_CFG) ? cfg->machine_type : MACHINE_AUTO;
    if ((cfg_machine == MACHINE_IIE)&&(cfg->OBSOLETE_enhanced_font_enabled != 0))
    {
        // convert old config setting
        cfg_machine = MACHINE_IIE_ENH;
    }

    SET_IFLAG(cfg->scanline_emulation,   IFLAGS_SCANLINEEMU);
    SET_IFLAG(cfg->forced_monochrome,    IFLAGS_FORCED_MONO);
    SET_IFLAG(cfg->video7_enabled,       IFLAGS_VIDEO7);
    SET_IFLAG(cfg->debug_lines_enabled,  IFLAGS_DEBUG_LINES);
    SET_IFLAG(cfg->test_mode_enabled,    IFLAGS_TEST);
    if (IS_STORED_IN_CONFIG(cfg, videx_selection))
    {
        cfg_videx_selection = cfg->videx_selection;
    }
    if(IS_STORED_IN_CONFIG(cfg, rendering_fx))
    {
        cfg_rendering_fx = cfg->rendering_fx;
    }

    if(IS_STORED_IN_CONFIG(cfg, color_style))
    {
        cfg_color_style = cfg->color_style;
    }

    config_setflags();
    set_machine(cfg_machine);

    input_switch_mode = cfg->input_switch_mode;
    unenhance_font = (cfg_machine == MACHINE_IIE);

    color_mode = (cfg->color_mode <= 2) ? cfg->color_mode : 0;

    cfg_local_charset = cfg->local_charset;
    if (cfg_local_charset >= MAX_FONT_COUNT)
        cfg_local_charset = 0;

    cfg_alt_charset = cfg->alt_charset;
    if (cfg_alt_charset >= MAX_FONT_COUNT)
        cfg_alt_charset = 0;

    // load both character sets
    reload_charsets |= 3;

    // reload color palette
    reload_colors = true;
}

void config_load_defaults(void)
{
    SET_IFLAG(1, IFLAGS_SCANLINEEMU);
    SET_IFLAG(0, IFLAGS_DEBUG_LINES);
    SET_IFLAG(0, IFLAGS_FORCED_MONO);
    SET_IFLAG(1, IFLAGS_VIDEO7);
    SET_IFLAG(0, IFLAGS_TEST);

    cfg_color_style         = 2; // improved
    color_mode              = COLOR_MODE_BW;
    cfg_machine             = MACHINE_AUTO;

    input_switch_mode       = ModeSwitchLangCycle;
    unenhance_font          = false;
    cfg_rendering_fx        = FX_ENABLED;

    cfg_local_charset       = DEFAULT_LOCAL_CHARSET;
    cfg_alt_charset         = DEFAULT_ALT_CHARSET;
    cfg_videx_selection     = 0;

    config_setflags();
    set_machine(detected_machine);

    // reload both character sets
    reload_charsets |= 3;
}

void DELAYED_COPY_CODE(config_save)(void)
{
    // the write buffer size must be a multiple of FLASH_PAGE_SIZE so round up
    const int new_config_size = (sizeof(struct config_t) + FLASH_PAGE_SIZE - 1) & -FLASH_PAGE_SIZE;
    struct config_t *new_config = malloc(new_config_size);
    memset(new_config, 0xff, new_config_size);
    memset(new_config, 0, sizeof(struct config_t));

    // prepare header
    new_config->magic_word = CFG_MAGIC_WORD_VALUE;
    new_config->size       = sizeof(struct config_t);

    // set config properties
    new_config->scanline_emulation      = IS_IFLAG(IFLAGS_SCANLINEEMU);
    new_config->forced_monochrome       = IS_IFLAG(IFLAGS_FORCED_MONO);
    new_config->video7_enabled          = IS_IFLAG(IFLAGS_VIDEO7);
    new_config->debug_lines_enabled     = IS_IFLAG(IFLAGS_DEBUG_LINES);
    new_config->test_mode_enabled       = IS_IFLAG(IFLAGS_TEST);
    new_config->videx_selection         = cfg_videx_selection;
    new_config->rendering_fx            = cfg_rendering_fx;
    new_config->color_style             = cfg_color_style;
    new_config->color_mode              = color_mode;
    new_config->machine_type            = cfg_machine;
    new_config->local_charset           = cfg_local_charset;
    new_config->alt_charset             = cfg_alt_charset;
    new_config->input_switch_mode       = input_switch_mode;

    // update flash
    config_flash_write(cfg, (uint8_t *)new_config, new_config_size);

    free(new_config);
}
