# MIT License
# Copyright (c) 2024 Thorsten Brehm
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
from tmds_table_gen import *

# LORES RGB values
LoresPalette = """
black:     000000
magenta:   bf1a3d
darkblue:  452dff
purple:    fe43ff
darkgreen: 00813d
grey1:     808080
mediumblue:369cff
lightblue: c39eff
brown:     3e6a00
orange:    fc6e00
grey2:     808080
pink:      ff86c1
green:     3aec00
yellow:    c2eb00
aqua:      57ffc0
white:     ffffff
"""

LoresRgb2 = [
    (0x00, 0x00, 0x00), # black
    (0x9d, 0x09, 0x66), # magenta
    (0x2a, 0x2a, 0xe5), # darkblue
    (0xc7, 0x34, 0xff), # purple
    (0x00, 0x80, 0x00), # darkgreen
    (0x80, 0x80, 0x80), # grey1
    (0x0d, 0xa1, 0xff), # mediumblue
    (0xaa, 0xaa, 0xff), # lightblue
    (0x55, 0x55, 0x00), # brown
    (0xf2, 0x5e, 0x00), # orange
    (0x80, 0x80, 0x80), # grey2
    (0xff, 0x89, 0xe5), # pink
    (0x38, 0xcb, 0x00), # green
    (0xd5, 0xd5, 0x1a), # yellow
    (0x62, 0xf6, 0x99), # aqua
    (0xff, 0xff, 0xff)  # white
]

def parseRgbData(s):
    Lines = s.split("\n")
    Data = []
    for l in Lines:
        l = l.strip()
        if l == "":
            continue
        rgbString = l.split(":")[1].strip()
        r = int(rgbString[0:2], 16)
        g = int(rgbString[2:4], 16)
        b = int(rgbString[4:6], 16)
        Data.append((r,g,b))
    return Data

def doublePixEncode(value):
    e = TMDSEncode()
    sym1 = e.encode(value,0,1)
    sym2 = e.encode(value^1,0,1)
    return hex(sym1 | (sym2<<10))

# get RGB data
LoresRgb1 = parseRgbData(LoresPalette)

# print RGB TMDS symbols for a double pixel
print("LORES palette TMDS symbols:")
print("// R        G        B")
for (r,g,b) in LoresRgb2:
    print(doublePixEncode(r)+", "+doublePixEncode(g)+", "+doublePixEncode(b)+",")
