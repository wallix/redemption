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

   New RDP Orders Coder / Decoder : Common parts and constants

*/

#if !defined(__RDPORDERSCOMMON_HPP__)
#define __RDPORDERSCOMMON_HPP__

#include "constants.hpp"
#include "stream.hpp"
#include "rect.hpp"
#include "altoco.hpp"
#include "bitmap.hpp"
#include "client_info.hpp"

//MS-RDPEGDI 3.3.5.1.1.1       Construction of a Primary Drawing Order
//====================================================================

//  All primary drawing orders MUST conform to the structure and rules defined
//  in section 2.2.2.2.1.1.2.

// To efficiently construct a primary drawing order, the server MUST use a
// Primary Drawing Order History (section 3.2.1.1) store. This store holds three
// pieces of information:

//  - Last primary order type constructed.

//  - Current bounding rectangle.

//  - Per-order record of the last value used in each field.

//  These stored records allow the server to use the minimum amount of data when
//  constructing an order; if a field is unchanged from the value that it had
//  when the order type was last sent, it SHOULD NOT be included in the order
//  being constructed. Hence, only fields that have new values are required to
//  be sent to the client. The fields that are present in the order MUST be
//  indicated by the fieldFlags field.

//  If all of the Coord-type fields (see section 2.2.2.2.1.1.1.1) in an order
//  can be represented as a signed delta in the range -127 to 128 from the
//  previous field value, the size of the order SHOULD be optimized by using
//  delta-coordinates (see sections 2.2.2.2.1.1.1.1 and 2.2.2.2.1.1.2). In that
//  case, all of the fields SHOULD be represented using delta-coordinates, and
//  the TS_DELTA_COORDINATES (0x10) flag MUST be used in the primary drawing
//  order header to indicate this fact.

//  Before a given order is sent, the server MUST also ensure that all of the
//  data required to process the order is accessible to the client. For example,
//  if the order refers to a cached item, that item MUST be present in the
//  client-side cache when the order is processed. Or, if palettized color is
//  being used, the correct palette MUST be applied at the client-side.

//  Once a primary drawing order has been constructed and transmitted to the
//  client, the server MUST update the records in the Primary Drawing Order
//  History (section 3.3.1.2) to ensure that future encodings use the minimum
//  fields and data required.

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

    enum SecondaryOrderType {
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
        TS_CACHE_GLYPH     = 3,
     // TS_CACHE_BITMAP_UNCOMPRESSED_REV2 : Cache Bitmap - Revision 2
     // (MS-RDPEGDI section 2.2.2.2.1.2.3)
        TS_CACHE_BITMAP_UNCOMPRESSED_REV2 = 4,
     // TS_CACHE_BITMAP_COMPRESSED_REV2 : Cache Bitmap - Revision 2
     // (MS-RDPEGDI section 2.2.2.2.1.2.3)
        TS_CACHE_BITMAP_COMPRESSED_REV2    = 5,
     // TS_CACHE_BRUSH : Cache Brush
     // (MS-RDPEGDI section 2.2.2.2.1.2.7)
        TS_CACHE_BRUSH    = 7,
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

#endif
