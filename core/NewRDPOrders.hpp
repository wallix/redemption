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

#if !defined(__NEWRDPORDERS_HPP__)
#define __NEWRDPORDERS_HPP__

#include "constants.hpp"
#include "stream.hpp"
#include "rect.hpp"
#include "altoco.hpp"
#include "bitmap.hpp"

// MS-RDPEGDI : 2.2.2.2.1.2.1.2 Two-Byte Unsigned Encoding
// =======================================================
// (TWO_BYTE_UNSIGNED_ENCODING)

// The TWO_BYTE_UNSIGNED_ENCODING structure is used to encode a value in
// the range 0x0000 to 0x7FFF by using a variable number of bytes.
// For example, 0x1A1B is encoded as { 0x9A, 0x1B }.
// The most significant bit of the first byte encodes the number of bytes
// in the structure.

// c (1 bit): A 1-bit, unsigned integer field that contains an encoded
// representation of the number of bytes in this structure. 0 implies val2 field
// is not present, if 1 val2 is present.

// val1 (7 bits): A 7-bit, unsigned integer field containing the most
// significant 7 bits of the value represented by this structure.

// val2 (1 byte): An 8-bit, unsigned integer containing the least significant
// bits of the value represented by this structure.

// MS-RDPEGDI : 2.2.2.2.1.2.1.3 Two-Byte Signed Encoding
// =====================================================
// (TWO_BYTE_SIGNED_ENCODING)

// The TWO_BYTE_SIGNED_ENCODING structure is used to encode a value in
// the range -0x3FFF to 0x3FFF by using a variable number of bytes. For example,
// -0x1A1B is encoded as { 0xDA, 0x1B }, and -0x0002 is encoded as { 0x42 }.
// The most significant bits of the first byte encode the number of bytes in
// the structure and the sign.

// c (1 bit): A 1-bit, unsigned integer field containing an encoded
// representation of the number of bytes in this structure. 0 implies that val2
// is not present, 1 implies it is present.

// s (1 bit): A 1-bit, unsigned integer field containing an encoded
// representation of whether the value is positive or negative. 0 implies
// the value is positive, 1 implies that the value is negative

// val1 (6 bits): A 6-bit, unsigned integer field containing the most
// significant 6 bits of the value represented by this structure.

// val2 (1 byte): An 8-bit, unsigned integer containing the least
// significant bits of the value represented by this structure.

// MS-RDPEGDI : 2.2.2.2.1.2.1.4 Four-Byte Unsigned Encoding
// ========================================================
// (FOUR_BYTE_UNSIGNED_ENCODING)

// The FOUR_BYTE_UNSIGNED_ENCODING structure is used to encode a value in the
// range 0x00000000 to 0x3FFFFFFF by using a variable number of bytes.
// For example, 0x001A1B1C is encoded as { 0x9A, 0x1B, 0x1C }. The two most
// significant bits of the first byte encode the number of bytes in the
// structure.

// c (2 bits): A 2-bit, unsigned integer field containing an encoded
// representation of the number of bytes in this structure.
// 0 : val1 only (1 Byte), 1 : val1 and val2 (2 Bytes),
// 2: val1, val2 and val3 (3 Bytes), 3: val1, val2, val3, val4 (4 Bytes)

// val1 (6 bits): A 6-bit, unsigned integer field containing the most
// significant 6 bits of the value represented by this structure.

// val2 (1 byte): An 8-bit, unsigned integer containing the second most
// significant bits of the value represented by this structure.

// val3 (1 byte): An 8-bit, unsigned integer containing the third most
// significant bits of the value represented by this structure.

// val4 (1 byte): An 8-bit, unsigned integer containing the least
// significant bits of the value represented by this structure.

struct RDPPen {
    uint8_t style;
    uint8_t width;
    uint32_t color;
    RDPPen(uint8_t style, uint8_t width, uint32_t color)
        : style(style), width(width), color(color) {}

    RDPPen() : style(0), width(0), color(0) {
    }

    bool operator==(const RDPPen &other) const {
        return  (this->style == other.style)
             && (this->width == other.width)
             && (this->color == other.color)
             ;
    }
};

struct RDPBrush {
    int8_t org_x;
    int8_t org_y;
    uint8_t style;
    uint8_t hatch;
    uint8_t extra[7];

    RDPBrush() :
        org_x(0),
        org_y(0),
        style(0),
        hatch(0)
        {
            memset(this->extra, 0, 7);
        }

    RDPBrush(int8_t org_x, int8_t org_y, uint8_t style, uint8_t hatch,
             const uint8_t * extra = (const uint8_t*)"\0\0\0\0\0\0\0") :
        org_x(org_x),
        org_y(org_y),
        style(style),
        hatch(hatch)
        {
            memcpy(this->extra, extra, 7);
        }

    bool operator==(const RDPBrush &other) const {
        return  (this->org_x == other.org_x)
             && (this->org_y == other.org_y)
             && (this->style == other.style)
             && (this->hatch == other.hatch)
             && ((this->style != 3) || (0 == memcmp(this->extra, other.extra, 7)))
             ;
    }

};


namespace RDP {

    // control byte
    // ------------
    enum {
        STANDARD   = 0x01, // type of order bit 1
        SECONDARY  = 0x02, // type of order bit 2
        BOUNDS     = 0x04, // the current drawing order is clipped
        CHANGE     = 0x08, // new order (order byte is there)
        DELTA      = 0x10, // coordinate fields are 1 byte delta
        LASTBOUNDS = 0x20, // use previous bounds (no bounds sent)
        SMALL      = 0x40, // -1 on number of bytes for fields
        TINY       = 0x80, // -2 on number of bytes for fields
    };

    enum {
        DESTBLT    = 0,
        PATBLT     = 1,
        SCREENBLT  = 2,
        LINE       = 9,
        RECT       = 10,
        DESKSAVE   = 11,
        MEMBLT     = 13,
        TRIBLT     = 14,
        POLYLINE   = 22,
        GLYPHINDEX = 27,
        TEXT2      = 27,
    };

    enum {
     // TS_CACHE_BITMAP_UNCOMPRESSED - Cache Bitmap - Revision 1
     // (MS-RDPEGDI section 2.2.2.2.1.2.2)
        TS_CACHE_BITMAP_UNCOMPRESSED  = 0,
     // TS_CACHE_COLOR_TABLE - Cache Color Table
     // (MS-RDPEGDI section 2.2.2.2.1.2.4)
        TS_CACHE_COLOR_TABLE      = 1,
     // TS_CACHE_BITMAP_COMPRESSED - Cache Bitmap - Revision 1
     // (MS-RDPEGDI section 2.2.2.2.1.2.2)
        TS_CACHE_BITMAP_COMPRESSED      = 2,
     // TS_CACHE_GLYPH : Cache Glyph - Revision 1
     // (MS-RDPEGDI section 2.2.2.2.1.2.5)
     // or Cache Glyph - Revision 2
     // (MS-RDPEGDI section 2.2.2.2.1.2.6) (choice through extra flags)
        FONTCACHE     = 3,
     // TS_CACHE_BITMAP_UNCOMPRESSED_REV2 : Cache Bitmap - Revision 2
     // (MS-RDPEGDI section 2.2.2.2.1.2.3)
        TS_CACHE_BITMAP_UNCOMPRESSED_REV2 = 4,
     // TS_CACHE_BITMAP_COMPRESSED_REV2 : Cache Bitmap - Revision 2
     // (MS-RDPEGDI section 2.2.2.2.1.2.3)
        TS_CACHE_BITMAP_COMPRESSED_REV2    = 5,
     // TS_CACHE_BRUSH : Cache Brush
     // (MS-RDPEGDI section 2.2.2.2.1.2.7)
        BRUSHCACHE    = 7,
     // TS_CACHE_BITMAP_COMPRESSED_REV3 : Cache Bitmap - Revision 3
     // (MS-RDPEGDI section 2.2.2.2.1.2.8)
        TS_CACHE_BITMAP_COMPRESSED_REV3     = 8
    };

    enum e_bounds {
        LEFT = 0,
        TOP = 1,
        RIGHT = 2,
        BOTTOM = 3
    };

} /* namespace */



inline static bool is_1_byte(int16_t value){
    return (value >= -128) && (value <= 127);
}

inline static uint8_t pounder_bound(int16_t delta, uint8_t pound)
{
    return ((pound * (delta != 0)) << (4 * is_1_byte(delta)));
}


class RDPSecondaryOrderHeader {
    // MS-RDPEGDI : 2.2.2.2.1.2.1.1 Secondary Drawing Order Header
    // (SECONDARY_DRAWING_ORDER_HEADER)
    // The SECONDARY_DRAWING_ORDER_HEADER structure is included in all secondary
    // drawing orders.

    // controlFlags (1 byte): An 8-bit, unsigned integer. The control byte that
    // identifies the class of the drawing order. This field MUST contain the
    // TS_STANDARD (0x01) and TS_SECONDARY (0x02) flags to indicate that the
    // order is a secondary drawing order (see section 2.2.2.1.1).

    // orderLength (2 bytes): A 16-bit, signed integer. The encoded length in
    // bytes of the secondary drawing order, including the size of the header.
    // When constructing the order, the value in the orderLength field MUST be
    // 13 bytes less than the actual order length. Hence, when decoding the
    // order, the orderLength field MUST be adjusted by adding 13 bytes. These
    // adjustments are for historical reasons.

    // extraFlags (2 bytes): A 16-bit, unsigned integer. Flags specific to each
    // secondary drawing order.

    // orderType (1 byte): An 8-bit, unsigned integer. Identifies the type of
    // secondary drawing order. MUST be one of the following values.
    // Value Meaning
    // -----------------------------------------+-------------------------------
    // | TS_CACHE_BITMAP_UNCOMPRESSED 0x00      | Cache Bitmap - Revision 1    |
    // |                                        | (section 2.2.2.2.1.2.2)      |
    // |                                        | Secondary Drawing Order with |
    // |                                        | an uncompressed bitmap.      |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_COLOR_TABLE 0x01              | Cache Color Table            |
    // |                                        | (section 2.2.2.2.1.2.4)      |
    // |                                        | Secondary Drawing Order.     |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_BITMAP_COMPRESSED 0x02        | Cache Bitmap - Revision 1    |
    // |                                        | (section 2.2.2.2.1.2.2)      |
    // |                                        | Secondary Drawing Order      |
    // |                                        | with a compressed bitmap.    |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_GLYPH 0x03                    | Cache Glyph - Revision 1     |
    // |                                        | (section 2.2.2.2.1.2.5)      |
    // |                                        | or Cache Glyph - Revision 2  |
    // |                                        | (section 2.2.2.2.1.2.6)      |
    // |                                        | Secondary Drawing Order.     |
    // |                                        | The version is indicated by  |
    // |                                        | the extraFlags field.        |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_BITMAP_UNCOMPRESSED_REV2 0x04 | Cache Bitmap - Revision 2    |
    // |                                        | (section 2.2.2.2.1.2.3)      |
    // |                                        | Secondary Drawing Order with |
    // |                                        | an uncompressed bitmap.      |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_BITMAP_COMPRESSED_REV2 0x05   | Cache Bitmap - Revision 2    |
    // |                                        | (section 2.2.2.2.1.2.3)      |
    // |                                        | Secondary Drawing Order with |
    // |                                        | a compressed bitmap.         |
    // +----------------------------------------+------------------------------+
    // | TS_CACHE_BRUSH 0x07                    | Cache Brush                  |
    // |                                        | (section 2.2.2.2.1.2.7)      |
    // |                                        | Secondary Drawing Order.     |
    // +----------------------------------------|------------------------------+
    // | TS_CACHE_BITMAP_COMPRESSED_REV3 0x08   | Cache Bitmap - Revision 3    |
    // |                                        | (section 2.2.2.2.1.2.8)      |
    // |                                        | Secondary Drawing Order with |
    // |                                        | a compressed bitmap.         |
    // +----------------------------------------+------------------------------+

    public:
    uint16_t length;
    unsigned flags;
    unsigned type;

    RDPSecondaryOrderHeader(Stream & stream){
        this->length = stream.in_uint16_le();
        this->flags = stream.in_uint16_le();
        this->type = stream.in_uint8();
    }
    RDPSecondaryOrderHeader(uint16_t length, unsigned flags, unsigned type):
        length(length), flags(flags), type(type)
    {
    }
};


class RDPColCache {
    /*****************************************************************************/
    // [MS-RDPGDI] 2.2.2.2.1.2.4 Cache Color Table (CACHE_COLOR_TABLE_ORDER)

    // The Cache Color Table Secondary Drawing Order is used by the server
    // to instruct the client to store a color table in a particular Color Table
    // Cache entry. Color tables are used in the MemBlt (section 2.2.2.2.1.1.2.9)
    // and Mem3Blt (section 2.2.2.2.1.1.2.10) Primary Drawing Orders.

    // Support for color table caching is not negotiated in the Color Table Cache
    // Capability Set (section 2.2.1.1), but is instead implied by support for
    // the MemBlt (section 2.2.2.2.1.1.2.9) and Mem3Blt (section 2.2.2.2.1.1.2.10)
    // Primary Drawing Orders. If support for these orders is advertised in the
    // Order Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3), the existence
    // of a color table cache with entries for six palettes is assumed when
    // palettized color is being used, and the Cache Color Table is used to
    // update these palettes.

    // header (6 bytes): A Secondary Order Header, as defined in section
    // 2.2.2.2.1.2.1.1. The embedded orderType field MUST be set to
    // TS_CACHE_COLOR_TABLE (0x01).

    // cacheIndex (1 byte): An 8-bit, unsigned integer. An entry in the Cache
    // Color Table where the color table MUST be stored. This value MUST be in
    // the range 0 to 5 (inclusive).

    // numberColors (2 bytes): A 16-bit, unsigned integer. The number of Color
    // Quad (section 2.2.2.2.1.2.4.1) structures in the colorTable field. This
    // field MUST be set to 256 entries.

    // colorTable (variable): A Color Table composed of an array of Color Quad
    // (section 2.2.2.2.1.2.4.1) structures. The number of entries in the array
    // is given by the numberColors field.

    // 2.2.2.2.1.2.4.1 Color Quad (TS_COLOR_QUAD)
    // The TS_COLOR_QUAD structure is used to express the red, green, and blue
    // components necessary to reproduce a color in the additive RGB space.

    // blue (1 byte): An 8-bit, unsigned integer. The blue RGB color component.

    // green (1 byte): An 8-bit, unsigned integer. The green RGB color component.

    // red (1 byte): An 8-bit, unsigned integer. The red RGB color component.

    // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in this
    // field are arbitrary and MUST be ignored.

    public:
    uint32_t palette[6][256];

    RDPColCache()
    {
        memset(this->palette, 0, sizeof(palette));
    }

    void emit(Stream & stream, uint8_t cacheIndex)
    {
        using namespace RDP;

        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        uint16_t len = 1027 - 7;    // length after type minus 7
        stream.out_uint16_le(len);
        stream.out_uint16_le(0);    // flags
        stream.out_uint8(TS_CACHE_COLOR_TABLE); // type

        stream.out_uint8(cacheIndex);
        stream.out_uint16_le(256); /* num colors */
        for (int i = 0; i < 256; i++) {
            int color = this->palette[cacheIndex][i];
            uint8_t r = color >> 16;
            uint8_t g = color >> 8;
            uint8_t b = color;
            stream.out_uint8(r);
            stream.out_uint8(g);
            stream.out_uint8(b);
            stream.out_uint8(0x00);
        }
    }

    void receive(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        using namespace RDP;

        uint8_t cacheIndex = stream.in_uint8();
        assert(cacheIndex >= 0 && cacheIndex < 6);

        uint16_t numberColors = stream.in_uint16_le();
        assert(numberColors == 256);

        for (size_t i = 0; i < 256; i++) {
            this->palette[cacheIndex][i] = stream.in_uint32_le();
        }
    }

    #define warning remove printf in operator== and show palette differences in test code
    bool operator==(const RDPColCache & other) const {
        for (uint8_t cacheIndex = 0; cacheIndex < 6 ; ++cacheIndex){
            for (size_t i = 0; i < 256 ; ++i){
                if (this->palette[cacheIndex][i] != other.palette[cacheIndex][i]){
                    printf("palette differs at index %d: %x != %x\n",
                        (int)i, this->palette[cacheIndex][i], other.palette[cacheIndex][i]);
                    return false;
                }
            }
        }
        return true;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(
            buffer,
            sz,
            "RDPColCache("
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...]"
            "[%d, %d, %d,...])\n",
            this->palette[0][0], this->palette[0][1], this->palette[0][2],
            this->palette[1][0], this->palette[1][1], this->palette[1][2],
            this->palette[2][0], this->palette[2][1], this->palette[2][2],
            this->palette[3][0], this->palette[3][1], this->palette[3][2],
            this->palette[4][0], this->palette[4][1], this->palette[4][2],
            this->palette[5][0], this->palette[5][1], this->palette[5][2]);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

};


class RDPBmpCache {
    // [MS-RDPGDI] 2.2.2.2.1.2.2 Cache Bitmap - Revision 1 (CACHE_BITMAP_ORDER)
    // ========================================================================

    // The Cache Bitmap - Revision 1 Secondary Drawing Order is used by the
    // server to instruct the client to store a bitmap in a particular Bitmap
    // Cache entry. This order only supports memory-based bitmap caching.
    // Support for Revision 1 bitmap caching is negotiated in the Bitmap Cache
    // Capability Set (Revision 1) (see [MS-RDPBCGR] section 2.2.7.1.4.1).

    // header (6 Bytes) : The Secondary Drawing Order Header
    //  (section 2.2.2.2.1.2.1.1).

    //The embedded orderType field MUST be set to one of the following values.
    // header::orderType
    // +-----------------------------------+-----------------------------------+
    // | 0x00 TS_CACHE_BITMAP_UNCOMPRESSED | The bitmap data in the            |
    // |                                   | bitmapDataStream field is         |
    // |                                   | uncompressed.                     |
    // +-----------------------------------------------------------------------+
    // | 0x02 TS_CACHE_BITMAP_COMPRESSED   | The bitmap data in the            |
    // |                                   | bitmapDataStream field is         |
    // |                                   | compressed.                       |
    // +-----------------------------------+-----------------------------------+

    // header::extraFlags
    //  The embedded extraFlags field MAY contain the following flag.
    //  0x00000400 NO_BITMAP_COMPRESSION_HDR Indicates that the bitmapComprHdr
    //  field is not present (removed for bandwidth efficiency to save 8 bytes).

    // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
    //  which to store the bitmap data. The bitmap cache ID MUST be in the range
    //  negotiated by the Bitmap Cache Capability Set (Revision 1)
    //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

    // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in this
    //  field are arbitrary and MUST be ignored.

    // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the bitmap
    //  in pixels.

    // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
    //  bitmap in pixels.

    // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color depth
    //  of the bitmap data in bits-per-pixel. This field MUST be one of the
    //  following values.
    //  0x08 8-bit color depth.
    //  0x10 16-bit color depth.
    //  0x18 24-bit color depth.
    //  0x20 32-bit color depth.

    // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    // the data in the bitmapComprHdr and bitmapDataStream fields.

    // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the bitmap
    //  cache (specified by the cacheId field) where the bitmap MUST be stored.
    //  The bitmap cache index MUST be in the range negotiated by the Bitmap
    //  Cache Capability Set (Revision 1) (see [MS-RDPBCGR] section
    //  2.2.7.1.4.1).

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
    //  [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3) describing the bitmap data in
    //  the bitmapDataStream. This field MUST be present if the
    //  TS_CACHE_BITMAP_COMPRESSED (0x02) flag is present in the header field,
    //  but the NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not.

    // bitmapDataStream (variable): A variable-length byte array containing
    //  bitmap data (the format of this data is defined in [MS-RDPBCGR] section
    //  2.2.9.1.1.3.1.2.2).

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
    // ----------------------------------------------------------
    //  The TS_BITMAP_DATA structure wraps the bitmap data bytestream for a
    //  screen area rectangle containing a clipping taken from the server-side
    //  screen frame buffer.

    // destLeft (2 bytes): A 16-bit, unsigned integer. Left bound of the
    //  rectangle.

    // destTop (2 bytes): A 16-bit, unsigned integer. Top bound of the
    //  rectangle.

    // destRight (2 bytes): A 16-bit, unsigned integer. Right bound of the
    //  rectangle.

    // destBottom (2 bytes): A 16-bit, unsigned integer. Bottom bound of the
    //  rectangle.

    // width (2 bytes): A 16-bit, unsigned integer. The width of the rectangle.

    // height (2 bytes): A 16-bit, unsigned integer. The height of the
    //  rectangle.

    // bitsPerPixel (2 bytes): A 16-bit, unsigned integer. The color depth of
    //  the rectangle data in bits-per-pixel.

    // Flags (2 bytes): A 16-bit, unsigned integer. The flags describing the
    //  format of the bitmap data in the bitmapDataStream field.

    // +----------------------------------+------------------------------------+
    // | 0x0001 BITMAP_COMPRESSION        | Indicates that the bitmap data is  |
    // |                                  | compressed.This implies that the   |
    // |                                  | bitmapComprHdr field is present if |
    // |                                  | the NO_BITMAP_COMPRESSION_HDR      |
    // |                                  | (0x0400) flag is not set.          |
    // +----------------------------------+------------------------------------+
    // | 0x0400 NO_BITMAP_COMPRESSION_HDR | Indicates that the bitmapComprHdr  |
    // |                                  | field is not present (removed for  |
    // |                                  | bandwidth efficiency to save 8     |
    // |                                  | bytes).                            |
    // +----------------------------------+------------------------------------+

    // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    //  the data in the bitmapComprHdr and bitmapDataStream fields.

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure
    //  (see Compressed Data Header (TS_CD_HEADER) (section 2.2.9.1.1.3.1.2.3))
    //  specifying the bitmap data in the bitmapDataStream. This field MUST be
    //  present if the BITMAP_COMPRESSION (0x0001) flag is present in the Flags
    //  field, but the NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not.

    // bitmapDataStream (variable): A variable-sized array of bytes.
    //  Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).

    // Compressed bitmaps not in 32 bpp format are compressed using Interleaved
    // RLE and encapsulated in an RLE Compresssed Bitmap Stream structure (see
    // section 2.2.9.1.1.3.1.2.4) while compressed bitmap data at a color depth
    // of 32 bpp is compressed using RDP 6.0 Bitmap Compression and stored
    // inside an RDP 6.0 Bitmap Compressed Stream structure (see section
    // 2.2.2.5.1 in [MS-RDPEGDI]).

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.3 Compressed Data Header (TS_CD_HEADER)
    // -------------------------------------------------------------------

    // The TS_CD_HEADER structure is used to describe compressed bitmap data.

    // cbCompFirstRowSize (2 bytes): A 16-bit, unsigned integer. The field
    //  MUST be set to 0x0000.

    // cbCompMainBodySize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the compressed bitmap data (which follows this header).

    // cbScanWidth (2 bytes): A 16-bit, unsigned integer. The width of the
    //  bitmap (which follows this header) in pixels (this value MUST be
    //  divisible by 4).

    // cbUncompressedSize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the bitmap data (which follows this header) after it has been
    //  decompressed.

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.4 RLE Compressed Bitmap Stream
    // ----------------------------------------------------------

    // (RLE_BITMAP_STREAM)
    // The RLE_BITMAP_STREAM structure contains a stream of bitmap data
    // compressed using Interleaved Run-Length Encoding (RLE). Compressed
    // bitmap data MUST follow a Compressed Data Header (section
    // 2.2.9.1.1.3.1.2.3) structure unless exclusion of this header has been
    // negotiated in the General Capability Set (section 2.2.7.1.1).

    // A compressed bitmap is sent as a series of compression orders that
    // instruct the decoder how to reassemble a compressed bitmap (a particular
    // bitmap can have many valid compressed representations). A compression
    // order consists of a one-byte order header, followed by an optional
    // encoded run length, followed by optional data associated with the
    // compression order. Some orders require the decoder to refer to the
    // previous scanline of bitmap data and because of this fact the first
    // scanline sometimes requires special cases for decoding.

    // Standard Compression Orders begin with a one byte order header. The high
    // order bits of this header contain a code identifier, while the low order
    // bits store the length of the associated run (unless otherwise
    // specified).

    // There are two forms of Standard Compression Orders:
    // - The regular form contains a 3-bit code identifier and a 5-bit run
    //  length.
    // - The lite form contains a 4-bit code identifier and a 4-bit run
    //  length.

    // For both the regular and lite forms a run length of zero indicates an
    // extended run (a MEGA run), where the byte following the order header
    // contains the encoded length of the associated run. The encoded run
    // length is calculated using the following formula (unless otherwise
    // specified):

    // EncodedMegaRunLength = RunLength - (MaximumNonMegaRunLength + 1)

    // The maximum run length that can be stored in a non-MEGA regular order is
    // 31, while a non-MEGA lite order can only store a maximum run length of
    // 15.

    // Extended Compression Orders begin with a one byte order header which
    // contains an 8-bit code identifier. There are two types of Extended
    // Compression Orders:

    // The MEGA_MEGA type stores the length of the associated run in the two
    // bytes following the order header (in little-endian order). In the
    // MEGA_MEGA form the stored run length is the plain unsigned 16-bit length
    // of the run.

    // The single-byte type is used to encode short, commonly occurring
    // foreground/background sequences and single black or white pixels.

    // Pseudo-code describing how to decompress a compressed bitmap stream can
    // be found in section 3.1.9.

    // rleCompressedBitmapStream (variable): An array of compression codes
    //  describing compressed structures in the bitmap.

    // Background Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Background Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline. If there is
    // no previous scanline then each pixel in the run MUST be black.

    // When encountering back-to-back background runs, the decompressor MUST
    // write a one-pixel foreground run to the destination buffer before
    // processing the second background run if both runs occur on the first
    // scanline or after the first scanline (if the first run is on the first
    // scanline, and the second run is on the second scanline, then a one-pixel
    // foreground run MUST NOT be written to the destination buffer). This
    // one-pixel foreground run is counted in the length of the run.

    // The run length encodes the number of pixels in the run. There is no data
    // associated with Background Run Orders.

    // +-----------------------+-----------------------------------------------+
    // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
    // |                       | background run. The run length is stored in   |
    // |                       | the five low-order bits of  the order header  |
    // |                       | byte. If this value is zero, then the run     |
    // |                       | length is encoded in the byte following the   |
    // |                       | order header and MUST be incremented by 32 to |
    // |                       | give the final value.                         |
    // +-----------------------+-----------------------------------------------+
    // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
    // |                       | background run. The run length is stored in   |
    // |                       | the two bytes following the order header      |
    // |                       | (in little-endian format).                    |
    // +-----------------------+-----------------------------------------------+

    // Foreground Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Foreground Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline XOR’ed with
    // the current foreground color. If there is no previous scanline, then
    // each pixel in the run MUST be set to the current foreground color (the
    // initial foreground color is white).

    // The run length encodes the number of pixels in the run.
    // If the order is a "set" variant, then in addition to encoding a run of
    // pixels, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +---------------------------+-------------------------------------------+
    // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
    // |                           | regular-form foreground run. The run      |
    // |                           | length is stored in the five low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 32 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
    // |                           | foreground run. The run length is stored  |
    // |                           | in the two bytes following the order      |
    // |                           | header (in little-endian format).         |
    // +---------------------------+-------------------------------------------+
    // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
    // |                           | variant lite-form foreground run. The run |
    // |                           | length is stored in the four low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 16 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
    // |                           | variant MEGA_MEGA foreground run. The run |
    // |                           | length is stored in the two bytes         |
    // |                           | following the order header (in            |
    // |                           | little-endian format).                    |
    // +---------------------------+-------------------------------------------+

    // Dithered Run Orders
    // ~~~~~~~~~~~~~~~~~~~

    // A Dithered Run Order encodes a run of pixels which is composed of two
    // alternating colors. The two colors are encoded (in little-endian format)
    // in the bytes following the optional run length.

    // The run length encodes the number of pixel-pairs in the run (not pixels).

    // +-----------------------------+-----------------------------------------+
    // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
    // |                             | lite-form dithered run. The run length  |
    // |                             | is stored in the four low-order bits of |
    // |                             | the order header byte. If this value is |
    // |                             | zero, then the run length is encoded in |
    // |                             | the byte following the order header and |
    // |                             | MUST be incremented by 16 to give the   |
    // |                             | final value.                            |
    // +-----------------------------+-----------------------------------------+
    // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
    // |                             | MEGA_MEGA dithered run. The run length  |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    // Color Run Orders
    // ~~~~~~~~~~~~~~~~

    // A Color Run Order encodes a run of pixels where each pixel is the same
    // color. The color is encoded (in little-endian format) in the bytes
    // following the optional run length.

    // The run length encodes the number of pixels in the run.

    // +--------------------------+--------------------------------------------+
    // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
    // |                          | regular-form color run. The run length is  |
    // |                          | stored in the five low-order bits of the   |
    // |                          | order header byte. If this value is zero,  |
    // |                          | then the run length is encoded in the byte |
    // |                          | following the order header and MUST be     |
    // |                          | incremented by 32 to give the final value. |
    // +--------------------------+--------------------------------------------+
    // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
    // |                          | color run. The run length is stored in the |
    // |                          | two bytes following the order header (in   |
    // |                          | little-endian format).                     |
    // +--------------------------+--------------------------------------------+

    // Foreground / Background Image Orders
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // A Foreground/Background Image Order encodes a binary image where each
    // pixel in the image that is not on the first scanline fulfils exactly one
    // of the following two properties:

    // (a) The pixel matches the uncompressed pixel on the previous scanline
    // XOR'ed with the current foreground color.

    // (b) The pixel matches the uncompressed pixel on the previous scanline.

    // If the pixel is on the first scanline then it fulfils exactly one of the
    // following two properties:

    // (c) The pixel is the current foreground color.

    // (d) The pixel is black.

    // The binary image is encoded as a sequence of byte-sized bitmasks which
    // follow the optional run length (the last bitmask in the sequence can be
    // smaller than one byte in size). If the order is a "set" variant then the
    // bitmasks MUST follow the bytes which specify the new foreground color.
    // Each bit in the encoded bitmask sequence represents one pixel in the
    // image. A bit that has a value of 1 represents a pixel that fulfils
    // either property (a) or (c), while a bit that has a value of 0 represents
    // a pixel that fulfils either property (b) or (d). The individual bitmasks
    // MUST each be processed from the low-order bit to the high-order bit.

    // The run length encodes the number of pixels in the run.

    // If the order is a "set" variant, then in addition to encoding a binary
    // image, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +--------------------------------+--------------------------------------+
    // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
    // |                                | regular-form foreground/background   |
    // |                                | image. The run length is encoded in  |
    // |                                | the five low-order bits of the order |
    // |                                | header byte and MUST be multiplied   |
    // |                                | by 8 to give the final value. If     |
    // |                                | this value is zero, then the run     |
    // |                                | length is encoded in the byte        |
    // |                                | following the order header and MUST  |
    // |                                | be incremented by 1 to give the      |
    // |                                | final value.                         |
    // +--------------------------------+--------------------------------------+
    // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
    // |                                | MEGA_MEGA foreground/background      |
    // |                                | image. The run length is stored in   |
    // |                                | the two bytes following the order    |
    // |                                | header (in little-endian format).    |
    // +--------------------------------+--------------------------------------+
    // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
    // |                                | "set" variant lite-form              |
    // |                                | foreground/background image. The run |
    // |                                | length is encoded in the four        |
    // |                                | low-order bits of the order header   |
    // |                                | byte and MUST be multiplied by 8 to  |
    // |                                | give the final value. If this value  |
    // |                                | is zero, then the run length is      |
    // |                                | encoded in the byte following the    |
    // |                                | order header and MUST be incremented |
    // |                                | by 1 to give the final value.        |
    // +--------------------------------+--------------------------------------+
    // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
    // |                                | "set" variant MEGA_MEGA              |
    // |                                | foreground/background image. The run |
    // |                                | length is stored in the two bytes    |
    // |                                | following the order header (in       |
    // |                                | little-endian format).               |
    // +-----------------------------------------------------------------------+

    // Color Image Orders
    // ~~~~~~~~~~~~~~~~~~

    // A Color Image Order encodes a run of uncompressed pixels.

    // The run length encodes the number of pixels in the run. So, to compute
    // the actual number of bytes which follow the optional run length, the run
    // length MUST be multiplied by the color depth (in bits-per-pixel) of the
    // bitmap data.

    // +-----------------------------+-----------------------------------------+
    // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
    // |                             | regular-form color image. The run       |
    // |                             | length is stored in the five low-order  |
    // |                             | bits of the order header byte. If this  |
    // |                             | value is zero, then the run length is   |
    // |                             | encoded in the byte following the order |
    // |                             | header and MUST be incremented by 32 to |
    // |                             | give the final value.                   |
    // +-----------------------------+-----------------------------------------+
    // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
    // |                             | MEGA_MEGA color image. The run length   |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    // Special Orders
    // ~~~~~~~~~~~~~~

    // +----------------------+------------------------------------------------+
    // | 0xF9 SPECIAL_FGBG_1  | The compression order encodes a                |
    // |                      | foreground/background image with an 8-bit      |
    // |                      | bitmask of 0x03.                               |
    // +----------------------+------------------------------------------------+
    // | 0xFA SPECIAL_FGBG_2  | The compression order encodes a                |
    // |                      | foreground/background image with an 8-bit      |
    // |                      | bitmask of 0x05.                               |
    // +----------------------+------------------------------------------------+
    // | 0xFD WHITE           | The compression order encodes a single white   |
    // |                      | pixel.                                         |
    // +----------------------+------------------------------------------------+
    // | 0xFE BLACK           | The compression order encodes a single black   |
    // |                      | pixel.                                         |
    // +----------------------+------------------------------------------------+

    public:
    int cache_id;
    int width;
    int height;
    int bpp;
    int cache_idx;
    uint8_t * data;
    uint8_t orderType;

    RDPBmpCache(int orderType, Bitmap & bmp, int cache_id, int cache_idx) :
                    cache_id(cache_id),
                    width(bmp.cx), height(bmp.cy), bpp(bmp.bpp),
                    cache_idx(cache_idx),
                    data(bmp.data_co),
                    orderType(orderType)
    {
    }

    RDPBmpCache()
    {
    }

    void emit(Stream & stream)
    {
        using namespace RDP;
        switch (this->orderType){
            case TS_CACHE_BITMAP_UNCOMPRESSED:
                this->emit_raw_v1(stream);
            break;
            case TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                this->emit_raw_v2(stream);
            break;
        }
    }

    void emit_raw_v1(Stream & stream)
    {
        using namespace RDP;
        unsigned int row_size = align4(width * nbbytes(bpp));

        #warning this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
//        LOG(LOG_INFO, "out_uint8::Standard and secondary");

        stream.out_uint16_le(9 + this->height * row_size  - 7); // length after orderType - 7
//        LOG(LOG_INFO, "out_uint16_le::len %d\n", 9 + this->height * row_size - 7);

        stream.out_uint16_le(8);        // extraFlags
//        LOG(LOG_INFO, "out_uint16_le::flags %d\n", 8);

        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED); // type
//        LOG(LOG_INFO, "out_uint8::orderType %d\n", RAW_BMPCACHE);

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

        stream.out_uint8(this->cache_id);
//        LOG(LOG_INFO, "out_uint8::cache_id %d\n", cache_id);

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.

        stream.out_clear_bytes(1);
//        LOG(LOG_INFO, "out_uint8::pad\n");

        // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the
        // bitmap in pixels.

        stream.out_uint8(this->width);
//        LOG(LOG_INFO, "out_uint8::width=%d\n", width);

        // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
        //  bitmap in pixels.

        stream.out_uint8(this->height);
//        LOG(LOG_INFO, "out_uint8::height=%d\n", height);

        // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color
        //  depth of the bitmap data in bits-per-pixel. This field MUST be one
        //  of the following values.
        //  0x08 8-bit color depth.
        //  0x10 16-bit color depth.
        //  0x18 24-bit color depth.
        //  0x20 32-bit color depth.

        stream.out_uint8(this->bpp);
//        LOG(LOG_INFO, "out_uint8::bpp=%d\n", bpp);

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.

//        LOG(LOG_INFO, "out_uint16::bufsize=%d\n", bufsize);
        stream.out_uint16_le(this->height * row_size);

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).

        stream.out_uint16_le(this->cache_idx);

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        for (int y = 0 ; y < this->height; y++) {
            stream.out_copy_bytes(this->data + y * row_size, row_size);
        }
    }

    // MS-RDPEGDI 2.2.2.2.1.2.3
    // ========================
    // Cache Bitmap - Revision 2 (CACHE_BITMAP_REV2_ORDER)
    // ---------------------------------------------------
    // The Cache Bitmap - Revision 2 Secondary Drawing Order is used by the
    // server to instruct the client to store a bitmap in a particular Bitmap
    // Cache entry. This order supports persistent disk bitmap caching and uses
    // a compact encoding format. Support for Revision 2 bitmap caching is
    // negotiated in the Bitmap Cache Capability Set (Revision 2)
    // (see [MS-RDPBCGR] section 2.2.7.1.4.2).

    // header (6 bytes): A Secondary Drawing Order Header
    // (section 2.2.2.2.1.2.1.1). The embedded orderType field MUST be set to
    // one of the following values:

    // header::orderType
    // +----------------------------------------+------------------------------+
    // | 0x04 TS_CACHE_BITMAP_UNCOMPRESSED_REV2 | The bitmap data in the       |
    // |                                        | bitmapDataStream field is    |
    // |                                        | uncompressed.                |
    // +-----------------------------------------------------------------------+
    // | 0x05 TS_CACHE_BITMAP_COMPRESSED_REV2   | The bitmap data in the       |
    // |                                        | bitmapDataStream field is    |
    // |                                        | compressed.                  |
    // +----------------------------------------+------------------------------+

    // The format of the embedded extraFlags field is specified as follow :
    // bits 0 to 2  : cacheId
    // bits 3 to 6  : bitsPerPixelId
    // bits 7 to 15 : flags

    // header::extraFlags::cacheId[0..2] (3 bits)
    // cacheId (3 bits): A 3-bit, unsigned integer. The bitmap cache into which
    // to store the bitmap data. The bitmap cache ID MUST be in the range
    // negotiated by the Bitmap Cache Capability Set (Revision 2)
    // (see [MS-RDPBCGR] section 2.2.7.1.4.2).

    // header::extraFlags::bitsPerPixelId[3..6]
    // bitsPerPixelId (4 bits): A 4-bit, unsigned integer. The color depth of
    // the bitmap data in bits-per-pixel. MUST be one of the following values.

    // +--------------------+-------------------+
    // | 0x03 CBR2_8BPP     | 8 bits per pixel  |
    // +--------------------+-------------------+
    // | 0x04 CBR2_16BPP    | 16 bits per pixel |
    // +--------------------+-------------------+
    // | 0x05 CBR2_24BPP    | 24 bits per pixel |
    // +--------------------+-------------------+
    // | 0x06 CBR2_32BPP    | 32 bits per pixel |
    // +--------------------+-------------------+

    // header::extraFlags::flags[7..15]
    // A 9-bit, unsigned integer. Operational flags.

    // +----------------------------------------+------------------------------+
    // | 0x01 CBR2_HEIGHT_SAME_AS_WIDTH         | Implies that the bitmap      |
    // |                                        | height is the same as the    |
    // |                                        | bitmap width. If this flag is|
    // |                                        | set, the bitmapHeight field  |
    // |                                        | MUST NOT be present.         |
    // +----------------------------------------+------------------------------+
    // +----------------------------------------+------------------------------+
    // | 0x02 CBR2_PERSISTENT_KEY_PRESENT       | Implies that the bitmap is   |
    // |                                        | intended to be persisted,    |
    // |                                        | and the key1 and key2 fields |
    // |                                        | MUST be present.             |
    // +----------------------------------------+------------------------------+
    // | 0x08 CBR2_NO_BITMAP_COMPRESSION_HEADER | Indicates that the           |
    // |                                        | bitmapComprHdr field is not  |
    // |                                        | present (removed for         |
    // |                                        | bandwidth efficiency to save |
    // |                                        | 8 bytes).                    |
    // +----------------------------------------+------------------------------+
    // | 0x10 CBR2_DO_NOT_CACHE                 | Implies that the cacheIndex  |
    // |                                        | field MUST be ignored, and   |
    // |                                        | the bitmap MUST be placed in |
    // |                                        | the last entry of the bitmap |
    // |                                        | cache specified by cacheId   |
    // |                                        | field.                       |
    // +----------------------------------------+------------------------------+

    // key1 (4 bytes): A 32-bit, unsigned integer. The low 32 bits of the 64-bit
    //                 persistent bitmap cache key.

    // key2 (4 bytes): A 32-bit, unsigned integer. The high 32 bits of the
    //                 64-bit persistent bitmap cache key.

    // bitmapWidth (variable): A Two-Byte Unsigned Encoding (section
    //                         2.2.2.2.1.2.1.2) structure. The width of the
    //                         bitmap in pixels.

    // bitmapHeight (variable): A Two-Byte Unsigned Encoding (section
    //                          2.2.2.2.1.2.1.2) structure. The height of the
    //                          bitmap in pixels.

    // bitmapLength (variable): A Four-Byte Unsigned Encoding (section
    //                          2.2.2.2.1.2.1.4) structure. The size in bytes
    //                          of the data in the bitmapComprHdr and
    //                          bitmapDataStream fields.

    // cacheIndex (variable): A Two-Byte Unsigned Encoding (section
    //                        2.2.2.2.1.2.1.2) structure. An entry in the bitmap
    //                        cache (specified by the cacheId field) where the
    //                        bitmap MUST be stored. If the CBR2_DO_NOT_CACHE
    //                        flag is not set in the header field, the bitmap
    //                        cache index MUST be in the range negotiated by the
    //                        Bitmap Cache Capability Set (Revision 2) (see
    //                        [MS-RDPBCGR] section 2.2.7.1.4.2). Otherwise, if
    //                        the CBR2_DO_NOT_CACHE flag is set, the cacheIndex
    //                        MUST be set to BITMAPCACHE_WAITING_LIST_INDEX
    //                        (32767).

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
    //                           [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3)
    //                           describing the bitmap data in the
    //                           bitmapDataStream. This field MUST be present if
    //                           the TS_CACHE_BITMAP_COMPRESSED_REV2 (0x05) flag
    //                           is present in the header field, but the
    //                           CBR2_NO_BITMAP_COMPRESSION_HDR (0x08) flag is
    //                           not.

    // bitmapDataStream (variable): A variable-length byte array containing
    //                              bitmap data (the format of this data is
    //                              defined in [MS-RDPBCGR] section
    //                              2.2.9.1.1.3.1.2.2).

    void emit_raw_v2(Stream & stream)
    {
        using namespace RDP;
        unsigned int row_size = align4(width * nbbytes(bpp));

        #warning this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);

        // here is length, it will be computed when packet will be complete
        uint8_t * length_ptr = stream.p;
        stream.skip_uint8(2);

        int bitsPerPixelId = nbbytes(this->bpp)+2;

        #warning some optimisations are possible here if we manage flags, but what will we do with persistant bitmaps ? We definitely do not want to save them on disk from here. There must be some kind of persistant structure where to save them and check if they exist.
        uint16_t flags = 0;

        // header::extraFlags : (flags:9, bitsPerPixelId:3, cacheId:3)
        stream.out_uint16_le((flags << 6)
            |((bitsPerPixelId << 3)& 0x38)
            | (this->cache_id & 7));

        // header::orderType
        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED_REV2);

        // key1 and key1 are not here because flags is not set
        // to CBR2_PERSISTENT_KEY_PRESENT
        // ---------------------------------------------------
        // key1 (4 bytes): A 32-bit, unsigned integer. The low 32 bits of the 64-bit
        //                 persistent bitmap cache key.

        // key2 (4 bytes): A 32-bit, unsigned integer. The high 32 bits of the
        //                 64-bit persistent bitmap cache key.

        // bitmapWidth (variable): A Two-Byte Unsigned Encoding (section
        //                         2.2.2.2.1.2.1.2) structure. The width of the
        //                         bitmap in pixels.
        stream.out_uint8(align4(this->width));

        // bitmapHeight (variable): A Two-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.2) structure. The height of the
        //                          bitmap in pixels.
        stream.out_uint8(this->height);

        // bitmapLength (variable): A Four-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.4) structure. The size in bytes
        //                          of the data in the bitmapComprHdr and
        //                          bitmapDataStream fields.
        stream.out_uint16_be((this->height * row_size) | 0x4000);

        // cacheIndex (variable): A Two-Byte Unsigned Encoding (section
        //                        2.2.2.2.1.2.1.2) structure. An entry in the bitmap
        //                        cache (specified by the cacheId field) where the
        //                        bitmap MUST be stored. If the CBR2_DO_NOT_CACHE
        //                        flag is not set in the header field, the bitmap
        //                        cache index MUST be in the range negotiated by the
        //                        Bitmap Cache Capability Set (Revision 2) (see
        //                        [MS-RDPBCGR] section 2.2.7.1.4.2). Otherwise, if
        //                        the CBR2_DO_NOT_CACHE flag is set, the cacheIndex
        //                        MUST be set to BITMAPCACHE_WAITING_LIST_INDEX
        //                        (32767).
        stream.out_2BUE(this->cache_idx);

        // No compression header in our case
        // ---------------------------------
        // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
        //                           [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3)
        //                           describing the bitmap data in the
        //                           bitmapDataStream. This field MUST be present if
        //                           the TS_CACHE_BITMAP_COMPRESSED_REV2 (0x05) flag
        //                           is present in the header field, but the
        //                           CBR2_NO_BITMAP_COMPRESSION_HDR (0x08) flag is
        //                           not.

        // bitmapDataStream (variable): A variable-length byte array containing
        //                              bitmap data (the format of this data is
        //                              defined in [MS-RDPBCGR] section
        //                              2.2.9.1.1.3.1.2.2).

        // for uncompressed bitmaps the format is quite simple
        stream.out_copy_bytes(this->data, this->height * row_size);
        stream.set_length(-12, length_ptr);
    }

    void receive(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
            this->receive_raw_v1(stream, control, header);
    }

    void receive_raw_v2(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        using namespace RDP;
        #warning DO NOT USE : partial implementation, we do not know yet how to manage persistant bitmap storage
    }

    void receive_raw_v1(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        using namespace RDP;

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

        this->cache_id = stream.in_uint8();

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.

        stream.skip_uint8(1);

        // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the
        // bitmap in pixels.

        this->width = stream.in_uint8();

        // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
        //  bitmap in pixels.

        this->height = stream.in_uint8();

        // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color
        //  depth of the bitmap data in bits-per-pixel. This field MUST be one
        //  of the following values.
        //  0x08 8-bit color depth.
        //  0x10 16-bit color depth.
        //  0x18 24-bit color depth.
        //  0x20 32-bit color depth.

        this->bpp = stream.in_uint8();

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.

        uint16_t bufsize = stream.in_uint16_le();

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).

        this->cache_idx = stream.in_uint16_le();

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        #warning I should be able to create a bitmap instead of doing this
        int row_size = align4(width * nbbytes(bpp));
        assert(row_size * height == bufsize);
        this->data = new uint8_t[bufsize];
        memcpy(this->data, stream.in_uint8p(bufsize), bufsize);
    }

    bool operator==(const RDPBmpCache & other) const {
        return true;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(buffer, sz, "RDPBmpCache()");
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

};


class Bounds {

    // bounds byte (which clipping is sent if any)
    // -------------------------------------------
    // order of clipping field always is top, left, right, bottom
    // 0x01: top    bound (absolute 2 bytes)
    // 0x02: left   bound (absolute 2 bytes)
    // 0x04: right  bound (absolute 2 bytes)
    // 0x08: bottom bound (absolute 2 bytes)
    // 0x10: top    bound (relative 1 byte)
    // 0x20: left   bound (relative 1 byte)
    // 0x40: right  bound (relative 1 byte)
    // 0x80: bottom bound (relative 1 byte)


    public:

        uint8_t bounds_flags;
        int16_t absolute_bounds[4];
        int16_t delta_bounds[4];

        Bounds(const Rect & oldclip, const Rect & newclip){
            int16_t old_bounds[4];

            using namespace RDP;
            old_bounds[LEFT]   = oldclip.x;
            old_bounds[TOP]    = oldclip.y;
            old_bounds[RIGHT]  = oldclip.x + oldclip.cx - 1;
            old_bounds[BOTTOM] = oldclip.y + oldclip.cy - 1;

            this->absolute_bounds[LEFT]   = newclip.x;
            this->absolute_bounds[TOP]    = newclip.y;
            this->absolute_bounds[RIGHT]   = newclip.x + newclip.cx - 1;
            this->absolute_bounds[BOTTOM] = newclip.y + newclip.cy - 1;

            this->delta_bounds[LEFT]   = this->absolute_bounds[LEFT]   - old_bounds[LEFT];
            this->delta_bounds[TOP]    = this->absolute_bounds[TOP]    - old_bounds[TOP];
            this->delta_bounds[RIGHT]  = this->absolute_bounds[RIGHT]  - old_bounds[RIGHT];
            this->delta_bounds[BOTTOM] = this->absolute_bounds[BOTTOM] - old_bounds[BOTTOM];

            this->bounds_flags =
                pounder_bound(this->delta_bounds[LEFT],  (1<<LEFT))
              | pounder_bound(this->delta_bounds[TOP],   (1<<TOP))
              | pounder_bound(this->delta_bounds[RIGHT], (1<<RIGHT))
              | pounder_bound(this->delta_bounds[BOTTOM],(1<<BOTTOM))
              ;
        }

    void emit(Stream & stream)
    {
        using namespace RDP;

        if (this->bounds_flags != 0){
            stream.out_uint8(this->bounds_flags);
            for (unsigned b = LEFT ; b <= BOTTOM ; ++b){
                if (this->bounds_flags & (1<<b)) {
                    stream.out_uint16_le(this->absolute_bounds[b]);
                }
                else if (this->bounds_flags & (0x10<<b)) {
                    stream.out_uint8(this->delta_bounds[b]);
                }
            }
        }
    }
};


class RDPPrimaryOrderHeader
{
    public:
    uint8_t control;
    uint32_t fields;

    RDPPrimaryOrderHeader(uint8_t control, uint32_t fields)
        : control(control), fields(fields) {}

    void emit_coord(Stream & stream, uint32_t base, int16_t coord, int16_t oldcoord) const {
        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base){
                stream.out_uint8(coord-oldcoord);
            }
        }
        else {
            if (this->fields & base){
                stream.out_uint16_le(coord);
            }
        }
    }

    void receive_coord(Stream & stream, uint32_t base, int16_t & coord) const
    {
        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base) {
                coord = coord + stream.in_sint8();
            }
        }
        else {
            if (this->fields & base) {
                coord = stream.in_sint16_le();
            }
        }
    }


    void emit_rect(Stream & stream, uint32_t base, const Rect & rect, const Rect & oldr) const {
        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base){
                stream.out_uint8(rect.x-oldr.x);
            }
            if (this->fields & (base << 1)){
                stream.out_uint8(rect.y-oldr.y);
            }
            if (this->fields & (base << 2)){
                stream.out_uint8(rect.cx-oldr.cx);
            }
            if (this->fields & (base << 3)){
                stream.out_uint8(rect.cy-oldr.cy);
            }
        }
        else {
            if (this->fields & base){
                stream.out_uint16_le(rect.x);
            }
            if (this->fields & (base << 1)){
                stream.out_uint16_le(rect.y);
            }
            if (this->fields & (base << 2)){
                stream.out_uint16_le(rect.cx);
            }
            if (this->fields & (base << 3)){
                stream.out_uint16_le(rect.cy);
            }
        }
    }

    void receive_rect(Stream & stream, uint32_t base, Rect & rect) const
    {
        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base) {
                rect.x = rect.x + stream.in_sint8();
            }
            if (this->fields & (base << 1)) {
                rect.y = rect.y + stream.in_sint8();
            }
            if (this->fields & (base << 2)) {
                rect.cx = rect.cx + stream.in_sint8();
            }
            if (this->fields & (base << 3)) {
                rect.cy = rect.cy + stream.in_sint8();
            }
        }
        else {
            if (this->fields & base) {
                rect.x = stream.in_sint16_le();
            }
            if (this->fields & (base << 1)) {
                rect.y = stream.in_sint16_le();
            }
            if (this->fields & (base << 2)) {
                rect.cx = stream.in_sint16_le();
            }
            if (this->fields & (base << 3)) {
                rect.cy = stream.in_sint16_le();
            }
        }
    }

    void emit_src(Stream & stream, uint32_t base,
                  uint16_t srcx, uint16_t srcy,
                  uint16_t oldx, uint16_t oldy) const {

        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base){
                stream.out_uint8(srcx-oldx);
            }
            if (this->fields & (base << 1)){
                stream.out_uint8(srcy-oldy);
            }
        }
        else {
            if (this->fields & base){
                stream.out_uint16_le(srcx);
            }
            if (this->fields & (base << 1)){
                stream.out_uint16_le(srcy);
            }
        }
    }

    void receive_src(Stream & stream, uint32_t base,
                     uint16_t & srcx, uint16_t & srcy) const
    {
        using namespace RDP;

        if (this->control & DELTA){
            if (this->fields & base) {
                srcx = srcx + stream.in_sint8();
            }
            if (this->fields & (base << 1)) {
                srcy = srcy + stream.in_sint8();
            }
        }
        else {
            if (this->fields & base) {
                srcx = stream.in_sint16_le();
            }
            if (this->fields & (base << 1)) {
                srcy = stream.in_sint16_le();
            }
        }
    }

    void emit_pen(Stream & stream, uint32_t base,
                  const RDPPen & pen,
                  const RDPPen & old_pen) const {

        using namespace RDP;
        if (this->fields & base) {
            stream.out_uint8(pen.style);
        }
        if (this->fields & (base << 1)) {
            stream.out_sint8(pen.width);
         }
        if (this->fields & (base << 2)) {
            stream.out_uint8(pen.color);
            stream.out_uint8(pen.color >> 8);
            stream.out_uint8(pen.color >> 16);
        }
    }

    void receive_pen(Stream & stream, uint32_t base, RDPPen & pen) const
    {
        using namespace RDP;

        if (this->fields & base) {
            pen.style = stream.in_uint8();
        }

        if (this->fields & (base << 1)) {
            pen.width = stream.in_uint8();
        }

        if (this->fields & (base << 2)) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            pen.color = r + (g << 8) + (b << 16);
        }
    }

    void emit_brush(Stream & stream, uint32_t base,
                  const RDPBrush & brush,
                  const RDPBrush & old_brush) const {

        using namespace RDP;
        if (this->fields & base) {
            stream.out_sint8(brush.org_x);
        }
        if (this->fields & (base << 1)) {
            stream.out_sint8(brush.org_y);
         }
        if (this->fields & (base << 2)) {
            stream.out_uint8(brush.style);
        }
        if (this->fields & (base << 3)) {
            stream.out_uint8(brush.hatch);
        }

        if (this->fields & (base << 4)){
            stream.out_uint8(brush.extra[0]);
            stream.out_uint8(brush.extra[1]);
            stream.out_uint8(brush.extra[2]);
            stream.out_uint8(brush.extra[3]);
            stream.out_uint8(brush.extra[4]);
            stream.out_uint8(brush.extra[5]);
            stream.out_uint8(brush.extra[6]);
        }
    }

    void receive_brush(Stream & stream, uint32_t base, RDPBrush & brush) const
    {
        using namespace RDP;

        if (this->fields & base) {
            brush.org_x = stream.in_sint8();
        }
        if (this->fields & (base << 1)) {
            brush.org_y = stream.in_sint8();
        }
        if (this->fields & (base << 2)) {
            brush.style = stream.in_uint8();
        }
        if (this->fields & (base << 3)) {
            brush.hatch = stream.in_uint8();
        }
        if (this->fields & (base << 4)){
            brush.extra[0] = stream.in_uint8();
            brush.extra[1] = stream.in_uint8();
            brush.extra[2] = stream.in_uint8();
            brush.extra[3] = stream.in_uint8();
            brush.extra[4] = stream.in_uint8();
            brush.extra[5] = stream.in_uint8();
            brush.extra[6] = stream.in_uint8();
        }
    }
};

// Common part for Primary Drawing Orders
// ---------------------------------------

// common part of Primary Drawing Orders (last_order, bounding rectangle)
// updated as each order is executed

class RDPOrderCommon {
    public:

    // for primary orders : kept in state
    uint8_t order;
    Rect clip;

    RDPOrderCommon(int order, Rect clip) :
        order(order), clip(clip) {}

    bool operator==(const RDPOrderCommon &other) const {
        return  (this->order == other.order)
             && (this->clip == other.clip);
    }

    void emit(Stream & stream, RDPPrimaryOrderHeader & header, const RDPOrderCommon & oldcommon)
    {

        using namespace RDP;

        Bounds bounds(oldcommon.clip, this->clip);

        header.control |= (this->order != oldcommon.order) * CHANGE;

        if (header.control & BOUNDS){
            header.control |= ((bounds.bounds_flags == 0) * LASTBOUNDS);
        }

        int size = 1;
        switch (this->order)
        {
            case TRIBLT:
            case TEXT2:
                size = 3;
                break;

            case PATBLT:
            case MEMBLT:
            case LINE:
            //case POLYGON2:
            //case ELLIPSE2:
                size = 2;
                break;
            case RECT:
            case SCREENBLT:
            case DESTBLT:
            default:
                size = 1;
        }

        int realsize = (header.fields == 0)      ?  0  :
                       (header.fields < 0x100)   ?  1  :
                       (header.fields < 0x10000) ?  2  :
                                             3;

        switch (size - realsize){
            case 3:
                header.control |= TINY | SMALL;
            break;
            case 2:
                header.control |= TINY;
            break;
            case 1:
                header.control |= SMALL;
            break;
            default:;
        }

        // know control is known
        stream.out_uint8(header.control);

        if (header.control & CHANGE){
            stream.out_uint8(order);
        }

        if (header.fields){
            stream.out_uint8(header.fields & 0xFF);
            if (header.fields >= 0x100){
                stream.out_uint8((header.fields >> 8) & 0xFF);
                if (header.fields >= 0x10000){
                    stream.out_uint8((header.fields >> 16) & 0xFF);
                }
            }
        }

        if (header.control & BOUNDS){
            if (!(header.control & LASTBOUNDS)){
                bounds.emit(stream);
            }
        }
        else {
            this->clip = oldcommon.clip;
        }
    }

    const  RDPPrimaryOrderHeader receive(Stream & stream, uint8_t control)
    {

        using namespace RDP;

        RDPPrimaryOrderHeader header(control, 0);

//        LOG(LOG_INFO, "reading control (%p): %.2x %s%s%s%s%s%s%s%s\n", stream.p,
//            control,
//            (control & STANDARD  )?"STD ":"    ",
//            (control & SECONDARY )?"SEC ":"    ",
//            (control & BOUNDS    )?"BOU ":"    ",
//            (control & CHANGE    )?"CHA ":"    ",
//            (control & DELTA     )?"DTA ":"    ",
//            (control & LASTBOUNDS)?"LBO ":"    ",
//            (control & SMALL     )?"SMA ":"    ",
//            (control & TINY      )?"TIN ":"    "
//        );

        if (control & CHANGE) {
            this->order = stream.in_uint8();
        }

        size_t size = 1;
        switch (this->order)
        {
            case TRIBLT:
            case TEXT2:
                size = 3;
                break;

            case PATBLT:
            case MEMBLT:
            case LINE:
            //case POLYGON2:
            //case ELLIPSE2:
                size = 2;
                break;
            case RECT:
            case SCREENBLT:
            case DESTBLT:
            default:
                size = 1;
        }
        if (header.control & SMALL) {
            size = (size<=1)?0:size-1;
        }
        if (header.control & TINY) {
            size = (size<=2)?0:size-2;
        }

        for (size_t i = 0; i < size; i++) {
            int bits = stream.in_uint8();
            header.fields |= bits << (i * 8);
        }

//            LOG(LOG_INFO, "control=%.2x order=%d  size=%d fields=%.6x assert=%d\n", control, order, size, fields, (0 == (fields & ~0x3FF)));

        switch (this->order){
        case DESTBLT:
            assert(!(header.fields & ~0x1F));
        break;
        case PATBLT:
            assert(!(header.fields & ~0xFFF));
        break;
        case SCREENBLT:
            assert(!(header.fields & ~0x7F));
        break;
        case LINE:
            assert(!(header.fields & ~0x3FF));
        break;
        case RECT:
            assert(!(header.fields & ~0x7F));
        break;
        case DESKSAVE:
        break;
        case MEMBLT:
            assert(!(header.fields & ~0x1FF));
        break;
        case TEXT2:
            assert(!(header.fields & ~0x3FFFFF));
        break;
        default:
            LOG(LOG_INFO, "Order is Unknown (%u)\n", this->order);
            assert(false);
        }

        if (header.control & BOUNDS) {
            if (!(header.control & LASTBOUNDS)){
                int bound_fields = stream.in_uint8();
                uint16_t bounds[4] = {
                     this->clip.x,
                     this->clip.y,
                     this->clip.x + this->clip.cx - 1 ,
                     this->clip.y + this->clip.cy - 1};

                if (bound_fields & 1) {
                    bounds[0] = stream.in_sint16_le();
                } else if (bound_fields & 0x10) {
                    bounds[0] += stream.in_sint8();
                }

                if (bound_fields & 2) {
                    bounds[1] = stream.in_sint16_le();
                } else if (bound_fields & 0x20) {
                    bounds[1] += stream.in_sint8();
                }

                if (bound_fields & 4) {
                    bounds[2] = stream.in_sint16_le();
                } else if (bound_fields & 0x40) {
                    bounds[2] += stream.in_sint8();
                }

                if (bound_fields & 8) {
                    bounds[3] = stream.in_sint16_le();
                } else if (bound_fields & 0x80) {
                    bounds[3] += stream.in_sint8();
                }

                this->clip.x = bounds[0];
                this->clip.y = bounds[1];
                this->clip.cx = bounds[2] - bounds[0] +1;
                this->clip.cy = bounds[3] - bounds[1] +1;
            }
        }

        return header;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(
            buffer,
            sz,
            "order(%d clip(%d,%d,%d,%d)):",
            this->order,
            this->clip.x, this->clip.y, this->clip.cx, this->clip.cy);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }


};


// Primary Drawing Orders
// -------------------------

class RDPOpaqueRect {
    public:
    Rect rect;
    uint32_t color;

    RDPOpaqueRect(Rect rect, int color) :
        rect(rect), color(color)
        {}

    bool operator==(const RDPOpaqueRect &other) const {
        return  (this->rect == other.rect)
             && (this->color == other.color);
    }

    // order to stream returns true if state clip must be changed
    // it does not change state by itself
    void emit(Stream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPOpaqueRect & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // OPAQUERECT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: red color byte
        // 0x20: green color byte
        // 0x40: blue color byte

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        uint32_t diff_color = this->color ^ oldcmd.color;

//        LOG(LOG_INFO, "emit opaque rect old_color = %.6x new_color = %.6x\n", oldcmd.color, this->color);

        header.fields =   (dr.dleft                != 0) * 0x01
                        | (dr.dtop                 != 0) * 0x02
                        | (dr.dwidth               != 0) * 0x04
                        | (dr.dheight              != 0) * 0x08
                        | ((diff_color & 0xFF)     != 0) * 0x10
                        | ((diff_color & 0xFF00)   != 0) * 0x20
                        | ((diff_color & 0xFF0000) != 0) * 0x40
                        ;

        #warning dirty hack to fix color problems with opaque_rect. It seems I will have to pass in bpp if I want to fix it. (And how it should be fixed is not obvious as - as far I can see - support for 16bpp and 15bpp is undocumented in official documentation provided by Microsoft). It seems I will have to guess.
        if (diff_color) { header.fields |= 0x70; }
        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->color);
        }
        if (header.fields & 0x20){
                stream.out_uint8(this->color >> 8);
        }
        if (header.fields & 0x40){
                stream.out_uint8(this->color >> 16);
        }
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        uint32_t old_color = this->color;

        if (header.fields & 0x10) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0xffff00) | i;
        }
        if (header.fields & 0x20) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0xff00ff) | (i << 8);
        }
        if (header.fields & 0x40) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0x00ffff) | (i << 16);
        }

        LOG(LOG_INFO, "receive opaque rect old_color = %.6x new_color = %.6x\n", old_color, this->color);

    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "opaquerect(rect(%d,%d,%d,%d) color=%x)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy, this->color);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }


};

class RDPScrBlt {
    public:
    Rect rect;
    uint8_t rop;
    uint16_t srcx;
    uint16_t srcy;

    RDPScrBlt(Rect rect, uint8_t rop, uint16_t srcx, uint16_t srcy) :
        rect(rect), rop(rop), srcx(srcx), srcy(srcy)
        {}

    bool operator==(const RDPScrBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->srcx == other.srcx)
             && (this->srcy == other.srcy)
             ;
    }

    void emit(Stream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPScrBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // SCREENBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: srcx coordinate
        // 0x40: srcy coordinate

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= ((dr.fully_relative()
                   && is_1_byte(this->srcx - oldcmd.srcx)
                   && is_1_byte(this->srcy - oldcmd.srcy))
                                                          * DELTA);

        header.fields = (dr.dleft     != 0               ) * 0x01
                      | (dr.dtop      != 0               ) * 0x02
                      | (dr.dwidth    != 0               ) * 0x04
                      | (dr.dheight   != 0               ) * 0x08
                      | (this->rop    != oldcmd.rop      ) * 0x10
                      | ((this->srcx - oldcmd.srcx) != 0 ) * 0x20
                      | ((this->srcy - oldcmd.srcy) != 0 ) * 0x40
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->rop);
        }

        header.emit_src(stream, 0x20,
                        this->srcx, this->srcy,
                        oldcmd.srcx, oldcmd.srcy);

    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }

        header.receive_src(stream, 0x20, this->srcx, this->srcy);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "scrblt(rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            this->rop, this->srcx, this->srcy);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};

//2.2.2.2.1.1.2.1 DstBlt (DSTBLT_ORDER)
// ------------------------------------

// The DstBlt Primary Drawing Order is used to paint a rectangle by using a
// destination-only raster operation.

// Encoding order number: 0 (0x00)
// Negotiation order number: 0 (0x00)
// Number of fields: 5
// Number of field encoding bytes: 1
// Maximum encoded field length: 9 bytes

// nLeftRect (variable): Left coordinate of the destination rectangle specified
//                       using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): Top coordinate of the destination rectangle specified
//                      using a Coord Field (section 2.2.2.2.1.1.1.1)

// nWidth (variable): Width of the destination rectangle specified by using a
//                    Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): Height of the destination rectangle specified by using a
//                     Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): Index of the ternary raster operation to perform (see section
//                2.2.2.2.1.1.1.7 below). The resultant ROP3 operation MUST only
//                depend on the destination bits (there MUST NOT be any
//                dependence on source or pattern bits).

//                ie: Only 4 possible values:
//                      0x00 = BLACKNESS,
//                      0x55 = Dn (DSTINVERT),
//                      0xAA = D (do nothing)
//                      0xFF = WHITENESS

//2.2.2.2.1.1.1.7 Ternary Raster Operation Index (ROP3_OPERATION_INDEX)
// --------------------------------------------------------------------

// The ROP3_OPERATION_INDEX structure is used to define how the bits in a source
// bitmap, destination bitmap, and a selected brush or pen are combined by using
// Boolean operators.

// rop3Index (1 byte): An 8-bit, unsigned integer. This field contains an index
// of a raster operation code that describes a Boolean operation, in Reverse
// Polish Notation, to perform on the bits in a source bitmap (S), destination
// bitmap (D), and selected brush or pen (P). This operation is a combination
// of the AND (a), OR (o), NOT (n), and XOR (x) Boolean operators.

// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
// | 0x01 | ROP: 0x00010289               |
// |      | RPN: DPSoon                   |
// +------+-------------------------------+
// | 0x02 | ROP: 0x00020C89               |
// |      | RPN: DPSona                   |
// +------+-------------------------------+
// | 0x03 | ROP: 0x000300AA               |
// |      | RPN: PSon                     |
// +------+-------------------------------+
// | 0x04 | ROP: 0x00040C88               |
// |      | RPN: SDPona                   |
// +------+-------------------------------+
// | 0x05 | ROP: 0x000500A9               |
// |      | RPN: DPon                     |
// +------+-------------------------------+
// | 0x06 | ROP: 0x00060865               |
// |      | RPN: PDSxnon                  |
// +------+-------------------------------+
// | 0x07 | ROP: 0x000702C5               |
// |      | RPN: PDSaon                   |
// +------+-------------------------------+
// | 0x08 | ROP: 0x00080F08               |
// |      | RPN: SDPnaa                   |
// +------+-------------------------------+
// | 0x09 | ROP: 0x00090245               |
// |      | RPN: PDSxon                   |
// +------+-------------------------------+
// | 0x0A | ROP: 0x000A0329               |
// |      | RPN: DPna                     |
// +------+-------------------------------+
// | 0x0B | ROP: 0x000B0B2A               |
// |      | RPN: PSDnaon                  |
// +------+-------------------------------+
// | 0x0C | ROP: 0x000C0324               |
// |      | RPN: SPna                     |
// +------+-------------------------------+
// | 0x0D | ROP: 0x000D0B25               |
// |      | RPN: PDSnaon                  |
// +------+-------------------------------+
// | 0x0E | ROP: 0x000E08A5               |
// |      | RPN: PDSonon                  |
// +------+-------------------------------+
// | 0x0F | ROP: 0x000F0001               |
// |      | RPN: Pn                       |
// +------+-------------------------------+
// | 0x10 | ROP: 0x00100C85               |
// |      | RPN: PDSona                   |
// +------+-------------------------------+
// | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
// |      | RPN: DSon                     |
// +------+-------------------------------+
// | 0x12 | ROP: 0x00120868               |
// |      | RPN: SDPxnon                  |
// +------+-------------------------------+
// | 0x13 | ROP: 001302C8                 |
// |      | RPN: SDPaon                   |
// +------+-------------------------------+
// | 0x14 | ROP: 0x0140869                |
// |      | RPN: DPSxnon                  |
// +------+-------------------------------+
// | 0x15 | ROP: 0x001502C9               |
// |      | RPN: DPSaon                   |
// +------+-------------------------------+
// | 0x16 | ROP: 0x00165CCA               |
// |      | RPN: PSDPSanaxx               |
// +------+-------------------------------+
// | 0x17 | ROP: 0x00171D54               |
// |      | RPN: SSPxDSxaxn               |
// +------+-------------------------------+
// | 0x18 | ROP: 0x00180D59               |
// |      | RPN: SPxPDxa                  |
// +------+-------------------------------+
// | 0x19 | ROP: 0x00191CC8               |
// |      | RPN: SDPSanaxn                |
// +------+-------------------------------+
// | 0x1A | ROP: 0x001A06C5               |
// |      | RPN: PDSPaox                  |
// +------+-------------------------------+
// | 0x1B | ROP: 0x001B0768               |
// |      | RPN: SDPSxaxn                 |
// +------+-------------------------------+
// | 0x1C | ROP: 0x001C06CA               |
// |      | RPN: PSDPaox                  |
// +------+-------------------------------+
// | 0x1D | ROP: 0x001D0766               |
// |      | RPN: DSPDxaxn                 |
// +------+-------------------------------+
// | 0x1E | ROP: 0x001E01A5               |
// |      | RPN: PDSox                    |
// +------+-------------------------------+
// | 0x1F | ROP: 0x001F0385               |
// |      | RPN: PDSoan                   |
// +------+-------------------------------+
// | 0x20 | ROP: 0x00200F09               |
// |      | RPN: DPSnaa                   |
// +------+-------------------------------+
// | 0x21 | ROP: 0x00210248               |
// |      | RPN: SDPxon                   |
// +------+-------------------------------+
// | 0x22 | ROP: 0x00220326               |
// |      | RPN: DSna                     |
// +------+-------------------------------+
// | 0x23 | ROP: 0x00230B24               |
// |      | RPN: SPDnaon                  |
// +------+-------------------------------+
// | 0x24 | ROP: 0x00240D55               |
// |      | RPN: SPxDSxa                  |
// +------+-------------------------------+
// | 0x25 | ROP: 0x00251CC5               |
// |      | RPN: PDSPanaxn                |
// +------+-------------------------------+
// | 0x26 | ROP: 0x002606C8               |
// |      | RPN: SDPSaox                  |
// +------+-------------------------------+
// | 0x27 | ROP: 0x00271868               |
// |      | RPN: SDPSxnox                 |
// +------+-------------------------------+
// | 0x28 | ROP: 0x00280369               |
// |      | RPN: DPSxa                    |
// +------+-------------------------------+
// | 0x29 | ROP: 0x002916CA               |
// |      | RPN: PSDPSaoxxn               |
// +------+-------------------------------+
// | 0x2A | ROP: 0x002A0CC9               |
// |      | RPN: DPSana                   |
// +------+-------------------------------+
// | 0x2B | ROP: 0x002B1D58               |
// |      | RPN: SSPxPDxaxn               |
// +------+-------------------------------+
// | 0x2C | ROP: 0x002C0784               |
// |      | RPN: SPDSoax                  |
// +------+-------------------------------+
// | 0x2D | ROP: 0x002D060A               |
// |      | RPN: PSDnox                   |
// +------+-------------------------------+
// | 0x2E | ROP: 0x002E064A               |
// |      | RPN: PSDPxox                  |
// +------+-------------------------------+
// | 0x2F | ROP: 0x002F0E2A               |
// |      | RPN: PSDnoan                  |
// +------+-------------------------------+
// | 0x30 | ROP: 0x0030032A               |
// |      | RPN: PSna                     |
// +------+-------------------------------+
// | 0x31 | ROP: 0x00310B28               |
// |      | RPN: SDPnaon                  |
// +------+-------------------------------+
// | 0x32 | ROP: 0x00320688               |
// |      | RPN: SDPSoox                  |
// +------+-------------------------------+
// | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
// |      | RPN: Sn                       |
// +------+-------------------------------+
// | 0x34 | ROP: 0x003406C4               |
// |      | RPN: SPDSaox                  |
// +------+-------------------------------+
// | 0x35 | ROP: 0x00351864               |
// |      | RPN: SPDSxnox                 |
// +------+-------------------------------+
// | 0x36 | ROP: 0x003601A8               |
// |      | RPN: SDPox                    |
// +------+-------------------------------+
// | 0x37 | ROP: 0x00370388               |
// |      | RPN: SDPoan                   |
// +------+-------------------------------+
// | 0x38 | ROP: 0x0038078A               |
// |      | RPN: PSDPoax                  |
// +------+-------------------------------+
// | 0x39 | ROP: 0x00390604               |
// |      | RPN: SPDnox                   |
// +------+-------------------------------+
// | 0x3A | ROP: 0x003A0644               |
// |      | RPN: SPDSxox                  |
// +------+-------------------------------+
// | 0x3B | ROP: 0x003B0E24               |
// |      | RPN: SPDnoan                  |
// +------+-------------------------------+
// | 0x3C | ROP: 0x003C004A               |
// |      | RPN: PSx                      |
// +------+-------------------------------+
// | 0x3D | ROP: 0x003D18A4               |
// |      | RPN: SPDSonox                 |
// +------+-------------------------------+
// | 0x3E | ROP: 0x003E1B24               |
// |      | RPN: SPDSnaox                 |
// +------+-------------------------------+
// | 0x3F | ROP: 0x003F00EA               |
// |      | RPN: PSan                     |
// +------+-------------------------------+
// | 0x40 | ROP: 0x00400F0A               |
// |      | RPN: PSDnaa                   |
// +------+-------------------------------+
// | 0x41 | ROP: 0x00410249               |
// |      | RPN: DPSxon                   |
// +------+-------------------------------+
// | 0x42 | ROP: 0x00420D5D               |
// |      | RPN: SDxPDxa                  |
// +------+-------------------------------+
// | 0x43 | ROP: 0x00431CC4               |
// |      | RPN: SPDSanaxn                |
// +------+-------------------------------+
// | 0x44 | ROP: 0x00440328 (SRCERASE)    |
// |      | RPN: SDna                     |
// +------+-------------------------------+
// | 0x45 | ROP: 0x00450B29               |
// |      | RPN: DPSnaon                  |
// +------+-------------------------------+
// | 0x46 | ROP: 0x004606C6               |
// |      | RPN: DSPDaox                  |
// +------+-------------------------------+
// | 0x47 | ROP: 0x0047076A               |
// |      | RPN: PSDPxaxn                 |
// +------+-------------------------------+
// | 0x48 | ROP: 0x00480368               |
// |      | RPN: SDPxa                    |
// +------+-------------------------------+
// | 0x49 | ROP: 0x004916C5               |
// |      | RPN: PDSPDaoxxn               |
// +------+-------------------------------+
// | 0x4A | ROP: 0x004A0789               |
// |      | RPN: DPSDoax                  |
// +------+-------------------------------+
// | 0x4B | ROP: 0x004B0605               |
// |      | RPN: PDSnox                   |
// +------+-------------------------------+
// | 0x4C | ROP: 0x004C0CC8               |
// |      | RPN: SDPana                   |
// +------+-------------------------------+
// | 0x4D | ROP: 0x004D1954               |
// |      | RPN: SSPxDSxoxn               |
// +------+-------------------------------+
// | 0x4E | ROP: 0x004E0645               |
// |      | RPN: PDSPxox                  |
// +------+-------------------------------+
// | 0x4F | ROP: 0x004F0E25               |
// |      | RPN: PDSnoan                  |
// +------+-------------------------------+
// | 0x50 | ROP: 0x00500325               |
// |      | RPN: PDna                     |
// +------+-------------------------------+
// | 0x51 | ROP: 0x00510B26               |
// |      | RPN: DSPnaon                  |
// +------+-------------------------------+
// | 0x52 | ROP: 0x005206C9               |
// |      | RPN: DPSDaox                  |
// +------+-------------------------------+
// | 0x53 | ROP: 0x00530764               |
// |      | RPN: SPDSxaxn                 |
// +------+-------------------------------+
// | 0x54 | ROP: 0x005408A9               |
// |      | RPN: DPSonon                  |
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
// | 0x56 | ROP: 0x005601A9               |
// |      | RPN: DPSox                    |
// +------+-------------------------------+
// | 0x57 | ROP: 0x00570389               |
// |      | RPN: DPSoan                   |
// +------+-------------------------------+
// | 0x58 | ROP: 0x00580785               |
// |      | RPN: PDSPoax                  |
// +------+-------------------------------+
// | 0x59 | ROP: 0x00590609               |
// |      | RPN: DPSnox                   |
// +------+-------------------------------+
// | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
// |      | RPN: DPx                      |
// +------+-------------------------------+
// | 0x5B | ROP: 0x005B18A9               |
// |      | RPN: DPSDonox                 |
// +------+-------------------------------+
// | 0x5C | ROP: 0x005C0649               |
// |      | RPN: DPSDxox                  |
// +------+-------------------------------+
// | 0x5D | ROP: 0x005D0E29               |
// |      | RPN: DPSnoan                  |
// +------+-------------------------------+
// | 0x5E | ROP: 0x005E1B29               |
// |      | RPN: DPSDnaox                 |
// +------+-------------------------------+
// | 0x5F | ROP: 0x005F00E9               |
// |      | RPN: DPan                     |
// +------+-------------------------------+
// | 0x60 | ROP: 0x00600365               |
// |      | RPN: PDSxa                    |
// +------+-------------------------------+
// | 0x61 | ROP: 0x006116C6               |
// |      | RPN: DSPDSaoxxn               |
// +------+-------------------------------+
// | 0x62 | ROP: 0x00620786               |
// |      | RPN: DSPDoax                  |
// +------+-------------------------------+
// | 0x63 | ROP: 0x00630608               |
// |      | RPN: SDPnox                   |
// +------+-------------------------------+
// | 0x64 | ROP: 0x00640788               |
// |      | RPN: SDPSoax                  |
// +------+-------------------------------+
// | 0x65 | ROP: 0x00650606               |
// |      | RPN: DSPnox                   |
// +------+-------------------------------+
// | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
// |      | RPN: DSx                      |
// +------+-------------------------------+
// | 0x67 | ROP: 0x006718A8               |
// |      | RPN: SDPSonox                 |
// +------+-------------------------------+
// | 0x68 | ROP: 0x006858A6               |
// |      | RPN: DSPDSonoxxn              |
// +------+-------------------------------+
// | 0x69 | ROP: 0x00690145               |
// |      | RPN: PDSxxn                   |
// +------+-------------------------------+
// | 0x6A | ROP: 0x006A01E9               |
// |      | RPN: DPSax                    |
// +------+-------------------------------+
// | 0x6B | ROP: 0x006B178A               |
// |      | RPN: PSDPSoaxxn               |
// +------+-------------------------------+
// | 0x6C | ROP: 0x006C01E8               |
// |      | RPN: SDPax                    |
// +------+-------------------------------+
// | 0x6D | ROP: 0x006D1785               |
// |      | RPN: PDSPDoaxxn               |
// +------+-------------------------------+
// | 0x6E | ROP: 0x006E1E28               |
// |      | RPN: SDPSnoax                 |
// +------+-------------------------------+
// | 0x6F | ROP: 0x006F0C65               |
// |      | RPN: PDSxnan                  |
// +------+-------------------------------+
// | 0x70 | ROP: 0x00700CC5               |
// |      | RPN: PDSana                   |
// +------+-------------------------------+
// | 0x71 | ROP: 0x00711D5C               |
// |      | RPN: SSDxPDxaxn               |
// +------+-------------------------------+
// | 0x72 | ROP: 0x00720648               |
// |      | RPN: SDPSxox                  |
// +------+-------------------------------+
// | 0x73 | ROP: 0x00730E28               |
// |      | RPN: SDPnoan                  |
// +------+-------------------------------+
// | 0x74 | ROP: 0x00740646               |
// |      | RPN: DSPDxox                  |
// +------+-------------------------------+
// | 0x75 | ROP: 0x00750E26               |
// |      | RPN: DSPnoan                  |
// +------+-------------------------------+
// | 0x76 | ROP: 0x00761B28               |
// |      | RPN: SDPSnaox                 |
// +------+-------------------------------+
// | 0x77 | ROP: 0x007700E6               |
// |      | RPN: DSan                     |
// +------+-------------------------------+
// | 0x78 | ROP: 0x007801E5               |
// |      | RPN: PDSax                    |
// +------+-------------------------------+
// | 0x79 | ROP: 0x00791786               |
// |      | RPN: DSPDSoaxxn               |
// +------+-------------------------------+
// | 0x7A | ROP: 0x007A1E29               |
// |      | RPN: DPSDnoax                 |
// +------+-------------------------------+
// | 0x7B | ROP: 0x007B0C68               |
// |      | RPN: SDPxnan                  |
// +------+-------------------------------+
// | 0x7C | ROP: 0x007C1E24               |
// |      | RPN: SPDSnoax                 |
// +------+-------------------------------+
// | 0x7D | ROP: 0x007D0C69               |
// |      | RPN: DPSxnan                  |
// +------+-------------------------------+
// | 0x7E | ROP: 0x007E0955               |
// |      | RPN: SPxDSxo                  |
// +------+-------------------------------+
// | 0x7F | ROP: 0x007F03C9               |
// |      | RPN: DPSaan                   |
// +------+-------------------------------+
// | 0x80 | ROP: 0x008003E9               |
// |      | RPN: DPSaa                    |
// +------+-------------------------------+
// | 0x81 | ROP: 0x00810975               |
// |      | RPN: SPxDSxon                 |
// +------+-------------------------------+
// | 0x82 | ROP: 0x00820C49               |
// |      | RPN: DPSxna                   |
// +------+-------------------------------+
// | 0x83 | ROP: 0x00831E04               |
// |      | RPN: SPDSnoaxn                |
// +------+-------------------------------+
// | 0x84 | ROP: 0x00840C48               |
// |      | RPN: SDPxna                   |
// +------+-------------------------------+
// | 0x85 | ROP: 0x00851E05               |
// |      | RPN: PDSPnoaxn                |
// +------+-------------------------------+
// | 0x86 | ROP: 0x008617A6               |
// |      | RPN: DSPDSoaxx                |
// +------+-------------------------------+
// | 0x87 | ROP: 0x008701C5               |
// |      | RPN: PDSaxn                   |
// +------+-------------------------------+
// | 0x88 | ROP: 0x008800C6 (SRCAND)      |
// |      | RPN: DSa                      |
// +------+-------------------------------+
// | 0x89 | ROP: 0x00891B08               |
// |      | RPN: SDPSnaoxn                |
// +------+-------------------------------+
// | 0x8A | ROP: 0x008A0E06               |
// |      | RPN: DSPnoa                   |
// +------+-------------------------------+
// | 0x8B | ROP: 0x008B0666               |
// |      | RPN: DSPDxoxn                 |
// +------+-------------------------------+
// | 0x8C | ROP: 0x008C0E08               |
// |      | RPN: SDPnoa                   |
// +------+-------------------------------+
// | 0x8D | ROP: 0x008D0668               |
// |      | RPN: SDPSxoxn                 |
// +------+-------------------------------+
// | 0x8E | ROP: 0x008E1D7C               |
// |      | RPN: SSDxPDxax                |
// +------+-------------------------------+
// | 0x8F | ROP: 0x008F0CE5               |
// |      | RPN: PDSanan                  |
// +------+-------------------------------+
// | 0x90 | ROP: 0x00900C45               |
// |      | RPN: PDSxna                   |
// +------+-------------------------------+
// | 0x91 | ROP: 0x00911E08               |
// |      | RPN: SDPSnoaxn                |
// +------+-------------------------------+
// | 0x92 | ROP: 0x009217A9               |
// |      | RPN: DPSDPoaxx                |
// +------+-------------------------------+
// | 0x93 | ROP: 0x009301C4               |
// |      | RPN: SPDaxn                   |
// +------+-------------------------------+
// | 0x94 | ROP: 0x009417AA               |
// |      | RPN: PSDPSoaxx                |
// +------+-------------------------------+
// | 0x95 | ROP: 0x009501C9               |
// |      | RPN: DPSaxn                   |
// +------+-------------------------------+
// | 0x96 | ROP: 0x00960169               |
// |      | RPN: DPSxx                    |
// +------+-------------------------------+
// | 0x97 | ROP: 0x0097588A               |
// |      | RPN: PSDPSonoxx               |
// +------+-------------------------------+
// | 0x98 | ROP: 0x00981888               |
// |      | RPN: SDPSonoxn                |
// +------+-------------------------------+
// | 0x99 | ROP: 0x00990066               |
// |      | RPN: DSxn                     |
// +------+-------------------------------+
// | 0x9A | ROP: 0x009A0709               |
// |      | RPN: DPSnax                   |
// +------+-------------------------------+
// | 0x9B | ROP: 0x009B07A8               |
// |      | RPN: SDPSoaxn                 |
// +------+-------------------------------+
// | 0x9C | ROP: 0x009C0704               |
// |      | RPN: SPDnax                   |
// +------+-------------------------------+
// | 0x9D | ROP: 0x009D07A6               |
// |      | RPN: DSPDoaxn                 |
// +------+-------------------------------+
// | 0x9E | ROP: 0x009E16E6               |
// |      | RPN: DSPDSaoxx                |
// +------+-------------------------------+
// | 0x9F | ROP: 0x009F0345               |
// |      | RPN: PDSxan                   |
// +------+-------------------------------+
// | 0xA0 | ROP: 0x00A000C9               |
// |      | RPN: DPa                      |
// +------+-------------------------------+
// | 0xA1 | ROP: 0x00A11B05               |
// |      | RPN: PDSPnaoxn                |
// +------+-------------------------------+
// | 0xA2 | ROP: 0x00A20E09               |
// |      | RPN: DPSnoa                   |
// +------+-------------------------------+
// | 0xA3 | ROP: 0x00A30669               |
// |      | RPN: DPSDxoxn                 |
// +------+-------------------------------+
// | 0xA4 | ROP: 0x00A41885               |
// |      | RPN: PDSPonoxn                |
// +------+-------------------------------+
// | 0xA5 | ROP: 0x00A50065               |
// |      | RPN: PDxn                     |
// +------+-------------------------------+
// | 0xA6 | ROP: 0x00A60706               |
// |      | RPN: DSPnax                   |
// +------+-------------------------------+
// | 0xA7 | ROP: 0x00A707A5               |
// |      | RPN: PDSPoaxn                 |
// +------+-------------------------------+
// | 0xA8 | ROP: 0x00A803A9               |
// |      | RPN: DPSoa                    |
// +------+-------------------------------+
// | 0xA9 | ROP: 0x00A90189               |
// |      | RPN: DPSoxn                   |
// +------+-------------------------------+
// | 0xAA | ROP: 0x00AA0029               |
// |      | RPN: D                        |
// +------+-------------------------------+
// | 0xAB | ROP: 0x00AB0889               |
// |      | RPN: DPSono                   |
// +------+-------------------------------+
// | 0xAC | ROP: 0x00AC0744               |
// |      | RPN: SPDSxax                  |
// +------+-------------------------------+
// | 0xAD | ROP: 0x00AD06E9               |
// |      | RPN: DPSDaoxn                 |
// +------+-------------------------------+
// | 0xAE | ROP: 0x00AE0B06               |
// |      | RPN: DSPnao                   |
// +------+-------------------------------+
// | 0xAF | ROP: 0x00AF0229               |
// |      | RPN: DPno                     |
// +------+-------------------------------+
// | 0xB0 | ROP: 0x00B00E05               |
// |      | RPN: PDSnoa                   |
// +------+-------------------------------+
// | 0xB1 | ROP: 0x00B10665               |
// |      | RPN: PDSPxoxn                 |
// +------+-------------------------------+
// | 0xB2 | ROP: 0x00B21974               |
// |      | RPN: SSPxDSxox                |
// +------+-------------------------------+
// | 0xB3 | ROP: 0x00B30CE8               |
// |      | RPN: SDPanan                  |
// +------+-------------------------------+
// | 0xB4 | ROP: 0x00B4070A               |
// |      | RPN: PSDnax                   |
// +------+-------------------------------+
// | 0xB5 | ROP: 0x00B507A9               |
// |      | RPN: DPSDoaxn                 |
// +------+-------------------------------+
// | 0xB6 | ROP: 0x00B616E9               |
// |      | RPN: DPSDPaoxx                |
// +------+-------------------------------+
// | 0xB7 | ROP: 0x00B70348               |
// |      | RPN: SDPxan                   |
// +------+-------------------------------+
// | 0xB8 | ROP: 0x00B8074A               |
// |      | RPN: PSDPxax                  |
// +------+-------------------------------+
// | 0xB9 | ROP: 0x00B906E6               |
// |      | RPN: DSPDaoxn                 |
// +------+-------------------------------+
// | 0xBA | ROP: 0x00BA0B09               |
// |      | RPN: DPSnao                   |
// +------+-------------------------------+
// | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
// |      | RPN: DSno                     |
// +------+-------------------------------+
// | 0xBC | ROP: 0x00BC1CE4               |
// |      | RPN: SPDSanax                 |
// +------+-------------------------------+
// | 0xBD | ROP: 0x00BD0D7D               |
// |      | RPN: SDxPDxan                 |
// +------+-------------------------------+
// | 0xBE | ROP: 0x00BE0269               |
// |      | RPN: DPSxo                    |
// +------+-------------------------------+
// | 0xBF | ROP: 0x00BF08C9               |
// |      | RPN: DPSano                   |
// +------+-------------------------------+
// | 0xC0 | ROP: 0x00C000CA (MERGECOPY)   |
// |      | RPN: PSa                      |
// +------+-------------------------------+
// | 0xC1 | ROP: 0x00C11B04               |
// |      | RPN: SPDSnaoxn                |
// +------+-------------------------------+
// | 0xC2 | ROP: 0x00C21884               |
// |      | RPN: SPDSonoxn                |
// +------+-------------------------------+
// | 0xC3 | ROP: 0x00C3006A               |
// |      | RPN: PSxn                     |
// +------+-------------------------------+
// | 0xC4 | ROP: 0x00C40E04               |
// |      | RPN: SPDnoa                   |
// +------+-------------------------------+
// | 0xC5 | ROP: 0x00C50664               |
// |      | RPN: SPDSxoxn                 |
// +------+-------------------------------+
// | 0xC6 | ROP: 0x00C60708               |
// |      | RPN: SDPnax                   |
// +------+-------------------------------+
// | 0xC7 | ROP: 0x00C707AA               |
// |      | RPN: PSDPoaxn                 |
// +------+-------------------------------+
// | 0xC8 | ROP: 0x00C803A8               |
// |      | RPN: SDPoa                    |
// +------+-------------------------------+
// | 0xC9 | ROP: 0x00C90184               |
// |      | RPN: SPDoxn                   |
// +------+-------------------------------+
// | 0xCA | ROP: 0x00CA0749               |
// |      | RPN: DPSDxax                  |
// +------+-------------------------------+
// | 0xCB | ROP: 0x00CB06E4               |
// |      | RPN: SPDSaoxn                 |
// +------+-------------------------------+
// | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
// |      | RPN: S                        |
// +------+-------------------------------+
// | 0xCD | ROP: 0x00CD0888               |
// |      | RPN: SDPono                   |
// +------+-------------------------------+
// | 0xCE | ROP: 0x00CE0B08               |
// |      | RPN: SDPnao                   |
// +------+-------------------------------+
// | 0xCF | ROP: 0x00CF0224               |
// |      | RPN: SPno                     |
// +------+-------------------------------+
// | 0xD0 | ROP: 0x00D00E0A               |
// |      | RPN: PSDnoa                   |
// +------+-------------------------------+
// | 0xD1 | ROP: 0x00D1066A               |
// |      | RPN: PSDPxoxn                 |
// +------+-------------------------------+
// | 0xD2 | ROP: 0x00D20705               |
// |      | RPN: PDSnax                   |
// +------+-------------------------------+
// | 0xD3 | ROP: 0x00D307A4               |
// |      | RPN: SPDSoaxn                 |
// +------+-------------------------------+
// | 0xD4 | ROP: 0x00D41D78               |
// |      | RPN: SSPxPDxax                |
// +------+-------------------------------+
// | 0xD5 | ROP: 0x00D50CE9               |
// |      | RPN: DPSanan                  |
// +------+-------------------------------+
// | 0xD6 | ROP: 0x00D616EA               |
// |      | RPN: PSDPSaoxx                |
// +------+-------------------------------+
// | 0xD7 | ROP: 0x00D70349               |
// |      | RPN: DPSxan                   |
// +------+-------------------------------+
// | 0xD8 | ROP: 0x00D80745               |
// |      | RPN: PDSPxax                  |
// +------+-------------------------------+
// | 0xD9 | ROP: 0x00D906E8               |
// |      | RPN: SDPSaoxn                 |
// +------+-------------------------------+
// | 0xDA | ROP: 0x00DA1CE9               |
// |      | RPN: DPSDanax                 |
// +------+-------------------------------+
// | 0xDB | ROP: 0x00DB0D75               |
// |      | RPN: SPxDSxan                 |
// +------+-------------------------------+
// | 0xDC | ROP: 0x00DC0B04               |
// |      | RPN: SPDnao                   |
// +------+-------------------------------+
// | 0xDD | ROP: 0x00DD0228               |
// |      | RPN: SDno                     |
// +------+-------------------------------+
// | 0xDE | ROP: 0x00DE0268               |
// |      | RPN: SDPxo                    |
// +------+-------------------------------+
// | 0xDF | ROP: 0x00DF08C8               |
// |      | RPN: SDPano                   |
// +------+-------------------------------+
// | 0xE0 | ROP: 0x00E003A5               |
// |      | RPN: PDSoa                    |
// +------+-------------------------------+
// | 0xE1 | ROP: 0x00E10185               |
// |      | RPN: PDSoxn                   |
// +------+-------------------------------+
// | 0xE2 | ROP: 0x00E20746               |
// |      | RPN: DSPDxax                  |
// +------+-------------------------------+
// | 0xE3 | ROP: 0x00E306EA               |
// |      | RPN: PSDPaoxn                 |
// +------+-------------------------------+
// | 0xE4 | ROP: 0x00E40748               |
// |      | RPN: SDPSxax                  |
// +------+-------------------------------+
// | 0xE5 | ROP: 0x00E506E5               |
// |      | RPN: PDSPaoxn                 |
// +------+-------------------------------+
// | 0xE6 | ROP: 0x00E61CE8               |
// |      | RPN: SDPSanax                 |
// +------+-------------------------------+
// | 0xE7 | ROP: 0x00E70D79               |
// |      | RPN: SPxPDxan                 |
// +------+-------------------------------+
// | 0xE8 | ROP: 0x00E81D74               |
// |      | RPN: SSPxDSxax                |
// +------+-------------------------------+
// | 0xE9 | ROP: 0x00E95CE6               |
// |      | RPN: DSPDSanaxxn              |
// +------+-------------------------------+
// | 0xEA | ROP: 0x00EA02E9               |
// |      | RPN: DPSao                    |
// +------+-------------------------------+
// | 0xEB | ROP: 0x00EB0849               |
// |      | RPN: DPSxno                   |
// +------+-------------------------------+
// | 0xEC | ROP: 0x00EC02E8               |
// |      | RPN: SDPao                    |
// +------+-------------------------------+
// | 0xED | ROP: 0x00ED0848               |
// |      | RPN: SDPxno                   |
// +------+-------------------------------+
// | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
// |      | RPN: DSo                      |
// +------+-------------------------------+
// | 0xEF | ROP: 0x00EF0A08               |
// |      | RPN: SDPnoo                   |
// +------+-------------------------------+
// | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
// |      | RPN: P                        |
// +------+-------------------------------+
// | 0xF1 | ROP: 0x00F10885               |
// |      | RPN: PDSono                   |
// +------+-------------------------------+
// | 0xF2 | ROP: 0x00F20B05               |
// |      | RPN: PDSnao                   |
// +------+-------------------------------+
// | 0xF3 | ROP: 0x00F3022A               |
// |      | RPN: PSno                     |
// +------+-------------------------------+
// | 0xF4 | ROP: 0x00F40B0A               |
// |      | RPN: PSDnao                   |
// +------+-------------------------------+
// | 0xF5 | ROP: 0x00F50225               |
// |      | RPN: PDno                     |
// +------+-------------------------------+
// | 0xF6 | ROP: 0x00F60265               |
// |      | RPN: PDSxo                    |
// +------+-------------------------------+
// | 0xF7 | ROP: 0x00F708C5               |
// |      | RPN: PDSano                   |
// +------+-------------------------------+
// | 0xF8 | ROP: 0x00F802E5               |
// |      | RPN: PDSao                    |
// +------+-------------------------------+
// | 0xF9 | ROP: 0x00F90845               |
// |      | RPN: PDSxno                   |
// +------+-------------------------------+
// | 0xFA | ROP: 0x00FA0089               |
// |      | RPN: DPo                      |
// +------+-------------------------------+
// | 0xFB | ROP: 0x00FB0A09 (PATPAINT)    |
// |      | RPN: DPSnoo                   |
// +------+-------------------------------+
// | 0xFC | ROP: 0x00FC008A               |
// |      | RPN: PSo                      |
// +------+-------------------------------+
// | 0xFD | ROP: 0x00FD0A0A               |
// |      | RPN: PSDnoo                   |
// +------+-------------------------------+
// | 0xFE | ROP: 0x00FE02A9               |
// |      | RPN: DPSoo                    |
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+

//For example, by using the previous table, it can be determined that the raster operation at
//index 0xEE (SRCPAINT) replaces the values of the pixels in the destination bitmap with a
//combination of pixel values of the destination and source bitmaps.
//For more information about ternary raster operations, see [MSDN-TRO] and [WGFX] section
//11.1.


class RDPDestBlt {
    public:
    Rect rect;
    uint8_t rop;

    RDPDestBlt(Rect rect, uint8_t rop) :
        rect(rect), rop(rop)
        {}

    bool operator==(const RDPDestBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             ;
    }

    void emit(Stream & stream,
                        RDPOrderCommon & common,
                        const RDPOrderCommon & oldcommon,
                        const RDPDestBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // DESTBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft   != 0) * 0x01
                      | (dr.dtop    != 0) * 0x02
                      | (dr.dwidth  != 0) * 0x04
                      | (dr.dheight != 0) * 0x08
                      | (this->rop != oldcmd.rop) * 0x10
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->rop);
        }
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "destblt(rect(%d,%d,%d,%d) rop=%x)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            this->rop);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

};



class RDPMemBlt {
    public:
    uint16_t cache_id;
    Rect rect;
    uint8_t rop;
    uint16_t srcx;
    uint16_t srcy;
    uint16_t cache_idx;

    RDPMemBlt(uint16_t cache_id, Rect rect, uint8_t rop, uint16_t srcx, uint16_t srcy, uint16_t cache_idx) :
        cache_id(cache_id),
        rect(rect),
        rop(rop),
        srcx(srcx),
        srcy(srcy),
        cache_idx(cache_idx)
        {
        }

    bool operator==(const RDPMemBlt &other) const {
        return  (this->cache_id == other.cache_id)
             && (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->srcx == other.srcx)
             && (this->srcy == other.srcy)
             && (this->cache_idx == other.cache_idx)
             ;
    }

    void emit(Stream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPMemBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // MEMBLT fields bytes
        // ------------------------------
        // 0x01: cacheId
        // 0x02: x coordinate
        // 0x04: y coordinate
        // 0x08: cx coordinate
        // 0x10: cy coordinate
        // 0x20: rop byte
        // 0x40: srcx
        // 0x80: srcy

        // 0x100: cacheidx

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= ((dr.fully_relative()
                   && is_1_byte(this->srcx - oldcmd.srcx)
                   && is_1_byte(this->srcy - oldcmd.srcy))
                                                           * DELTA);

        header.fields = (this->cache_id != oldcmd.cache_id  ) * 0x001
                      | (dr.dleft   != 0                    ) * 0x002
                      | (dr.dtop    != 0                    ) * 0x004
                      | (dr.dwidth  != 0                    ) * 0x008
                      | (dr.dheight != 0                    ) * 0x010
                      | (this->rop != oldcmd.rop            ) * 0x020
                      | ((this->srcx - oldcmd.srcx) != 0    ) * 0x040
                      | ((this->srcy - oldcmd.srcy) != 0    ) * 0x080
                      | (this->cache_idx != oldcmd.cache_idx) * 0x100
                      ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x01){
            stream.out_uint16_le(this->cache_id);
        }

        header.emit_rect(stream, 0x02, this->rect, oldcmd.rect);

        if (header.fields & 0x20){
            stream.out_uint8(this->rop);
        }

        header.emit_src(stream, 0x40, this->srcx, this->srcy, oldcmd.srcx, oldcmd.srcy);

        if (header.fields & 0x100){
            stream.out_uint16_le(this->cache_idx);
        }
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x01) {
            this->cache_id = stream.in_uint16_le();
        }

        header.receive_rect(stream, 0x02, this->rect);

        if (header.fields & 0x20) {
            this->rop = stream.in_uint8();
        }

        header.receive_src(stream, 0x40, this->srcx, this->srcy);

        if (header.fields & 0x100) {
            this->cache_idx = stream.in_uint16_le();
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg  += snprintf(
            buffer+lg,
            sz-lg,
            "memblt(cache_id=%d rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d cache_idx=%d)\n",
            this->cache_id,
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            this->rop,
            this->srcx, this->srcy,
            this->cache_idx);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }


};


class RDPPatBlt {
    public:
    Rect rect;
    uint8_t rop;
    uint32_t back_color;
    uint32_t fore_color;
    RDPBrush brush;

    RDPPatBlt(const Rect rect, uint8_t rop,
              uint32_t back_color, uint32_t fore_color,
              const RDPBrush & brush) :
        rect(rect),
        rop(rop),
        back_color(back_color),
        fore_color(fore_color),
        brush(brush)
        {
        }

    bool operator==(const RDPPatBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->back_color == other.back_color)
             && (this->fore_color == other.fore_color)
             && (this->brush.org_x == other.brush.org_x)
             && (this->brush.org_y == other.brush.org_y)
             && (this->brush.style == other.brush.style)
             && (this->brush.hatch == other.brush.hatch)
             && ((this->brush.style != 0x03)
                || (0 == memcmp(this->brush.extra, other.brush.extra, 7)))
             ;
    }

    void emit(Stream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPPatBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // PATBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: Back color (3 bytes)
        // 0x40: Fore color (3 bytes)
        // 0x80: Brush Org X (1 byte)

        // 0x0100: Brush Org Y (1 byte)
        // 0x0200: Brush style (1 byte)
        // 0x0400: Brush Hatch (1 byte)

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft   != 0) * 0x01
                       | (dr.dtop     != 0) * 0x02
                       | (dr.dwidth   != 0) * 0x04
                       | (dr.dheight  != 0) * 0x08
                       | (this->rop         != oldcmd.rop        ) *  0x10
                       | (this->back_color  != oldcmd.back_color ) *  0x20
                       | (this->fore_color  != oldcmd.fore_color ) *  0x40

                       | (this->brush.org_x != oldcmd.brush.org_x) *  0x80
                       | (this->brush.org_y != oldcmd.brush.org_y) * 0x100
                       | (this->brush.style != oldcmd.brush.style) * 0x200
                       | (this->brush.hatch != oldcmd.brush.hatch) * 0x400
                       | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0) * 0x800
                       ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10) {
            stream.out_uint8(this->rop);
        }
        if (header.fields & 0x20) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x40) {
            stream.out_uint8(this->fore_color);
            stream.out_uint8(this->fore_color >> 8);
            stream.out_uint8(this->fore_color >> 16);
        }

        header.emit_brush(stream, 0x80, this->brush, oldcmd.brush);
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }
        if (header.fields & 0x20) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }
        if (header.fields & 0x40) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->fore_color = r + (g << 8) + (b << 16);
        }

        header.receive_brush(stream, 0x080, this->brush);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);

        if (this->brush.style == 0x3){
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d brush.extra=[%.2x %.2x %.2x %.2x %.2x %.2x %.2x])\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                this->rop,
                this->back_color,
                this->fore_color,
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch,
                this->brush.extra[0],
                this->brush.extra[1],
                this->brush.extra[2],
                this->brush.extra[3],
                this->brush.extra[4],
                this->brush.extra[5],
                this->brush.extra[6]
                );
        }
        else {
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d)\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                this->rop,
                this->back_color,
                this->fore_color,
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch
                );
        }
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};

class RDPLineTo {
    // LINE fields bytes
    // ------------------
    // 0x01: Back mode
    // 0x02: x_start coordinate
    // 0x04: y_start coordinate
    // 0x08: x_end coordinate
    // 0x10: y_end byte
    // 0x20: BackColor (3 bytes)
    // 0x40: rop2
    // 0x80: pen style

    // 0x100: pen width
    // 0x200: pen color (3 bytes)

    public:
    uint8_t back_mode;
    int16_t startx;
    int16_t starty;
    int16_t endx;
    int16_t endy;
    uint32_t back_color;
    uint8_t rop2;
    RDPPen pen;

    RDPLineTo(uint8_t back_mode,
              int16_t startx, int16_t starty, int16_t endx, int16_t endy,
              uint32_t back_color,
              uint8_t rop2,
              const RDPPen & pen) :
        back_mode(back_mode),
        startx(startx),
        starty(starty),
        endx(endx),
        endy(endy),
        back_color(back_color),
        rop2(rop2),
        pen(pen)
        {
        }

    bool operator==(const RDPLineTo &other) const {
        return  (this->startx == other.startx)
             && (this->starty == other.starty)
             && (this->endx == other.endx)
             && (this->endy == other.endy)
             && (this->rop2 == other.rop2)
             && (this->back_color == other.back_color)
             && (this->pen.style == other.pen.style)
             && (this->pen.width == other.pen.width)
             && (this->pen.color == other.pen.color)
             ;
    }

    void emit(Stream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPLineTo & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!(common.clip.rect_contains_pt(this->startx, this->starty)
           && common.clip.rect_contains_pt(this->endx, this->endy))){
           header.control |= BOUNDS;
        }

        header.control |= (is_1_byte(this->startx - oldcmd.startx)
                 && is_1_byte(this->starty - oldcmd.starty)
                 && is_1_byte(this->endx - oldcmd.endx)
                 && is_1_byte(this->endy - oldcmd.endy)) * DELTA;

        header.fields =
                (this->back_mode  != oldcmd.back_mode  ) * 0x001
              | (this->startx     != oldcmd.startx     ) * 0x002
              | (this->starty     != oldcmd.starty     ) * 0x004
              | (this->endx       != oldcmd.endx       ) * 0x008
              | (this->endy       != oldcmd.endy       ) * 0x010
              | (this->back_color != oldcmd.back_color ) * 0x020
              | (this->rop2       != oldcmd.rop2       ) * 0x040
              | (this->pen.style  != oldcmd.pen.style  ) * 0x080
              | (this->pen.width  != oldcmd.pen.width  ) * 0x100
              | (this->pen.color  != oldcmd.pen.color  ) * 0x200
              ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x001) { stream.out_uint16_le(this->back_mode); }

        header.emit_coord(stream, 0x02, this->startx, oldcmd.startx);
        header.emit_coord(stream, 0x04, this->starty, oldcmd.starty);
        header.emit_coord(stream, 0x08, this->endx,   oldcmd.endx);
        header.emit_coord(stream, 0x10, this->endy,   oldcmd.endy);

        if (header.fields & 0x20) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x40) { stream.out_uint8(this->rop2); }
        header.emit_pen(stream, 0x80, this->pen, oldcmd.pen);
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x001) {
            this->back_mode = stream.in_uint16_le();
        }

        header.receive_coord(stream, 0x002, this->startx);
        header.receive_coord(stream, 0x004, this->starty);
        header.receive_coord(stream, 0x008, this->endx);
        header.receive_coord(stream, 0x010, this->endy);

        if (header.fields & 0x020) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }

        if (header.fields & 0x040) {
            this->rop2 = stream.in_uint8();
        }

        header.receive_pen(stream, 0x080, this->pen);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);

        lg += snprintf(
            buffer+lg,
            sz-lg,
            "lineto(back_mode=%.2x "
                "startx=%d starty=%d endx=%d endy=%d "
                "rop2=%d back_color=%.6x"
                "pen.style=%d pen.width=%d pen.color=%.6x "
                "\n",
            this->back_mode,
            this->startx, this->starty, this->endx, this->endy,
            this->rop2,
            this->back_color,
            this->pen.style,
            this->pen.width,
            this->pen.color
            );
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};

class RDPGlyphIndex {
// GLYPHINDEX_ORDER fields bytes
// ------------------------------
// 0x01: cache_id (font = ID of the glyph cache)
// 0x02: flAccel (flags)
// 0x04: UICharInc
// 0x08: fOpRedundant (mix_mode)
// 0x10: back_color
// 0x20: fore_color

// Coordinates of background
// 0x40: bk_left (x coordinate)
// 0x80: bk_top (y coordinate)
// 0x100: bk_right (x + cx coordinate)
// 0x200: bk_bottom (y + cy coordinate)

// Coordinates of opaque rectangle
// 0x400: op_left (x coordinate)
// 0x800: op_top (y coordinate)
// 0x1000: op_right (x + cx coordinate)
// 0x2000: op_bottom (y + cy coordinate)

// 0x4000 - 0x40000 : brush

// 0x80000 glyph_x
// 0x100000 glyph_y
// 0x200000 data_len, data

    public:
    uint8_t cache_id;
    int16_t fl_accel;
    int16_t ui_charinc;
    int16_t f_op_redundant;
    uint32_t back_color;
    uint32_t fore_color;
    Rect bk;
    Rect op;
    RDPBrush brush;
    int16_t glyph_x;
    int16_t glyph_y;
    uint8_t data_len;
    uint8_t data[256];

    RDPGlyphIndex(uint8_t cache_id, uint8_t fl_accel, uint8_t ui_charinc,
        uint8_t f_op_redundant, uint32_t back_color, uint32_t fore_color,
        const Rect & bk, const Rect & op, const RDPBrush & brush,
        int16_t glyph_x, int16_t glyph_y,
        uint8_t data_len, uint8_t * data) :
            cache_id(cache_id),
            fl_accel(fl_accel),
            ui_charinc(ui_charinc),
            f_op_redundant(f_op_redundant),
            back_color(back_color),
            fore_color(fore_color),
            bk(bk),
            op(op),
            brush(brush),
            glyph_x(glyph_x),
            glyph_y(glyph_y),
            data_len(data_len)
        {
            memcpy(this->data, data, data_len);
        }

    bool operator==(const RDPGlyphIndex &other) const {
        return  (this->cache_id == other.cache_id)
             && (this->fl_accel == other.fl_accel)
             && (this->ui_charinc == other.ui_charinc)
             && (this->f_op_redundant == other.f_op_redundant)
             && (this->back_color == other.back_color)
             && (this->fore_color == other.fore_color)
             && (this->bk == other.bk)
             && (this->op == other.op)
             && (this->brush == other.brush)
             && (this->glyph_x == other.glyph_x)
             && (this->glyph_y == other.glyph_y)
             && (this->data_len == other.data_len)
             && (0 == memcmp(this->data, other.data, this->data_len))
             ;
    }

    void emit(Stream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPGlyphIndex & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->bk)){
            header.control |= BOUNDS;
        }

        if (this->op.cx > 1 && !common.clip.contains(this->op)){
            header.control |= BOUNDS;
        }

        // DELTA flag Does not seems to works as usual for this function
        // (at least in rdesktop)
        // hence coordinates fields are transmitted as absolute fields
        // DELTA may also raise some problems with empty rects,
        // if initial rect is empty DELTA should not be used
//        control |=
//            (is_1_byte(this->bk.x               - oldcmd.bk.x                )
//          && is_1_byte(this->bk.y               - oldcmd.bk.y                )
//          && is_1_byte(this->bk.x + this->bk.cx - oldcmd.bk.x + oldcmd.bk.cx )
//          && is_1_byte(this->bk.y + this->bk.cy - oldcmd.bk.y + oldcmd.bk.cy )
//          && is_1_byte(this->op.x               - oldcmd.op.x                )
//          && is_1_byte(this->op.y               - oldcmd.op.y                )
//          && is_1_byte(this->op.x + this->op.cx - oldcmd.op.x + oldcmd.op.cx )
//          && is_1_byte(this->op.y + this->op.cy - oldcmd.op.y + oldcmd.op.cy )
//          && is_1_byte(this->glyph_x            - oldcmd.glyph_x             )
//          && is_1_byte(this->glyph_y            - oldcmd.glyph_y             )
//                                                                      ) * DELTA;

        header.fields =
            (this->cache_id           != oldcmd.cache_id            ) * 0x000001
          | (this->fl_accel           != oldcmd.fl_accel            ) * 0x000002
          | (this->ui_charinc         != oldcmd.ui_charinc          ) * 0x000004
          | (this->f_op_redundant     != oldcmd.f_op_redundant      ) * 0x000008

          | (this->back_color         != oldcmd.back_color          ) * 0x000010
          | (this->fore_color         != oldcmd.fore_color          ) * 0x000020

          | (this->bk.x               != oldcmd.bk.x                ) * 0x000040
          | (this->bk.y               != oldcmd.bk.y                ) * 0x000080
          | (this->bk.x + this->bk.cx != oldcmd.bk.x + oldcmd.bk.cx ) * 0x000100
          | (this->bk.y + this->bk.cy != oldcmd.bk.y + oldcmd.bk.cy ) * 0x000200

          | (this->op.x               != oldcmd.op.x                ) * 0x000400
          | (this->op.y               != oldcmd.op.y                ) * 0x000800
          | (this->op.x + this->op.cx != oldcmd.op.x + oldcmd.op.cx ) * 0x001000
          | (this->op.y + this->op.cy != oldcmd.op.y + oldcmd.op.cy ) * 0x002000

          | (this->brush.org_x        != oldcmd.brush.org_x         ) * 0x004000
          | (this->brush.org_y        != oldcmd.brush.org_y         ) * 0x008000
          | (this->brush.style        != oldcmd.brush.style         ) * 0x010000
          | (this->brush.hatch        != oldcmd.brush.hatch         ) * 0x020000
          | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0  ) * 0x040000

          | (this->glyph_x            != oldcmd.glyph_x             ) * 0x080000
          | (this->glyph_y            != oldcmd.glyph_y             ) * 0x100000
          | (data_len                 != 0                          ) * 0x200000
              ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x001) { stream.out_uint8(this->cache_id); }
        if (header.fields & 0x002) { stream.out_uint8(this->fl_accel); }
        if (header.fields & 0x004) { stream.out_uint8(this->ui_charinc); }
        if (header.fields & 0x008) { stream.out_uint8(this->f_op_redundant); }
        if (header.fields & 0x010) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x020) {
            stream.out_uint8(this->fore_color);
            stream.out_uint8(this->fore_color >> 8);
            stream.out_uint8(this->fore_color >> 16);
        }

        header.emit_coord(stream, 0x0040, this->bk.x, oldcmd.bk.x);
        header.emit_coord(stream, 0x0080, this->bk.y, oldcmd.bk.y);
        header.emit_coord(stream, 0x0100, this->bk.x + this->bk.cx - 1,
                                         oldcmd.bk.x + oldcmd.bk.cx - 1);
        header.emit_coord(stream, 0x0200, this->bk.y + this->bk.cy - 1,
                                         oldcmd.bk.y + oldcmd.bk.cy - 1);

        header.emit_coord(stream, 0x0400, this->op.x, oldcmd.op.x);
        header.emit_coord(stream, 0x0800, this->op.y, oldcmd.op.y);
        header.emit_coord(stream, 0x1000, this->op.x + this->op.cx - 1,
                                         oldcmd.op.x + oldcmd.op.cx - 1);
        header.emit_coord(stream, 0x2000, this->op.y + this->op.cy - 1,
                                         oldcmd.op.y + oldcmd.op.cy - 1);

        header.emit_brush(stream, 0x4000, this->brush, oldcmd.brush);

        header.emit_coord(stream, 0x080000, this->glyph_x, oldcmd.glyph_x);
        header.emit_coord(stream, 0x100000, this->glyph_y, oldcmd.glyph_y);

        if (header.fields & 0x200000) {
            stream.out_uint8(this->data_len);
            stream.out_copy_bytes(this->data, this->data_len);
        }

    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x001) { this->cache_id = stream.in_uint8(); }
        if (header.fields & 0x002) { this->fl_accel = stream.in_uint8(); }
        if (header.fields & 0x004) { this->ui_charinc = stream.in_uint8(); }
        if (header.fields & 0x008) { this->f_op_redundant = stream.in_uint8(); }

        if (header.fields & 0x010) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }

        if (header.fields & 0x020) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->fore_color = r + (g << 8) + (b << 16);
        }

        int16_t bk_left = this->bk.x;
        int16_t bk_top = this->bk.y;
        int16_t bk_right = this->bk.x + this->bk.cx - 1;
        int16_t bk_bottom = this->bk.y + this->bk.cy - 1;

        header.receive_coord(stream, 0x0040, bk_left);
        header.receive_coord(stream, 0x0080, bk_top);
        header.receive_coord(stream, 0x0100, bk_right);
        header.receive_coord(stream, 0x0200, bk_bottom);

        this->bk.x = bk_left;
        this->bk.y = bk_top;
        this->bk.cx = bk_right - this->bk.x + 1;
        this->bk.cy = bk_bottom - this->bk.y + 1;

        int16_t op_left = this->op.x;
        int16_t op_top = this->op.y;
        int16_t op_right = this->op.x + this->op.cx - 1;
        int16_t op_bottom = this->op.y + this->op.cy - 1;

        header.receive_coord(stream, 0x0400, op_left);
        header.receive_coord(stream, 0x0800, op_top);
        header.receive_coord(stream, 0x1000, op_right);
        header.receive_coord(stream, 0x2000, op_bottom);

        this->op.x = op_left;
        this->op.y = op_top;
        this->op.cx = op_right - this->op.x + 1;
        this->op.cy = op_bottom - this->op.y + 1;

        header.receive_brush(stream, 0x4000, this->brush);

        header.receive_coord(stream, 0x080000, this->glyph_x);
        header.receive_coord(stream, 0x100000, this->glyph_y);

        if (header.fields & 0x200000){
            this->data_len = stream.in_uint8();
            memcpy(this->data, stream.in_uint8p(this->data_len), this->data_len);
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "glyphIndex(cache_id=%.2x "
                "fl_accel=%d ui_charinc=%d f_op_redundant=%d "
                "back_color=%.6x fore_color=%.6x "
                "bk=(%d,%d,%d,%d) "
                "op=(%d,%d,%d,%d) "
                "brush.(org_x=%.2x, org_y=%.2x, style=%d hatch=%d extra=[%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x]) "
                "glyph_x=%.2x glyph_y=%.2x data_len=%d "
                "\n",
            this->cache_id,
            this->fl_accel,
            this->ui_charinc,
            this->f_op_redundant,
            this->back_color,
            this->fore_color,
            this->bk.x, this->bk.y, this->bk.cx, this->bk.cy,
            this->op.x, this->op.y, this->op.cx, this->op.cy,
            this->brush.org_x,this->brush.org_y,this->brush.style,this->brush.hatch,
            this->brush.extra[0],this->brush.extra[1],this->brush.extra[2],this->brush.extra[3],
            this->brush.extra[4],this->brush.extra[5],this->brush.extra[6],
            this->glyph_x, this->glyph_y,
            this->data_len);
        lg += snprintf(buffer+lg, sz-lg,"[");
        for (int i = 0; i < this->data_len; i++){
            lg += snprintf(buffer+lg, sz-lg,"%.2x(%c) ",
            (char)this->data[i],
            this->data[i] & 0x80 ? ' ':(char)this->data[i]);
        }
        lg += snprintf(buffer+lg, sz-lg,"]");
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};


class RDPOrderState{
    public:
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect orect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex text;

    RDPOrderState() :
        common(0, Rect()),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        orect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        text(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"")
        {}

};


#endif
