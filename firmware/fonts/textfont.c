#include "textfont.h"

#define DEFAULT_ALT_CHARACTER_ROM textfont_iie_us_enhanced

const uint8_t *character_roms[24] = {
    textfont_iie_us_enhanced,
    textfont_iie_uk_enhanced,
    textfont_iie_fr_ca_enhanced,
    textfont_iie_de_enhanced,
    textfont_iie_spanish_enhanced,
    textfont_iie_it_enhanced,
    textfont_iie_hebrew_enhanced,
    textfont_iie_se_fi_enhanced,
    textfont_clone_pravetz_cyrillic,
    textfont_iie_us_reactive,
    textfont_iie_us_unenhanced,
    textfont_iiplus_us,
    textfont_iiplus_videx_lowercase1,
    textfont_iiplus_videx_lowercase2,
    textfont_iiplus_pigfont,
    textfont_iiplus_jp_katakana,

    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
    DEFAULT_ALT_CHARACTER_ROM,
};
