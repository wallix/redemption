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

   RDP Capabilities :

*/

#if !defined(__RDP_CAPABILITIES_GLYPHCACHE_HPP__)
#define __RDP_CAPABILITIES_GLYPHCACHE_HPP__


// 2.2.7.1.8 Glyph Cache Capability Set (TS_GLYPHCACHE_CAPABILITYSET)
// ==================================================================

// The TS_GLYPHCACHE_CAPABILITYSET structure advertises the glyph support level
// and associated cache sizes. This capability is only sent from client to
// server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
// capability set. This field MUST be set to CAPSTYPE_GLYPHCACHE (16).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
// of the capability data, including the size of the capabilitySetType and
// lengthCapability fields.

// GlyphCache (40 bytes): An array of 10 TS_CACHE_DEFINITION structures. An
// ordered specification of the layout of each of the glyph caches with IDs 0
// through to 9 ([MS-RDPEGDI] section 3.1.1.1.2).

// FragCache (4 bytes): Fragment cache data. The maximum number of entries
// allowed in the cache is 256, and the largest allowed maximum size of an
// element is 256 bytes.

// GlyphSupportLevel (2 bytes): A 16-bit, unsigned integer. The level of glyph
// support.

// +-------------------------------+-------------------------------------------+
// |         Value                 |                    Meaning                |
// +-------------------------------+-------------------------------------------+
// | 0x0000 GLYPH_SUPPORT_NONE     | The client does not support glyph caching.|
// |                               | All text output will be sent to the client|
// |                               | as expensive Bitmap Updates (see sections |
// |                               | 2.2.9.1.1.3.1.2 and 2.2.9.1.2.1.2).       |
// +-------------------------------+-------------------------------------------+
// | 0x0001 GLYPH_SUPPORT_PARTIAL  | Indicates support for Revision 1 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.5).      |
// +-------------------------------+-------------------------------------------+
// | 0x0002 GLYPH_SUPPORT_FULL     | Indicates support for Revision 1 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.5).      |
// +-------------------------------+-------------------------------------------+
// | 0x0003 GLYPH_SUPPORT_ENCODE   | Indicates support for Revision 2 Cache    |
// |                               | Glyph Secondary Drawing Orders (see       |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.2.6).      |
// +-------------------------------+-------------------------------------------+

//If the GlyphSupportLevel is greater than GLYPH_SUPPORT_NONE (0), the client
//  MUST support the GlyphIndex Primary Drawing Order (see [MS-RDPEGDI] section
//  2.2.2.2.1.1.2.13) or the FastIndex Primary Drawing Order (see [MS-RDPEGDI]
//  section 2.2.2.2.1.1.2.14). If the FastIndex Primary Drawing Order is not
//  supported, then support for the GlyphIndex Primary Drawing Order is assumed
//  by the server (order support is specified in the Order Capability Set, as
//  described in section 2.2.7.1.3).

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field MUST be ignored.

static inline void out_glyphcache_caps(Stream & stream)
{
    stream.out_uint16_le(RDP_CAPSET_GLYPHCACHE);
    uint16_t offset_length = stream.get_offset(0);
    stream.out_uint16_le(0);
    static const char glyphcache[] = {
    0xFE, 0x00, 0x04, 0x00, 0xFE, 0x00, 0x04, 0x00,
    0xFE, 0x00, 0x08, 0x00, 0xFE, 0x00, 0x08, 0x00,
    0xFE, 0x00, 0x10, 0x00, 0xFE, 0x00, 0x20, 0x00,
    0xFE, 0x00, 0x40, 0x00, 0xFE, 0x00, 0x80, 0x00,
    0xFE, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x08};
    stream.out_copy_bytes(glyphcache, 40);
    stream.out_uint32_le(0x01000100);
    stream.out_uint16_le(0x0000);
    stream.out_uint16_le(0);
    stream.set_out_uint16_le(stream.get_offset(offset_length+2), offset_length);

}

#endif
