/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/RDP/mppc.hpp"
#include "core/RDP/mppc/mppc_utils.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/stream.hpp"

#include <limits>
#include <cinttypes>

static uint8_t HuffLenLEC[] = {
    0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8, // 0
    0x8, 0x8, 0x9, 0x8, 0x9, 0x9, 0x9, 0x9, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, // 16
    0x8, 0x9, 0x9, 0xa, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa, // 32
    0x9, 0x9, 0xa, 0x9, 0xa, 0x9, 0xa, 0x9, 0x9, 0x9, 0xa, 0xa, 0x9, 0xa, 0x9, 0x9, // 48

    0x8, 0x9, 0x9, 0x9, 0x9, 0xa, 0xa, 0xa, 0x9, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 64
    0x9, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 80
    0x8, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 96
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, // 112

    0x7, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 128
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 144
    0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xd, 0xa, 0xa, 0xa, 0xa, // 160
    0xa, 0xa, 0xb, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 176

    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, // 192
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, // 208
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, // 224
    0x8, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0x9, 0x8, 0x7, // 240

    0xd, 0xd, 0x7, 0x7, 0xa, 0x7, 0x7, 0x6, 0x6, 0x6, 0x6, 0x5, 0x6, 0x6, 0x6, 0x5, // 256
    0x6, 0x5, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, // 272
    0x8, 0x5, 0x6, 0x7, 0x7, 0xd                                                    // 288
};

static uint16_t HuffCodeLEC[] =
{
    0x0004, 0x0024, 0x0014, 0x0011, 0x0051, 0x0031, 0x0071, 0x0009, // 0
    0x0049, 0x0029, 0x0069, 0x0015, 0x0095, 0x0055, 0x00d5, 0x0035,
    0x00b5, 0x0075, 0x001d, 0x00f5, 0x011d, 0x009d, 0x019d, 0x005d,
    0x000d, 0x008d, 0x015d, 0x00dd, 0x01dd, 0x003d, 0x013d, 0x00bd,
    0x004d, 0x01bd, 0x007d, 0x006b, 0x017d, 0x00fd, 0x01fd, 0x0003,
    0x0103, 0x0083, 0x0183, 0x026b, 0x0043, 0x016b, 0x036b, 0x00eb,
    0x0143, 0x00c3, 0x02eb, 0x01c3, 0x01eb, 0x0023, 0x03eb, 0x0123,
    0x00a3, 0x01a3, 0x001b, 0x021b, 0x0063, 0x011b, 0x0163, 0x00e3,

    0x00cd, 0x01e3, 0x0013, 0x0113, 0x0093, 0x031b, 0x009b, 0x029b, // 64
    0x0193, 0x0053, 0x019b, 0x039b, 0x005b, 0x025b, 0x015b, 0x035b,
    0x0153, 0x00d3, 0x00db, 0x02db, 0x01db, 0x03db, 0x003b, 0x023b,
    0x013b, 0x01d3, 0x033b, 0x00bb, 0x02bb, 0x01bb, 0x03bb, 0x007b,
    0x002d, 0x027b, 0x017b, 0x037b, 0x00fb, 0x02fb, 0x01fb, 0x03fb,
    0x0007, 0x0207, 0x0107, 0x0307, 0x0087, 0x0287, 0x0187, 0x0387,
    0x0033, 0x0047, 0x0247, 0x0147, 0x0347, 0x00c7, 0x02c7, 0x01c7,
    0x0133, 0x03c7, 0x0027, 0x0227, 0x0127, 0x0327, 0x00a7, 0x00b3,

    0x0019, 0x01b3, 0x0073, 0x02a7, 0x0173, 0x01a7, 0x03a7, 0x0067, // 128
    0x00f3, 0x0267, 0x0167, 0x0367, 0x00e7, 0x02e7, 0x01e7, 0x03e7,
    0x01f3, 0x0017, 0x0217, 0x0117, 0x0317, 0x0097, 0x0297, 0x0197,
    0x0397, 0x0057, 0x0257, 0x0157, 0x0357, 0x00d7, 0x02d7, 0x01d7,
    0x03d7, 0x0037, 0x0237, 0x0137, 0x0337, 0x00b7, 0x02b7, 0x01b7,
    0x03b7, 0x0077, 0x0277, 0x07ff, 0x0177, 0x0377, 0x00f7, 0x02f7,
    0x01f7, 0x03f7, 0x03ff, 0x000f, 0x020f, 0x010f, 0x030f, 0x008f,
    0x028f, 0x018f, 0x038f, 0x004f, 0x024f, 0x014f, 0x034f, 0x00cf,

    0x000b, 0x02cf, 0x01cf, 0x03cf, 0x002f, 0x022f, 0x010b, 0x012f, // 192
    0x032f, 0x00af, 0x02af, 0x01af, 0x008b, 0x03af, 0x006f, 0x026f,
    0x018b, 0x016f, 0x036f, 0x00ef, 0x02ef, 0x01ef, 0x03ef, 0x001f,
    0x021f, 0x011f, 0x031f, 0x009f, 0x029f, 0x019f, 0x039f, 0x005f,
    0x004b, 0x025f, 0x015f, 0x035f, 0x00df, 0x02df, 0x01df, 0x03df,
    0x003f, 0x023f, 0x013f, 0x033f, 0x00bf, 0x02bf, 0x014b, 0x01bf,
    0x00ad, 0x00cb, 0x01cb, 0x03bf, 0x002b, 0x007f, 0x027f, 0x017f,
    0x012b, 0x037f, 0x00ff, 0x02ff, 0x00ab, 0x01ab, 0x006d, 0x0059,

    0x17ff, 0x0fff, 0x0039, 0x0079, 0x01ff, 0x0005, 0x0045, 0x0034, // 256
    0x000c, 0x002c, 0x001c, 0x0000, 0x003c, 0x0002, 0x0022, 0x0010,
    0x0012, 0x0008, 0x0032, 0x000a, 0x002a, 0x001a, 0x003a, 0x0006,
    0x0026, 0x0016, 0x0036, 0x000e, 0x002e, 0x001e, 0x003e, 0x0001,
    0x00ed, 0x0018, 0x0021, 0x0025, 0x0065, 0x1fff
};

static uint16_t HuffIndexLEC[512] = {
    0x007b, 0xff1f, 0xff0d, 0xfe27, 0xfe00, 0xff05, 0xff17, 0xfe68, 0x00c5, 0xfe07,
    0xff13, 0xfec0, 0xff08, 0xfe18, 0xff1b, 0xfeb3, 0xfe03, 0x00a2, 0xfe42, 0xff10,
    0xfe0b, 0xfe02, 0xfe91, 0xff19, 0xfe80, 0x00e9, 0xfe3a, 0xff15, 0xfe12, 0x0057,
    0xfed7, 0xff1d, 0xff0e, 0xfe35, 0xfe69, 0xff22, 0xff18, 0xfe7a, 0xfe01, 0xff23,
    0xff14, 0xfef4, 0xfeb4, 0xfe09, 0xff1c, 0xfec4, 0xff09, 0xfe60, 0xfe70, 0xff12,
    0xfe05, 0xfe92, 0xfea1, 0xff1a, 0xfe0f, 0xff07, 0xfe56, 0xff16, 0xff02, 0xfed8,
    0xfee8, 0xff1e, 0xfe1d, 0x003b, 0xffff, 0xff06, 0xffff, 0xfe71, 0xfe89, 0xffff,
    0xffff, 0xfe2c, 0xfe2b, 0xfe20, 0xffff, 0xfebb, 0xfecf, 0xfe08, 0xffff, 0xfee0,
    0xfe0d, 0xffff, 0xfe99, 0xffff, 0xfe04, 0xfeaa, 0xfe49, 0xffff, 0xfe17, 0xfe61,
    0xfedf, 0xffff, 0xfeff, 0xfef6, 0xfe4c, 0xffff, 0xffff, 0xfe87, 0xffff, 0xff24,
    0xffff, 0xfe3c, 0xfe72, 0xffff, 0xffff, 0xfece, 0xffff, 0xfefe, 0xffff, 0xfe23,
    0xfebc, 0xfe0a, 0xfea9, 0xffff, 0xfe11, 0xffff, 0xfe82, 0xffff, 0xfe06, 0xfe9a,
    0xfef5, 0xffff, 0xfe22, 0xfe4d, 0xfe5f, 0xffff, 0xff03, 0xfee1, 0xffff, 0xfeca,
    0xfecc, 0xffff, 0xfe19, 0xffff, 0xfeb7, 0xffff, 0xffff, 0xfe83, 0xfe29, 0xffff,
    0xffff, 0xffff, 0xfe6c, 0xffff, 0xfeed, 0xffff, 0xffff, 0xfe46, 0xfe5c, 0xfe15,
    0xffff, 0xfedb, 0xfea6, 0xffff, 0xffff, 0xfe44, 0xffff, 0xfe0c, 0xffff, 0xfe95,
    0xfefc, 0xffff, 0xffff, 0xfeb8, 0x16c9, 0xffff, 0xfef0, 0xffff, 0xfe38, 0xffff,
    0xffff, 0xfe6d, 0xfe7e, 0xffff, 0xffff, 0xffff, 0xffff, 0xfe5b, 0xfedc, 0xffff,
    0xffff, 0xfeec, 0xfe47, 0xfe1f, 0xffff, 0xfe7f, 0xfe96, 0xffff, 0xffff, 0xfea5,
    0xffff, 0xfe10, 0xfe40, 0xfe32, 0xfebf, 0xffff, 0xffff, 0xfed4, 0xfef1, 0xffff,
    0xffff, 0xffff, 0xfe75, 0xffff, 0xffff, 0xfe8d, 0xfe31, 0xffff, 0xfe65, 0xfe1b,
    0xffff, 0xfee4, 0xfefb, 0xffff, 0xffff, 0xfe52, 0xffff, 0xfe0e, 0xffff, 0xfe9d,
    0xfeaf, 0xffff, 0xffff, 0xfe51, 0xfed3, 0xffff, 0xff20, 0xffff, 0xfe2f, 0xffff,
    0xffff, 0xfec1, 0xfe8c, 0xffff, 0xffff, 0xffff, 0xfe3f, 0xffff, 0xffff, 0xfe76,
    0xffff, 0xfefa, 0xfe53, 0xfe25, 0xffff, 0xfe64, 0xfee5, 0xffff, 0xffff, 0xfeae,
    0xffff, 0xfe13, 0xffff, 0xfe88, 0xfe9e, 0xffff, 0xfe43, 0xffff, 0xffff, 0xfea4,
    0xfe93, 0xffff, 0xffff, 0xffff, 0xfe3d, 0xffff, 0xffff, 0xfeeb, 0xfed9, 0xffff,
    0xfe14, 0xfe5a, 0xffff, 0xfe28, 0xfe7d, 0xffff, 0xffff, 0xfe6a, 0xffff, 0xffff,
    0xff01, 0xfec6, 0xfec8, 0xffff, 0xffff, 0xfeb5, 0xffff, 0xffff, 0xffff, 0xfe94,
    0xfe78, 0xffff, 0xffff, 0xffff, 0xfea3, 0xffff, 0xffff, 0xfeda, 0xfe58, 0xffff,
    0xfe1e, 0xfe45, 0xfeea, 0xffff, 0xfe6b, 0xffff, 0xffff, 0xfe37, 0xffff, 0xffff,
    0xffff, 0xfe7c, 0xfeb6, 0xffff, 0xffff, 0xfef8, 0xffff, 0xffff, 0xffff, 0xfec7,
    0xfe9b, 0xffff, 0xffff, 0xffff, 0xfe50, 0xffff, 0xffff, 0xfead, 0xfee2, 0xffff,
    0xfe1a, 0xfe63, 0xfe4e, 0xffff, 0xffff, 0xfef9, 0xffff, 0xfe73, 0xffff, 0xffff,
    0xffff, 0xfe30, 0xfe8b, 0xffff, 0xffff, 0xfebd, 0xfe2e, 0x0100, 0xffff, 0xfeee,
    0xfed2, 0xffff, 0xffff, 0xffff, 0xfeac, 0xffff, 0xffff, 0xfe9c, 0xfe84, 0xffff,
    0xfe24, 0xfe4f, 0xfef7, 0xffff, 0xffff, 0xfee3, 0xfe62, 0xffff, 0xffff, 0xffff,
    0xffff, 0xfe8a, 0xfe74, 0xffff, 0xffff, 0xfe3e, 0xffff, 0xffff, 0xffff, 0xfed1,
    0xfebe, 0xffff, 0xffff, 0xfe2d, 0xffff, 0xfe4a, 0xfef3, 0xffff, 0xffff, 0xfedd,
    0xfe5e, 0xfe16, 0xffff, 0xfe48, 0xfea8, 0xffff, 0xfeab, 0xfe97, 0xffff, 0xffff,
    0xfed0, 0xffff, 0xffff, 0xfecd, 0xfeb9, 0xffff, 0xffff, 0xffff, 0xfe2a, 0xffff,
    0xffff, 0xfe86, 0xfe6e, 0xffff, 0xffff, 0xffff, 0xfede, 0xffff, 0xffff, 0xfe5d,
    0xfe4b, 0xfe21, 0xffff, 0xfeef, 0xfe98, 0xffff, 0xffff, 0xfe81, 0xffff, 0xffff,
    0xffff, 0xfea7, 0xffff, 0xfeba, 0xfefd, 0xffff, 0xffff, 0xffff, 0xfecb, 0xffff,
    0xffff, 0xfe6f, 0xfe39, 0xffff, 0xffff, 0xffff, 0xfe85, 0xffff, 0x010c, 0xfee6,
    0xfe67, 0xfe1c, 0xffff, 0xfe54, 0xfeb2, 0xffff, 0xffff, 0xfe9f, 0xffff, 0xffff,
    0xffff, 0xfe59, 0xfeb1, 0xffff, 0xfec2, 0xffff, 0xffff, 0xfe36, 0xfef2, 0xffff,
    0xffff, 0xfed6, 0xfe77, 0xffff, 0xffff, 0xffff, 0xfe33, 0xffff, 0xffff, 0xfe8f,
    0xfe55, 0xfe26, 0x010a, 0xff04, 0xfee7, 0xffff, 0x0121, 0xfe66, 0xffff, 0xffff,
    0xffff, 0xfeb0, 0xfea0, 0xffff, 0x010f, 0xfe90, 0xffff, 0xffff, 0xfed5, 0xffff,
    0xffff, 0xfec3, 0xfe34, 0xffff, 0xffff, 0xffff, 0xfe8e, 0xffff, 0x0111, 0xfe79,
    0xfe41, 0x010b
};

static uint16_t LECHTab[] = { 511, 0, 508, 448, 494, 347, 486, 482 };

static uint8_t HuffLenLOM[] = {
    0x4, 0x2, 0x3, 0x4, 0x3, 0x4, 0x4, 0x5, 0x4, 0x5, 0x5, 0x6, 0x6, 0x7, 0x7, 0x8,
    0x7, 0x8, 0x8, 0x9, 0x9, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9
};

static uint16_t HuffCodeLOM[] = {
    0x0001, 0x0000, 0x0002, 0x0009, 0x0006, 0x0005, 0x000d, 0x000b,
    0x0003, 0x001b, 0x0007, 0x0017, 0x0037, 0x000f, 0x004f, 0x006f,
    0x002f, 0x00ef, 0x001f, 0x005f, 0x015f, 0x009f, 0x00df, 0x01df,
    0x003f, 0x013f, 0x00bf, 0x01bf, 0x007f, 0x017f, 0x00ff, 0x01ff
};

static uint16_t HuffIndexLOM[] = {
    0xfe1, 0xfe0, 0xfe2, 0xfe8, 0xe, 0xfe5, 0xfe4, 0xfea, 0xff1, 0xfe3, 0x15, 0xfe7,
    0xfef, 0x46, 0xff0, 0xfed, 0xfff, 0xff7, 0xffb, 0x19, 0xffd, 0xff4, 0x12c, 0xfeb,
    0xffe, 0xff6, 0xffa, 0x89, 0xffc, 0xff3, 0xff8, 0xff2
};

static uint8_t LOMHTab[] = { 0, 4, 10, 19 };

static uint8_t CopyOffsetBitsLUT[] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11,
    11, 12, 12, 13, 13, 14, 14, 15
};

static uint32_t CopyOffsetBaseLUT[] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 32769, 49153,
    65537
};

static uint8_t LOMBitsLUT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 6, 6,
    8, 8, 14, 14
};

static uint16_t LOMBaseLUT[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 22, 26, 30, 34, 42, 50, 58, 66,
    82, 98, 114, 130, 194, 258, 514, 2, 2
};

static const size_t RDP_60_HIST_BUF_LEN      = 65536;
static const size_t RDP_60_HIST_BUF_MIDDLE   = RDP_60_HIST_BUF_LEN / 2;
static const size_t RDP_60_OFFSET_CACHE_SIZE = 8;

static inline void cache_add(uint16_t * offset_cache, uint16_t copy_offset)
{
    assert(copy_offset);
    assert((copy_offset != offset_cache[0]) && (copy_offset != offset_cache[1]) &&
        (copy_offset != offset_cache[2]) && (copy_offset != offset_cache[3]));

    *(offset_cache+3) = *(offset_cache+2);
    *(offset_cache+2) = *(offset_cache+1);
    *(offset_cache+1) = *offset_cache;
    *offset_cache = copy_offset;

}

static inline void cache_swap(uint16_t * offset_cache, uint16_t LUTIndex)
{
    assert(LUTIndex);

    uint16_t t = *offset_cache;
    *offset_cache              = *(offset_cache + LUTIndex);
    *(offset_cache + LUTIndex) = t;
}

struct rdp_mppc_60_dec : public rdp_mppc_dec
{
    uint8_t    history_buf[RDP_60_HIST_BUF_LEN];
    uint16_t   offset_cache[RDP_60_OFFSET_CACHE_SIZE];
    uint8_t  * history_buf_end;
    uint8_t  * history_ptr;

    rdp_mppc_60_dec()
    : history_buf{0}
    , offset_cache{0}
    , history_buf_end(this->history_buf + RDP_60_HIST_BUF_LEN - 1)
    , history_ptr(this->history_buf)
    {}

protected:
    static inline uint16_t LEChash(uint16_t key)
    {
        return ((key & 0x1ff) ^ (key  >> 9) ^ (key >> 4) ^ (key >> 7));
    }

    static inline uint16_t LOMhash(uint16_t key)
    {
        return ((key & 0x1f) ^ (key  >> 5) ^ (key >> 9));
    }

    static inline uint16_t miniLEChash(uint16_t key)
    {
        uint16_t h;
        h = ((((key >> 8) ^ (key & 0xff)) >> 2) & 0xf);
        if (key >> 9) {
            h = ~h;
        }
        return (h % 12);
    }

    static inline uint8_t miniLOMhash(uint16_t key)
    {
        uint8_t h;
        h = (key >> 4) & 0xf;
        return ((h ^ (h >> 2) ^ (h >> 3)) & 0x3);
    }

    static inline uint16_t getLECindex(uint16_t huff)
    {
        uint16_t h = HuffIndexLEC[ ::rdp_mppc_60_dec::LEChash(huff)];
        if ((h ^ huff) >> 9) {
            return h & 0x1ff;
        }
        return HuffIndexLEC[LECHTab[ ::rdp_mppc_60_dec::miniLEChash(huff)]];
    }

    static inline uint16_t getLOMindex(uint16_t huff)
    {
        uint16_t h = HuffIndexLOM[ ::rdp_mppc_60_dec::LOMhash(huff)];
        if ((h ^ huff) >> 5) {
            return h & 0x1f;
        }
        return HuffIndexLOM[LOMHTab[ ::rdp_mppc_60_dec::miniLOMhash(huff)]];
    }

    static inline uint32_t transposebits(uint32_t x)
    {
        x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
        x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
        x = ((x & 0x0f0f0f0f) << 4) | ((x >> 4) & 0x0f0f0f0f);
        if ((x >> 8) == 0) {
            return x;
        }
        x = ((x & 0x00ff00ff) << 8) | ((x >> 8) & 0x00ff00ff);
        if ((x >> 16) == 0) {
            return x;
        }
        x = ((x & 0x0000ffff) << 16) | ((x >> 16) & 0x0000ffff);
        return x;
    }

public:
    /**
     * decompress RDP 6 data
     *
     * @param cbuf    compressed data
     * @param len     length of compressed data
     * @param ctype   compression flags
     * @param roff    starting offset of uncompressed data
     * @param rlen    length of uncompressed data
     *
     * @return        True on success, False on failure
     */
    int decompress_60(uint8_t const * cbuf, int len, int ctype, uint32_t * roff, uint32_t * rlen)
    {
        //LOG(LOG_INFO, "decompress_60");

        uint16_t * offset_cache = this->offset_cache;   /* Copy Offset cache                          */
        uint8_t  * history_ptr  = this->history_ptr;    /* points to next free slot in bistory_buf    */
        uint32_t   d32          = 0;                    /* we process 4 compressed uint8_ts at a time */
        uint16_t   lom;                                 /* length of match                            */
        uint16_t   LUTIndex;                            /* LookUp table Index                         */
        uint8_t  * src_ptr      = nullptr;              /* used while copying compressed data         */
        uint8_t const * cptr    = cbuf;                 /* points to next uint8_t in cbuf             */
        uint8_t    cur_uint8_t  = 0;                    /* last uint8_t fetched from cbuf             */
        int        bits_left    = 0;                    /* bits left in d32 for processing            */
        int        cur_bits_left;                       /* bits left in cur_uint8_t for processing    */
        int        tmp, i;

        *rlen = 0;

        /* get next free slot in history buffer */
        *roff       = history_ptr - this->history_buf;

        if (ctype & PACKET_AT_FRONT) {
            /* slid history_buf and reset history_buf to middle */
            memmove(this->history_buf,
                (this->history_buf + (history_ptr - this->history_buf - RDP_60_HIST_BUF_MIDDLE)),
                RDP_60_HIST_BUF_MIDDLE);
            history_ptr       = this->history_buf + RDP_60_HIST_BUF_MIDDLE;
            this->history_ptr = history_ptr;
            *roff             = RDP_60_HIST_BUF_MIDDLE;
        }

        if (ctype & PACKET_FLUSHED) {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP_60_HIST_BUF_LEN);
            memset(offset_cache, 0, RDP_60_OFFSET_CACHE_SIZE);
            *roff = 0;
        }

        if ((ctype & PACKET_COMPRESSED) != PACKET_COMPRESSED) {
            /* data in cbuf is not compressed - copy to history buf as is */
            memcpy(history_ptr, cbuf, len);
            history_ptr       += len;
            *rlen             =  history_ptr - this->history_ptr;
            this->history_ptr =  history_ptr;
            return true;
        }

        /* load initial data */
        tmp = 0;
        while (cptr < cbuf + len) {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       += 8;
            if (tmp >= 32) {
                break;
            }
        }

        d32 = this->transposebits(d32);

        if (cptr < cbuf + len) {
            cur_uint8_t   = this->transposebits(*cptr++);
            cur_bits_left = 8;
        }
        else {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        uint32_t i32 = 0;
        while (bits_left >= 8) {
            /* Decode Huffman Code for Literal/EOS/CopyOffset */
            uint16_t copy_offset = 0;
            for (i = 0x5; i <= 0xd; i++) {
                if (i == 0xc) {
                    continue;
                }
                i32 = this->transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = this->getLECindex(i32);
                if (i == HuffLenLEC[i32]) {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            if (i32 < 256) {
                *history_ptr++ = static_cast<uint8_t>(i32);
            }
            else if (i32 > 256 && i32 < 289) {
                LUTIndex    = i32 - 257;
                tmp         = CopyOffsetBitsLUT[LUTIndex];
                copy_offset = CopyOffsetBaseLUT[LUTIndex] - 0x1;
                if (tmp != 0) {
                    copy_offset += this->transposebits(d32 & (0xffffffff << (32 - tmp)));
                }
                cache_add(offset_cache, copy_offset);
                d32       <<= tmp;
                bits_left -=  tmp;
            }
            else if ( i32 > 288 && i32 < 293) {
                LUTIndex    = i32 - 289;
                copy_offset = *(offset_cache + LUTIndex);
                if (LUTIndex != 0) {
                    cache_swap(offset_cache, LUTIndex);
                }
            }
            else if (i32 == 256) {
                break;
            }

            /*
            ** get more bits before we process length of match
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;
            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    if (cur_bits_left) {
                        uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                        d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                        bits_left += cur_bits_left;
                        tmp       -= cur_bits_left;
                    }
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = this->transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp) {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len) {
                        cur_uint8_t   = this->transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset){
                continue;
            }

            for (i = 0x2; i <= 0x9; i++) {
                i32 = this->transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = this->getLOMindex(i32);
                if (i == HuffLenLOM[i32]) {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            tmp       =   LOMBitsLUT[i32];
            lom       =   LOMBaseLUT[i32];
            if(tmp != 0) {
                lom += this->transposebits(d32 & (0xffffffff << (32 - tmp)));
            }
            d32       <<= tmp;
            bits_left -=  tmp;

            /* now that we have copy_offset and LoM, process them */
            src_ptr = history_ptr - copy_offset;
            tmp     = (lom > copy_offset) ? copy_offset : lom;
            uint32_t i32 = 0;
            if (src_ptr >= this->history_buf) {
                while (tmp > 0) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset) {
                    i32            = ((i32 >= copy_offset)) ? 0 : i32;
                    *history_ptr++ = *(src_ptr + i32++);
                    lom--;
             }
            }
            else {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (tmp && (src_ptr <= this->history_buf_end)) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                src_ptr = this->history_buf;
                while (tmp > 0) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset) {
                    i32            = ((i32 > copy_offset)) ? 0 : i32;
                    *history_ptr++ = *(src_ptr + i32++);
                    lom--;
                }
            }

            /*
            ** get more bits before we restart the loop
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;

            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    if (cur_bits_left) {
                        uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                        d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                        bits_left += cur_bits_left;
                        tmp       -= cur_bits_left;
                    }
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = this->transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp) {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len) {
                        cur_uint8_t   = this->transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

        }   /* end while (bits_left >= 8) */

        if (ctype & PACKET_FLUSHED) {
            *rlen = history_ptr - this->history_buf;
        }
        else {
            *rlen = history_ptr - this->history_ptr;
        }

        this->history_ptr = history_ptr;

        return true;
    }   // decompress_60

    int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) override
    {
        uint32_t roff = 0;
        int      result;

        rlen   = 0;
        result = this->decompress_60(cbuf, len, ctype, &roff, &rlen);
        rdata  = this->history_buf + roff;

        return result;
    }
};  // struct rdp_mppc_60_dec


////////////////////
//
// Compressor
//
////////////////////

static inline void insert_n_bits_60(uint8_t n, uint32_t data,
    uint8_t * outputBuffer, uint8_t & bits_left, uint16_t & opb_index, bool verbose)
{
    if (verbose) {
        LOG(LOG_INFO, "data=%u bit=%u", data, n);
    }

    assert(bits_left > 0);

    while (n)
    {
        uint8_t tmp = data << (8 - bits_left);
        outputBuffer[opb_index] |= tmp;
        if (bits_left >= n) {
            bits_left -= n;
            n = 0;
        }
        else {
            data >>= bits_left;
            n -= bits_left;
            bits_left = 0;
        }

        if (bits_left == 0)
        {
            opb_index++;
            bits_left = 8;
        }
    }
}

static inline void encode_literal_60(uint16_t c, uint8_t * outputBuffer,
    uint8_t & bits_left, uint16_t & opb_index, bool verbose)
{
    insert_n_bits_60(HuffLenLEC[c], HuffCodeLEC[c],
        outputBuffer, bits_left, opb_index, verbose);
}

struct rdp_mppc_60_enc : public rdp_mppc_enc
{
    static const size_t MINIMUM_MATCH_LENGTH             = 3;
    static const size_t MAXIMUM_MATCH_LENGTH             = 514;
    static const size_t MAXIMUM_HASH_BUFFER_UNDO_ELEMENT = 256;
    static const size_t CACHED_OFFSET_COUNT              = 4;

    using offset_type = uint16_t;
    using hash_table_manager = rdp_mppc_enc_hash_table_manager<offset_type,
                                                               MINIMUM_MATCH_LENGTH,
                                                               MAXIMUM_HASH_BUFFER_UNDO_ELEMENT>;
    using hash_type = hash_table_manager::hash_type;

    // The shared state necessary to support the transmission and reception
    //     of RDP6.0-BC compressed data between a client and server requires
    //     a history buffer and a current offset into the history buffer
    //    (HistoryOffset).
    uint8_t    historyBuffer[RDP_60_HIST_BUF_LEN];   /* contains uncompressed data */
    uint16_t   historyOffset{0};   /* next free slot in historyBuffer */

    // In addition to the history buffer and HistoryOffset, a small cache
    //     MUST also be managed by the client and server endpoints. This
    //     cache is referred to as the OffsetCache and is used to store the
    //     last four unique copy-offsets encountered during data compression
    //     (copy-offsets are described in [MS-RDPBCGR] section 3.1.8.1). This
    //     saves on bandwidth in cases where there are many repeated
    //     copy-offsets.
    // Whenever the history buffer is initialized or reinitialized, the
    //     OffsetCache MUST be emptied
    uint16_t offsetCache[CACHED_OFFSET_COUNT];

    uint8_t    outputBuffer[RDP_60_HIST_BUF_LEN];    /* contains compressed data              */
    uint16_t   bytes_in_opb{0};    /* compressed bytes available in         */
                                /*     outputBuffer                      */

    uint8_t    flags{0};           /* PACKET_COMPRESSED, PACKET_AT_FRONT,   */
                                /*     PACKET_FLUSHED etc                */
    uint8_t    flagsHold{PACKET_FLUSHED};

    hash_table_manager hash_tab_mgr;

    explicit rdp_mppc_60_enc(bool verbose = false)
        : rdp_mppc_enc(RDP_60_HIST_BUF_LEN - 1, verbose)
        // The HistoryOffset MUST start initialized to zero, while the
        //     history buffer MUST be filled with zeros. After it has been
        //     initialized, the entire history buffer is immediately
        //     regarded as valid.
        , historyBuffer{0}
        , offsetCache{0}
        , outputBuffer{0}
    {}

    static inline int cache_find(uint16_t const * offset_cache, uint16_t copy_offset)
    {
        for (int i = 0; i < 4; i++) {
            if (offset_cache[i] == copy_offset) {
                return i;
            }
        }

        return -1;
    }

    template <typename T> size_t indexOfEqualOrSmallerEntry(T data, T * table)
    {
        size_t index;
        for (index = 0; *(table + index) < data; index++);
        if (*(table + index) > data) {
            assert(index > 0);

            return index - 1;
        }
        return index;
    }

private:
    void compress_60(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "compress_60: uncompressed_data_size=%u historyOffset=%u",
                uncompressed_data_size, this->historyOffset);
        }

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wtype-limits")
        static_assert(std::numeric_limits<decltype(uncompressed_data_size)>::max() < RDP_60_HIST_BUF_LEN,
          "LOG(LOG_ERR, \"compress_60: input stream too large, max=%zu got=%u\","
          "\nRDP_60_HIST_BUF_LEN - 1u, uncompressed_data_size)");
        REDEMPTION_DIAGNOSTIC_POP

        this->flags = PACKET_COMPR_TYPE_RDP6;

        this->hash_tab_mgr.clear_undo_history();

        if ((uncompressed_data == nullptr) || (uncompressed_data_size <= 0) ||
            (uncompressed_data_size >= RDP_60_HIST_BUF_LEN - 1)){
            return;
        }

        uint16_t opb_index = 0; /* index into outputBuffer                        */
        uint8_t  bits_left = 8; /* unused bits in current uint8_t in outputBuffer */

        uint16_t saved_offset_cache[CACHED_OFFSET_COUNT];
        ::memcpy(saved_offset_cache, this->offsetCache, sizeof(saved_offset_cache));

        ::memset(this->outputBuffer, 0, RDP_60_HIST_BUF_LEN);

        if ((this->historyOffset + uncompressed_data_size + 1U) >= RDP_60_HIST_BUF_LEN) {
            /* historyBuffer cannot hold uncompressed_data - rewind it */
            ::memmove(this->historyBuffer,
                this->historyBuffer + (this->historyOffset - RDP_60_HIST_BUF_MIDDLE),
                RDP_60_HIST_BUF_MIDDLE);
            this->historyOffset =  RDP_60_HIST_BUF_MIDDLE;
            this->flagsHold     |= PACKET_AT_FRONT;
            this->hash_tab_mgr.reset();
            if (this->verbose) {
                LOG(LOG_INFO, "compress_60: flagsHold |= PACKET_AT_FRONT");
            }
        }

        // add/append new data to historyBuffer
        ::memcpy(this->historyBuffer + this->historyOffset, uncompressed_data, uncompressed_data_size);

        offset_type ctr = 0;

        // if we are at start of history buffer, do not attempt to compress
        //  first RDP_60_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1 bytes, even
        //  minimum LoM is 2
        if (this->historyOffset == 0) {
            // encode first two bytes as literals
            ctr = MINIMUM_MATCH_LENGTH - 1;
            for (offset_type i = 0; i < ctr; ++i) {
                ::encode_literal_60(
                    this->historyBuffer[this->historyOffset + i],
                    this->outputBuffer, bits_left, opb_index, this->verbose);
            }

            this->hash_tab_mgr.update_indirect(this->historyBuffer, 0);
            this->hash_tab_mgr.update_indirect(this->historyBuffer, 1);
        }

        uint16_t lom = 0;
        // we need at least 3 bytes to look for match
        for (; ctr + (MINIMUM_MATCH_LENGTH - 1) < uncompressed_data_size; ctr += lom) {
            offset_type     offset         = this->historyOffset + ctr;
            const uint8_t * data           = this->historyBuffer + offset;
            hash_type       hash           = this->hash_tab_mgr.sign(data);
            offset_type     previous_match = this->hash_tab_mgr.get_offset(hash);

            this->hash_tab_mgr.update(hash, offset);

            // check that we have a pattern match, hash is not enough

            if (0 != ::memcmp(data, this->historyBuffer + previous_match, MINIMUM_MATCH_LENGTH)) {
                // no match found; encode literal uint8_t
                ::encode_literal_60(*data, this->outputBuffer, bits_left, opb_index, this->verbose);
                lom = 1;
            }
            else {
                // we have a match - compute hash and Length of Match for triplets
                this->hash_tab_mgr.update_indirect(this->historyBuffer, offset + 1);

                for (lom = MINIMUM_MATCH_LENGTH;
                     (ctr + lom < uncompressed_data_size) && (lom < MAXIMUM_MATCH_LENGTH); lom++) {
                    this->hash_tab_mgr.update_indirect(this->historyBuffer, offset + lom - 1);
                    if (this->historyBuffer[offset + lom] !=
                        this->historyBuffer[previous_match + lom]) {
                        break;
                    }
                }

                assert(!::memcmp(this->historyBuffer + previous_match, this->historyBuffer + offset, lom));

                /////////////////////////////////////////////////////////////
                // Fix 'short compressed stream' issue with MSTSC. Length of
                //  compressed stream must be greater than 3 bytes.
                /////////////////////////////////////////////////////////////
                if (lom == uncompressed_data_size) {
                    lom--;
                }

                // encode copy_offset and insert into output buffer
                uint32_t copy_offset = offset - previous_match;

                if (this->verbose) {
                    LOG(LOG_INFO, "LoM=%u copy_offset=%u", lom, copy_offset);
                }

                int offsetCacheIndex;
                int LUTIndex;
                if ((offsetCacheIndex = cache_find(this->offsetCache, copy_offset)) != -1) {
                    assert((offsetCacheIndex >= 0) && (offsetCacheIndex <= 3));

                    if (this->verbose) {
                        LOG(LOG_INFO, "offsetCacheIndex=%d", offsetCacheIndex);
                    }

                    if (offsetCacheIndex != 0) {
                        cache_swap(this->offsetCache, offsetCacheIndex);
                    }

                    LUTIndex = offsetCacheIndex + 289;
                    if (this->verbose) {
                        LOG(LOG_INFO, "LUTIndex=%d", LUTIndex);
                    }

                    ::insert_n_bits_60(HuffLenLEC[LUTIndex], HuffCodeLEC[LUTIndex],
                        this->outputBuffer, bits_left, opb_index, this->verbose);
                }
                else {
                    cache_add(this->offsetCache, copy_offset);

                    LUTIndex = indexOfEqualOrSmallerEntry<uint32_t>(copy_offset + 1,
                        CopyOffsetBaseLUT);
                    assert((CopyOffsetBaseLUT[LUTIndex] == (copy_offset + 1)) ||
                        ((CopyOffsetBaseLUT[LUTIndex] < (copy_offset + 1)) &&
                         (CopyOffsetBaseLUT[LUTIndex + 1] > (copy_offset + 1))));
                    if (this->verbose) {
                        LOG(LOG_INFO, "LUTIndex=%d", LUTIndex);
                    }
                    int HuffmanIndex = LUTIndex + 257;
                    ::insert_n_bits_60(HuffLenLEC[HuffmanIndex], HuffCodeLEC[HuffmanIndex],
                        this->outputBuffer, bits_left, opb_index, this->verbose);

                    int ExtraBitsLength = CopyOffsetBitsLUT[LUTIndex];
                    if (ExtraBitsLength) {
                        if (this->verbose) {
                            LOG(LOG_INFO, "ExtraBitsLength=%d", ExtraBitsLength);
                        }
                        int ExtraBits   = copy_offset & ((1 << ExtraBitsLength) - 1);
                        if (this->verbose) {
                            LOG(LOG_INFO, "ExtraBits=%d", ExtraBits);
                        }
                        ::insert_n_bits_60(ExtraBitsLength, ExtraBits, this->outputBuffer,
                            bits_left, opb_index, this->verbose);
                    }
                }

                assert(lom <= 514);
                LUTIndex = indexOfEqualOrSmallerEntry<uint16_t>(lom, LOMBaseLUT);
                assert((LOMBaseLUT[LUTIndex] == lom) ||
                    ((LOMBaseLUT[LUTIndex] < lom) && (LOMBaseLUT[LUTIndex + 1] > lom)));
                if (this->verbose) {
                    LOG(LOG_INFO, "LUTIndex=%d", LUTIndex);
                }
                ::insert_n_bits_60(HuffLenLOM[LUTIndex], HuffCodeLOM[LUTIndex],
                    this->outputBuffer, bits_left, opb_index, this->verbose);

                int ExtraBitsLength = LOMBitsLUT[LUTIndex];
                if (ExtraBitsLength) {
                    if (this->verbose) {
                        LOG(LOG_INFO, "ExtraBitsLength=%d", ExtraBitsLength);
                    }
                    int ExtraBits   = (lom - 2) & ((1 << ExtraBitsLength) - 1);
                    if (this->verbose) {
                        LOG(LOG_INFO, "ExtraBits=%d", ExtraBits);
                    }
                    ::insert_n_bits_60(ExtraBitsLength, ExtraBits, this->outputBuffer,
                        bits_left, opb_index, this->verbose);
                }
            }
        }

        // add remaining data if any to the output
        while (uncompressed_data_size - ctr > 0) {
            ::encode_literal_60(uncompressed_data[ctr], this->outputBuffer, bits_left, opb_index, this->verbose);
            ++ctr;
        }

        // add End-of-Stream (EOS) marker
        ::encode_literal_60(256, this->outputBuffer, bits_left, opb_index, this->verbose);

        if (opb_index >= uncompressed_data_size) {
            if (this->verbose) {
                LOG(LOG_INFO, "compress_60: opb_index >= uncompressed_data_size");
            }
            if (!this->hash_tab_mgr.undo_last_changes()) {
                ::memset(this->historyBuffer, 0, RDP_60_HIST_BUF_LEN);
                this->historyOffset = 0;

                ::memset(this->offsetCache, 0, RDP_60_OFFSET_CACHE_SIZE);

                if (this->flagsHold & PACKET_AT_FRONT) {
                    if (this->verbose) {
                        LOG(LOG_INFO, "compress_60: this->flagsHold & PACKET_AT_FRONT");
                    }
                    this->flagsHold &= ~PACKET_AT_FRONT;
                }
                this->flagsHold |= PACKET_FLUSHED;

                this->hash_tab_mgr.reset();

                if (this->verbose) {
                    LOG(LOG_INFO, "compress_60: Unable to undo changes made in hash table.");
                }
            }
            else{
                ::memcpy(this->offsetCache, saved_offset_cache, sizeof(this->offsetCache));
            }

            return;
        }

        this->historyOffset += uncompressed_data_size;

        // if bits_left == 8, opb_index has already been incremented
        this->bytes_in_opb  =  opb_index + (bits_left != 8);
        this->flags         |= PACKET_COMPRESSED;
        this->flags         |= this->flagsHold;
        this->flagsHold     =  0;
    }   // void compress_60(const uint8_t * uncompressed_data, int uncompressed_data_size)

    void do_compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size, uint16_t /*reserved*/) override
    {
        this->compress_60(uncompressed_data, uncompressed_data_size);
        compressedType       = (this->flags & PACKET_COMPRESSED) ? this->flags        : 0;
        compressed_data_size = (this->flags & PACKET_COMPRESSED) ? this->bytes_in_opb : 0;
    }

public:
    void get_compressed_data(OutStream & stream) const override
    {
        if (stream.tailroom() < static_cast<size_t>(this->bytes_in_opb)) {
            LOG(LOG_ERR, "rdp_mppc_60_enc::get_compressed_data: Buffer too small");
            throw Error(ERR_BUFFER_TOO_SMALL);
        }

        stream.out_copy_bytes(this->outputBuffer, this->bytes_in_opb);
    }
};  // struct rdp_mppc_60_enc
