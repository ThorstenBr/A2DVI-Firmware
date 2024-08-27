#!/usr/bin/env python3

# BSD 3-Clause License
#
# Copyright (c) 2021, Luke Wren
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# The key fact is that, if x is even, and the encoder currently has a running
# imbalance of 0, encoding x followed by x + 1 produces a symbol pair with a
# net balance of 0.
#
# This is a reasonable constraint, because we only want RGB565 (so 6 valid
# channel data bits -> data is multiple of 4), and can probably tolerate
# 0.25LSB of noise :)
#
# This means that encoding a half-horizontal-resolution scanline buffer is a
# simple LUT operation for each colour channel, because we have made the
# encoding process stateless by guaranteeing 0 balance.

def popcount(x):
	n = 0
	while x:
		n += 1
		x = x & (x - 1)
	return n

# Equivalent to N1(q) - N0(q) in the DVI spec
def byteimbalance(x):
	return 2 * popcount(x) - 8

# This is a direct translation of "Figure 3-5. T.M.D.S. Encode Algorithm" on
# page 29 of DVI 1.0 spec

class TMDSEncode:
	ctrl_syms = {
		0b00: 0b1101010100,
		0b01: 0b0010101011,
		0b10: 0b0101010100,
		0b11: 0b1010101011
	}
	def __init__(self):
		self.imbalance = 0
		self.recent = 0

	def encode(self, d, c, de):
		if not de:
			self.imbalance = 0
			return self.ctrl_syms[c]
		# Minimise transitions
		q_m = d & 0x1
		if popcount(d) > 4 or (popcount(d) == 4 and not d & 0x1):
			for i in range(7):
				q_m = q_m | (~(q_m >> i ^ d >> i + 1) & 0x1) << i + 1
		else:
			for i in range(7):
				q_m = q_m | ( (q_m >> i ^ d >> i + 1) & 0x1) << i + 1
			q_m = q_m | 0x100
		# Correct DC balance
		inversion_mask = 0x2ff
		q_out = 0
		if self.imbalance == 0 or byteimbalance(q_m & 0xff) == 0:
			q_out = q_m ^ (0 if q_m & 0x100 else inversion_mask)
			if q_m & 0x100:
				self.imbalance += byteimbalance(q_m & 0xff)
			else:
				self.imbalance -= byteimbalance(q_m & 0xff)
		elif (self.imbalance > 0) == (byteimbalance(q_m & 0xff) > 0):
			q_out = q_m ^ inversion_mask
			self.imbalance += ((q_m & 0x100) >> 7) - byteimbalance(q_m & 0xff)
		else:
			q_out = q_m
			self.imbalance += byteimbalance(q_m & 0xff) - ((~q_m & 0x100) >> 7)
		self.recent = q_out
		return q_out

# Turn a bitmap of width n into n pairs of pseudo-differential bits
def differentialise(x, n):
	accum = 0
	for i in range(n):
		accum <<= 2
		if x & (1 << (n - 1)):
			accum |= 0b01
		else:
			accum |= 0b10
		x <<= 1
	return accum

enc = TMDSEncode()


###
# Pixel-doubled table:

# for i in range(0, 256, 4):
# 	sym0 = enc.encode(i, 0, 1)
# 	sym1 = enc.encode(i ^ 1, 0, 1)
# 	assert(enc.imbalance == 0)
# 	print(f"0x{sym0 | (sym1 << 10):05x}u,")

###
# Fullres 1bpp table: (each entry is 2 words, 4 pixels)

# (note trick here is that encoding 0x00 or 0xff sets imbalance to -8, and
# (encoding 0x01 or 0xfe returns imbalance to 0, so we alternate between these
# (two pairs of dark/light colours. Creates some fairly subtle vertical
# (banding, but it's cheap.

# for i in range(1 << 4):
# 	syms = list(enc.encode((0xff if i & 1 << j else 0) ^ j & 0x01, 0, 1) for j in range(4))
# 	print(f"0x{syms[0] | syms[1] << 10:05x}, 0x{syms[2] | syms[3] << 10:05x}")
# 	assert(enc.imbalance == 0)

###
# Fullres table stuff:

# def disptable_format(sym):
# 	return sym | ((popcount(sym) * 2 - 10 & 0x3f) << 26)

# print("// Non-negative running disparity:")
# for i in range(0, 256, 4):
# 	enc.imbalance = 1
# 	print("0x{:08x},".format(disptable_format(enc.encode(i, 0, 1))))

# print("// Negative running disparity:")
# for i in range(0, 256, 4):
# 	enc.imbalance = -1
# 	print("0x{:08x},".format(disptable_format(enc.encode(i, 0, 1))))

###
# Control symbols:

# for i in range(4):
# 	sym = enc.encode(0, i, 0)
# 	print(f"0x{sym << 10 | sym:05x},")


###
# Find zero-balance symbols:

# for i in range(256):
# 	enc.imbalance = 0
# 	sym = enc.encode(i, 0, 1)
# 	if enc.imbalance == 0:
# 		print(f"{i:02x}: {sym:03x}")

###
# Generate 2bpp table based on above experiment:

if __name__=="__main__":
	levels_2bpp_even = [0x05, 0x50, 0xaf, 0xfa]
	levels_2bpp_odd  = [0x04, 0x51, 0xae, 0xfb]

	for i1, p1 in enumerate(levels_2bpp_odd):
		for i0, p0 in enumerate(levels_2bpp_even):
			sym0 = enc.encode(p0, 0, 1)
			sym1 = enc.encode(p1, 0, 1)
			assert(enc.imbalance == 0)
			print(f".word 0x{sym1 << 10 | sym0:05x} // {i0:02b}, {i1:02b}")
