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

#include <stdlib.h>
#include "applebus/buffers.h"
#include "config/config.h"

#include "render.h"

bool mono_rendering = false;

void DELAYED_COPY_CODE(render_loop)()
{
    for(;;)
    {
#if 0
        config_handler();

        if(romx_changed || (machinefont != current_machine))
        {
            switch_font();
            romx_changed = 0;
            machinefont = current_machine;
        }
#endif

        update_text_flasher();

#if 0
        if(!(mono_palette & 0x8))
        {
            if((current_machine == MACHINE_IIGS) && !(soft_switches & SOFTSW_MONOCHROME)) {
                text_fore = lores_palette[APPLE_FORE];
                text_back = lores_palette[APPLE_BACK];
                text_border = lores_palette[APPLE_BORDER];
            } else {
                text_fore = mono_colors[1];
                text_back = mono_colors[0];
                text_border = mono_colors[0];
            }
        } else if(mono_palette == 0xF) {
            text_fore = lores_palette[TERMINAL_FORE];
            text_back = lores_palette[TERMINAL_BACK];
            text_border = lores_palette[TERMINAL_BORDER];
        } else {
            int palette = mono_palette & 0x7;
            text_fore = mono_colors[palette*2+1];
            text_back = mono_colors[palette*2];
            text_border = (palette == 0x6) ? text_fore : text_back;
        }
#endif

        dvi0.scanline_emulation = (internal_flags & IFLAGS_SCANLINEEMU) != 0;

        mono_rendering = (soft_switches & SOFTSW_MONOCHROME)||(internal_flags & IFLAGS_FORCED_MONO);

#if 0
        if(internal_flags & IFLAGS_TEST)
        {
            render_testpattern();
            // Assume the RP2040 has been hard reset and try to default to text display
            if(busactive && (testdone == 0)) { // was ((soft_switches & SOFTSW_MODE_MASK) != 0)
                soft_switches |= SOFTSW_TEXT_MODE;
                internal_flags &= ~IFLAGS_TEST;
                testdone = 1;
                render_about_init();
            }
 #if defined(ANALOG_GS)
        } else if(soft_switches & SOFTSW_SHR) {
            vga_prepare_frame();
            render_shr();
 #endif
#endif

        {
#if 0
            if(status_line[0] != 0)
            {
                render_status_line();
            }
#endif

            switch(soft_switches & SOFTSW_MODE_MASK)
            {
                case 0:
                    if(soft_switches & SOFTSW_DGR)
                    {
                        render_dgr();
                    }
                    else
                    {
                        render_lores();
                    }
                    break;
                case SOFTSW_MIX_MODE:
                    if((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR))
                    {
                        render_mixed_dgr();
                    }
                    else
                    {
                        render_mixed_lores();
                    }
                    break;
                case SOFTSW_HIRES_MODE:
                    if(soft_switches & SOFTSW_DGR)
                    {
#if 0
                        render_dhgr();
#endif
                    }
                    else
                    {
                        render_hires();
                    }
                    break;
                case SOFTSW_HIRES_MODE|SOFTSW_MIX_MODE:
                    if((soft_switches & (SOFTSW_80COL | SOFTSW_DGR)) == (SOFTSW_80COL | SOFTSW_DGR))
                    {
#if 0
                        render_mixed_dhgr();
#endif
                    }
                    else
                    {
                        render_mixed_hires();
                    }
                    break;
                default:
#if 0
                    mono_rendering |= (mono_palette & 0x8);
#endif
                    render_text();
                    break;
            }

        }
    }
}
