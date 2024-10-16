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

#include "tmds.h"
#include "config/config.h"
#include "util/dmacopy.h"

uint32_t __attribute__((section (".appledata."))) tmds_dhgr_red[16*16];
uint32_t __attribute__((section (".appledata."))) tmds_dhgr_green[16*16];
uint32_t __attribute__((section (".appledata."))) tmds_dhgr_blue[16*16];

// TMDS symbols for DHGR RGB colors - for each two pixel combination
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
const uint32_t __in_flash("chr_rom") tmds_dhgr_default[16*16*3] = {
 // red
 0x7fd00, 0xc0de6, 0x7fd00, 0xc01fb, 0xc0e67, 0x7f940, 0xc0ebc, 0xc05de,
 0xc06de, 0xc0dbc, 0x7f980, 0xc0d67, 0xc02fb, 0x402ff, 0xc0ee6, 0x402ff,
 0xf9d01, 0x465e6, 0xf9d01, 0x461f9, 0x79931, 0xf9d40, 0x791e8, 0x461de,
 0x462de, 0x79943, 0xf9d80, 0x79998, 0xf9a05, 0xf9e01, 0x466e6, 0xf9e01,
 0x7fd00, 0xc0de6, 0x7fd00, 0xc01fb, 0xc0e67, 0x7f940, 0xc0ebc, 0xc05de,
 0xc06de, 0xc0dbc, 0x7f980, 0xc0d67, 0xc02fb, 0x402ff, 0xc0ee6, 0x402ff,
 0xfed00, 0xc15e6, 0xfed00, 0x411fb, 0x7c133, 0x7ed40, 0x7c1e8, 0xc11de,
 0xc12de, 0x7c2e8, 0x7ed80, 0xc1567, 0x412fb, 0xfee00, 0xc16e6, 0xfee00,
 0xf3d01, 0x4c5e6, 0xf3d01, 0x261f9, 0x4cd33, 0x2623f, 0x4cde8, 0x261de,
 0x262de, 0x4cee8, 0xf3d80, 0x4ce33, 0x262fa, 0xf3e01, 0x4c6e6, 0xf3e01,
 0xefd00, 0xd05e6, 0xefd00, 0x501fb, 0x8f133, 0x6fd40, 0x8f1e8, 0xd01de,
 0xd02de, 0x8f2e8, 0x6fd80, 0x8f233, 0x502fb, 0xefe00, 0xd06e6, 0xefe00,
 0x105fe, 0x90de6, 0x105fe, 0x105fb, 0x7a133, 0xfb940, 0x7a1e8, 0x10dde,
 0x90edc, 0x7a2e8, 0xfb980, 0x7a233, 0x106fb, 0x106fe, 0x7a2e4, 0x106fe,
 0xf7d00, 0xc85e6, 0xf7d00, 0x481fb, 0xc8667, 0xf7940, 0x77243, 0x485de,
 0x486de, 0x77143, 0xf7980, 0xc8567, 0x482fb, 0xf7e00, 0xc86e6, 0xf7e00,
 0x085fe, 0xb7918, 0x085fe, 0x085fb, 0xb7131, 0x085bf, 0x37a43, 0x885de,
 0x886de, 0xb7143, 0x0857f, 0xb7198, 0x086fb, 0x086fe, 0xb7a18, 0x086fe,
 0xef901, 0x50de6, 0xef901, 0x505fa, 0xba133, 0xef940, 0xba1e8, 0x505de,
 0x50edc, 0xba2e8, 0xef980, 0xba233, 0xef205, 0xefa01, 0x50ee6, 0xefa01,
 0xdfd00, 0xe05e6, 0xdfd00, 0x601fb, 0xe0667, 0x5fd40, 0x1fa43, 0xe01de,
 0xe02de, 0x1f943, 0x5fd80, 0xe0567, 0x602fb, 0xdfe00, 0xe06e6, 0xdfe00,
 0x0c1fe, 0x661e6, 0x0c1fe, 0x0c1fb, 0x8cd33, 0x0c1bf, 0x8cde8, 0x0c5de,
 0x662dc, 0x8cee8, 0x0c17f, 0x8ce33, 0xd9e05, 0x0c2fe, 0x662e6, 0x0c2fe,
 0x011ff, 0x3ed18, 0x011ff, 0x811fb, 0x3e931, 0xbed40, 0xbc1e8, 0x815de,
 0x816de, 0x3e943, 0xbed80, 0x3e998, 0x812fb, 0x012ff, 0x81ee6, 0x012ff,
 0xbfd00, 0x3f119, 0xbfd00, 0x3fd04, 0x3f298, 0x3fd40, 0x3f243, 0x3fd20,
 0x3fe20, 0x3f143, 0x3fd80, 0x3f231, 0x3fe04, 0xbfe00, 0x80ee7, 0xbfe00,
 0x061fe, 0x865e6, 0x061fe, 0x061fb, 0xb9133, 0x0663f, 0xb91e8, 0x065de,
 0x066de, 0xb92e8, 0x0657e, 0xb9233, 0x062fb, 0x062fe, 0x866e6, 0x062fe,
 0xbfd00, 0x3f119, 0xbfd00, 0x3fd04, 0x3f298, 0x3fd40, 0x3f243, 0x3fd20,
 0x3fe20, 0x3f143, 0x3fd80, 0x3f231, 0x3fe04, 0xbfe00, 0x80ee7, 0xbfe00,
 // green
 0x7fd00, 0xc0de6, 0x7f980, 0xc059f, 0xc0e67, 0x7f940, 0xc0eec, 0xc06f9,
 0xc01fd, 0xc05ee, 0x7f980, 0xc0d67, 0xc069f, 0xc057d, 0xc0ee6, 0x402ff,
 0xf9d01, 0x465e6, 0xf9d80, 0x4619f, 0x79931, 0xf9d40, 0x466ec, 0x79a07,
 0x79907, 0x465ec, 0xf9d80, 0x79998, 0x4629f, 0x79986, 0x466e6, 0xf9e01,
 0xdfd00, 0xe05e6, 0x5fd80, 0xe019f, 0xe0667, 0x5fd40, 0xe06ec, 0x602fd,
 0x601fd, 0xe05ec, 0x5fd80, 0xe0567, 0xe029f, 0xe017d, 0xe06e6, 0xdfe00,
 0x081ff, 0xd81e6, 0xe7d80, 0x5819f, 0xd8267, 0xe7d40, 0xd82ec, 0xd82f8,
 0xd81f8, 0xd81ec, 0xe7d80, 0xd8167, 0x5829f, 0xd8179, 0xd82e6, 0x882fe,
 0xf3d01, 0x4c5e6, 0xf3d80, 0x2619f, 0x4cd33, 0x2623f, 0x4c6ec, 0x99e07,
 0x99d07, 0x4c5ec, 0xf3d80, 0x4ce33, 0x2629f, 0x4cd87, 0x4c6e6, 0xf3e01,
 0xefd00, 0xd05e6, 0x6fd80, 0xd019f, 0x8f133, 0x6fd40, 0xd06ec, 0x0fe07,
 0x0fd07, 0xd05ec, 0x6fd80, 0x8f233, 0xd029f, 0xd017d, 0xd06e6, 0xefe00,
 0x041ff, 0x84de6, 0x04d7e, 0x04d9f, 0x51d33, 0x04e3f, 0x84eec, 0xbb207,
 0xbb107, 0x84dec, 0x04d7e, 0x51e33, 0x04e9f, 0xbb186, 0x84ee6, 0x046fe,
 0x009ff, 0x81de6, 0xbf580, 0x01d9f, 0x81e67, 0xbf540, 0x81eec, 0xbe207,
 0xbe107, 0x81dec, 0xbf580, 0x81d67, 0x01e9f, 0x81d79, 0x81ee6, 0x01afe,
 0xff500, 0x41de6, 0xfe580, 0xfe160, 0x41e67, 0xfe540, 0x41eec, 0x7e207,
 0x7e107, 0x41dec, 0xfe580, 0x41d67, 0xfe260, 0x41d79, 0x41ee6, 0xff600,
 0xfbd00, 0x44de6, 0xfb980, 0x4459f, 0x7b131, 0xfb940, 0x44eec, 0x7b207,
 0x7b107, 0x44dec, 0xfb980, 0x7b198, 0x4469f, 0x7b186, 0x44ee6, 0xfbe00,
 0xdfd00, 0xe05e6, 0x5fd80, 0xe019f, 0xe0667, 0x5fd40, 0xe06ec, 0x602fd,
 0x601fd, 0xe05ec, 0x5fd80, 0xe0567, 0xe029f, 0xe017d, 0xe06e6, 0xdfe00,
 0x0c1fe, 0x661e6, 0x0c17f, 0x0c59f, 0x8cd33, 0x0c1bf, 0x662ec, 0x59e07,
 0x59d07, 0x661ec, 0x0c17f, 0x8ce33, 0x0c69f, 0x8cd87, 0x662e6, 0x0c2fe,
 0xf7d00, 0x985e6, 0x1817f, 0x9819f, 0x27d31, 0x181bf, 0xa7e11, 0x27e07,
 0x27d07, 0xa7d11, 0x1817f, 0x27d98, 0x9829f, 0x9817d, 0x986e6, 0x182fe,
 0xdf501, 0x619e6, 0xdf580, 0xde560, 0x61d33, 0xde6c0, 0x61aec, 0x5e607,
 0x5e507, 0x619ec, 0xdf580, 0x61e33, 0x61a9e, 0x61d87, 0x61ae6, 0xdf601,
 0x061fe, 0x865e6, 0x0657e, 0x0659f, 0xb9133, 0x0663f, 0x866ec, 0xb9a07,
 0xb9907, 0x865ec, 0x0657e, 0xb9233, 0x0669f, 0xb9986, 0x866e6, 0x062fe,
 0xbfd00, 0x3f119, 0x3fd80, 0x3fe20, 0x3f298, 0x3fd40, 0x3fa11, 0x3f207,
 0x3f107, 0x3f113, 0x3fd80, 0x3f231, 0x3fd20, 0x3f186, 0x80ee7, 0xbfe00,
 // blue
 0x7fd00, 0xc06f6, 0x7fd00, 0x402ff, 0x7fd00, 0x7f940, 0x7fd00, 0xc0577,
 0xc0677, 0x402ff, 0x7f980, 0x402ff, 0x7fd00, 0xc06f6, 0xc05f6, 0x402ff,
 0x025fe, 0x826f6, 0x025fe, 0x026fe, 0x025fe, 0x025bf, 0x025fe, 0x82577,
 0x82677, 0x026fe, 0x0257f, 0x026fe, 0x025fe, 0x826f6, 0x825f6, 0x026fe,
 0x7fd00, 0xc06f6, 0x7fd00, 0x402ff, 0x7fd00, 0x7f940, 0x7fd00, 0xc0577,
 0xc0677, 0x402ff, 0x7f980, 0x402ff, 0x7fd00, 0xc06f6, 0xc05f6, 0x402ff,
 0xbfd00, 0x3fe08, 0xbfd00, 0xbfe00, 0xbfd00, 0x3fd40, 0xbfd00, 0x80d77,
 0x80e77, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3fe08, 0x3fd08, 0xbfe00,
 0x7fd00, 0xc06f6, 0x7fd00, 0x402ff, 0x7fd00, 0x7f940, 0x7fd00, 0xc0577,
 0xc0677, 0x402ff, 0x7f980, 0x402ff, 0x7fd00, 0xc06f6, 0xc05f6, 0x402ff,
 0xefd00, 0xd02f6, 0xefd00, 0xefe00, 0xefd00, 0x6fd40, 0xefd00, 0xd0177,
 0xd0277, 0xefe00, 0x6fd80, 0xefe00, 0xefd00, 0xd02f6, 0xd01f6, 0xefe00,
 0x7fd00, 0xc06f6, 0x7fd00, 0x402ff, 0x7fd00, 0x7f940, 0x7fd00, 0xc0577,
 0xc0677, 0x402ff, 0x7f980, 0x402ff, 0x7fd00, 0xc06f6, 0xc05f6, 0x402ff,
 0x081ff, 0x622f6, 0x081ff, 0x082ff, 0x081ff, 0xddd40, 0x081ff, 0x62177,
 0x62277, 0x082ff, 0xddd80, 0x082ff, 0x081ff, 0x622f6, 0x621f6, 0x082ff,
 0x221fe, 0xa22f6, 0x221fe, 0x222fe, 0x221fe, 0x221bf, 0x221fe, 0xa2177,
 0xa2277, 0x222fe, 0x2217f, 0x222fe, 0x221fe, 0xa22f6, 0xa21f6, 0x222fe,
 0xbfd00, 0x3fe08, 0xbfd00, 0xbfe00, 0xbfd00, 0x3fd40, 0xbfd00, 0x80d77,
 0x80e77, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3fe08, 0x3fd08, 0xbfe00,
 0xdfd00, 0xe02f6, 0xdfd00, 0xdfe00, 0xdfd00, 0x5fd40, 0xdfd00, 0xe0177,
 0xe0277, 0xdfe00, 0x5fd80, 0xdfe00, 0xdfd00, 0xe02f6, 0xe01f6, 0xdfe00,
 0xbfd00, 0x3fe08, 0xbfd00, 0xbfe00, 0xbfd00, 0x3fd40, 0xbfd00, 0x80d77,
 0x80e77, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3fe08, 0x3fd08, 0xbfe00,
 0x7fd00, 0xc06f6, 0x7fd00, 0x402ff, 0x7fd00, 0x7f940, 0x7fd00, 0xc0577,
 0xc0677, 0x402ff, 0x7f980, 0x402ff, 0x7fd00, 0xc06f6, 0xc05f6, 0x402ff,
 0x025fe, 0x826f6, 0x025fe, 0x026fe, 0x025fe, 0x025bf, 0x025fe, 0x82577,
 0x82677, 0x026fe, 0x0257f, 0x026fe, 0x025fe, 0x826f6, 0x825f6, 0x026fe,
 0xfdd00, 0x426f6, 0xfdd00, 0xfde00, 0xfdd00, 0xfd940, 0xfdd00, 0x42577,
 0x42677, 0xfde00, 0xfd980, 0xfde00, 0xfdd00, 0x426f6, 0x425f6, 0xfde00,
 0xbfd00, 0x3fe08, 0xbfd00, 0xbfe00, 0xbfd00, 0x3fd40, 0xbfd00, 0x80d77,
 0x80e77, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3fe08, 0x3fd08, 0xbfe00
};

// TMDS symbols for DHGR RGB colors - for each two pixel combination
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
uint32_t DELAYED_COPY_DATA(tmds_dhgr_original)[16*16*3] = {
 // red
 0x7fd00, 0xc0dc7, 0xc01f7, 0xc0dec, 0xc0dc7, 0xc017f, 0x7f241, 0xc0e8f,
 0x7fa20, 0xc06e7, 0xc017f, 0xc0e3d, 0xc06e7, 0xc06fc, 0xc0e3e, 0xc06fa,
 0xf1d03, 0x71d38, 0xf1d09, 0x4e1ec, 0x4e1c7, 0xf1d81, 0xf1e41, 0x4e273,
 0xf1d60, 0xf1e18, 0xf1d81, 0x4e23d, 0xf1e18, 0x242fe, 0x71ec1, 0xf1e05,
 0xfdd00, 0xc253d, 0x421f7, 0x435ec, 0xc25c7, 0x4217f, 0xc22be, 0x43673,
 0x7de20, 0xc22e7, 0x4217f, 0xc263d, 0xc22e7, 0xc22fc, 0xc223f, 0xc22fa,
 0xfb901, 0x7b138, 0x7b10d, 0x91e47, 0x91dc6, 0x4457e, 0xfb241, 0x91e71,
 0xfba20, 0x91e1d, 0x4457e, 0x44e3d, 0x91e1d, 0x446fc, 0x91e3c, 0x91ef0,
 0x241fe, 0x71d38, 0x71d0d, 0x71a47, 0x4e1c7, 0x2417f, 0xf1e41, 0x71a71,
 0xf1d60, 0x71e19, 0x2417f, 0x4e23d, 0x71a1d, 0x242fe, 0x71ec1, 0x71af0,
 0xdfd00, 0xe053d, 0x601f7, 0xe05ec, 0xe05c7, 0x6017f, 0xe02be, 0xe0673,
 0x5fe20, 0xe02e7, 0x6017f, 0xe063d, 0xe02e7, 0xe02fc, 0xe023f, 0xe02fa,
 0x101ff, 0x2f938, 0x105f7, 0x2f913, 0x2f938, 0x1057f, 0xafa41, 0x2fa8c,
 0x906de, 0x906e7, 0x1057f, 0x2fac2, 0x906e7, 0x906fc, 0x2fac1, 0x906fa,
 0x231fc, 0x9cd38, 0x9cd0d, 0x9c647, 0x9c5c6, 0x2317e, 0x9ce43, 0x9c671,
 0x232de, 0x232e7, 0x2317e, 0xa323d, 0x9c61d, 0x232fc, 0x9c63c, 0x232fa,
 0x881fe, 0x37938, 0x881f7, 0xd81ec, 0xd81c7, 0x8817f, 0x37e41, 0xd8273,
 0xb7e20, 0x37e18, 0x8817f, 0xd823d, 0x37e18, 0x37e03, 0x37ac1, 0x37e05,
 0x061fe, 0x39d38, 0x061f7, 0x87647, 0x865c7, 0x0617f, 0xb9e41, 0x87671,
 0x062df, 0x862e7, 0x0617f, 0x8663d, 0xb9e18, 0x862fc, 0x39ec1, 0x862fa,
 0xdfd00, 0xe053d, 0x601f7, 0xe05ec, 0xe05c7, 0x6017f, 0xe02be, 0xe0673,
 0x5fe20, 0xe02e7, 0x6017f, 0xe063d, 0xe02e7, 0xe02fc, 0xe023f, 0xe02fa,
 0x309fc, 0x8f538, 0x309f6, 0x8f513, 0xb09c7, 0x3097e, 0x30abe, 0x8f68c,
 0x30ade, 0x30ae7, 0x3097e, 0xb0a3d, 0x30ae7, 0x30afc, 0x8f6c1, 0x30afa,
 0x061fe, 0x39d38, 0x061f7, 0x87647, 0x875c6, 0x0617f, 0xb9e41, 0x87671,
 0x072de, 0x862e7, 0x0617f, 0x876c3, 0x8761d, 0x862fc, 0x39ec1, 0x862fa,
 0x00dfe, 0x3f138, 0x00df7, 0x3f113, 0x81dc7, 0x00d7f, 0xbf241, 0x3f28c,
 0x00edf, 0x80ee7, 0x00d7f, 0x81e3d, 0xbf218, 0x80efc, 0x3f2c1, 0x80efa,
 0xefd00, 0x8f938, 0x301f7, 0x8f247, 0xb05c7, 0x3017f, 0xb06bc, 0x8f271,
 0x306de, 0x306e7, 0x3017f, 0xb063d, 0x8f21d, 0x306fc, 0x8fac1, 0x306fa,
 0x015fe, 0x3e938, 0x015f7, 0xbc247, 0x839c7, 0x0157f, 0xbea41, 0xbc271,
 0x016df, 0x816e7, 0x0157f, 0x83a3d, 0xbea18, 0x816fc, 0x3eac1, 0x816fa,
 // green
 0x7fd00, 0xc05ee, 0xc0e79, 0xc0d8f, 0xc0677, 0xc017f, 0x7f211, 0xc02f7,
 0xc01f7, 0xc053f, 0xc017f, 0xc02df, 0x7f284, 0xc057b, 0xc06e7, 0xc06fa,
 0xfbd00, 0x445ee, 0xc4679, 0xc458f, 0xc45dc, 0x4417f, 0x7ba11, 0x442f7,
 0x7b909, 0xc453e, 0x4417f, 0x442df, 0x7ba84, 0xc462f, 0x446e7, 0x446fa,
 0x211fe, 0x219ee, 0x9e686, 0xa1d8e, 0x9e523, 0x2117f, 0xa1aec, 0x212f7,
 0xa19f4, 0x9e5c1, 0x2117f, 0x212df, 0xa1a79, 0x9e6d0, 0x21ae7, 0x21afa,
 0x341fc, 0x341ee, 0x63a87, 0x6398e, 0x63d23, 0x3417e, 0xe3e11, 0xe3e09,
 0xe3d09, 0x6393c, 0x3417e, 0xe3e21, 0xe3e84, 0x63ed0, 0x63a1d, 0x63af0,
 0xf7d00, 0xf7111, 0x77286, 0x48d8f, 0x77123, 0x2217f, 0xf7211, 0x222f7,
 0xf7109, 0x771c1, 0x2217f, 0x222df, 0xf7284, 0x772d0, 0xf7218, 0xf7205,
 0xdfd00, 0xe01ee, 0xe0679, 0xe058f, 0xe0277, 0x6017f, 0xe02ee, 0x602f7,
 0xe01f6, 0xe013f, 0x6017f, 0x602df, 0xe027b, 0xe017b, 0xe02e7, 0xe02fa,
 0x041ff, 0x845ee, 0x3ba86, 0x3b970, 0x3b923, 0x0457f, 0xbba11, 0x046f7,
 0xbb909, 0x3b9c1, 0x0457f, 0x046df, 0xbba84, 0x3bad0, 0x846e7, 0x846fa,
 0x021ff, 0x825ee, 0x3da86, 0x3d970, 0x3d923, 0x8217f, 0x3de11, 0x822f7,
 0x3dd09, 0x3d9c1, 0x8217f, 0x822df, 0x3de84, 0x3dad0, 0x826e7, 0x826fa,
 0xfdd00, 0x425ee, 0x7d286, 0x7d170, 0x7d123, 0xfd980, 0x7da11, 0xfda08,
 0x7d909, 0x7d1c1, 0xfd980, 0xfda20, 0x7da84, 0x7d2d0, 0x426e7, 0x426fa,
 0x101ff, 0xcf911, 0x4fa86, 0x4f18e, 0x4f923, 0xcf981, 0xcfa11, 0xcfa09,
 0xcf909, 0x4f9c1, 0xcf981, 0xcfa21, 0xcfa84, 0x4fad0, 0xcfa18, 0xcfa05,
 0xdfd00, 0xe01ee, 0xe0679, 0xe058f, 0xe0277, 0x6017f, 0xe02ee, 0x602f7,
 0xe01f6, 0xe013f, 0x6017f, 0x602df, 0xe027b, 0xe017b, 0xe02e7, 0xe02fa,
 0x081ff, 0x885ee, 0x37a86, 0x37970, 0x37923, 0x8817f, 0x37e11, 0x882f7,
 0x37d09, 0x379c1, 0x8817f, 0x882df, 0x37e84, 0x37ad0, 0x886e7, 0x886fa,
 0x211fe, 0xa11ee, 0x1ee86, 0x1ed70, 0x1ed23, 0x2117f, 0x9ee11, 0x212f7,
 0x9ed09, 0x1edc1, 0x2117f, 0x212df, 0x9ee84, 0x1eed0, 0xa12e7, 0xa12fa,
 0xded01, 0x341ee, 0x8be86, 0xb418f, 0x8bd23, 0x3417e, 0xe3e11, 0x5ee09,
 0xe3d09, 0x8bdc1, 0x3417e, 0x5ee21, 0xe3e84, 0x8bed0, 0x342e7, 0x342fa,
 0x061fe, 0x861ee, 0x39e86, 0x8758e, 0x39d23, 0x0617f, 0xb9e11, 0x062f7,
 0xb9d09, 0x39dc1, 0x0617f, 0x062df, 0xb9e84, 0x39ed0, 0x862e7, 0x862fa,
 0x015fe, 0x815ee, 0x3ea86, 0xbc18e, 0x3e923, 0x0157f, 0xbea11, 0x016f7,
 0xbe909, 0x3e9c1, 0x0157f, 0x016df, 0xbea84, 0x3ead0, 0x816e7, 0x816fa,
 // blue
 0x7fd00, 0x7f20c, 0xc0d3e, 0x402ff, 0x7fd00, 0xc017f, 0x7fd00, 0xc0e3d,
 0xc0dc7, 0x402ff, 0xc017f, 0x402ff, 0x7fd00, 0xc0e3e, 0x7fd00, 0xc06fa,
 0x021ff, 0xbce0c, 0x3cdc1, 0x022ff, 0x021ff, 0x0317f, 0x021ff, 0x3cec2,
 0x3cd38, 0x022ff, 0x0317f, 0x022ff, 0x021ff, 0x3cec1, 0x021ff, 0x832fa,
 0x901fe, 0xcfa0c, 0x4f13c, 0x902fe, 0x901fe, 0x9017f, 0x901fe, 0x7063d,
 0x4f938, 0x902fe, 0x9017f, 0x902fe, 0x901fe, 0x4f23c, 0x901fe, 0x4f2f0,
 0xbfd00, 0x3fa0c, 0x3f1c1, 0xbfe00, 0xbfd00, 0x3fd80, 0xbfd00, 0x3f2c2,
 0x3f138, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3f2c1, 0xbfd00, 0x3fe04,
 0x7fd00, 0x7f20c, 0xc0d3e, 0x402ff, 0x7fd00, 0xc017f, 0x7fd00, 0xc0e3d,
 0xc0dc7, 0x402ff, 0xc017f, 0x402ff, 0x7fd00, 0xc0e3e, 0x7fd00, 0xc06fa,
 0xdfd00, 0xe02f3, 0xe053e, 0xdfe00, 0xdfd00, 0x6017f, 0xdfd00, 0xe063d,
 0xe053d, 0xdfe00, 0x6017f, 0xdfe00, 0xdfd00, 0xe023f, 0xdfd00, 0xe02fa,
 0x7fd00, 0x7f20c, 0xc0d3e, 0x402ff, 0x7fd00, 0xc017f, 0x7fd00, 0xc0e3d,
 0xc0dc7, 0x402ff, 0xc017f, 0x402ff, 0x7fd00, 0xc0e3e, 0x7fd00, 0xc06fa,
 0x309fc, 0x30af3, 0x8f5c1, 0x30afc, 0x309fc, 0x3097e, 0x309fc, 0xb0a3d,
 0x8f538, 0x30afc, 0x3097e, 0x30afc, 0x309fc, 0x8f6c1, 0x309fc, 0x30afa,
 0xf1d03, 0xf1e0c, 0x4e13e, 0x242fe, 0xf1d03, 0xf1d81, 0xf1d03, 0x4e23d,
 0x71d38, 0x242fe, 0xf1d81, 0x242fe, 0xf1d03, 0x71ec1, 0xf1d03, 0xf1e05,
 0xbfd00, 0x3fa0c, 0x3f1c1, 0xbfe00, 0xbfd00, 0x3fd80, 0xbfd00, 0x3f2c2,
 0x3f138, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3f2c1, 0xbfd00, 0x3fe04,
 0xdfd00, 0xe02f3, 0xe053e, 0xdfe00, 0xdfd00, 0x6017f, 0xdfd00, 0xe063d,
 0xe053d, 0xdfe00, 0x6017f, 0xdfe00, 0xdfd00, 0xe023f, 0xdfd00, 0xe02fa,
 0xbfd00, 0x3fa0c, 0x3f1c1, 0xbfe00, 0xbfd00, 0x3fd80, 0xbfd00, 0x3f2c2,
 0x3f138, 0xbfe00, 0x3fd80, 0xbfe00, 0xbfd00, 0x3f2c1, 0xbfd00, 0x3fe04,
 0x7fd00, 0x7f20c, 0xc0d3e, 0x402ff, 0x7fd00, 0xc017f, 0x7fd00, 0xc0e3d,
 0xc0dc7, 0x402ff, 0xc017f, 0x402ff, 0x7fd00, 0xc0e3e, 0x7fd00, 0xc06fa,
 0xefd00, 0xb06f1, 0x8f13c, 0x302fe, 0xefd00, 0x3017f, 0xefd00, 0xb063d,
 0x8f938, 0x302fe, 0x3017f, 0x302fe, 0xefd00, 0x8fac1, 0xefd00, 0x306fa,
 0x7fd00, 0x7f20c, 0xc0d3e, 0x402ff, 0x7fd00, 0xc017f, 0x7fd00, 0xc0e3d,
 0xc0dc7, 0x402ff, 0xc017f, 0x402ff, 0x7fd00, 0xc0e3e, 0x7fd00, 0xc06fa,
 0x015fe, 0xbea0c, 0xbc13c, 0x016fe, 0x015fe, 0x0157f, 0x015fe, 0x83a3d,
 0x3e938, 0x016fe, 0x0157f, 0x016fe, 0x015fe, 0x3eac1, 0x015fe, 0x816fa

};

// TMDS symbols for DHGR RGB colors - for each two pixel combination
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
uint32_t DELAYED_COPY_DATA(tmds_dhgr_improved)[16*16*3] = {
 // red
 0x7fd00, 0x7f904, 0xc05f6, 0xc0ebc, 0xc0d8f, 0xc056f, 0xc02bf, 0x7f288,
 0xc0ee3, 0xc0e1f, 0xc01df, 0xc067d, 0xc02fd, 0xc06fc, 0x402ff, 0x402ff,
 0xfed00, 0x7ed04, 0xc19f4, 0x7c117, 0xc198f, 0x4196f, 0x7e5c0, 0x7e688,
 0x7e21c, 0xc16e3, 0x7e521, 0x41a7d, 0x7e603, 0xc12fc, 0xfee00, 0xfee00,
 0xfdd00, 0xfd904, 0x42df4, 0x7d243, 0x42d8f, 0x7d2c4, 0xfd1c0, 0xfd288,
 0x7d21c, 0x7d21c, 0xfd121, 0x7d283, 0xfd203, 0x426fc, 0xfde00, 0x282fe,
 0x101ff, 0xafd04, 0x90df4, 0x45d17, 0x90d8f, 0x45e39, 0xfb1c0, 0x45dd8,
 0x45ee1, 0x45ee1, 0xfb121, 0xaf283, 0xfb203, 0x10efc, 0x102ff, 0x102ff,
 0xded01, 0xded04, 0x5c1f4, 0x63917, 0x5c18f, 0x63ec4, 0xe3dc0, 0xe3e88,
 0x63e1c, 0x63ae1, 0xe3d21, 0x63e83, 0xe3e03, 0x342fc, 0xdee01, 0xdee01,
 0x0c1fe, 0x5bd06, 0xb11f4, 0x8e517, 0xb118f, 0x8eec4, 0x5bdc0, 0x5be88,
 0x8ee1c, 0x8e6e1, 0xb11dc, 0x8ee83, 0x5be03, 0x312fc, 0x0c2fe, 0x0c2fe,
 0x101ff, 0xcfd04, 0xf01f4, 0x4f117, 0xf018f, 0x7016f, 0x4fdc0, 0x4fe88,
 0x4fa1c, 0xf02e3, 0x4fd21, 0x7027d, 0x4fe03, 0x702fc, 0x102ff, 0x102ff,
 0x221fe, 0xa21f9, 0x1dd0b, 0x76117, 0x1dd70, 0x1dec4, 0x9ddc0, 0x9de88,
 0x1de1c, 0x762e1, 0x9dd21, 0x1de83, 0x9de03, 0xa22fc, 0x222fe, 0x222fe,
 0x061fe, 0xd7d04, 0x871f4, 0xb8517, 0x8718f, 0xb8ec4, 0x8713e, 0xb9e88,
 0xb8e1c, 0xb86e1, 0x871dc, 0xb8e83, 0xb9e03, 0x072fc, 0x062fe, 0x062fe,
 0xd7d01, 0xd7d04, 0x871f4, 0xb8517, 0xb858e, 0xb8639, 0x57dc0, 0xb85d8,
 0xb86e1, 0xb86e1, 0x57d21, 0xb8e83, 0x57e03, 0x072fc, 0xd7e01, 0xd7e01,
 0xf7d00, 0xf7904, 0xc85f4, 0x76117, 0xc858f, 0x772c4, 0x779c0, 0x77a88,
 0x7721c, 0x7721c, 0x77921, 0x77283, 0x77a03, 0x486fc, 0xf7a01, 0xf7e00,
 0x201ff, 0x20df9, 0xa0df4, 0xa0ebc, 0xa0d8f, 0x9f2c4, 0x9f5c0, 0x9f688,
 0x9f21c, 0xa0ee3, 0xa0ddc, 0x9f283, 0x9f603, 0x20efc, 0x202ff, 0x20afe,
 0xbfd00, 0x80df9, 0x3f10b, 0x3f243, 0x3f170, 0x3f2c4, 0xbf1c0, 0xbf288,
 0x3f21c, 0x3f21c, 0xbf121, 0x3f283, 0xbf203, 0x80efc, 0x00aff, 0x00efe,
 0x00dfe, 0x00dfb, 0xbf109, 0x3f243, 0x81d8f, 0x3f2c4, 0xbf1c0, 0xbf288,
 0x3f21c, 0x3f21c, 0xbf121, 0x3f283, 0xbf203, 0x80efc, 0x00efe, 0x00efe,
 0xbfd00, 0x3fd04, 0x3f10b, 0x3f243, 0x3f170, 0x3f2c4, 0x3f9c0, 0x3fa88,
 0x3f21c, 0x3f21c, 0x3f921, 0x3f283, 0x3fa03, 0x3fe02, 0x802ff, 0xbfe00,
 0xbfd00, 0x3fd04, 0x3f10b, 0x3f243, 0x3f170, 0x3f2c4, 0x3fe40, 0x3fa88,
 0x3f21c, 0x3f21c, 0x3f921, 0x3f283, 0x3fe02, 0x3fe01, 0x802ff, 0xbfe00,
 // green
 0x7fd00, 0xc0df1, 0xc057e, 0xc06be, 0xc05de, 0xc056f, 0xc0e1f, 0x7fa01,
 0x7f2a0, 0xc053f, 0xc01df, 0xc056f, 0xc067d, 0xc0e37, 0x402ff, 0x402ff,
 0xfed00, 0x43d0f, 0x4157e, 0x416be, 0x439dc, 0x43e39, 0x43a1f, 0xfea01,
 0xfc6a0, 0x43d3c, 0xfc521, 0x43e39, 0x43e78, 0x43d63, 0xfea01, 0xfee00,
 0xdfd00, 0x5f905, 0x6057e, 0x606be, 0xe05dc, 0xe063b, 0xe061f, 0xdfa01,
 0x5faa0, 0xe053e, 0x5f921, 0xe063b, 0x5fa82, 0xe0637, 0xdfa01, 0xdfe00,
 0x105fe, 0xaf905, 0x9057e, 0x906be, 0xaf921, 0x2fac4, 0xaf9a0, 0x106fe,
 0xafaa0, 0x2f9c1, 0xaf921, 0x2fac4, 0xafa82, 0xaf161, 0x102ff, 0x106fe,
 0xf7d00, 0x7710e, 0x4857e, 0x486be, 0x48ddc, 0x772c4, 0x48e1f, 0xf7a01,
 0xf72a0, 0x771c1, 0xf7121, 0x772c4, 0x77286, 0x77161, 0xf7e00, 0x222fe,
 0x0c1fe, 0x8e50f, 0x3117e, 0x312be, 0xb11dc, 0x8eec4, 0xb121f, 0x5be03,
 0x5bea0, 0x8edc1, 0xb11dc, 0x8eec4, 0x8e678, 0x8e563, 0x0c2fe, 0x0c2fe,
 0xd7d01, 0x87d0e, 0x3817e, 0x382be, 0xb81dc, 0x87ec4, 0xb821f, 0x382fc,
 0x3825f, 0x87dc1, 0x381de, 0x87ec4, 0x3827d, 0x87963, 0xd7e01, 0xd7e01,
 0xbfd00, 0x3f905, 0x3f981, 0x3fa41, 0x3f921, 0x3f2c4, 0x3f9a0, 0xbfa01,
 0x3faa0, 0x3f1c1, 0x3f921, 0x3f2c4, 0x3fa82, 0x3f161, 0x006ff, 0x806fe,
 0x281fe, 0x17d0e, 0xa817e, 0xa82be, 0x17d23, 0x17ec4, 0x17ee0, 0xa82fc,
 0x97ea0, 0x17dc1, 0x97d21, 0x17ec4, 0xa827d, 0x17d61, 0x282fe, 0x282fe,
 0x101ff, 0x4f10f, 0xcf981, 0xcfa41, 0x705dc, 0x4fac4, 0x7061f, 0xcfa03,
 0xcfaa0, 0x4f9c1, 0xcf921, 0x4fac4, 0x4f278, 0x4f163, 0x102ff, 0x102ff,
 0xf7d00, 0xf7105, 0x4857e, 0x486be, 0xc85dc, 0x772c4, 0xc861f, 0xf7a01,
 0x77aa0, 0x771c1, 0x77921, 0x772c4, 0x4867d, 0x77161, 0xf7a01, 0xf7e00,
 0x0c1fe, 0x8e50f, 0x3117e, 0x312be, 0xb11dc, 0x8eec4, 0xb121f, 0x5be03,
 0x5bea0, 0x8edc1, 0xb11dc, 0x8eec4, 0x8e678, 0x8e563, 0x0c2fe, 0x0c2fe,
 0x209fe, 0x9e10f, 0xa097e, 0xa0abe, 0xa19dc, 0x9e239, 0x9e21e, 0x9f603,
 0x9f6a0, 0x9e13c, 0x9f521, 0x9e239, 0x9e278, 0x9e163, 0x20afe, 0x20afe,
 0xe7d01, 0x58d0f, 0x3217e, 0x322be, 0x585dc, 0x8dec4, 0x58e1e, 0x322fc,
 0xe7aa0, 0x8ddc1, 0xe7921, 0x8dec4, 0x58e78, 0x58d63, 0xe7e01, 0xe7e01,
 0xbfd00, 0x3f905, 0x3fd80, 0x80ebe, 0x3f123, 0x3f2c4, 0x3f2e0, 0x3fe01,
 0x3faa0, 0x3f1c1, 0x3f921, 0x3f2c4, 0x80e7d, 0x3f2c8, 0x802ff, 0xbfe00,
 0xbfd00, 0x3f10e, 0x3fd80, 0x3fe40, 0x3f123, 0x3f2c4, 0x3f2e0, 0x3fe01,
 0x3faa0, 0x3f1c1, 0x3f921, 0x3f2c4, 0x80e7d, 0x3f2c8, 0x802ff, 0xbfe00,
 // blue
 0x7fd00, 0xc0e37, 0xc01ef, 0x402ff, 0x7fd00, 0xc056f, 0x7fd00, 0xc059f,
 0xc0d3e, 0xc055f, 0xc01df, 0x402ff, 0x7fd00, 0xc0d73, 0x7fd00, 0x402ff,
 0x881fe, 0x6719c, 0x67ab0, 0x882fe, 0x881fe, 0x67239, 0x881fe, 0x6719c,
 0x5853e, 0x3215f, 0xe7921, 0x882fe, 0x881fe, 0x67171, 0x881fe, 0x882fe,
 0xfbd00, 0xac19e, 0x441ef, 0xfbe00, 0xfbd00, 0xc416f, 0xfbd00, 0xc419f,
 0xc453e, 0xc415f, 0xc41de, 0xfbe00, 0xfbd00, 0xac18f, 0xfbd00, 0xfbe00,
 0xbfd00, 0x3f161, 0x3fd10, 0xbfe00, 0xbfd00, 0x3f2c4, 0xbfd00, 0x3f161,
 0x3f1c1, 0x80d5f, 0x3f921, 0xbfe00, 0xbfd00, 0x3f18c, 0xbfd00, 0xbfe00,
 0x7fd00, 0xc0e37, 0xc01ef, 0x402ff, 0x7fd00, 0xc056f, 0x7fd00, 0xc059f,
 0xc0d3e, 0xc055f, 0xc01df, 0x402ff, 0x7fd00, 0xc0d73, 0x7fd00, 0x402ff,
 0x0c1fe, 0x8e59c, 0x0c1ef, 0x0c2fe, 0x0c1fe, 0x8eec4, 0x0c1fe, 0x8ed61,
 0xb113e, 0x3115f, 0xb11dc, 0x0c2fe, 0x0c1fe, 0x8e571, 0x0c1fe, 0x0c2fe,
 0x7fd00, 0xc0e37, 0xc01ef, 0x402ff, 0x7fd00, 0xc056f, 0x7fd00, 0xc059f,
 0xc0d3e, 0xc055f, 0xc01df, 0x402ff, 0x7fd00, 0xc0d73, 0x7fd00, 0x402ff,
 0x081ff, 0x6719c, 0xe7911, 0x082ff, 0x081ff, 0x67ac4, 0x081ff, 0x67961,
 0x5853e, 0xe79a0, 0xe7921, 0x082ff, 0x081ff, 0x67171, 0x081ff, 0x082ff,
 0x101ff, 0x4f961, 0x4fab0, 0x102ff, 0x101ff, 0x4fac4, 0x101ff, 0x4f961,
 0x7053e, 0x4fa0b, 0xcf921, 0x102ff, 0x101ff, 0x4f98c, 0x101ff, 0x102ff,
 0x021ff, 0xe819e, 0x57d11, 0x022ff, 0x021ff, 0xe823b, 0x021ff, 0xe819e,
 0xe813e, 0x6815f, 0x57d21, 0x022ff, 0x021ff, 0xe818f, 0x021ff, 0x022ff,
 0xf7d00, 0x77161, 0xf7910, 0xf7e00, 0xf7d00, 0x772c4, 0xf7d00, 0x77161,
 0xc853e, 0x4855f, 0x77921, 0xf7e00, 0xf7d00, 0x48d8f, 0xf7d00, 0xf7e00,
 0xbfd00, 0x3f161, 0x3fd10, 0xbfe00, 0xbfd00, 0x3f2c4, 0xbfd00, 0x3f161,
 0x3f1c1, 0x80d5f, 0x3f921, 0xbfe00, 0xbfd00, 0x3f18c, 0xbfd00, 0xbfe00,
 0x7fd00, 0xc0e37, 0xc01ef, 0x402ff, 0x7fd00, 0xc056f, 0x7fd00, 0xc059f,
 0xc0d3e, 0xc055f, 0xc01df, 0x402ff, 0x7fd00, 0xc0d73, 0x7fd00, 0x402ff,
 0xddd01, 0x5c59c, 0x63eb0, 0xdde01, 0xddd01, 0x5c639, 0xddd01, 0x5c59c,
 0x6313e, 0xdcda0, 0xdcd21, 0xdde01, 0xddd01, 0x5c571, 0xddd01, 0xdde01,
 0x7fd00, 0xc0e37, 0xc01ef, 0x402ff, 0x7fd00, 0xc056f, 0x7fd00, 0xc059f,
 0xc0d3e, 0xc055f, 0xc01df, 0x402ff, 0x7fd00, 0xc0d73, 0x7fd00, 0x402ff,
 0xbfd00, 0x3f161, 0x3fd10, 0xbfe00, 0xbfd00, 0x3f2c4, 0xbfd00, 0x3f161,
 0x3f1c1, 0x80d5f, 0x3f921, 0xbfe00, 0xbfd00, 0x3f18c, 0xbfd00, 0xbfe00
};

void DELAYED_COPY_CODE(tmds_color_load_dhgr)(uint color_style)
{
    const uint32_t* pSource = tmds_dhgr_default;

    switch(color_style)
    {
        case 1:
            pSource = tmds_dhgr_original;
            break;
        case 2:
            pSource = tmds_dhgr_improved;
            break;
        case 0: // fall-through
        default:
            break;
    }

    memcpy32(tmds_dhgr_red,   pSource,           sizeof(tmds_dhgr_red));
    memcpy32(tmds_dhgr_green, &pSource[16*16],   sizeof(tmds_dhgr_green));
    memcpy32(tmds_dhgr_blue,  &pSource[16*16*2], sizeof(tmds_dhgr_blue));
}

