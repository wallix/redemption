/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Common parts and constants
*/


#pragma once

#include <assert.h>

#include "utils/stream.hpp"
#include "utils/rect.hpp"
#include "utils/colors.hpp"

// MS-RDPEGDI 3.3.5.1.1.1       Construction of a Primary Drawing Order
// ====================================================================

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
//  when the order type was last sent, it SHOULD NOT be include " in the order
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

inline void emit_rdp_color(OutStream & stream, RDPColor color)
{
    BGRColor const bgr = color.as_bgr();
    stream.out_uint8(bgr.red());
    stream.out_uint8(bgr.green());
    stream.out_uint8(bgr.blue());
}

inline void receive_rdp_color(InStream & stream, RDPColor & color)
{
    uint8_t r = stream.in_uint8();
    uint8_t g = stream.in_uint8();
    uint8_t b = stream.in_uint8();
    color = RDPColor::from(BGRColor(b, g, r).to_u32());
}

struct RDPPen {
    uint8_t style;
    uint8_t width;
    RDPColor color;
    RDPPen(uint8_t style, uint8_t width, RDPColor color)
        : style(style), width(width), color(color) {}

    RDPPen() : style(0), width(0), color{} {
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
             const uint8_t * extra = reinterpret_cast<const uint8_t*>("\0\0\0\0\0\0\0")) :
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

// [MS-RDPEGDI] - 2.2.2.2.1.1.1.3 Two-Byte Header Variable Field
//  (VARIABLE2_FIELD)
// =============================================================
// The VARIABLE2_FIELD structure is used to encode a variable-length byte-
//  stream that holds a maximum of 32,767 bytes. This structure is always
//  situated at the end of an order.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |             cbData            |       rgbData (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cbData (2 bytes): A 16-bit, unsigned integer. The number of bytes present
//  in the rgbData field.

// rgbData (variable): Variable-length, binary data. The size of this data,
//  in bytes, is given by the cbData field.

// [MS-RDPEGDI] - 2.2.2.2.1.1.1.5 Delta-Encoded Rectangles
//  (DELTA_RECTS_FIELD)
// =======================================================
// The DELTA_RECTS_FIELD structure is used to encode a series of rectangles.
//  Each rectangle is encoded as a (left, top, width, height) quadruple with
//  the left and top components of each quadruple containing the delta from
//  the left and top components of the previous rectangle; the first
//  rectangle in the series is implicitly assumed to be (0, 0, 0, 0). The
//  number of rectangles is order-dependent and not specified by any field
//  within the DELTA_RECTS_FIELD structure. Instead, a separate field within
//  the order that contains the DELTA_RECTS_FIELD structure MUST be used to
//  specify the number of rectangles (this field SHOULD be placed immediately
//  before the DELTA_RECTS_FIELD structure in the order encoding). The
//  maximum number of rectangles that can be encoded by this structure is 45.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      zeroBits (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |               deltaEncodedRectangles (variable)               |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// zeroBits (variable): A variable-length byte field. The zeroBits field is
//  used to indicate the absence of a left, top, width, or height component.
//  The size, in bytes, of the zeroBits field is given by ceil(NumRects / 2)
//  where NumRects is the number of rectangles being encoded. Each rectangle
//  in the series requires four zero-bits (two rectangles per byte) to
//  indicate whether a left, top, width, or height component is zero (and not
//  present), starting with the most significant bits, so that for the first
//  rectangle the left-zero flag is set at (zeroBits[0] & 0x80), the top-zero
//  flag is set at (zeroBits[0] & 0x40), the width-zero flag is set at
//  (zeroBits[0] & 0x20), and the height-zero flag is set at (zeroBits[0] &
//  0x10).

// deltaEncodedRectangles (variable): A variable-length byte field. The
//  deltaEncodedRectangles field contains a series of (left, top, width,
//  height) quadruples with the left and top components in each quadruple
//  specifying the delta from the left and top components of the previous
//  rectangle in the series; the first rectangle in the series is implicitly
//  assumed to be (0, 0, 0, 0).

//  Assume there are two rectangles specified in (left, top, right, bottom)
//   quadruples:

//   1: (L1, T1, R1, B1)
//   2: (L2, T2, R2, B2)

//  Assuming Rectangle 1 is the first in the series, and by using the (left,
//   top, width, height) quadruple encoding scheme, these two rectangles
//   would be specified as:

//   1: (L1, T1, R1 - L1, B1 - T1)
//   2: (L2 - L1, T2 - T1, R2 - L2, B2 - T2)

//  The presence of the left, top, width, or height component for a given
//   quadruple is dictated by the individual bits of the zeroBits field. If
//   the zero bit is set for a given left, top, width, or height component,
//   its value is unchanged from the previous corresponding left, top, width,
//   or height value in the series (a delta of zero), and no data is
//   provided. If the zero bit is not set for a left, right, width, or height
//   component, its value is encoded in a packed signed format:

//   * If the high bit (0x80) is not set in the first encoding byte, the
//     field is 1 byte long and is encoded as a signed delta in the lower 7
//     bits of the byte.

//   * If the high bit of the first encoding byte is set, the lower 7 bits of
//     the first byte and the 8 bits of the next byte are concatenated (the
//     first byte containing the high order bits) to create a 15-bit signed
//     delta value.

struct DeltaEncodedRectangle {
    int16_t leftDelta;
    int16_t topDelta;
    int16_t width;
    int16_t height;
};


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
        TINY       = 0x80  // -2 on number of bytes for fields
    };

    enum {
        DESTBLT         = 0,
        PATBLT          = 1,
        SCREENBLT       = 2,
        NINEGRID        = 7,
        LINE            = 9,
        RECT            = 10,
        DESKSAVE        = 11,
        MEMBLT          = 13,
        MEM3BLT         = 14,
        MULTIDSTBLT     = 15,
        MULTIPATBLT     = 16,
        MULTISCRBLT     = 17,
        MULTIOPAQUERECT = 18,
        POLYGONSC       = 20,
        POLYGONCB       = 21,
        POLYLINE        = 22,
        ELLIPSESC       = 25,
        ELLIPSECB       = 26,
        GLYPHINDEX      = 27

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



// [MS-RDPEGDI] - 2.2.2.2.1.3.1.1 Alternate Secondary Drawing Order Header
//  (ALTSEC_DRAWING_ORDER_HEADER)
// =======================================================================
// The ALTSEC_DRAWING_ORDER_HEADER structure is included in all alternate
//  secondary drawing orders.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  controlFlags |
// +---------------+

// controlFlags (1 byte): An 8-bit, unsigned integer. The control byte that
//  identifies the class and type of the drawing order.

//  The format of the controlFlags byte is described by the following bitmask
//   diagram.

//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
//  |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |cla| orderType |
//  | ss|           |
//  +---+-----------+

//  class (2 bits): A 2-bit, unsigned integer. This field MUST contain only
//   the TS_SECONDARY (0x02) flag to indicate that the order is an alternate
//   secondary drawing order (see section 2.2.2.2.1).

//  orderType (6 bits): A 6-bit, unsigned integer. Identifies the type of
//   alternate secondary drawing order.

//  +----------------------------------+---------------------------------------+
//  | Value                            | Meaning                               |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_SWITCH_SURFACE         | Switch Surface Alternate Secondary    |
//  | 0x00                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.3).                       |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_CREATE_OFFSCR_BITMAP   | Create Offscreen Bitmap Alternate     |
//  | 0x01                             | Secondary Drawing Order (see section  |
//  |                                  | 2.2.2.2.1.3.2).                       |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_STREAM_BITMAP_FIRST    | Stream Bitmap First (Revision 1 and   |
//  | 0x02                             | 2) Alternate Secondary Drawing Order  |
//  |                                  | (see section 2.2.2.2.1.3.5.1).        |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_STREAM_BITMAP_NEXT     | Stream Bitmap Next Alternate          |
//  | 0x03                             | Secondary Drawing Order (see section  |
//  |                                  | 2.2.2.2.1.3.5.2).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_CREATE_NINEGRID_BITMAP | Create NineGrid Bitmap Alternate      |
//  | 0x04                             | Secondary Drawing Order (see section  |
//  |                                  | 2.2.2.2.1.3.4).                       |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_FIRST             | Draw GDI+ First Alternate Secondary   |
//  | 0x05                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.6.2).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_NEXT              | Draw GDI+ Next Alternate Secondary    |
//  | 0x06                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.6.3).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_END               | Draw GDI+ End Alternate Secondary     |
//  | 0x07                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.6.4).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_CACHE_FIRST       | Draw GDI+ First Alternate Secondary   |
//  | 0x08                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.6.2).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_CACHE_NEXT        | Draw GDI+ Cache Next Alternate        |
//  | 0x09                             | Secondary Drawing Order (see section  |
//  |                                  | 2.2.2.2.1.3.6.3).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_GDIP_CACHE_END         | Draw GDI+ Cache End Alternate         |
//  | 0x0A                             | Secondary Drawing Order (see section  |
//  |                                  | 2.2.2.2.1.3.6.4).                     |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_WINDOW                 | Windowing Alternate Secondary Drawing |
//  | 0x0B                             | Order (see [MS-RDPERP] section        |
//  |                                  | 2.2.1.3).                             |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_COMPDESK_FIRST         | Desktop Composition Alternate         |
//  | 0x0C                             | Secondary Drawing Order (see          |
//  |                                  | [MS-RDPEDC] section 2.2.1.1).         |
//  +----------------------------------+---------------------------------------+
//  | TS_ALTSEC_FRAME_MARKER           | Frame Marker Alternate Secondary      |
//  | 0x0D                             | Drawing Order (see section            |
//  |                                  | 2.2.2.2.1.3.7).                       |
//  +----------------------------------+---------------------------------------+

enum {
    TS_ALTSEC_SWITCH_SURFACE
  , TS_ALTSEC_CREATE_OFFSCR_BITMAP
  , TS_ALTSEC_STREAM_BITMAP_FIRST
  , TS_ALTSEC_STREAM_BITMAP_NEXT
  , TS_ALTSEC_CREATE_NINEGRID_BITMAP
  , TS_ALTSEC_GDIP_FIRST
  , TS_ALTSEC_GDIP_NEXT
  , TS_ALTSEC_GDIP_END
  , TS_ALTSEC_GDIP_CACHE_FIRST
  , TS_ALTSEC_GDIP_CACHE_NEXT
  , TS_ALTSEC_GDIP_CACHE_END
  , TS_ALTSEC_WINDOW
  , TS_ALTSEC_COMPDESK_FIRST
  , TS_ALTSEC_FRAME_MARKER
};

class AltsecDrawingOrderHeader {
public:
    uint8_t controlFlags;
    uint8_t class_;
    uint8_t orderType;

    enum {
          SwitchSurface        = 0x00
        , CreateOffscrBitmap   = 0x01
        , StreamBitmapFirst    = 0x02
        , StreamBitmapNext     = 0x03
        , CreateNinegridBitmap = 0x04
        , GdipFirst            = 0x05
        , GdipNext             = 0x06
        , GdipEnd              = 0x07
        , GdipCacheFirst       = 0x08
        , GdipCacheNext        = 0x09
        , GdipCacheEnd         = 0x0A
        , Window               = 0x0B
        , CompdeskFirst        = 0x0C
        , FrameMarker          = 0x0D
    };

    explicit AltsecDrawingOrderHeader(InStream & stream) {
        this->controlFlags = stream.in_uint8();

        this->class_    = (this->controlFlags & 0x03);
        this->orderType = (this->controlFlags >> 2);
    }

    explicit AltsecDrawingOrderHeader(uint8_t controlFlags) {
        this->controlFlags = controlFlags;

        this->class_    = (this->controlFlags & 0x03);
        this->orderType = (this->controlFlags >> 2);
    }

    AltsecDrawingOrderHeader(uint8_t class_, uint8_t orderType) {
        this->class_    = (class_ & 0x03);
        this->orderType = (orderType & 0x3F);

        this->controlFlags = (this->class_ | (this->orderType << 2));
    }
};  // class AltsecDrawingOrderHeader

} /* namespace RDP */



inline static bool is_1_byte(int16_t value){
    return (value >= -128) && (value <= 127);
}

inline static uint8_t pounder_bound(int16_t delta, uint8_t pound)
{
    return ((pound * (delta != 0)) << (4 * is_1_byte(delta)));
}



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

    Bounds(const Rect oldclip, const Rect newclip)
    {
        int16_t old_bounds[4];

        using namespace RDP;
        old_bounds[LEFT]   = oldclip.x;
        old_bounds[TOP]    = oldclip.y;
        old_bounds[RIGHT]  = oldclip.x + oldclip.cx - 1;
        old_bounds[BOTTOM] = oldclip.y + oldclip.cy - 1;

        this->absolute_bounds[LEFT]   = newclip.x;
        this->absolute_bounds[TOP]    = newclip.y;
        this->absolute_bounds[RIGHT]  = newclip.x + newclip.cx - 1;
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

    void emit(OutStream & stream)
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

    void emit_coord(OutStream & stream, uint32_t base, int16_t coord, int16_t oldcoord) const {
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

    void receive_coord(InStream & stream, uint32_t base, int16_t & coord) const
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

    void emit_rect(OutStream & stream, uint32_t base, const Rect rect, const Rect oldr) const
    {
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

    void receive_rect(InStream & stream, uint32_t base, Rect & rect) const
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

    void emit_src(OutStream & stream, uint32_t base,
                  uint16_t srcx, uint16_t srcy,
                  uint16_t oldx, uint16_t oldy) const
    {
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

    void receive_src(InStream & stream, uint32_t base,
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

    void emit_pen(OutStream & stream, uint32_t base,
                  const RDPPen & pen,
                  const RDPPen & /*old_pen*/) const {

        using namespace RDP;
        if (this->fields & base) {
            stream.out_uint8(pen.style);
        }
        if (this->fields & (base << 1)) {
            stream.out_sint8(pen.width);
         }
        if (this->fields & (base << 2)) {
            emit_rdp_color(stream, pen.color);
        }
    }

    void receive_pen(InStream & stream, uint32_t base, RDPPen & pen) const
    {
        using namespace RDP;

        if (this->fields & base) {
            pen.style = stream.in_uint8();
        }

        if (this->fields & (base << 1)) {
            pen.width = stream.in_uint8();
        }

        if (this->fields & (base << 2)) {
            receive_rdp_color(stream, pen.color);
        }
    }

    void emit_brush(OutStream & stream, uint32_t base,
                  const RDPBrush & brush,
                  const RDPBrush & /*old_brush*/) const {

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

    void receive_brush(InStream & stream, uint32_t base, RDPBrush & brush) const
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

class RDPOrderCommon {
    public:

    // for primary orders : kept in state
    uint8_t order;
    Rect clip;

    RDPOrderCommon(int order, const Rect clip) :
        order(order), clip(clip)
    {
    }

    bool operator==(const RDPOrderCommon &other) const {
        return  (this->order == other.order)
             && (this->clip == other.clip);
    }

private:
    void _emit(OutStream & stream, RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        int size = 1;
        switch (this->order)
        {
            case MEM3BLT:
            case GLYPHINDEX:
                size = 3;
                break;

            case PATBLT:
            case MEMBLT:
            case LINE:
            case POLYGONCB:
            case ELLIPSECB:
            case MULTIOPAQUERECT:
            case MULTIPATBLT:
            case MULTISCRBLT:
                size = 2;
                break;
            case RECT:
            case SCREENBLT:
            case DESTBLT:
            case MULTIDSTBLT:
            case POLYLINE:
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
    }

public:
    void emit(OutStream & stream, RDPPrimaryOrderHeader & header, const RDPOrderCommon & oldcommon)
    {
        using namespace RDP;

        Bounds bounds(oldcommon.clip, this->clip);

        header.control |= (this->order != oldcommon.order) * CHANGE;

        if (header.control & BOUNDS){
            header.control |= ((bounds.bounds_flags == 0) * LASTBOUNDS);
        }

        this->_emit(stream, header);

        if (header.control & BOUNDS){
            if (!(header.control & LASTBOUNDS)){
                bounds.emit(stream);
            }
        }
        else {
            this->clip = oldcommon.clip;
        }
    }

    const  RDPPrimaryOrderHeader receive(InStream & stream, uint8_t control)
    {
        using namespace RDP;

        RDPPrimaryOrderHeader header(control, 0);

//        LOG(LOG_INFO, "reading control (%p): %.2x %s%s%s%s%s%s%s%s", stream.p,
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
        switch (this->order) {
        case MEM3BLT:
        case GLYPHINDEX:
            size = 3;
            break;
        case PATBLT:
        case MEMBLT:
        case LINE:
        case ELLIPSECB:
        case POLYGONCB:
        case MULTIOPAQUERECT:
        case MULTIPATBLT:
        case MULTISCRBLT:
            size = 2;
            break;
        case RECT:
        case SCREENBLT:
        case DESTBLT:
        case POLYGONSC:
        case POLYLINE:
        case ELLIPSESC:
        case MULTIDSTBLT:
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

//        LOG(LOG_INFO, "control=%.2x order=%d  size=%d fields=%.6x assert=%d",
//            header.control, this->order, size, header.fields, (0 == (header.fields & ~0x3FF)));

        switch (this->order){
        case DESTBLT:
            assert(!(header.fields & ~0x1F));
            break;
        case MULTIDSTBLT:
            assert(!(header.fields & ~0x7F));
            break;
        case MULTIOPAQUERECT:
            assert(!(header.fields & ~0x1FF));
            break;
        case MULTIPATBLT:
            assert(!(header.fields & ~0x3FFF));
            break;
        case MULTISCRBLT:
            assert(!(header.fields & ~0x1FF));
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
        case MEM3BLT:
            assert(!(header.fields & ~0xFFFF));
            break;
        case GLYPHINDEX:
            assert(!(header.fields & ~0x3FFFFF));
            break;
        case POLYGONSC:
            assert(!(header.fields & ~0x7F));
            break;
        case POLYGONCB:
            assert(!(header.fields & ~0x1FFF));
            break;
        case POLYLINE:
            assert(!(header.fields & ~0x7F));
            break;
        case ELLIPSESC:
            assert(!(header.fields & ~0x7F));
            break;
        case ELLIPSECB:
            assert(!(header.fields & ~0x1FFF));
            break;
        default:
            LOG(LOG_INFO, "Order is Unknown (%u)\n", this->order);
            assert(false);
        }

        if (header.control & BOUNDS) {
            if (!(header.control & LASTBOUNDS)){
                int bound_fields = stream.in_uint8();
                uint16_t bounds[4] =
                    { static_cast<uint16_t>(this->clip.x)
                    , static_cast<uint16_t>(this->clip.y)
                    , static_cast<uint16_t>(this->clip.x + this->clip.cx - 1)
                    , static_cast<uint16_t>(this->clip.y + this->clip.cy - 1)
                    };

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
                this->clip.cx = bounds[2] - bounds[0] + 1;
                this->clip.cy = bounds[3] - bounds[1] + 1;
            }
        }

        return header;
    }

    // TODO remove flag in common.str to draw clip or not, it is confusing. Better to have 2 functions
    size_t str(char * buffer, size_t sz, bool showclip = true) const
    {
        size_t lg = sz;
        if (showclip){
            lg  = snprintf(buffer, sz, "order(%d clip(%d,%d,%d,%d)):",
                this->order,
                this->clip.x, this->clip.y, this->clip.cx, this->clip.cy);
        }
        else {
            lg  = snprintf(buffer, sz, "order(%d):", this->order);
        }
        return (lg < sz)?lg:sz;
    }
};


class RDPSecondaryOrderHeader {
    // MS-RDPEGDI : 2.2.2.2.1.2.1.1 Secondary Drawing Order Header
    // (SECONDARY_DRAWING_ORDER_HEADER)
    // The SECONDARY_DRAWING_ORDER_HEADER structure is include " in all secondary
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
    uint16_t order_length;
    unsigned flags;
    unsigned type;

    explicit RDPSecondaryOrderHeader(InStream & stream) {
        this->order_length = stream.in_uint16_le();
        this->flags        = stream.in_uint16_le();
        this->type         = stream.in_uint8();
    }

    RDPSecondaryOrderHeader(uint16_t order_length, unsigned flags, unsigned type):
        order_length(order_length), flags(flags), type(type) {
    }

    uint16_t order_data_length() const {
        return   this->order_length
               + 13                     /* Protocol defined adjustment. */
               - 6;                     /* Size of header.              */
    }
};

