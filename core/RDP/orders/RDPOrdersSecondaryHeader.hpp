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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Secondary Orders Header

*/

#if !defined(__RDPORDERSSECONDARYHEADER_HPP__)
#define __RDPORDERSSECONDARYHEADER_HPP__

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



#endif
