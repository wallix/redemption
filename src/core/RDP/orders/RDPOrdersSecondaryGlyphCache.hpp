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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou

    New RDP Orders Coder / Decoder : GlyphCache Secondary Drawing Order
*/


#pragma once

#include <memory>

#include "utils/bitfu.hpp"
#include "utils/stream.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"

using std::size_t; /*NOLINT*/

// [MS-RDPEGDI] - 2.2.2.2.1.2.5 Cache Glyph - Revision 1 (CACHE_GLYPH_ORDER)
// =========================================================================

// The Cache Glyph - Revision 1 Secondary Drawing Order is used by the server
//  to instruct the client to store a glyph in a particular glyph cache entry
//  (section 3.1.1.1.2). Support for glyph caching is specified in the Glyph
//  Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.8).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +-------------------------------+---------------+---------------+
// |              ...              |    cacheId    |    cGlyphs    |
// +-------------------------------+---------------+---------------+
// |                      glyphData (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                  unicodeCharacters (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// header (6 bytes): A Secondary Order Header, as defined in section
//  2.2.2.2.1.2.1.1. The embedded orderType field MUST be set to
//  TS_CACHE_GLYPH (0x03).

//  The embedded extraFlags field MAY contain the following flags.

// +--------------------------+------------------------------------------------+
// | Value                    | Meaning                                        |
// +--------------------------+------------------------------------------------+
// | CG_GLYPH_UNICODE_PRESENT | Indicates that the unicodeCharacters field is  |
// | 0x0010                   | present.                                       |
// +--------------------------+------------------------------------------------+

enum {
      CG_GLYPH_UNICODE_PRESENT = 0x0010
};

// cacheId (1 byte): An 8-bit, unsigned integer. The ID of the glyph cache
//  in which the glyph data MUST be stored. This value MUST be in the range 0
//  to 9 (inclusive).

// cGlyphs (1 byte): An 8-bit, unsigned integer. The number of glyph entries
//  in the glyphData field.

// glyphData (variable): An array of Cache Glyph Data (section
//  2.2.2.2.1.2.5.1) structures that describes each of the glyphs contained
//  in this order (the number of glyphs is specified by the cGlyphs field).

// unicodeCharacters (variable): An array of Unicode characters. Contains the
//  Unicode character representation of each glyph in the glyphData field.
//  The number of bytes in the field is given by cGlyphs * 2. This field MUST
//  NOT be null-terminated. This string is used for diagnostic purposes only
//  and is not necessary for successfully decoding and caching the glyphs in
//  the glyphData field.

// [MS-RDPEGDI] - 2.2.2.2.1.2.5.1 Cache Glyph Data (TS_CACHE_GLYPH_DATA)
// =====================================================================

// The TS_CACHE_GLYPH_DATA structure contains information describing a single
//  glyph that is to be stored in a glyph cache (section 3.1.1.1.2). The ID
//  of this destination glyph cache is specified by the cacheId field of the
//  container Cache Glyph (Revision 1) Order (section 2.2.2.2.1.2.5).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           cacheIndex          |               x               |
// +-------------------------------+-------------------------------+
// |               y               |               cx              |
// +-------------------------------+-------------------------------+
// |               cy              |         aj (variable)         |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cacheIndex (2 bytes): A 16-bit, unsigned integer. The index of the target
//  entry in the destination glyph cache where the glyph data MUST be stored.
//  This value MUST be greater than or equal to 0, and less than the maximum
//  number of entries allowed in the destination glyph cache. The maximum
//  number of entries allowed in each of the ten glyph caches is specified in
//  the GlyphCache field of the Glyph Cache Capability Set ([MS-RDPBCGR]
//  section 2.2.7.1.8).

// x (2 bytes): A 16-bit, signed integer. The X component of the coordinate
//  that defines the origin of the character within the glyph bitmap. The
//  top-left corner of the bitmap is (0, 0).

// y (2 bytes): A 16-bit, signed integer. The Y component of the coordinate
//  that defines the origin of the character within the glyph bitmap. The
//  top-left corner of the bitmap is (0, 0).

// cx (2 bytes): A 16-bit, unsigned integer. The width of the glyph bitmap in
//  pixels.

// cy (2 bytes): A 16-bit, unsigned integer. The height of the glyph bitmap
//  in pixels.

// aj (variable): A variable-sized byte array containing a 1-bit-per-pixel
//  bitmap of the glyph. The individual scan lines are encoded in top-down
//  order, and each scan line MUST be byte-aligned. Once the array has been
//  populated with bitmap data, it MUST be padded to a double-word boundary
//  (the size of the structure in bytes MUST be a multiple of 4). For
//  examples of 1-bit-per-pixel encoded glyph bitmaps, see sections 4.6.1
//  and 4.6.2.

class RDPGlyphCache {
public:
    uint8_t    cacheId;
    uint8_t    cGlyphs = 1;
    uint16_t   cacheIndex;
    int16_t    x;
    int16_t    y;
    uint16_t   cx;
    uint16_t   cy;
    std::unique_ptr<uint8_t[]> aj;

    RDPGlyphCache() = default;

    RDPGlyphCache( uint8_t cacheId
                 //, uint8_t cGlyphs
                 , uint16_t cacheIndex
                 , int16_t x
                 , int16_t y
                 , uint16_t cx
                 , uint16_t cy
                 , std::unique_ptr<uint8_t[]> aj
                 )
    : cacheId(cacheId)
    ,
     cacheIndex(cacheIndex)
    , x(x)
    , y(y)
    , cx(cx)
    , cy(cy)
    , aj(std::move(aj)) {
    }

    inline uint16_t datasize() const {
        return align4(nbbytes(this->cx) * this->cy);
    }

    uint16_t total_order_size() const {
        return 18 + /* controlFlags(1) + orderLength(2) + extraFlags(2) + orderType(1) +
                     *     cacheId(1) + cGlyphs(1) + cacheIndex(2) + x(2) + y(2) + cx(2) +
                     *     cy(2)
                     */
               this->datasize();
    }

    void emit(OutStream & stream) const {
        using namespace RDP;
        uint16_t size = this->datasize();

        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        uint16_t len = (size + 12) - 7;     // length after type minus 7
        stream.out_uint16_le(len);
        stream.out_uint16_le(8);            // flags
        stream.out_uint8(TS_CACHE_GLYPH);   // type

        stream.out_uint8(cacheId);
        stream.out_uint8(this->cGlyphs);
        stream.out_uint16_le(this->cacheIndex);
        stream.out_uint16_le(this->x);
        stream.out_uint16_le(this->y);
        stream.out_uint16_le(this->cx);
        stream.out_uint16_le(this->cy);
        stream.out_copy_bytes(this->aj.get(), size);
    }

    void receive(InStream & stream, const RDPSecondaryOrderHeader & /*unused*/) {
        this->cacheId    = stream.in_uint8();
        this->cGlyphs    = stream.in_uint8();
        this->cacheIndex = stream.in_uint16_le();
        this->x          = stream.in_uint16_le();
        this->y          = stream.in_uint16_le();
        this->cx         = stream.in_uint16_le();
        this->cy         = stream.in_uint16_le();

        uint16_t size = this->datasize();

        this->aj = std::make_unique<uint8_t[]>(size);
        memcpy(this->aj.get(), stream.in_uint8p(size), size);
    }

    size_t str(char * buffer, size_t sz) const {
        size_t lg = snprintf( buffer, sz
                            , "GlyphCache(cacheId=%u cGlyphs=%u cacheIndex=%u x=%d y=%d cx=%u cy=%u data=("
                            , unsigned(this->cacheId), unsigned(this->cGlyphs), unsigned(this->cacheIndex)
                            , this->x, this->y, unsigned(this->cx), unsigned(this->cy));
        uint16_t c = this->datasize();
        for (uint16_t i = 0; i < c; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }
            lg += snprintf(buffer + lg, sz - lg, "%02x", unsigned(this->aj[i]));
        }
        lg += snprintf(buffer + lg, sz - lg, ")(%u))", unsigned(c));
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        LOG(level, "%s", buffer);
    }
};

