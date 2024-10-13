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

uint8_t videx_vterm_mem_selected;   // true -> videx memory is accessible at $C800-$CFFF
uint8_t __attribute__((section (".appledata."))) videx_vram[2048];

uint_fast16_t videx_bankofs;   // selected videx memory bank offset
uint_fast8_t  videx_crtc_idx;  // selected CRTC register

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
uint8_t videx_crtc_regs[18] =
{
    0x7b, 0x50, 0x62, 0x29,
    0x1b, 0x08, 0x18, 0x19,
    0x00, 0x08, 0xc0, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

// select the video memory bank (for better performance, we remember the buffer offset instead)
#define VIDEX_SET_BANK(address) videx_bankofs = (address & 0x000c) << 7;

VIDEXFUNC void videx_reg_read(uint_fast16_t address)
{
    // select the video memory bank
    VIDEX_SET_BANK(address);
}

// Shadow accesses to card registers in $C0n0 - $C0nF range
VIDEXFUNC void videx_reg_write(uint_fast16_t address, uint_fast8_t data)
{
    // select the video memory bank
    VIDEX_SET_BANK(address);

    if(address & 0x0001)
    {
        // set current register value (only first 16 registers are writable)
        if(videx_crtc_idx < 16)
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

VIDEXFUNC void videx_c8xx_read(uint_fast16_t address)
{
    if(address >= 0xce00)
    {
        // accesses to $CE00-$CFFF deactivates the card's memory
        videx_vterm_mem_selected = false;
    }
}

// Shadow bus accesses to the $C800-$CFFF memory space
VIDEXFUNC void videx_c8xx_write(uint_fast16_t address, uint_fast8_t data)
{
    if(address < 0xcc00)
    {
        // ignore ROM writes
        return;
    }

    if(address < 0xce00)
    {
        // this is the window into the card's video RAM
        uint16_t vaddr = videx_bankofs + (address & 0x01ff);
        videx_vram[vaddr] = data;
    }
    else
    {
        // accesses to $CE00-$CFFF deactivates the card's memory
        videx_vterm_mem_selected = false;
    }
}
