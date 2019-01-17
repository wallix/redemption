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

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

// [MS-RDPBCGR] - 2.2.7.1.8 Glyph Cache Capability Set
//  (TS_GLYPHCACHE_CAPABILITYSET)
// ===================================================

// The TS_GLYPHCACHE_CAPABILITYSET structure advertises the glyph support
//  level and associated cache sizes. This capability is only sent from
//  client to server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       capabilitySetType       |        lengthCapability       |
// +-------------------------------+-------------------------------+
// |                           GlyphCache                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 (GlyphCache cont'd for 2 rows)                |
// +---------------------------------------------------------------+
// |                           FragCache                           |
// +-------------------------------+-------------------------------+
// |       GlyphSupportLevel       |           pad2octets          |
// +-------------------------------+-------------------------------+

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//  capability set. This field MUST be set to CAPSTYPE_GLYPHCACHE (16).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in
//  bytes of the capability data, including the size of the
//  capabilitySetType and lengthCapability fields.

// GlyphCache (40 bytes): An array of 10 TS_CACHE_DEFINITION structures. An
//  ordered specification of the layout of each of the glyph caches with IDs
//  0 through to 9 ([MS-RDPEGDI] section 3.1.1.1.2).

enum {
      NUMBER_OF_GLYPH_CACHES = 10
};

// FragCache (4 bytes): Fragment cache data. The maximum number of entries
//  allowed in the cache is 256, and the largest allowed maximum size of an
//  element is 256 bytes.

enum {
      MAXIMUM_NUMBER_OF_FRAGMENT_CACHE_ENTRIES = 256
};

enum {
      MAXIMUM_SIZE_OF_FRAGMENT_CACHE_ENTRIE = 256
};

// GlyphSupportLevel (2 bytes): A 16-bit, unsigned integer. The level of
//  glyph support.

//  +-----------------------+--------------------------------------------------+
//  | Value                 | Meaning                                          |
//  +-----------------------+--------------------------------------------------+
//  | GLYPH_SUPPORT_NONE    | The client does not support glyph caching. All   |
//  | 0x0000                | text output will be sent to the client as        |
//  |                       | expensive Bitmap Updates (see sections           |
//  |                       | 2.2.9.1.1.3.1.2 and 2.2.9.1.2.1.2).              |
//  +-----------------------+--------------------------------------------------+
//  | GLYPH_SUPPORT_PARTIAL | Indicates support for Revision 1 Cache Glyph     |
//  | 0x0001                | Secondary Drawing Orders (see [MS-RDPEGDI]       |
//  |                       | section 2.2.2.2.1.2.5).                          |
//  +-----------------------+--------------------------------------------------+
//  | GLYPH_SUPPORT_FULL    | Indicates support for Revision 1 Cache Glyph     |
//  | 0x0002                | Secondary Drawing Orders (see [MS-RDPEGDI]       |
//  |                       | section 2.2.2.2.1.2.5).                          |
//  +-----------------------+--------------------------------------------------+
//  | GLYPH_SUPPORT_ENCODE  | Indicates support for Revision 2 Cache Glyph     |
//  | 0x0003                | Secondary Drawing Orders (see [MS-RDPEGDI]       |
//  |                       | section 2.2.2.2.1.2.6).                          |
//  +-----------------------+--------------------------------------------------+

//  If the GlyphSupportLevel is greater than GLYPH_SUPPORT_NONE (0), the
//   client MUST support the GlyphIndex Primary Drawing Order (see
//   [MS-RDPEGDI] section 2.2.2.2.1.1.2.13) or the FastIndex Primary Drawing
//   Order (see [MS-RDPEGDI] section 2.2.2.2.1.1.2.14). If the FastIndex
//   Primary Drawing Order is not supported, then support for the GlyphIndex
//   Primary Drawing Order is assumed by the server (order support is
//   specified in the Order Capability Set, as described in section
//   2.2.7.1.3).

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//  field MUST be ignored.

// [MS-RDPBCGR] - 2.2.7.1.8.1 Cache Definition (TS_CACHE_DEFINITION)
// =================================================================

// The TS_CACHE_DEFINITION structure specifies details about a particular
//  cache in the Glyph Capability Set (section 2.2.7.1.8) structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          CacheEntries         |      CacheMaximumCellSize     |
// +-------------------------------+-------------------------------+

// CacheEntries (2 bytes): A 16-bit, unsigned integer. The number of entries
//  in the cache. The maximum number of entries allowed in a cache is 254,
//  and the largest allowed maximum size of an element is 2048 bytes.

enum {
      NUMBER_OF_GLYPH_CACHE_ENTRIES = 254
};

// CacheMaximumCellSize (2 bytes): A 16-bit, unsigned integer. The maximum
//  size in bytes of an entry in the cache.

struct GlyphCacheCaps : public Capability {
    enum {
        LENGTH_CAPABILITY = 52  // capabilitySetType(2) + lengthCapability(2) +
                                //     GlyphCache(40) + FragCache(4) +
                                //     GlyphSupportLevel(2) + pad2octets(2)
    };

    static constexpr uint8_t NUMBER_OF_CACHE = 10;

    struct CacheDefinition {
        uint16_t CacheEntries;
        uint16_t CacheMaximumCellSize;
    } GlyphCache[NUMBER_OF_CACHE] = {
        { 254,    4 }, { 254,    4 }, { 254,    8 }, { 254,    8 }, { 254,   16 },
        { 254,   32 }, { 254,   64 }, { 254,  128 }, { 254,  256 }, {  64, 2048 }
    };

    uint32_t FragCache =
        ( (256 << 16)   // Number of entries allowed in the cache (maximum is 256).
        | 256           // Maximum size of an element (largest allowed is 256 bytes.)
        );

    enum {
          GLYPH_SUPPORT_NONE    = 0x0000
        , GLYPH_SUPPORT_PARTIAL = 0x0001
        , GLYPH_SUPPORT_FULL    = 0x0002
        , GLYPH_SUPPORT_ENCODE  = 0x0003
    };

    uint16_t GlyphSupportLevel = GLYPH_SUPPORT_NONE;
    uint16_t pad2octets        = 0;

    GlyphCacheCaps()
    : Capability(CAPSTYPE_GLYPHCACHE, LENGTH_CAPABILITY) {}

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        for (auto const& glyph : this->GlyphCache) {
            stream.out_uint16_le(glyph.CacheEntries);
            stream.out_uint16_le(glyph.CacheMaximumCellSize);
        }
        stream.out_uint32_le(this->FragCache);
        stream.out_uint16_le(this->GlyphSupportLevel);
        stream.out_uint16_le(this->pad2octets);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len               = len;
        for (auto & glyph : this->GlyphCache) {
            glyph.CacheEntries         = stream.in_uint16_le();
            glyph.CacheMaximumCellSize = stream.in_uint16_le();
        }
        this->FragCache         = stream.in_uint32_le();
        this->GlyphSupportLevel = stream.in_uint16_le();
        this->pad2octets        = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s GlyphCache caps (%u bytes)", msg, this->len);
        for (uint8_t i = 0; i < NUMBER_OF_CACHE; ++i) {
            LOG( LOG_INFO, "     GlyphCache caps::GlyphCache[%u].CacheEntries=%u"
               , i, this->GlyphCache[i].CacheEntries);
            LOG( LOG_INFO, "     GlyphCache caps::GlyphCache[%u].CacheMaximumCellSize=%u"
               , i, this->GlyphCache[i].CacheMaximumCellSize);
        }
        LOG(LOG_INFO, "     GlyphCache caps::FragCache %u", this->FragCache);
        LOG(LOG_INFO, "     GlyphCache caps::GlyphSupportLevel %u", this->GlyphSupportLevel);
        LOG(LOG_INFO, "     GlyphCache caps::pad2octets %u", this->pad2octets);
    }
};  // struct GlyphCacheCaps
