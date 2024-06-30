#pragma once

#include <stdint.h>

#ifdef APPLE_MODEL_IIPLUS
    #define DEFAULT_LOCAL_CHARACTER_ROM textfont_iiplus_us
    #define DEFAULT_ALT_CHARACTER_ROM   textfont_iiplus_us
#else
    #define DEFAULT_LOCAL_CHARACTER_ROM textfont_iie_de_enhanced
    #define DEFAULT_ALT_CHARACTER_ROM   textfont_iie_us_enhanced
#endif

extern const uint8_t *character_roms[16];

#ifdef APPLE_MODEL_IIE
extern const uint8_t textfont_iie_us_enhanced[256 * 8];
extern const uint8_t textfont_iie_us_unenhanced[256 * 8];
extern const uint8_t textfont_iie_us_reactive[256 * 8];
extern const uint8_t textfont_iie_uk_enhanced[256 * 8];
extern const uint8_t textfont_iie_fr_ca_enhanced[256 * 8];
extern const uint8_t textfont_iie_de_enhanced[256 * 8];
extern const uint8_t textfont_iie_spanish_enhanced[256 * 8];
extern const uint8_t textfont_iie_it_enhanced[256 * 8];
extern const uint8_t textfont_iie_hebrew_enhanced[256 * 8];
extern const uint8_t textfont_iie_se_fi_enhanced[256 * 8];
#endif

#ifdef APPLE_MODEL_IIPLUS
extern const uint8_t textfont_iiplus_us[256 * 8];
extern const uint8_t textfont_iiplus_videx_lowercase1[256 * 8];
extern const uint8_t textfont_iiplus_videx_lowercase2[256 * 8];
extern const uint8_t textfont_iiplus_pigfont[256 * 8];
extern const uint8_t textfont_iiplus_jp_katakana[256 * 8];
#endif

extern const uint8_t textfont_clone_pravetz_cyrillic[256 * 8];

/////////////////////////////////////////////////
#if 0
extern const uint8_t videx_normal[128 * 16];
extern const uint8_t videx_inverse[128 * 16];
#endif
