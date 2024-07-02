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

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "applebus/buffers.h"
#include "config/config.h"
#include "fonts/textfont.h"
#include "menu.h"

#ifdef FEATURE_TEST
bool PrintMode80Column = false;
bool PrintModePage2    = false;
#endif

void __time_critical_func(centerY)(uint32_t y, const char* pMsg, TPrintMode PrintMode)
{
    uint32_t x = 0;
    while (pMsg[x] != 0)
        x++;
    x = 20-(x>>1);
    printXY(x, y, pMsg, PrintMode);
}

void __time_critical_func(printXY)(uint32_t x, uint32_t line, const char* pMsg, TPrintMode PrintMode)
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

void __time_critical_func(clearTextScreen)(void)
{
    // initialize the screen buffer area
    for (uint32_t i=0;i<40*26/4;i++)
    {
#ifndef FEATURE_TEST
        ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
#else
        if (!PrintModePage2)
        {
            ((uint32_t*)text_p1)[i] = 0xA0A0A0A0; // initialize with blanks
            // for 80columns in test mode
            ((uint32_t*)text_p3)[i] = 0xA0A0A0A0; // initialize with blanks
        }
        else
        {
            ((uint32_t*)text_p2)[i] = 0xA0A0A0A0; // initialize with blanks
            // for 80columns in test mode
            ((uint32_t*)text_p4)[i] = 0xA0A0A0A0; // initialize with blanks
        }
#endif
    }
}

void clearLine(uint8_t line, TPrintMode PrintMode)
{
    uint32_t ScreenOffset = (((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40));
    char blank = (PrintMode == PRINTMODE_NORMAL) ? ' '|0x80 : ' ';
    for (uint32_t i=0;i<40;i++)
    {
        text_p1[ScreenOffset+i] = blank; // initialize with blanks
    }
}

void __time_critical_func(showTitle)(TPrintMode PrintMode)
{
    // initialize the screen buffer area
    clearTextScreen();

    if (PrintMode == PRINTMODE_INVERSE)
    {
        clearLine(0, PrintMode);
        clearLine(22, PrintMode);
        clearLine(23, PrintMode);
    }


    centerY(0,  "A2DVI - FIRMWARE V" FW_VERSION, PrintMode);

    centerY(22, "(C) 2024 T.BREHM, R.PALAVEEV ET AL.", PrintMode);
    centerY(23, "GITHUB.COM/RALLEPALAVEEV/A2DVI", PrintMode);
    //centerY(23, "GITHUB.COM/THORSTENBR/A2DVI-FIRMWARE", PrintMode);
}

const char* MachineNames[MACHINE_MAX_CFG+1] =
{
    "APPLE II",
    "APPLE IIE",
    "APPLE IIGS",
    "AGAT7",
    "AGAT9",
    "BASIS",
    "PRAVETZ"
};

const char* MenuOnOff[2] =
{
    "DISABLED",
    "ENABLED"
};

const char* MenuColorMode[COLOR_MODE_AMBER+1] =
{
    "BLACK & WHITE",
    "GREEN",
    "AMBER"
};

const char* MenuForcedMono[2] =
{
    "COLOR",
    "MONOCHROME"
};

const char* MenuFontNames[MAX_CFG_FONT+1] =
{
    "IIE US", //0
    "IIE UK", //1
    "IIE FRENCH", //2
    "IIE GERMAN", //3
    "IIE SPANISH", //4
    "IIE ITALIAN", //5
    "IIE SWEDISH/FINNISH", //6
    "IIE HEBREW", //7
    "PRAVETZ CYRILLIC", //8
    "IIE US REACTIVE", //9
    "IIE US UNENHANCED", //10
    "II+ US", //11
    "II+ VIDEX LOWER 1",//12
    "II+ VIDEX LOWER 2",//13
    "II+ PIG FONT",//14
    "II+ JAPAN KATAKANA"//15
};

static uint8_t CurrentMenu = 0;

void menuOption(uint8_t y, uint8_t Selection, const char* pMenu, const char* pValue)
{
    printXY( 0, y, pMenu, (Selection == CurrentMenu) ? PRINTMODE_FLASH : PRINTMODE_NORMAL);
    if (pValue)
        printXY(20, y, pValue, PRINTMODE_NORMAL);
}

void showMenuFrame()
{
    showTitle(PRINTMODE_INVERSE);
    centerY(20, "'ESC' TO EXIT", PRINTMODE_NORMAL);
}

bool doMenuSelection(bool increase)
{
    switch(CurrentMenu)
    {
        case 1:
            if (increase)
            {
                if (cfg_machine == MACHINE_AUTO)
                    cfg_machine = 0;
                else
                if (cfg_machine < MACHINE_MAX_CFG)
                    cfg_machine++;
                if (cfg_machine == MACHINE_IIGS) // skip IIGS, since currently unsupported
                    cfg_machine++;
            }
            else
            {
                if (cfg_machine == 0)
                    cfg_machine = MACHINE_AUTO;
                else
                if (cfg_machine != MACHINE_AUTO)
                    cfg_machine--;
                if (cfg_machine == MACHINE_IIGS) // skip IIGS, since currently unsupported
                    cfg_machine--;
            }
            // update current machine type
            set_machine((cfg_machine == MACHINE_AUTO) ? detected_machine : cfg_machine);
            break;
        case 2:
            if (increase)
            {
                if (cfg_local_charset+1 <= MAX_CFG_FONT)
                    cfg_local_charset++;
            }
            else
            {
                if (cfg_local_charset > 0)
                    cfg_local_charset--;
            }
            break;
        case 3:
            language_switch_enabled = !language_switch_enabled;
            break;
        case 4:
            // only show US character set for alternate (alternate was always US/default ASCII)
            if (increase)
            {
                if (cfg_alt_charset == 0)
                    cfg_alt_charset = 9;
                else
                if ((cfg_alt_charset >= 9)&&(cfg_alt_charset<11))
                    cfg_alt_charset++;
            }
            else
            {
                if ((cfg_alt_charset >= 10)&&(cfg_alt_charset <= 11))
                    cfg_alt_charset--;
                else
                if (cfg_alt_charset == 9)
                    cfg_alt_charset = 0;
            }
            break;
        case 5:
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
        case 6:
            SET_IFLAG(!IS_IFLAG(IFLAGS_FORCED_MONO), IFLAGS_FORCED_MONO);
            break;
        case 7:
            SET_IFLAG(!IS_IFLAG(IFLAGS_SCANLINEEMU), IFLAGS_SCANLINEEMU);
            break;
        case 8:
            SET_IFLAG(!IS_IFLAG(IFLAGS_VIDEO7), IFLAGS_VIDEO7);
            break;
        case 9:  // about
            if (increase)
            {
                centerY(3,  "ABOUT A2DVI", PRINTMODE_NORMAL);
                centerY(7,  "......", PRINTMODE_NORMAL);
                centerY(11, "APPLE II FOREVER!", PRINTMODE_NORMAL);
                SET_IFLAG(0, IFLAGS_MENU_ENABLE);
                return true;
            }
            break;
        case 10: // restore
            if (increase)
            {
                config_load_defaults();
                set_machine((cfg_machine == MACHINE_AUTO) ? detected_machine : cfg_machine);
            }
            break;
        case 11: // flash
            if (increase)
            {
                config_save();
                showMenuFrame();
                centerY(11, "SAVED!", PRINTMODE_NORMAL);
                // clear menu mode when exiting
                SET_IFLAG(0, IFLAGS_MENU_ENABLE);
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void showMenu(char key)
{
    char s[10];
    showMenuFrame();

    // uppercase
    if ((key>='a')&&(key<='z'))
    {
        key = (key-'a')+'A';
    }

    int Cmd = -1;
    switch(key)
    {
        case 0: // reset command
            CurrentMenu = 1;
            break;
        case '1' ... '8':
            CurrentMenu = key-'1';
            break;
        case 'I':// fall through
        case 11: // UP
            if (CurrentMenu>1)
                CurrentMenu--;
            else
                CurrentMenu = 11;
            break;
        case 'M':// fall through
        case 9: // TAB
        case 10: // DOWN
            if (CurrentMenu<11)
                CurrentMenu++;
            else
                CurrentMenu = 1;
            break;
        case 'A':
            CurrentMenu = 9;
            Cmd = 1;
            break;
        case 'R':
            CurrentMenu = 10;
            Cmd = 1;
            break;
        case 'S':
            CurrentMenu = 11;
            Cmd = 1;
            break;
        case 'J':// fall-through
        case 127: // DEL
        case 8: //LEFT
            Cmd = 0;
            break;
        case 13: // fall-through
        case ' ':// fall-through
        case 'K':// fall-through
        case 21: //RIGHT
            Cmd = 1;
            break;
        case 27:
            // clear menu mode when exiting
            SET_IFLAG(0, IFLAGS_MENU_ENABLE);
            break;
        default:
            break;
    }

    if (CurrentMenu == 0)
    {
        CurrentMenu = 1;
    }

    if (Cmd != -1)
    {
        if (doMenuSelection(Cmd == 1))
            return;
    }

    {
        const uint8_t SlotX = 12;

        printXY(SlotX,2, "SLOT=", PRINTMODE_NORMAL);

        // show slot
        s[0] = 0x80 | ((cardslot == 0) ? '-' : '0'+cardslot);
        s[1] = 0;
        printXY(SlotX+5,2, s, PRINTMODE_NORMAL);

        // show current machine type
        if (current_machine > MACHINE_MAX_CFG)
            printXY(SlotX+8, 2, "-", PRINTMODE_NORMAL);
        else
            printXY(SlotX+8, 2, MachineNames[current_machine], PRINTMODE_NORMAL);
    }

    centerY(4, "- CONFIGURATION MENU -", PRINTMODE_NORMAL);

    menuOption(6,1, "1 MACHINE TYPE:",      (cfg_machine <= MACHINE_MAX_CFG) ? MachineNames[cfg_machine] : "AUTO DETECT");
    menuOption(7,2, "2 CHARACTER SET:",     (cfg_local_charset <= MAX_CFG_FONT) ? MenuFontNames[cfg_local_charset] : "?");
    menuOption(8,3, "3 ALT LANG SWITCH:",   MenuOnOff[language_switch_enabled]);
    menuOption(9,4, "4 ALT LANGUAGE:",      (cfg_alt_charset <= MAX_CFG_FONT) ? MenuFontNames[cfg_alt_charset] : "?");

    menuOption(11,5, "5 MONOCHROME COLOR:", MenuColorMode[color_mode]);
    menuOption(12,6, "6 COLOR MODES:",      MenuForcedMono[IS_IFLAG(IFLAGS_FORCED_MONO)]);
    menuOption(13,7, "7 SCANLINES:",        MenuOnOff[IS_IFLAG(IFLAGS_SCANLINEEMU)]);
    menuOption(14,8, "8 VIDEO7 MODES:",     MenuOnOff[IS_IFLAG(IFLAGS_VIDEO7)]);

    menuOption(16,9,  "A ABOUT", 0);
    menuOption(17,10, "R RESTORE DEFAULTS", 0);
    menuOption(18,11, "S SAVE TO FLASH", 0);
}
