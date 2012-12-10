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

#if !defined(__RDPORDERSSECONDARYGLYPHCACHE_HPP__)
#define __RDPORDERSSECONDARYGLYPHCACHE_HPP__



// MS-RDPEGDI 2.2.2.2.1.2.5     Cache Glyph - Revision 1 (CACHE_GLYPH_ORDER)
// =========================================================================
//  The Cache Glyph - Revision 1 Secondary Drawing Order is used by the server
//  to instruct the client to store a glyph in a particular Glyph Cache entry.
//  Support for glyph caching is negotiated in the Glyph Cache Capability Set
//  (see [MS-RDPBCGR] section 2.2.7.1.8).

//  header (6 bytes): A Secondary Order Header, as defined in section
//  2.2.2.2.1.2.1.1. The embedded orderType field MUST be set to TS_CACHE_GLYPH
// (0x03). The embedded extraFlags field MAY contain the following flag.

// +----------------------------------+----------------------------------------+
// | 0x00100 CG_GLYPH_UNICODE_PRESENT | Indicates that the unicodeCharacters   |
// |                                  | field is present.                      |
// +----------------------------------+----------------------------------------+

// cacheId (1 byte): An 8-bit, unsigned integer. The glyph cache into which to
//   store the glyph data. This value MUST be in the range negotiated by the
//   Glyph Cache Capability Set (see [MS-RDPBCGR] section 2.2.7.1.8).

// cGlyphs (1 byte): An 8-bit, unsigned integer. The number of glyph entries in
//   the glyphData field.

// glyphData (variable): The specification for each of the glyphs in this order
//   (the number of glyphs is specified by the cGlyphs field) defined using
//   Cache Glyph Data structures.

//    MS-RDPEGDI 2.2.2.2.1.2.5.1 Cache Glyph Data (TS_CACHE_GLYPH_DATA)
//    -----------------------------------------------------------------
//    The TS_CACHE_GLYPH_DATA structure contains information describing a single
//    glyph.

// glyphData::cacheIndex (2 bytes): A 16-bit, unsigned integer. The index within
//   a specified Glyph Cache where the glyph data MUST be stored. This value
//   MUST be in the range negotiated by the Glyph Cache Capability Set (see
//   [MS-RDPBCGR] section 2.2.7.1.8).

// glyphData::x (2 bytes): A 16-bit, signed integer. The X component of the
//   coordinate that defines the origin of the character within the glyph
//   bitmap. The top-left corner of the bitmap is (0, 0).

// glyphData::y (2 bytes): A 16-bit, signed integer. The Y component of the
//   coordinate that defines the origin of the character within the glyph
//   bitmap. The top-left corner of the bitmap is (0, 0).

// glyphData::cx (2 bytes): A 16-bit, unsigned integer. The width of the glyph
//   bitmap in pixels.

// glyphData::cy (2 bytes): A 16-bit, unsigned integer. The height of the glyph
//   bitmap in pixels.

// glyphData::aj (variable): A variable-sized byte array containing a
//   1-bit-per-pixel bitmap of the glyph. The individual scan lines are encoded
//   in top-down order, and each scan line MUST be byte-aligned.
//   Once the array has been populated with bitmap data, it MUST be padded to a
//   double-word boundary (the size of the structure in bytes MUST be a multiple
//   of 4). For examples of 1-bit-per-pixel encoded glyph bitmaps, see sections
//   4.6.1 and 4.6.2.

// unicodeCharacters (variable): Contains the Unicode character representation
//   of each glyph in the glyphData field. The number of bytes in the field is
//   given by cGlyphs * 2. This string is used for diagnostic purposes only and
//   is not necessary for successfully decoding and caching the glyphs in the
//   glyphData field.


class RDPGlyphCache {

    public:
    uint8_t cacheId;
    uint8_t cGlyphs;
    uint16_t glyphData_cacheIndex;
    uint16_t glyphData_x;
    uint16_t glyphData_y;
    uint16_t glyphData_cx;
    uint16_t glyphData_cy;
    size_t size;
    uint8_t * glyphData_aj;

    RDPGlyphCache() : cGlyphs(1), glyphData_aj(NULL)
    {

    }

    RDPGlyphCache( uint8_t  cacheId
                 , uint8_t  cGlyphs
                 , uint16_t glyphData_cacheIndex
                 , uint16_t glyphData_x
                 , uint16_t glyphData_y
                 , uint16_t glyphData_cx
                 , uint16_t glyphData_cy
                 , const uint8_t * glyphData_aj
                 )
        : cacheId(cacheId)
        , cGlyphs(1)
        , glyphData_cacheIndex(glyphData_cacheIndex)
        , glyphData_x(glyphData_x)
        , glyphData_y(glyphData_y)
        , glyphData_cx(glyphData_cx)
        , glyphData_cy(glyphData_cy)
    {
        size_t size = align4(nbbytes(glyphData_cx) * glyphData_cy);
        this->glyphData_aj = (uint8_t*)malloc(size);
        memcpy(this->glyphData_aj, glyphData_aj, size);
    }


    ~RDPGlyphCache()
    {
        if (this->glyphData_aj){
            free(this->glyphData_aj);
        }
    }

    void emit(Stream & stream) const
    {
        using namespace RDP;
        size_t size = align4(nbbytes(this->glyphData_cx) * this->glyphData_cy);

        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        uint16_t len = (size + 12) - 7;    // length after type minus 7
        stream.out_uint16_le(len);
        stream.out_uint16_le(8);    // flags
        stream.out_uint8(TS_CACHE_GLYPH); // type

        stream.out_uint8(cacheId);
        stream.out_uint8(this->cGlyphs);
        stream.out_uint16_le(this->glyphData_cacheIndex);
        stream.out_uint16_le(this->glyphData_x);
        stream.out_uint16_le(this->glyphData_y);
        stream.out_uint16_le(this->glyphData_cx);
        stream.out_uint16_le(this->glyphData_cy);
        stream.out_copy_bytes(this->glyphData_aj, size);
    }

    void receive(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        // using namespace RDP;

        // uint8_t cacheIndex = stream.in_uint8();
        // this->bpp = stream.in_uint8();
        // this->width = stream.in_uint8();
        // this->height = stream.in_uint8();
        // this->type = stream.in_uint8();
        // this->size = stream.in_uint8();
        // this->data = (uint8_t *)malloc(this->size);
        // memcpy(this->data, stream.in_uint8p(this->size), this->size);
    }

    bool operator==(const RDPColCache & other) const {
        return true;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(buffer, sz, "RDPBrushCache()\n");
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level) const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        LOG(level, buffer);
    }

    void print() const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        printf("%s", buffer);
    }


};


#endif
