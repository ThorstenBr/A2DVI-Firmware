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

#include <string.h>
#include "videx_vterm.h"

#include <pico/stdlib.h>
#include "applebus/buffers.h"
#include "config/config.h"

#include "render/render.h"

volatile bool videx_vterm_mem_selected;   // true -> videx memory is accessible at $C800-$CFFF
volatile uint8_t __attribute__((section (".appledata."))) videx_vram[2048];

static uint8_t videx_banknum;   // selected memory bank
static uint8_t videx_crtc_idx;  // selected CRTC register

// CRT controller registers (Ref. MC6845 datasheet):
//   register  r/w     name
//     00       w      Horiz. Total
//     01       w      Horiz. Displayed
//     02       w      Horiz. Sync Position
//     03       w      Horiz. Sync Width
//     04       w      Vert. Total
//     05       w      Vert. Total Adjust
//     06       w      Vert. Displayed
//     07       w      Vert. Sync Position
//     08       w      Interlace Mode
//     09       w      Max Scan Line Address
//     10       w      Cursor Start
//     11       w      Cursor End
//     12       w      Start Address (H)
//     13       w      Start Address (L)
//     14      r/w     Cursor (H)
//     15      r/w     Cursor (L)
//     16       r      Light Pen (H)
//     17       r      Light Pen (L)
static volatile uint8_t videx_crtc_regs[18] =
{
    0x7b, 0x50, 0x62, 0x29,
    0x1b, 0x08, 0x18, 0x19,
    0x00, 0x08, 0xc0, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

void videx_reg_read(uint_fast16_t address)
{
    // select the video memory bank
    videx_banknum = (address & 0x000c) >> 2;
}

// Shadow accesses to card registers in $C0n0 - $C0nF range
void videx_reg_write(uint_fast16_t address, uint_fast8_t data)
{
    // select the video memory bank
    videx_banknum = (address & 0x000c) >> 2;

    if(address & 0x0001)
    {
        // set current register value
        if(videx_crtc_idx < sizeof(videx_crtc_regs))
        {
            videx_crtc_regs[videx_crtc_idx] = data;
        }
    }
    else
    {
        // set the crtc register being accessed
        videx_crtc_idx = data;
    }
}

void videx_c8xx_read(uint_fast16_t address)
{
    if(address >= 0xce00)
    {
        // accesses to $CE00-$CFFF deactivates the card's memory
        videx_vterm_mem_selected = false;
    }
}

// Shadow bus accesses to the $C800-$CFFF memory space
void videx_c8xx_write(uint_fast16_t address, uint_fast8_t data)
{
    if(address < 0xcc00)
        return;  // ignore ROM reads

    if(address < 0xce00)
    {
        // this is the window into the card's video RAM
        uint16_t vaddr = ((uint16_t)videx_banknum << 9) | (address & 0x01ff);
        videx_vram[vaddr] = data;
    }
    else
    {
        // accesses to $CE00-$CFFF deactivates the card's memory
        videx_vterm_mem_selected = false;
    }
}

static uint_fast8_t videx_cursor_mask = 0;
static uint64_t videx_next_flash_tick = 0;

// Update the internal VideoTerm flashing state
//
// Only called from the render core
static void videx_vterm_update_flasher()
{
    uint64_t now = time_us_64();
    if(now > videx_next_flash_tick)
    {
        switch((videx_crtc_regs[10] >> 5) & 0x03)
        {
            case 0:
                // non-blinking cursor
                videx_cursor_mask = 0xff;
                videx_next_flash_tick = now + 312500u;
                break;
            case 1:
                // no cursor
                videx_cursor_mask = 0;
                videx_next_flash_tick = now + 312500u;
                break;
            case 2:
                // blink, 1/16th field rate
                videx_cursor_mask ^= 0xff;
                videx_next_flash_tick = now + 625000u;
                break;
            case 3:
                // blink, 1/32th field rate
                videx_cursor_mask ^= 0xff;
                videx_next_flash_tick = now + 312500u;
                break;
        }
    }
}

static inline uint_fast8_t char_videx_text_bits(uint8_t ch, uint_fast8_t glyph_line, bool has_cursor)
{
    uint_fast8_t bits;

    if(ch < 0x80)
    {
        bits = character_rom_videx_normal[((uint_fast16_t)ch << 4) + glyph_line];
    }
    else
    {
        bits = character_rom_videx_inverse[((uint_fast16_t)(ch & 0x7f) << 4) + glyph_line];
    }

    if(has_cursor)
    {
        bits ^= videx_cursor_mask;
    }

    return bits;
}

// renders one text lines with 9 horizontal pixels
static void render_videx_text_line(unsigned int line, uint16_t text_base_addr, uint16_t cursor_addr)
{
    const uint8_t color_offset = color_mode*12;

    const uint cursor_line_start = videx_crtc_regs[10] & 0xf;
    const uint cursor_line_end   = videx_crtc_regs[11] & 0xf;
    const uint crtc_mem_offset_of_line = text_base_addr + (line * 80);

    for(uint glyph_line = 0; glyph_line < 9; glyph_line++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);
        bool cursor_in_line = (glyph_line >= cursor_line_start) && (glyph_line <= cursor_line_end);

        // Note: Videx characters are 8 pixels wide so 80 columns fills an entire 640 pixel VGA line and
        // no left-padding is needed
        // (80 x 4 x 2 pixels = 640)
        uint32_t crtc_mem_offset = crtc_mem_offset_of_line;
        uint32_t bits, data;
        bool has_cursor;

        for(uint col = 0; col < 20; col++)
        {
            // Grab next 4 characters (might be misaligned)
            memcpy(&data, (void*) &videx_vram[crtc_mem_offset & 0x7FF], 4);

            // Grab 32 pixels from the next four characters
            has_cursor = (cursor_in_line) && (crtc_mem_offset == cursor_addr);
            bits       =  char_videx_text_bits(data, glyph_line, has_cursor);

            data     >>= 8;
            has_cursor = (cursor_in_line) && (crtc_mem_offset+1 == cursor_addr);
            bits      |= (char_videx_text_bits(data, glyph_line, has_cursor)) << 8;

            data     >>= 8;
            has_cursor = (cursor_in_line) && (crtc_mem_offset+2 == cursor_addr);
            bits      |=  char_videx_text_bits(data, glyph_line, has_cursor) << 16;

            data     >>= 8;
            has_cursor = (cursor_in_line) && (crtc_mem_offset+3 == cursor_addr);
            bits      |= (char_videx_text_bits(data, glyph_line, has_cursor)) << 24;

            // Render each pair of bits into a pair of pixels, least significant bit first
            for(int i = 0; i < 16; i++)
            {
                char symbol = (bits&3) + color_offset;
                *(tmdsbuf_blue++)  = tmds_mono_pixel_pair[symbol+8];
                *(tmdsbuf_green++) = tmds_mono_pixel_pair[symbol+4];
                *(tmdsbuf_red++)   = tmds_mono_pixel_pair[symbol+0];
                bits >>= 2;
            }

            crtc_mem_offset += 4;
        }

        dvi_send_scanline(tmdsbuf);
    }
}

void DELAYED_COPY_CODE(render_skip_lines)(uint row_count)
{
    for (uint row=0;row<row_count;row++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);
        for (uint32_t x=0;x<320;x++)
        {
            *(tmdsbuf_red++)   = TMDS_SYMBOL_0_0;
            *(tmdsbuf_green++) = TMDS_SYMBOL_0_0;
            *(tmdsbuf_blue++)  = TMDS_SYMBOL_0_0;
        }
        dvi_send_scanline(tmdsbuf);
    }
}

// Render a screen of VideoTerm text mode
//
// Only called from the render core
void DELAYED_COPY_CODE(render_videx_text)(void)
{
    // Compute these once at the start of the frame
    const uint16_t text_base_addr = ((videx_crtc_regs[12] & 0x3f) << 8) | videx_crtc_regs[13];
    const uint16_t cursor_addr    = ((videx_crtc_regs[14] & 0x3f) << 8) | videx_crtc_regs[15];

    // render 24 x 9 pixel lines = 216 lines
    // (lines are doubled, so VGA resolution is 216x2=432 lines)
    for(int line = 0; line < 24; line++)
    {
        render_videx_text_line(line, text_base_addr, cursor_addr);
    }

    videx_vterm_update_flasher();
}
