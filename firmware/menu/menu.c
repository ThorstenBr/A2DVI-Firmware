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

#include <pico.h>

#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "config/config.h"
#include "fonts/textfont.h"
#include "debug/debug.h"
#include "dvi/a2dvi.h"
#include "menu.h"

// number of elements in the menu
#define MENU_ENTRY_COUNT (19)

// number of non-config elements in the two column menu area
#define MENU_ENTRIES_NONCFG (6)

// offset of the first non-config menu element
#define MENU_OFS_NONCFG (MENU_ENTRY_COUNT-MENU_ENTRIES_NONCFG)

// extra commands start at this offset
#define MENU_EXTRA (0x10)

#ifdef FEATURE_TEST
bool PrintMode80Column = false;
bool PrintModePage2    = false;
#endif

static uint8_t MenuSelection      = 0;
static uint8_t MenuPage           = 0;
static uint8_t MenuPageItems      = 0;
       bool    IgnoreNextKeypress = false;
static bool    MenuNeedsRedraw;
static uint8_t MenuSubTitleToggle;
static uint8_t MenuOptionNr;

#define TEXT_OFFSET(line) ((((line) & 0x7) << 7) + ((((line) >> 3) & 0x3) * 40))

static void setLineColor(uint line, uint8_t color)
{
    uint ofs = TEXT_OFFSET(line);
    for (uint i=0;i<40;i++)
    {
        text_p3[ofs+i] = color;
    }
}

const char* DELAYED_COPY_CODE(getMenuString)(const char* pResource, uint Index)
{
    const char* pText = pResource;
    while (Index--)
    {
        if (!*pText)
            return "";
        while (*(++pText) != 0);
        pText++;
    }
    return pText;
}

void DELAYED_COPY_CODE(centerY)(uint32_t y, const char* pMsg, TPrintMode PrintMode)
{
    uint32_t x = 0;
    while (pMsg[x] != 0)
        x++;
    x = 20-(x>>1);
    printXY(x, y, pMsg, PrintMode);
}

void DELAYED_COPY_CODE(printXY)(uint32_t x, uint32_t line, const char* pMsg, TPrintMode PrintMode)
{
    uint32_t ScreenOffset = (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    char* pScreenArea = ((char*) text_p1) + ScreenOffset;
#ifdef FEATURE_TEST
    if (PrintModePage2)
        pScreenArea = ((char*) text_p2) + ScreenOffset;
    char* pScreenArea80 = ((PrintModePage2) ? ((char*)text_p4) + ScreenOffset : ((char*)text_p3) + ScreenOffset);
#endif
    for (uint32_t i=0;pMsg[i];i++)
    {
        char c = pMsg[i];
        switch(PrintMode)
        {
            case PRINTMODE_FLASH:
                c |= 0x40;
                break;
            case PRINTMODE_INVERSE:
                if ((c>='A')&&(c<='Z'))
                    c -= 'A'-1;
                break;
            case PRINTMODE_RAW:
                break;
            default:
            case PRINTMODE_NORMAL:
                c |= 0x80;
                break;
        }
#ifdef FEATURE_TEST
        if (PrintMode80Column)
        {
            if ((x+i)&1)
                pScreenArea[(x+i)>>1] = c;
            else
                pScreenArea80[(x+i)>>1] = c;
        }
        else
#endif
        {
            pScreenArea[i+x] = c;
        }
    }
}

void DELAYED_COPY_CODE(clearTextScreen)(void)
{
    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
#ifndef FEATURE_TEST
        ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
        ((uint32_t*)text_p3)[i] = 0xA0A0A0A0; // initialize with blanks
#else
        if (!PrintModePage2)
        {
            ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
            ((uint32_t*)text_p3)[i] = 0xA0A0A0A0; // initialize with blanks
        }
        else
        {
            ((uint32_t*)text_p2)[i] = 0xA0A0A0A0; // initialize with blanks
            ((uint32_t*)text_p4)[i] = 0xA0A0A0A0; // initialize with blanks
        }
#endif
    }
}

void DELAYED_COPY_CODE(clearLine)(uint8_t line, TPrintMode PrintMode)
{
    uint32_t ScreenOffset = (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    char blank = (PrintMode == PRINTMODE_NORMAL) ? ' '|0x80 : ' ';
    for (uint32_t i=0;i<40;i++)
    {
        text_p1[ScreenOffset+i] = blank; // initialize with blanks
    }
}

char DELAYED_COPY_DATA(TitleFirmware)[] =
    "A2DVI - FIRMWARE V" FW_VERSION;

char DELAYED_COPY_DATA(TitleCopyright)[] =
    "(C) 2024 THORSTEN BREHM, RALLE PALAVEEV ";

char DELAYED_COPY_DATA(TitleGitHub0)[] = "GITHUB.COM/RALLEPALAVEEV/A2DVI";
char DELAYED_COPY_DATA(TitleGitHub1)[] = "  GITHUB.COM/THORSTENBR/A2DVI-FIRMWARE  ";
char* DELAYED_COPY_DATA(TitleGitHub)[2] = {TitleGitHub0, TitleGitHub1};

char DELAYED_COPY_DATA(MachineNames)[] =
{
    "AUTO DETECT\0"
    "APPLE II\0"
    "APPLE IIE\0"
    "APPLE IIE ENHANCED\0"
    "AGAT7\0"
    "AGAT9\0"
    "BASIS\0"
    "PRAVETZ\0"
    "\0"
};

char DELAYED_COPY_DATA(MenuOnOff)[] =
    "DISABLED\0"
    "ENABLED\0"
    "\0";

char DELAYED_COPY_DATA(MenuPalNtsc)[] =
    "NTSC\0"
    "PAL\0"
    "\0";

char DELAYED_COPY_DATA(MenuButtonModes)[] =
{
//   12345678901234567890
    "DISABLED\0"
    "TOGGLE CHARACTER SET\0"
    "TOGGLE MONO/COLOR\0"
    "CYCLE DISPLAY MODE\0"
    "CHARSET+TOGGLE M/C\0"
    "CHARSET+CYCLE MODES\0"
    "\0"
};

char DELAYED_COPY_DATA(MenuRendering)[] =
//   12345678901234567890
    "DISABLED\0"
    "ENABLED\0"
    "DOUBLE HIRES ONLY\0"
    "DOUBLE LORES ONLY\0"
    "\0";


char DELAYED_COPY_DATA(MenuColorBW)[]    = "BLACK & WHITE";
char DELAYED_COPY_DATA(MenuColorGreen)[] = "GREEN";
char DELAYED_COPY_DATA(MenuColorAmber)[] = "AMBER";

char* DELAYED_COPY_DATA(MenuColorMode)[COLOR_MODE_AMBER+1] =
{
    MenuColorBW,
    MenuColorGreen,
    MenuColorAmber
};

char DELAYED_COPY_DATA(MenuForcedMono)[] =
    "COLOR\0"
    "MONOCHROME\0"
    "\0";

char DELAYED_COPY_DATA(MenuColorStyle)[] =
    "DEFAULT\0"
    "ORIGINAL (IIE)\0"
    "IMPROVED (IIGS/IIE)\0"
    "\0";

char DELAYED_COPY_DATA(MenuVideoMode)[] =
    "640x480p60\0"
    "720x480p60\0"
    "\0";

char DELAYED_COPY_DATA(MenuFontNames)[] =
    "IIE US\0" //0
    "IIE UK\0" //1
    "IIE FRENCH\0" //2
    "IIE GERMAN\0" //3
    "IIE SPANISH\0" //4
    "IIE ITALIAN\0" //5
    "IIE SWEDISH/FINNISH\0" //6
    "IIE HEBREW\0" //7
    "PRAVETZ CYRILLIC\0" //8
    "IIE US REACTIVE\0" //9
    "IIE US UNENHANCED\0" //10
    "II+ US\0" //11
    "II+ VIDEX LOWER 1\0"//12
    "II+ VIDEX LOWER 2\0"//13
    "II+ PIG FONT\0"//14
    "II+ JAPAN KATAKANA\0"//15

    "CUSTOM FONT 1\0"
    "CUSTOM FONT 2\0"
    "CUSTOM FONT 3\0"
    "CUSTOM FONT 4\0"
    "CUSTOM FONT 5\0"
    "CUSTOM FONT 6\0"
    "CUSTOM FONT 7\0"
    "CUSTOM FONT 8\0"
    "CUSTOM FONT 9\0"
    "CUSTOM FONT 10\0"
    "CUSTOM FONT 11\0"
    "CUSTOM FONT 12\0"
    "CUSTOM FONT 13\0"
    "CUSTOM FONT 14\0"
    "CUSTOM FONT 15\0"
    "CUSTOM FONT 16\0"
    "\0";

char DELAYED_COPY_DATA(MenuVidex)[] =
    "DISABLED\0"  //0
    "US\0"        //1
    "UPPERCASE\0" //2
    "GERMAN\0"    //3
    "FRENCH\0"    //4
    "SPANISH\0"   //5
    "KATAKANA\0"  //6
    "APL\0"       //7
    "SUPER SUB\0" //8
    "EPSON\0"     //9
    "SYMBOL\0"    //10
    "\0";

char DELAYED_COPY_DATA(MenuTitle)[] = "- CONFIGURATION MENU -";

char DELAYED_COPY_DATA(MenuPage0)[] =
    "0 MACHINE TYPE:\0"
    "1 CHARACTER SET:\0"
    "2 ALTCHR SWITCH:\0"
    "3 US CHARACTER SET:\0"
    "  \0"
    "4 MONOCHROME MODE:\0"
    "5 COLOR MODE:\0"
    "6 RGB COLOR STYLE:\0"
    "7 SCAN LINES:\0"
    "8 ANALOG RENDER FX:\0"
    "\0";

char DELAYED_COPY_DATA(MenuPage1)[] =
    "0 DVI VIDEO OUTPUT:\0"
    "1 VIDEO TIMING:\0"
    "2 DEBUG MONITOR:\0"
    "  \0"
    "             APPLE IIE\0"
    "3 VIDEO7:\0"
    "4 RAMWORKS MEMORY:\0"
    "  \0"
    "            APPLE II/II+\0"
    "5 VIDEX 80 COL:\0"
    "\0";

char DELAYED_COPY_DATA(MenuExtra)[] =
    "N NEXT PAGE...\0"
    "R RESTORE DEFAULTS\0"
    "L LOAD FROM FLASH\0"
    "S SAVE TO FLASH\0"
    "A ABOUT\0"
    "B DEBUG\0"
    "T TEST\0"
    "\0";

char DELAYED_COPY_DATA(MenuExit)[] = "'ESC' TO EXIT";

char DELAYED_COPY_DATA(MenuSpecialChars)[] = "[{\\~#$`^|}]";

void DELAYED_COPY_CODE(showTitle)(TPrintMode PrintMode)
{
    // initialize the screen buffer area
    clearTextScreen();

    if (PrintMode == PRINTMODE_INVERSE)
    {
        clearLine( 0, PrintMode);
        clearLine(22, PrintMode);
        clearLine(23, PrintMode);
    }

    centerY( 0, TitleFirmware,                   PrintMode);
    centerY(22, TitleCopyright,                  PrintMode);
    centerY(23, TitleGitHub[MenuSubTitleToggle], PrintMode);
}

void DELAYED_COPY_CODE(menuOption)(uint8_t y, const char* pValue)
{
    uint x = (MenuOptionNr >= (0x10+4)) ? 20:0;
    char MenuKey[2];

    const char* pMenu;
    if (MenuOptionNr < 0x10)
    {
        pMenu = getMenuString((MenuPage==0) ? MenuPage0 : MenuPage1, y);
        y += 4;
    }
    else
    {
        pMenu = getMenuString(MenuExtra, MenuOptionNr & 0x7);
    }

    TPrintMode PrintMode = (MenuOptionNr == MenuSelection) ? PRINTMODE_FLASH : PRINTMODE_NORMAL;
    if (pMenu[0] == ' ')
    {
        PrintMode = PRINTMODE_NORMAL;
        setLineColor(y, 0xA2); // light gray, blue
    }
    else
    {
        MenuKey[0] = pMenu[0];
        MenuKey[1] = 0;
        printXY(x, y, MenuKey, PRINTMODE_INVERSE);
    }

    printXY(x+2, y, &pMenu[2], PrintMode);

    if (pValue)
    {
        printXY(20, y, pValue, PRINTMODE_NORMAL);
        MenuPageItems = MenuOptionNr+1;
    }

    if (pMenu[0] != ' ')
        MenuOptionNr++;
}

void DELAYED_COPY_CODE(menuShowFrame)()
{
    soft_switches |= SOFTSW_TEXT_MODE;
    soft_switches &= ~(SOFTSW_MIX_MODE | SOFTSW_80COL | SOFTSW_PAGE_2 | SOFTSW_DGR | SOFTSW_HIRES_MODE);

    showTitle(PRINTMODE_INVERSE);
    centerY(21, MenuExit, PRINTMODE_NORMAL);
}

//   1234567890123456789012345678901234567890
static char DELAYED_COPY_DATA(AboutText)[]=
{
    "A2DVI IS A DVI/HDMI  GRAPHICS  CARD  FOR\0" //3
    "APPLE II COMPUTERS,  GENERATING  A  TRUE\0" //4
    "DIGITAL VIDEO SIGNAL WITHOUT ANY  ANALOG\0" //5
    "CONVERSION. THE CARD MONITORS  THE  6502\0" //6
    "BUS, CREATES A SHADOW COPY OF THE  VIDEO\0" //7
    "MEMORY WITHIN ITS  RASPBERRY  PICO  MCU,\0" //8
    "THEN GENERATES 'TMDS'  ENCODED  RGB  BIT\0" //9
    "STREAMS (3X252MBIT/S) FOR DVI/HDMI.\0"      //10
    " \0"                                        //11
    "MORE: GITHUB.COM/RALLEPALAVEEV/A2DVI\0"     //12
    "   GITHUB.COM/THORSTENBR/A2DVI-FIRMWARE\0"  //13
    " \0"                                        //14
    "A2DVI IS BASED ON PROJECTS 'APPLEII VGA'\0" //15
    "(C) 2021 MARK AIKENS & DAVID KUDER,  AND\0" //16
    "ON 'PICODVI' (C) 2021 LUKE WREN.\0"         //17
    "  MANY THANKS TO ALL! APPLE II FOREVER!\0"  //18
    " \0"                                        //19
    "           THORSTEN AND RALLE\0\0"          //20
};

void DELAYED_COPY_CODE(menuVideo7Text)()
{
    if ((internal_flags & (IFLAGS_IIE_REGS|IFLAGS_VIDEO7)) == (IFLAGS_IIE_REGS|IFLAGS_VIDEO7))
    {
        // initialize colors when Video 7 is enabled (on Apple //e)
        soft_switches |= (SOFTSW_80STORE | SOFTSW_DGR);

        for (uint line=0;line<24;line++)
        {
            uint8_t color;
            switch(line)
            {
                case 0: // fall-through
                case 22:// fall-through
                case 23:color = 0x4F;break; // white, dark-green
                case 21:color = 0xA2;break; // light gray, blue
                default:color = 0xD2;break; // yellow, blue
            }
            setLineColor(line, color);
        }
    }
    else
    {
        // Video 7 is disabled or Apple II only
        soft_switches &= ~(SOFTSW_80STORE | SOFTSW_DGR);
    }
}

void DELAYED_COPY_CODE(menuShowSaved)()
{
    menuShowFrame();
    menuVideo7Text();
    centerY(11, "SAVED!", PRINTMODE_NORMAL);
}

void DELAYED_COPY_CODE(menuShowAbout)()
{
    menuShowFrame();
    menuVideo7Text();
    char* pText = AboutText;
    for (uint y=0;*pText!=0;y++)
    {
        printXY(0,2+y, pText, PRINTMODE_NORMAL);
        while (*(++pText) != 0);
        pText++;
    }
}

void DELAYED_COPY_CODE(menuShowTest)()
{
    // initialize the screen buffer area
    clearTextScreen();
    soft_switches &= ~(SOFTSW_TEXT_MODE|SOFTSW_PAGE_2|SOFTSW_80STORE|SOFTSW_DGR);
    soft_switches |=  SOFTSW_MIX_MODE | SOFTSW_80COL;

    for (uint y=0;y<40;y++)
    {
        for (uint x=0;x<40;x++)
        {
            uint8_t color = ((x/10)+(y/10)*4) & 0xf;
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
    }

    // show character table
    for (uint i=0;i<255;i++)
    {
        uint x = (i & 63) + 8;
        uint line = 20+(i/64);
        if (line > 23)
            break;
        uint32_t ScreenOffset = (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
        volatile uint8_t* pScreen =  (x & 1) ? text_p1 : text_p3;

        pScreen[ScreenOffset+x/2] = i;
    }
}

void int2str(uint32_t value, char* pStrBuf, uint32_t digits)
{
    uint8_t done = 0;
    for (int32_t i=digits-1;i>=0;i--)
    {
        pStrBuf[i] = (done) ? (' '|0x80) : (0x80|'0')+(value % 10);
        value /= 10;
        done = (value == 0);
    }
    pStrBuf[digits]=0;
}

void DELAYED_COPY_CODE(menuShowDebug)()
{
    menuShowFrame();
    menuVideo7Text();

    // show detected machine and slot
    {
        const uint8_t X1 = 3;
        const uint8_t X2 = X1+17;

        printXY(X1,   3, "DETECTED MACHINE:", PRINTMODE_NORMAL);
        // show current machine type
        printXY(X2+1, 3, (current_machine > MACHINE_MAX_CFG) ? "-" : getMenuString(MachineNames, current_machine), PRINTMODE_NORMAL);

        // show slot
        printXY(X1,   4, "DETECTED SLOT:",    PRINTMODE_NORMAL);
        char s[16];
        s[0] = 0x80 | ((cardslot == 0) ? '-' : '0'+cardslot);
        s[1] = 0;
        printXY(X2+1,4, s, PRINTMODE_NORMAL);

        // show statistics
        printXY(X1, 6, "BUS CYCLES:", PRINTMODE_NORMAL);
        int2str(bus_cycle_counter, s, 14);
        printXY(X2, 6, s, PRINTMODE_NORMAL);

        printXY(X1,7, "BUS OVERFLOWS:", PRINTMODE_NORMAL);
        int2str(bus_overflow_counter, s, 14);
        printXY(X2, 7, s, PRINTMODE_NORMAL);

        printXY(X1, 8, "FRAME COUNTER:", PRINTMODE_NORMAL);
        int2str(frame_counter, s, 14);
        printXY(X2, 8, s, PRINTMODE_NORMAL);

        printXY(X1, 9, "SCALINE ERRORS:", PRINTMODE_NORMAL);
        int2str(a2dvi_scanline_errors(), s, 14);
        printXY(X2, 9, s, PRINTMODE_NORMAL);

        printXY(X1,10, "RESET COUNTER:", PRINTMODE_NORMAL);
        int2str(reset_counter, s, 14);
        printXY(X2,10, s, PRINTMODE_NORMAL);

        printXY(X1,11, "DEV REG ACCESS:", PRINTMODE_NORMAL);
        int2str(devicereg_counter, s, 14);
        printXY(X2,11, s, PRINTMODE_NORMAL);

        printXY(X1,12, "DEV MEM ACCESS:", PRINTMODE_NORMAL);
        int2str(devicemem_counter, s, 14);
        printXY(X2,12, s, PRINTMODE_NORMAL);

        printXY(X1,15, "AVAILABLE MEMORY:", PRINTMODE_NORMAL);
        int2str(getFreeHeap(), s, 14);
        printXY(X2,15, s, PRINTMODE_NORMAL);

        printXY(X1,16, "BOOT TIME (US):", PRINTMODE_NORMAL);
        int2str(boot_time, s, 14);
        printXY(X2, 16, s, PRINTMODE_NORMAL);

#if 0
        printXY(X1,17, "IFLAGS:", PRINTMODE_NORMAL);
        int2str(internal_flags, s, 8);
        printXY(X2, 17, s, PRINTMODE_NORMAL);

        printXY(X1,18, "SWFLAGS:", PRINTMODE_NORMAL);
        int2str(soft_switches, s, 8);
        printXY(X2, 18, s, PRINTMODE_NORMAL);
#endif
    }
}

bool DELAYED_COPY_CODE(menuDoSelectionPage0)(bool increase)
{
    switch(MenuSelection)
    {
        case 0: // MACHINE TYPE
            if (increase)
            {
                if (cfg_machine < MACHINE_MAX_CFG)
                    cfg_machine++;
            }
            else
            {
                if (cfg_machine > 0)
                    cfg_machine--;
            }
            // update current machine type
            set_machine((cfg_machine == MACHINE_AUTO) ? detected_machine : cfg_machine);
            break;
        case 1: // CHARACTER SET
            if (increase)
            {
                if (cfg_local_charset+1 < MAX_FONT_COUNT)
                {
                    cfg_local_charset++;
                    reload_charsets |= 1;
                }
            }
            else
            {
                if (cfg_local_charset > 0)
                {
                    cfg_local_charset--;
                    reload_charsets |= 1;
                }
            }
            break;
        case 2: // LANGUAGE SWITCH
            if (increase)
            {
                if (input_switch_mode < ModeSwitchLangCycle)
                    input_switch_mode++;
            }
            else
            {
                if (input_switch_mode > ModeSwitchDisabled)
                    input_switch_mode--;
            }
            if (!LANGUAGE_SWITCH_ENABLED())
            {
                language_switch = false;
            }
            break;
        case 3: // US FONT
            if (!LANGUAGE_SWITCH_ENABLED())
                break;
            // only show US character set for alternate (alternate was always US/default ASCII)
            if (increase)
            {
                if (cfg_alt_charset == 0)
                {
                    cfg_alt_charset = 9;
                    reload_charsets |= 2;
                }
                else
                if ((cfg_alt_charset >= 9)&&(cfg_alt_charset<11))
                {
                    cfg_alt_charset++;
                    reload_charsets |= 2;
                }
                else
                if (cfg_alt_charset == 11)
                {
                    cfg_alt_charset = MAX_FONT_COUNT-CUSTOM_FONT_COUNT;
                    reload_charsets |= 2;
                }
                else
                if ((cfg_alt_charset >= MAX_FONT_COUNT-CUSTOM_FONT_COUNT)&&(cfg_alt_charset < MAX_FONT_COUNT-1))
                {
                    cfg_alt_charset++;
                    reload_charsets |= 2;
                }
            }
            else
            {
                if (cfg_alt_charset > MAX_FONT_COUNT-CUSTOM_FONT_COUNT)
                {
                    cfg_alt_charset--;
                    reload_charsets |= 2;
                }
                else
                if (cfg_alt_charset == MAX_FONT_COUNT-CUSTOM_FONT_COUNT)
                {
                    cfg_alt_charset = 11;
                    reload_charsets |= 2;
                }
                else
                if ((cfg_alt_charset >= 10)&&(cfg_alt_charset <= 11))
                {
                    cfg_alt_charset--;
                    reload_charsets |= 2;
                }
                else
                if (cfg_alt_charset == 9)
                {
                    cfg_alt_charset = 0;
                    reload_charsets |= 2;
                }
            }
            break;
        case 4: // MONOCHROME MODE
            if (increase)
            {
                if (color_mode < 2)
                    color_mode++;
            }
            else
            {
                if (color_mode > 0)
                    color_mode--;
            }
            break;
        case 5: // COLOR vs MONOCHROME
            SET_IFLAG(!IS_IFLAG(IFLAGS_FORCED_MONO), IFLAGS_FORCED_MONO);
            break;
        case 6: // COLOR STYLE
            if (increase)
            {
                if (cfg_color_style < 2)
                {
                    cfg_color_style++;
                    reload_colors = true;
                }
            }
            else
            {
                if (cfg_color_style > 0)
                {
                    cfg_color_style--;
                    reload_colors = true;
                }
            }
            break;
        case 7: // SCANLINE EMULATION
            SET_IFLAG(!IS_IFLAG(IFLAGS_SCANLINEEMU), IFLAGS_SCANLINEEMU);
            break;
        case 8: // RENDERING
            if (increase)
            {
                if (cfg_rendering_fx < FX_DGR_ONLY)
                    cfg_rendering_fx++;
            }
            else
            {
                if (cfg_rendering_fx > 0)
                    cfg_rendering_fx--;
            }
            config_setflags();
            break;
    }
    return false;
}

bool DELAYED_COPY_CODE(menuDoSelectionPage1)(bool increase)
{
    switch(MenuSelection)
    {
        case 0: //640x480 vs 720x480
            cfg_video_mode ^= 1;
            cfg_video_mode |= 0x10;
            break;
        case 1: // PAL vs NTSC
            SET_IFLAG(!IS_IFLAG(IFLAGS_PAL), IFLAGS_PAL);
            break;
        case 2: // SCANLINES
            SET_IFLAG(!IS_IFLAG(IFLAGS_DEBUG_LINES), IFLAGS_DEBUG_LINES);
            SET_IFLAG(0, IFLAGS_TEST);
            break;
        case 3: // VIDEO7
            SET_IFLAG(!IS_IFLAG(IFLAGS_VIDEO7), IFLAGS_VIDEO7);
            if (IS_IFLAG(IFLAGS_VIDEO7))
            {
                soft_switches |= SOFTSW_V7_MODE3;
            }
            break;
        case 4: // RAMWORKS
            SET_IFLAG(!IS_IFLAG(IFLAGS_RAMWORKS), IFLAGS_RAMWORKS);
            break;
        case 5: // VIDEX
            if (increase)
            {
                if (cfg_videx_selection < VIDEX_FONT_COUNT)
                {
                    cfg_videx_selection++;
                    reload_charsets |= 4;
                }
            }
            else
            {
                if (cfg_videx_selection > 0)
                {
                    cfg_videx_selection--;
                    if (cfg_videx_selection > 0)
                        reload_charsets |= 4;
                }
            }
            // videx not supported on IIe/IIgs
            videx_enabled = (cfg_videx_selection>0)&&((internal_flags & (IFLAGS_IIGS_REGS|IFLAGS_IIE_REGS)) == 0);
            break;
    }
    return false;
}

bool DELAYED_COPY_CODE(menuDoSelection)(bool increase)
{
    if (MenuSelection<9)
    {
        return (MenuPage == 0) ? menuDoSelectionPage0(increase) : menuDoSelectionPage1(increase);
    }

    switch(MenuSelection)
    {
        case MENU_EXTRA+0: // next page
            MenuNeedsRedraw = 1;
            MenuPage ^= 1;
            MenuSelection = 0;
            break;
        case MENU_EXTRA+1: // restore
            if (increase)
            {
                config_load_defaults();
                set_machine((cfg_machine == MACHINE_AUTO) ? detected_machine : cfg_machine);
            }
            break;
        case MENU_EXTRA+2: // load config
            if (increase)
            {
                config_load();
                set_machine((cfg_machine == MACHINE_AUTO) ? detected_machine : cfg_machine);
            }
            break;
        case MENU_EXTRA+3: // flash
            if (increase)
            {
                config_save();
                menuShowSaved();
                IgnoreNextKeypress = true;
                return true;
            }
            break;
        case MENU_EXTRA+4:  // about
            if (increase)
            {
                menuShowAbout();
                IgnoreNextKeypress = true;
                return true;
            }
            break;
        case MENU_EXTRA+5:
            if (increase)
            {
                menuShowDebug();
                IgnoreNextKeypress = true;
                return true;
            }
            break;
        case MENU_EXTRA+6: // test
            if (increase)
            {
                menuShowTest();
                IgnoreNextKeypress = true;
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

static inline bool menuCheckKeys(char key)
{
    int Cmd = -1;

    // uppercase
    if ((key>='a')&&(key<='z'))
    {
        key = (key-'a')+'A';
    }

    switch(key)
    {
        // MENU ELEMENT SELECTION KEYS
        case '0' ... '9':
            MenuSelection = key-'0';
            if ((!LANGUAGE_SWITCH_ENABLED())&&(MenuSelection == 3)&&(MenuPage == 0))
                MenuSelection = 2;
            if (MenuSelection >= MenuPageItems)
            {
                MenuSelection = MenuPageItems-1;
            }
            break;
        case 'N':
            MenuSelection = MENU_EXTRA;
            Cmd = 1;
            break;
        case 'R':
            MenuSelection = MENU_EXTRA+1;
            Cmd = 1;
            break;
        case 'L':
            MenuSelection = MENU_EXTRA+2;
            Cmd = 1;
            break;
        case 'S':
            MenuSelection = MENU_EXTRA+3;
            Cmd = 1;
            break;
        case 'A':
            MenuSelection = MENU_EXTRA+4;
            Cmd = 1;
            break;
        case 'B':
            MenuSelection = MENU_EXTRA+5;
            Cmd = 1;
            break;
        case 'T':
            MenuSelection = MENU_EXTRA+6;
            Cmd = 1;
            break;

        // ARROW & CURSOR MOVEMENT KEYS
        case 'I':// fall through
        case 11: // UP
            if (MenuSelection > 0)
            {
                if (MenuSelection == MENU_EXTRA)
                    MenuSelection = MenuPageItems-1;
                else
                {
                    MenuSelection--;
                    if ((!LANGUAGE_SWITCH_ENABLED())&&(MenuSelection == 3)&&(MenuPage==0))
                        MenuSelection--;
                }
            }
            else
                MenuSelection = MENU_EXTRA+6;
            break;
        case 'M':// fall through
        case 9:  // TAB
        case 10: // DOWN
            if (MenuSelection < MenuPageItems-1)
            {
                MenuSelection++;
                if ((!LANGUAGE_SWITCH_ENABLED())&&(MenuSelection == 3)&&(MenuPage==0))
                    MenuSelection++;
            }
            else
            if (MenuSelection < MENU_EXTRA)
            {
                MenuSelection = MENU_EXTRA;
            }
            else
            if (MenuSelection < MENU_EXTRA+6)
                MenuSelection++;
            else
                MenuSelection = 0;
            break;
        case 'J':// fall-through
        case 127: // DEL
        case 8:   //LEFT
            if (MenuSelection <= MENU_EXTRA)
            {
                Cmd = 0;
            }
            else
            if (MenuSelection > MENU_EXTRA+3)
            {
                MenuSelection -= 3;
            }
            break;
        case 13: // fall-through
        case ' ':
            Cmd = 1;
            break;
        case 'K':// fall-through
        case 21: //RIGHT
            if (MenuSelection <= MENU_EXTRA)
            {
                Cmd = 1;
            }
            else
            if (MenuSelection+3 <= MENU_EXTRA+6)
            {
                MenuSelection += 3;
            }
            break;
        case 27: // ESCAPE
            // clear menu, Video 7 and MIX modes when exiting, enable TEXT mode
            soft_switches &= ~(SOFTSW_MENU_ENABLE | SOFTSW_MIX_MODE | SOFTSW_80COL | SOFTSW_80STORE | SOFTSW_DGR);
            soft_switches |= SOFTSW_TEXT_MODE;
            // abort the menu: do not redraw
            return true;
        case 22: // CTRL-V, shortcut to directly toggle 640x480 vs 720x480
        {
            cfg_video_mode ^= 1;
            cfg_video_mode |= 0x10;
            break;
        }
        case '!': // special debug feature
            if (IS_IFLAG(IFLAGS_DEBUG_LINES))
            {
                SET_IFLAG(!IS_IFLAG(IFLAGS_TEST), IFLAGS_TEST);
            }
            break;
        default:
            break;
    }

    if (Cmd != -1)
    {
        MenuNeedsRedraw = true;
        return menuDoSelection(Cmd == 1);
    }

    return false;
}

void DELAYED_COPY_CODE(menuShow)(char key)
{
    if (key == 0)
    {
        // reset command
        MenuSelection = 0;
        MenuPage = 0;
        MenuNeedsRedraw = true;
        IgnoreNextKeypress = false;
        MenuSubTitleToggle = (bus_cycle_counter & 1);
    }
    else
    if (key == 1)
    {
        MenuNeedsRedraw = true;
    }

    // never ignore the ESC key: the host utility always exits on ESC
    if ((IgnoreNextKeypress)&&(key != 27))
    {
        IgnoreNextKeypress = false;
    }
    else
    if (menuCheckKeys(key))
    {
        /* Yes, the menu stuff is too slow for the bus cycle loop. But
         * it doesn't matter here, just wipe the FIFO. */
        abus_clear_fifo();
        return;
    }

    if (MenuNeedsRedraw)
    {
        menuShowFrame();
        menuVideo7Text();
        setLineColor(2, 0xF2); // white, blue
        MenuNeedsRedraw = false;
    }

    MenuOptionNr = 0;
    centerY(2, MenuTitle, PRINTMODE_NORMAL);
    printXY(36, 2, (MenuPage==0) ? "1/2" : "2/2", PRINTMODE_NORMAL);

    uint Y=0; // y position
    if (MenuPage == 0)
    {
        //                0123456789012345678
        menuOption(Y++,  getMenuString(MachineNames, (cfg_machine <= MACHINE_MAX_CFG) ? cfg_machine : 0));
        menuOption(Y++,  (cfg_local_charset < MAX_FONT_COUNT) ? getMenuString(MenuFontNames, cfg_local_charset) : "?");
        menuOption(Y++,  getMenuString(MenuButtonModes, input_switch_mode));
        if ((input_switch_mode == ModeSwitchLanguage)||
            (input_switch_mode == ModeSwitchLangMonochrome)||
            (input_switch_mode == ModeSwitchLangCycle))
        {
            menuOption(Y, (cfg_alt_charset < MAX_FONT_COUNT) ? getMenuString(MenuFontNames, cfg_alt_charset) : "?");
        }
        else
        {
            MenuOptionNr++;
        }
        Y++;
        menuOption(Y++, 0);
        menuOption(Y++, MenuColorMode[color_mode]);
        menuOption(Y++, getMenuString(MenuForcedMono, IS_IFLAG(IFLAGS_FORCED_MONO)));
        menuOption(Y++, getMenuString(MenuColorStyle, cfg_color_style));
        menuOption(Y++, getMenuString(MenuOnOff, IS_IFLAG(IFLAGS_SCANLINEEMU)));
        menuOption(Y++, getMenuString(MenuRendering, cfg_rendering_fx));
        Y++;

        menuOption(Y+1, 0);
    }
    else
    {
        menuOption(Y++, getMenuString(MenuVideoMode, cfg_video_mode & 1));
        menuOption(Y++, getMenuString(MenuPalNtsc, IS_IFLAG(IFLAGS_PAL)));
        menuOption(Y++, getMenuString(MenuOnOff, IS_IFLAG(IFLAGS_DEBUG_LINES)));
        menuOption(Y++, 0);
        menuOption(Y++, 0);
        menuOption(Y++, getMenuString(MenuOnOff, IS_IFLAG(IFLAGS_VIDEO7)));
        menuOption(Y++, getMenuString(MenuOnOff, IS_IFLAG(IFLAGS_RAMWORKS)));
        menuOption(Y++, 0);
        menuOption(Y++, 0);
        menuOption(Y++, getMenuString(MenuVidex, cfg_videx_selection));
    }

    // show extra menu items
    MenuOptionNr = 0x10;
    menuOption(15, 0);

    menuOption(17, 0);
    menuOption(18, 0);
    menuOption(19, 0);

    menuOption(17, 0);
    menuOption(18, 0);
    menuOption(19, 0);

    if (MenuPage == 0)
    {
        // show some special characters, for immediate feedback when selecting character sets
        printXY(40-11, 21, MenuSpecialChars, PRINTMODE_NORMAL);
    }

    /* We're drawing the menu inside the bus cycle loop. That's too slow, of course.
     * But it doesn't matter, since we know the config utility isn't doing anything
     * interesting in the bus cycles immediately after writing the menu register.
     * So we're good to just ignore a few cycles. But we wipe the FIFO to simplify
     * testing, so we keep the statistics clean (overflow counter). Otherwise real/serious
     * bus FIFO overflows could get lost in the flood of expected/accepted overflows.
     */
    abus_clear_fifo();
}
