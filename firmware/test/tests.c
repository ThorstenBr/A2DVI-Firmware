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

/*
 * This file contains test cases. These are only built when the TEST
 * firmware is compiled. The test firmware disables the normal 6502
 * bus interface and simulates some write operations, cycling through
 * all supported video modes and settings.
 */

#include "pico/time.h"
#include "debug/debug.h"
#include "menu/menu.h"
#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "applebus/abus_pin_config.h"
#include "render/render.h"
#include "config/config.h"
#include "dvi/a2dvi.h"
#include "duck.h"

#ifdef FEATURE_TEST

const uint32_t SimulatedSlotNr = 1;

#define REG_SW_80STORE_OFF 0xc000
#define REG_SW_80STORE     0xc001
#define REG_SW_40COL       0xc00c
#define REG_SW_80COL       0xc00d
#define REG_SW_NORMCHAR    0xc00e
#define REG_SW_ALTCHAR     0xc00f

#define REG_SW_MONOCHROME  0xc021

#define REG_SW_TEXT_OFF    0xc050  // enables LORES by default
#define REG_SW_TEXT        0xc051
#define REG_SW_MIX_OFF     0xc052
#define REG_SW_MIX         0xc053
#define REG_SW_PAGE_1      0xc054
#define REG_SW_PAGE_2      0xc055
#define REG_SW_HIRES_OFF   0xc056
#define REG_SW_HIRES       0xc057

#define REG_SW_VIDEX_OFF   0xc058
#define REG_SW_VIDEX_ON    0xc059

#define REG_SW_DGR         0xc05e
#define REG_SW_DGR_OFF     0xc05f

#define REG_CARD          (0xc080 | (SimulatedSlotNr << 4))


#ifndef TEST_TMDS
#define TEST_40_COLUMNS
#define TEST_40_COLUMNS_COLOR
#define TEST_80_COLUMNS
#define TEST_VIDEX
#define TEST_LORES
#define TEST_HIRES
#define TEST_DOUBLE_LORES

#define TEST_MIX_MODES

#define TEST_PAGE_SWITCH
#define TEST_ALTCHAR_SWITCH
#endif

#define TEST_MENU

const uint32_t TestDelayMilliseconds = 300;//1*1000;

color_mode_t test_color_mode = COLOR_MODE_GREEN;
bool         test_scanline_mode = false;

void clearBothPages()
{
    PrintModePage2 = true;
    clearTextScreen();
    PrintModePage2 = false;
    clearTextScreen();
}

// simulate a write access with given address/data
static void simulateWrite(uint16_t address, uint8_t data)
{
    uint32_t card_select = (1u << (CONFIG_PIN_APPLEBUS_SELECT - CONFIG_PIN_APPLEBUS_DATA_BASE));
    if ((address & 0xCFF0) == REG_CARD) // Slot register area
        card_select = 0;
    abus_interface((address << 10) | card_select | data);
}

// simulate a read access with given address
static void simulateRead(uint16_t address)
{
    uint32_t card_select = (1u << (CONFIG_PIN_APPLEBUS_SELECT - CONFIG_PIN_APPLEBUS_DATA_BASE));
    uint32_t read_mode   = (1u << (CONFIG_PIN_APPLEBUS_RW     - CONFIG_PIN_APPLEBUS_DATA_BASE));
    if ((address & 0xCFF0) == REG_CARD) // Slot register area
        card_select = 0;
    abus_interface((address << 10) | card_select | read_mode);
}

void sleep(int Milliseconds)
{
    while (Milliseconds--)
    {
#if 1
        if (0 == Milliseconds % 100)
            gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
#endif
        debug_check_bootsel();
        sleep_ms(1);
    }
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void togglePages()
{
#ifdef TEST_PAGE_SWITCH
    for (uint i=0;i<2;i++)
    {
        simulateWrite(REG_SW_PAGE_2, 0); // show page 2
        sleep(500);
        simulateWrite(REG_SW_PAGE_1, 0); // show page 1
        sleep(500);
    }
#endif
}

void showTestConfig()
{
    if (language_switch)
        printXY(3,23, "US", PRINTMODE_NORMAL);
    else
        printXY(3,23, "LOCAL", PRINTMODE_NORMAL);

    if (test_color_mode == COLOR_MODE_BW)
        printXY(10,23, "BW", PRINTMODE_NORMAL);
    else
    if (test_color_mode == COLOR_MODE_GREEN)
        printXY(10,23, "GREEN", PRINTMODE_NORMAL);
    else
    if (test_color_mode == COLOR_MODE_AMBER)
        printXY(10,23, "AMBER", PRINTMODE_NORMAL);

    if (test_scanline_mode)
        printXY(17,23, "SCANLINES", PRINTMODE_NORMAL);

    if (cfg_video_mode & 1)
        printXY(27,23, "720", PRINTMODE_NORMAL);
    else
        printXY(27,23, "640", PRINTMODE_NORMAL);
}

void toggleAltChar()
{
#ifdef TEST_ALTCHAR_SWITCH
    // toggle mousetext vs default character set
    for (uint i=0;i<3;i++)
    {
        simulateWrite(REG_SW_ALTCHAR, 0);
        sleep(500);
        simulateWrite(REG_SW_NORMCHAR, 0);
        sleep(500);
    }
#endif
}

void testPrintChar(uint32_t x, uint32_t line, char c)
{
    uint32_t ScreenOffset = (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    char* pScreenArea = ((char*) text_p1) + ScreenOffset;
    if (PrintModePage2)
        pScreenArea = ((char*) text_p2) + ScreenOffset;
    char* pScreenArea80 = ((PrintModePage2) ? ((char*)text_p4) + ScreenOffset : ((char*)text_p3) + ScreenOffset);

    if (PrintMode80Column)
    {
        if (x&1)
            pScreenArea[x>>1] = c;
        else
            pScreenArea80[x>>1] = c;
    }
    else
    {
        pScreenArea[x] = c;
    }
}

void setLoresPixel(uint16_t x, uint8_t y, bool page2, uint8_t color)
{
     if (page2)
        x = x | 1024;

     color &= 0xF;
     uint8_t mask = 0xF0; // even rows in low nibble

     if ((y & 1) == 1)
     {
         // odd rows in high nibble
         mask = 0x0F;
         color <<= 4;
     }

     uint line = y >> 1;
     uint16_t address = 0x400+((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40)+x;
     apple_memory[address] = (apple_memory[address] & mask) | color;
}

void setLoresTestPattern(uint lines)
{
    for (uint y=0;y<lines;y++)
    {
        for (uint x=0;x<40;x++)
        {
            setLoresPixel(   x, y, false, (x+y)&0xf);  // page 1
            setLoresPixel(39-x, y, true,  (x+y)&0xf);  // page 2
        }
    }
}

void setHiresTestPattern()
{
    for (uint i=0;i<sizeof(A_DUCK_BIN)/4;i++)
    {
        uint32_t data = ((uint32_t*)A_DUCK_BIN)[i];
        ((uint32_t*)hgr_p1)[i] = data;
        ((uint32_t*)hgr_p2)[i] = data ^ 0xffffffff;
    }
}

void test40columns()
{
#ifdef TEST_40_COLUMNS
    clearBothPages();

    // prepare PAGE2
    PrintModePage2 = true;
    printXY(20-3,11, "PAGE 2", PRINTMODE_NORMAL);

    // prepare PAGE1
    PrintModePage2 = false;
    for (uint x=0;x<40;x++)
    {
        char s[3];
        if ((x&0xf)<10)
            s[0] = '0'+(x&0xf);
        else
            s[0] = 'A'+(x&0xf)-10;
        s[1] = 0;
        printXY(x,0, s, PRINTMODE_NORMAL);
        if (x<24)
            printXY(0,x, s, PRINTMODE_NORMAL);
    }
    printXY(20-13,2, "A2DVI Test: 40 column mode", PRINTMODE_NORMAL);
    for (uint i=0;i<255;i++)
    {
        testPrintChar((20-8)+(i&0xf), 4+(i>>4), i);
    }

    showTestConfig();

    simulateWrite(REG_SW_40COL, 0);          // disable 80column mode
    sleep(TestDelayMilliseconds);

    togglePages();                           // test both pages
    toggleAltChar();                         // test mouse text
#endif
}

void test80columns()
{
#ifdef TEST_80_COLUMNS
    PrintMode80Column = true;
    simulateWrite(REG_SW_80COL, 0); // enable 80column mode

    clearBothPages();

    // prepare PAGE2
    PrintModePage2 = true;
    printXY(40-3,11, "PAGE 2", PRINTMODE_NORMAL);

    // prepare PAGE1
    PrintModePage2 = false;
    printXY(40-13,2, "A2DVI Test: 80 column mode", PRINTMODE_NORMAL);
    for (uint x=0;x<80;x++)
    {
        char s[3];
        if ((x&0xf)<10)
            s[0] = '0'+(x&0xf);
        else
            s[0] = 'A'+(x&0xf)-10;
        s[1] = 0;
        printXY(x,0, s, PRINTMODE_NORMAL);
        if (x<24)
            printXY(0,x, s, PRINTMODE_NORMAL);
    }
    for (uint i=0;i<255;i++)
    {
        testPrintChar((40-8)+(i&0xf), 4+(i>>4), i);
    }

    showTestConfig();

    sleep(TestDelayMilliseconds);

    togglePages();                           // test both pages
    toggleAltChar();                         // test mouse text

    simulateWrite(REG_SW_40COL, 0);          // disable 80column mode
    PrintMode80Column = false;
#endif
}

void test40columns_color()
{
#ifdef TEST_40_COLUMNS_COLOR
    uint32_t save_iflags = internal_flags;

    internal_flags |= IFLAGS_VIDEO7;
    internal_flags &= ~IFLAGS_FORCED_MONO;

    clearBothPages();

    // prepare PAGE1
    PrintModePage2 = false;
    for (uint x=0;x<40;x++)
    {
        char s[3];
        if ((x&0xf)<10)
            s[0] = '0'+(x&0xf);
        else
            s[0] = 'A'+(x&0xf)-10;
        s[1] = 0;
        printXY(x,0, s, PRINTMODE_NORMAL);
        if (x<24)
            printXY(0,x, s, PRINTMODE_NORMAL);
    }
    printXY(20-16,2, "A2DVI Test: 40 column color mode", PRINTMODE_NORMAL);
    for (uint i=0;i<255;i++)
    {
        testPrintChar((20-8)+(i&0xf), 4+(i>>4), i);

        PrintMode80Column = true;
        testPrintChar(((20-8)+(i&0xf))<<1, 4+(i>>4), 0xD2);
        PrintMode80Column = false;
    }

    showTestConfig();

    simulateWrite(REG_SW_40COL,   0);        // disable 80column mode
    simulateWrite(REG_SW_80STORE, 0);        // enable 80STORE
    simulateWrite(REG_SW_DGR,     0);        // enable color mode

    sleep(TestDelayMilliseconds);

    toggleAltChar();                         // test mouse text
    simulateWrite(REG_SW_DGR_OFF,     0);    // disable color mode
    simulateWrite(REG_SW_80STORE_OFF, 0);    // disable 80STORE

    internal_flags = save_iflags;
#endif
}

void testLores()
{
#ifdef TEST_LORES
    simulateWrite(REG_SW_TEXT_OFF, 0);       // enable LORES graphics

    setLoresTestPattern(48);
    sleep(TestDelayMilliseconds);
    togglePages();                           // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0x80);  // enable MONOCHROME mode
    sleep(TestDelayMilliseconds);
    togglePages();                           // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0);     // disable MONOCHROME mode
    simulateWrite(REG_SW_TEXT, 0);           // enable text mode
#endif
}

void testLoresMix40()
{
#if (defined TEST_MIX_MODES) && ((defined TEST_40_COLUMNS)||(defined TEST_LORES))
    clearBothPages();
    simulateWrite(REG_SW_TEXT_OFF, 0);       // enable LORES graphics
    simulateWrite(REG_SW_MIX, 0);            // enable MIX MODE

    setLoresTestPattern(48-4*2);
    printXY(20-15,20, "A2DVI Test: LORES MIX MODE 40", PRINTMODE_NORMAL);
    printXY(20-10,21, "A2DVI TEST: FLASHING", PRINTMODE_FLASH);
    showTestConfig();

    // prepare PAGE2
    PrintModePage2 = true;
    printXY(20-3,20, "PAGE 2", PRINTMODE_NORMAL);
    PrintModePage2 = false;

    sleep(TestDelayMilliseconds);
    togglePages();                           // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0x80);  // enable MONOCHROME mode
    sleep(TestDelayMilliseconds);
    togglePages();                           // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0);     // disable MONOCHROME mode
    simulateWrite(REG_SW_MIX_OFF,    0);     // disable MIX MODE
    simulateWrite(REG_SW_TEXT,       0);     // enable text mode
#endif
}

void testLoresMix80()
{
#if (defined TEST_MIX_MODES) && ((defined TEST_80_COLUMNS)||(defined TEST_80_COLUMNS))
    PrintMode80Column = true;
    simulateWrite(REG_SW_80COL, 0);         // enable 80column mode
    clearBothPages();
    simulateWrite(REG_SW_TEXT_OFF, 0);      // enable LORES graphics
    simulateWrite(REG_SW_MIX,   0);         // enable MIX MODE

    setLoresTestPattern(48-4*2);
    printXY(40-15,20, "A2DVI Test: LORES MIX MODE 80", PRINTMODE_NORMAL);
    printXY(40-10,21, "A2DVI TEST: FLASHING", PRINTMODE_FLASH);
    showTestConfig();

    // prepare PAGE2
    PrintModePage2 = true;
    printXY(40-3,20, "PAGE 2", PRINTMODE_NORMAL);
    PrintModePage2 = false;

    sleep(TestDelayMilliseconds);
    togglePages();                          // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0x80); // enable MONOCHROME mode
    sleep(TestDelayMilliseconds);
    togglePages();                          // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0);    // disable MONOCHROME mode
    simulateWrite(REG_SW_MIX_OFF,    0);    // disable MIX MODE
    simulateWrite(REG_SW_TEXT,       0);    // enable text mode
    simulateWrite(REG_SW_40COL,      0);    // disable 80column mode
    PrintMode80Column = false;
#endif
}

void testDoubleLores()
{
#ifdef TEST_DOUBLE_LORES
    simulateWrite(REG_SW_TEXT_OFF, 0);      // enable LORES graphics
    simulateWrite(REG_SW_DGR,   0);         // enable double LORES

    setLoresTestPattern(48);
    sleep(TestDelayMilliseconds);

    simulateWrite(REG_SW_MONOCHROME, 0x80); // enable MONOCHROME mode
    sleep(TestDelayMilliseconds);

    simulateWrite(REG_SW_MONOCHROME, 0);    // disable MONOCHROME mode
    simulateWrite(REG_SW_DGR_OFF,    0);    // disable double LORES
    simulateWrite(REG_SW_TEXT,       0);    // enable text mode
#endif
}

void testHires()
{
#ifdef TEST_HIRES
    simulateWrite(REG_SW_HIRES, 0);         // enable HIRES graphics
    simulateWrite(REG_SW_TEXT_OFF, 0);      // disable TEXT mode

    setHiresTestPattern();
    sleep(TestDelayMilliseconds);

    togglePages();                          // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0x80); // enable MONOCHROME mode
    sleep(TestDelayMilliseconds);

    togglePages();                          // test both pages

    simulateWrite(REG_SW_MONOCHROME, 0);    // disable MONOCHROME mode
    simulateWrite(REG_SW_HIRES_OFF,  0);    // disable HIRES
    simulateWrite(REG_SW_TEXT,       0);    // enable text mode
#endif
}

void test_videx()
{
#ifdef TEST_VIDEX
    bool saved_videx_enabled = videx_enabled;
    if (!videx_enabled)
    {
        // temporarily enable the Videx support and load the Videx fonts
        videx_enabled = true;
        cfg_videx_selection = 1;
        reload_charsets |= 4;
    }

    simulateRead(REG_SW_VIDEX_ON);

    for (uint bank=0;bank<=3;bank++)
    {
        // select bank
        simulateRead(0xc0B0+(bank << 2));

        // select device ROM area
        simulateWrite(0xc300, 0);
        for (uint32_t b=0;b<0x200;b++)
        {
            simulateWrite(0xcc00+b, b&0xff);
        }
    }

    sleep(TestDelayMilliseconds);
    simulateRead(REG_SW_VIDEX_OFF);

    videx_enabled = saved_videx_enabled;
#endif
}

void test_config()
{
    simulateWrite(REG_CARD+0x4, 0); // load defaults

    simulateWrite(REG_CARD+0x0, 1); // enable scanline emulation
    simulateWrite(REG_CARD+0x1, 3); // set color mode to amber

    simulateWrite(REG_CARD+0x8, 3); // select german character set as main
    simulateWrite(REG_CARD+0x9, 0); // select US enhanced as alternate

    simulateWrite(REG_CARD+0x4, 2); // save config
    simulateWrite(REG_CARD+0x4, 1); // load config
}

void test_debug_menu()
{
    simulateWrite(REG_CARD+0xf, 11);  // unlock card
    simulateWrite(REG_CARD+0xf, 22);  // unlock card
    simulateWrite(REG_CARD+0x9, 0);   // enable menu
    simulateWrite(REG_CARD+0x9, 'B'); // debug page
    sleep(3000);
    simulateWrite(REG_CARD+0x9, 27);  // escape/exit
    simulateWrite(REG_CARD+0xf, 0);   // lock card
}

void test_menu()
{
#ifdef TEST_MENU
    simulateWrite(REG_CARD+0xf, 11); // unlock card
    simulateWrite(REG_CARD+0xf, 22); // unlock card

    simulateWrite(REG_CARD+0x9, 0); // enable menu
    sleep(3000);
    simulateWrite(REG_CARD+0x9, '6'); // select color style
    simulateWrite(REG_CARD+0x9, 21);  // right
    simulateWrite(REG_CARD+0x9, 21);  // right

    for (uint i=0;i<20;i++)
    {
        simulateWrite(REG_CARD+0x9, 10); // arrow down
        sleep(TestDelayMilliseconds/5);
    }
    simulateWrite(REG_CARD+0x9, 'N'); // menu page 2
    sleep(3000);
    for (uint i=0;i<10;i++)
    {
        simulateWrite(REG_CARD+0x9, 10); // arrow down
        sleep(TestDelayMilliseconds/5);
    }

    simulateWrite(REG_CARD+0x9, 22);  // CTRL-V: toggle video mode
    sleep(3000);

    simulateWrite(REG_CARD+0x9, 27); // escape/exit
    simulateWrite(REG_CARD+0xf, 0); // lock card
#endif
}

#ifdef FEATURE_TEST_TMDS
void render_tmds_test()
{
    static uint32_t count = 0;
    count++;

    uint8_t tmds_mode = (count/(5*60)) % 8;
    uint32_t r,g,b;

    for(uint y=0; y < 192; y++)
    {
        dvi_get_scanline(tmdsbuf);
        dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

        for(uint col=0; col < 320;col++)
        {
            uint16_t symbol = 0;
            //if ((y<=113)&&(y>=112))
            //if ((y<=117)&&(y>=116-4))
            if (y<128)
            {
                if (col == 0)
                {
                    symbol = y;
                }
                else
                if (col == 64)
                {
                    symbol = (y+128);//240/241  //244
                }
                else
                if (col == 128)
                {
                    symbol = (y+256);
                }
                else
                if (col == 192)
                {
                    symbol = (y+256+128); // 496
                }
            }
            switch(tmds_mode)
            {
                case 0:
                    r = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    g = 0x7fd00;
                    b = 0x7fd00;
                    break;
                case 1:
                    r = 0x7fd00;
                    g = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    b = 0x7fd00;
                    break;
                case 2:
                    r = 0x7fd00;
                    g = 0x7fd00;
                    b = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    break;
                case 3:
                    r = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    g = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    b = (col & 0x3) ? 0x7fd00 : 0x7fe00;
                    break;
                case 4:
                    r = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    g = 0x7fd00;
                    b = 0x7fd00;
                    break;
                case 5:
                    r = 0x7fd00;
                    g = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    b = 0x7fd00;
                    break;
                case 6:
                    r = 0x7fd00;
                    g = 0x7fd00;
                    b = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    break;
                case 7:
                    r = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    g = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    b = (col & 0x3) ? 0x7fd00 : 0xf1e03;
                    break;
                case 8:
                    r = tmds_hires_color_patterns_red[symbol];
                    g = tmds_hires_color_patterns_blue[0];
                    b = tmds_hires_color_patterns_red[0];
                    break;
                case 9:
                    r = tmds_hires_color_patterns_red[0];
                    g = tmds_hires_color_patterns_blue[0];
                    b = tmds_hires_color_patterns_red[symbol];
                    break;
                case 10:
                    r = tmds_hires_color_patterns_red[symbol];
                    g = tmds_hires_color_patterns_blue[symbol];
                    b = tmds_hires_color_patterns_red[symbol];
                    break;
                default:
                    r = tmds_hires_color_patterns_red[0];
                    g = tmds_hires_color_patterns_blue[symbol];
                    b = tmds_hires_color_patterns_red[0];
                    break;
            }

            *(tmdsbuf_blue++)  = r;
            *(tmdsbuf_green++) = g;
            *(tmdsbuf_red++)   = b;
        }
        dvi_send_scanline(tmdsbuf);
    }
}
#endif // TEST_TMDS

void test_loop()
{
    // initialize the Apple II bus interface
    abus_init();

    // simulate reset to pass the splash screen diagnostics
    simulateRead(0xFFFC);
    simulateRead(0xFFFD);
    simulateRead(0xFA62);
    for (uint i=0;i<6*1000*1000;i++)
    {
        simulateRead(0);
    }

    // unlock register area
    simulateWrite(REG_CARD+0xf, 11);
    simulateWrite(REG_CARD+0xf, 22);

    // disable scanlines
    simulateWrite(REG_CARD+0x0, 2);

#if 0
    test_config();
#else
    simulateWrite(REG_CARD+0x5, 6); // select swedish/finnish character set as main
    simulateWrite(REG_CARD+0x6, 0); // select US enhanced as alternate
#endif

    uint iteration = 0;
    uint32_t last_frame_counter = -1;

    // wait until DVI output is active
    while (frame_counter == 0);

    uint32_t last_memory_avail = getFreeHeap();
    uint32_t memory_leak_count = 0;

    while (1)
    {
        current_machine = MACHINE_IIE_ENH;
        internal_flags |= IFLAGS_IIE_REGS;

        // check stalled DVI output
        if (last_frame_counter == frame_counter)
        {
            // output has stalled!
            debug_sos();
        }
        last_frame_counter = frame_counter;

        // report memory leaks
        uint32_t heap_avail = getFreeHeap();
        if (heap_avail < last_memory_avail)
        {
            memory_leak_count++;
            last_memory_avail = heap_avail;
            if (memory_leak_count > 2)
            {
                /* Available heap may alternate between two values, due
                 * to different consumption depending on screen resolution,
                 * which we switch during the tests. But it must not
                 * decrease continuously. */
                debug_sos();
            }
        }

        {
            // toggle scanline emulation mode
            simulateWrite(REG_CARD+0xf, 11); // unlock register area
            simulateWrite(REG_CARD+0xf, 22);

            test_scanline_mode = !test_scanline_mode;
            simulateWrite(REG_CARD+0x0, test_scanline_mode ? 1 : 2);

            language_switch = !language_switch;

            // config setting is 1,2,3 for color_mode=0,1,2...
            test_color_mode = (test_color_mode == 2) ? 0 : test_color_mode+1;
            simulateWrite(REG_CARD+0x1, test_color_mode+1);

            simulateWrite(REG_CARD+0xf, 00); // lock register area
        }

        if (iteration & 2)
        {
            SET_IFLAG(IS_IFLAG(IFLAGS_INTERP_DHGR), (IFLAGS_INTERP_DGR|IFLAGS_INTERP_DHGR));
        }

        // test text modes
        test40columns();
        test80columns();
        test40columns_color();
        test_videx();

        // test menu
        test_menu();

        // text lo resolution graphics
        testLores();
        testDoubleLores();

        // test mix modes
        testLoresMix40();
        testLoresMix80();

        // test hires modes
        testHires();

        // show debug menu. And lock up when scanline errors occurred.
        do
        {
            test_debug_menu();
        } while (a2dvi_scanline_errors());

        iteration++;
    }
}

#endif // FEATURE_TEST
