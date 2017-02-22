/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder

*/


#pragma once

#include "utils/stream.hpp"
#include "RDPOrdersCommon.hpp"

// MS-RDPEGDI 2.2.2.2.1.2.7 Cache Brush (CACHE_BRUSH_ORDER)
// ========================================================

// The Cache Brush Secondary Drawing Order is used by the server to instruct the
// client to store a brush in a particular Brush Cache entry. Support for brush
// caching is negotiated in the Brush Cache Capability Set (see [MS-RDPBCGR]
// section 2.2.7.1.7).

// header (6 bytes): Secondary Order Header, as defined in sec. 2.2.2.2.1.2.1.1.
// The embedded orderType field MUST be set to TS_CACHE_BRUSH (0x07).

// cacheEntry (1 byte): An 8-bit, unsigned integer. The entry in a specified
// Brush Cache where the brush data MUST be stored. This value MUST be in the
// range 0 to 63 (inclusive).

// iBitmapFormat (1 byte): An 8-bit, unsigned integer. The color depth of the
// brush bitmap data.
// This field MUST be one of the following values.

// Note by CGR: what about 4BPP ? Compression format for 4BPP is described below...

// +----------------+-------------------------+
// | 0x01 BMF_1BPP  | 1 bits per pixel        |
// +----------------+-------------------------+
// | 0x03 BMF_8BPP  | 8 bits per pixel        |
// +----------------+-------------------------+
// | 0X04 BMF_16BPP | 15 or 16 bits per pixel |
// +----------------+-------------------------+
// | 0X05 BMF_24BPP | 24 bits per pixel       |
// +----------------+-------------------------+

// cx (1 byte): An 8-bit, unsigned integer. The width of the brush bitmap.

// cy (1 byte): An 8-bit, unsigned integer. The height of the brush bitmap.

// Style (1 byte): An 8-bit, unsigned integer. This field is not used, and
// SHOULD be set to 0x00 (the Windows Implementation set this field to 0).

// iBytes (1 byte): An 8-bit, unsigned integer. The size of the brushData field
// in bytes.

// brushData (variable): A variable-sized byte array containing binary brush
// data that represents an 8-by-8-pixel bitmap image. There are 64 pixels in a
// brush bitmap, and the space used to represent each pixel depends on the color
// depth of the brush bitmap and the number of colors used. The size of the
// brushData field in bytes is given by the iBytes field.

// In general, most brushes only use two colors (mono format), and the majority
// of the remaining ones use four colors or fewer.

// For mono format brushes (iBitmapFormat is BMF_1BPP), brushData contains
// 8 bytes of 1-bit-per-pixel data, each byte corresponding to a row of pixels
// in the brush. The rows are encoded in reverse order; that is, the pixels in
// the bottom row of the brush are encoded in the first byte of the brushData
// field, and the pixels in the top row are encoded in the eighth byte.

// For four-color brushes, a compression algorithm is used. If the data is
// compressed, the iBytes field is 20 for 256 color (iBitmapFormat is BMF_8BPP),
// 24 for 16-bit color (iBitmapFormat is BMF_16BPP), and 28 for 24-bit color
// (iBitmapFormat is BMF_24BPP).

// The compression algorithm reduces brush data size by storing each brush pixel
// as a 2-bit index (four possible values) into a translation table containing
// four entries. This equates to 2 bytes per brush bitmap line (16 bytes in
// total) followed by the translation table contents. This layout for four-color
// brushes conforms to the Compressed Color Brush (section 2.2.2.2.1.2.7.1)
// structure.

// For brushes using more than four colors, the data is simply copied
// uncompressed into the brushData at the appropriate color depth.

// 2.2.2.2.1.2.7.1 Compressed Color Brush (COMPRESSED_COLOR_BRUSH)
// ---------------------------------------------------------------
// The COMPRESSED_COLOR_BRUSH structure is used to hold a compressed version of
// a four-color 8-by-8-pixel brush.

// +-----------------+------------------+-----------------+-----------------+
// | p4  p3  p2  p1  | p8  p7  p6  p5   | p12 p11 p10 p9  | p16 p15 p14 p13 |
// +-----------------+------------------+-----------------+-----------------+
// | p20 p19 p18 p17 | p24 p23 p22 p21  | p28 p27 p26 p25 | p32 p31 p30 p29 |
// +-----------------+------------------+-----------------+-----------------+
// | p36 p35 p34 p33 | p40 p39 p38 p37  | p44 p43 p42 p41 | p48 p47 p46 p45 |
// +-----------------+------------------+-----------------+-----------------+
// | p52 p51 p50 p49 | p54 p55 p54 p53  | p60 p59 p58 p57 | p64 p63 p62 p61 |
// +-----------------+------------------+-----------------+-----------------+
// |          color1 |           color2 |          color3 |          color4 |
// +----+-------------------------------------------------------------------+
// + pN |                                                      colorN       +
// +----+-------------------------------------------------------------------+

// pN (2 bits): A 2-bit, unsigned integer field. The 2-bit value indicating the
// translation table entry to use for pixel N.

// 0 Use the first value in the translation table (color1).
// 1 Use the second value in the translation table (color2).
// 2 Use the third value in the translation table (color3).
// 3 Use the fourth value in the translation table (color4).

// colorN (variable): Translation table entry N. This entry is an index into the
// current palette or an RGB triplet value; the actual interpretation depends on
// the color depth of the bitmap data.

//  8 bpp : 1 byte Index into the current color palette.
// 15 bpp : 2 bytes RGB color triplet expressed in 5-5-5 format
//         (5 bits for red, 5 bits for green, and 5 bits for blue).
// 16 bpp : 2 bytes RGB color triplet expressed in 5-6-5 format
//         (5 bits for red, 6 bits for green, and 5 bits for blue).
// 24 bpp : 3 bytes RGB color triplet (1 byte per component).

enum {
      BMF_1BPP  = 0x01
    , BMF_8BPP  = 0x03
    , BMF_16BPP = 0x04
    , BMF_24BPP = 0x05
};

class RDPBrushCache {
public:
    uint8_t   cacheIndex;
    uint8_t   bpp;
    uint8_t   width;
    uint8_t   height;
    uint8_t   type;
    uint8_t   size;
    uint8_t * data;

    RDPBrushCache( uint8_t cacheIndex, uint8_t bpp, uint8_t width, uint8_t height, uint8_t type
                 , uint8_t size, const uint8_t * pattern)
    : cacheIndex(cacheIndex), bpp(bpp), width(width), height(height), type(type), size(size)
    {
        this->data = reinterpret_cast<uint8_t *>(malloc(this->size));
        memcpy(this->data, pattern, this->size);
    }

    ~RDPBrushCache()
    {
        if (this->data) {
            free(this->data);
        }
    }

    void emit(OutStream & stream) const
    {
        using namespace RDP;

        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        uint16_t len = (this->size + 6) - 7;    // length after type minus 7
        stream.out_uint16_le(len);
        stream.out_uint16_le(0);    // flags
        stream.out_uint8(TS_CACHE_BRUSH); // type

        stream.out_uint8(this->cacheIndex);
        stream.out_uint8(this->bpp);
        stream.out_uint8(this->width);
        stream.out_uint8(this->height);
        stream.out_uint8(this->type);
        stream.out_uint8(this->size);
        stream.out_copy_bytes(this->data, this->size);
    }

    void receive(InStream & stream, const RDPSecondaryOrderHeader &/* header*/)
    {
        using namespace RDP;

        this->cacheIndex = stream.in_uint8();
        this->bpp        = stream.in_uint8();
        this->width      = stream.in_uint8();
        this->height     = stream.in_uint8();
        this->type       = stream.in_uint8();
        uint8_t size     = stream.in_uint8();
        if (this->size < size) {
            free(this->data);
            this->data   = static_cast<uint8_t *>(malloc(size));
        }
        this->size       = size;
        memcpy(this->data, stream.in_uint8p(this->size), this->size);
    }

    bool operator==(const RDPBrushCache & other) const {
        return (  (this->cacheIndex == other.cacheIndex)
               && (this->bpp        == other.bpp       )
               && (this->width      == other.width     )
               && (this->height     == other.height    )
               && (this->type       == other.type      )
               && (this->size       == other.size      )
               && !memcmp(this->data, other.data, this->size)
               );
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(buffer, sz, "RDPBrushCache()\n");
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level) const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        LOG(level, "%s", buffer);
    }

    void print() const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        printf("%s", buffer);
    }
};

