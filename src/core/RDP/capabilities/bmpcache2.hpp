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


#pragma once

#include <cinttypes>

#include "core/RDP/capabilities/common.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.1.4.2 Revision 2 (TS_BITMAPCACHE_CAPABILITYSET_REV2)
// ==========================================================

// The TS_BITMAPCACHE_CAPABILITYSET_REV2 structure is used to advertise support
// for Revision 2 bitmap caches (see [MS-RDPEGDI] section 3.1.1.1.1). This
// capability is only sent from client to server.

// In addition to specifying bitmap caching parameters in the Revision 2 Bitmap
// Cache Capability Set, a client MUST also support the MemBlt and Mem3Blt
// Primary Drawing Orders (see [MS-RDPEGDI] sections 2.2.2.2.1.1.2.9 and
// 2.2.2.2.1.1.2.10, respectively) in order to receive the Cache Bitmap
// (Revision 2) Secondary Drawing Order (see [MS-RDPEGDI] section 2.2.2.2.1.2.3).

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//  capability set. This field MUST be set to CAPSTYPE_BITMAPCACHE_REV2 (19).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//  of the capability data, including the size of the capabilitySetType and
//  lengthCapability fields.

// CacheFlags (2 bytes): A 16-bit, unsigned integer. Properties which apply to
//   all the bitmap caches.

// +--------------------------------------+------------------------------------+
// | 0x0001 PERSISTENT_KEYS_EXPECTED_FLAG | Indicates that the client will send|
// |                                      | a Persistent Key List PDU during   |
// |                                      | the Connection Finalization phase  |
// |                                      | of the RDP Connection Sequence     |
// |                                      | (see section 1.3.1.1 for an        |
// |                                      | overview of the RDP Connection     |
// |                                      | Sequence phases).                  |
// +--------------------------------------+------------------------------------+
// | 0x0002 ALLOW_CACHE_WAITING_LIST_FLAG | Indicates that the client supports |
// |                                      | a cache waiting list. If a waiting |
// |                                      | list is supported, new bitmaps are |
// |                                      | cached on the second hit rather    |
// |                                      | than the first (that is, a bitmap  |
// |                                      | is sent twice before it is cached).|
// +--------------------------------------+------------------------------------+

enum {
      PERSISTENT_KEYS_EXPECTED_FLAG = 0x0001
    , ALLOW_CACHE_WAITING_LIST_FLAG = 0x0002
};

// pad2 (1 byte): An 8-bit, unsigned integer. Padding. Values in this field MUST
//   be ignored.

// NumCellCaches (1 byte): An 8-bit, unsigned integer. Number of bitmap caches
//  (with a maximum allowed value of 5).

// BitmapCache0CellInfo (4 bytes): A TS_BITMAPCACHE_CELL_CACHE_INFO structure.
//  Contains information about the structure of Bitmap Cache 0. The maximum
//  number of entries allowed in this cache is 600. This field is only valid if
//  NumCellCaches is greater than or equal to 1.

// BitmapCache1CellInfo (4 bytes): A TS_BITMAPCACHE_CELL_CACHE_INFO structure.
//  Contains information about the structure of Bitmap Cache 1. The maximum
//  number of entries allowed in this cache is 600. This field is only valid if
//  NumCellCaches is greater than or equal to 2.

// BitmapCache2CellInfo (4 bytes): A TS_BITMAPCACHE_CELL_CACHE_INFO structure.
//  Contains information about the structure of Bitmap Cache 2. The maximum
//  number of entries allowed in this cache is 65536. This field is only valid
//  if NumCellCaches is greater than or equal to 3.

// BitmapCache3CellInfo (4 bytes): A TS_BITMAPCACHE_CELL_CACHE_INFO structure.
//  Contains information about the structure of Bitmap Cache 3. The maximum
//  number of entries allowed in this cache is 4096. This field is only valid
//  if NumCellCaches is greater than or equal to 4.

// BitmapCache4CellInfo (4 bytes): A TS_BITMAPCACHE_CELL_CACHE_INFO structure.
//  Contains information about the structure of Bitmap Cache 4. The maximum
//  number of entries allowed in this cache is 2048. This field is only valid
//  if NumCellCaches is equal to 5.

// 2.2.7.1.4.2.1 Bitmap Cache Cell Info (TS_BITMAPCACHE_CELL_CACHE_INFO)
// =====================================================================

// The TS_BITMAPCACHE_CELL_CACHE_INFO structure contains information about a
// bitmap cache on the client.

// NumEntries (31 bits): A 31-bit, unsigned integer. Indicates the number of
// entries in the cache.

// k (1 bit): A 1-bit flag. Indicates that the bitmap cache is persistent across
//  RDP connections and that the client expects to receive a unique 64-bit
//  bitmap key in the Cache Bitmap (Revision 2) Secondary Drawing Order (see
//  [MS-RDPEGDI] section 2.2.2.2.1.2.3) for every bitmap inserted into this
//  cache. If this bit is set, 64-bit keys MUST be sent by the server.

enum {
    CAPLEN_BITMAPCACHE_REV2 = 40
};

struct BmpCache2Caps : public Capability {
    uint16_t cacheFlags;
    uint8_t pad1;
    uint8_t numCellCaches;
    uint32_t bitmapCache0CellInfo;
    uint32_t bitmapCache1CellInfo;
    uint32_t bitmapCache2CellInfo;
    uint32_t bitmapCache3CellInfo;
    uint32_t bitmapCache4CellInfo;
    BmpCache2Caps()
    : Capability(CAPSTYPE_BITMAPCACHE_REV2, CAPLEN_BITMAPCACHE_REV2)
    , cacheFlags(0)
    , pad1(0)
    , numCellCaches(0)
    , bitmapCache0CellInfo(0)
    , bitmapCache1CellInfo(0)
    , bitmapCache2CellInfo(0)
    , bitmapCache3CellInfo(0)
    , bitmapCache4CellInfo(0)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->cacheFlags);
        stream.out_uint8(this->pad1);
        stream.out_uint8(this->numCellCaches);
        stream.out_uint32_le(this->bitmapCache0CellInfo);
        stream.out_uint32_le(this->bitmapCache1CellInfo);
        stream.out_uint32_le(this->bitmapCache2CellInfo);
        stream.out_uint32_le(this->bitmapCache3CellInfo);
        stream.out_uint32_le(this->bitmapCache4CellInfo);
        stream.out_clear_bytes(12);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        if (len != CAPLEN_BITMAPCACHE_REV2 || !stream.in_check_rem(len)) {
            LOG(LOG_ERR, "Broken CAPSTYPE_BITMAPCACHE_REV2, need=%u (%" PRIu16 ") remains=%zu",
                    CAPLEN_BITMAPCACHE_REV2, len, stream.in_remain());
                    throw Error(ERR_MCS_PDU_TRUNCATED);
        }
        this->cacheFlags    = stream.in_uint16_le();
        this->pad1          = stream.in_uint8();
        this->numCellCaches = stream.in_uint8();
        this->bitmapCache0CellInfo = stream.in_uint32_le();
        this->bitmapCache1CellInfo = stream.in_uint32_le();
        this->bitmapCache2CellInfo = stream.in_uint32_le();
        this->bitmapCache3CellInfo = stream.in_uint32_le();
        this->bitmapCache4CellInfo = stream.in_uint32_le();
        stream.in_skip_bytes(12);
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s BitmapCache2 caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "BitmapCache2 caps::cacheFlags %u", this->cacheFlags);
        LOG(LOG_INFO, "BitmapCache2 caps::pad1 %u", this->pad1);
        LOG(LOG_INFO, "BitmapCache2 caps::numCellCache %u", this->numCellCaches);
        if (this->numCellCaches < 1){ return; }
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache0CellInfo NumEntries=%u persistent=%s",
            (this->bitmapCache0CellInfo & 0x7fffffff),
            (this->bitmapCache0CellInfo & 0x80000000) ? "yes" : "no");
        if (this->numCellCaches < 2){ return; }
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache1CellInfo NumEntries=%u persistent=%s",
            (this->bitmapCache1CellInfo & 0x7fffffff),
            (this->bitmapCache1CellInfo & 0x80000000) ? "yes" : "no");
        if (this->numCellCaches < 3){ return; }
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache2CellInfo NumEntries=%u persistent=%s",
            (this->bitmapCache2CellInfo & 0x7fffffff),
            (this->bitmapCache2CellInfo & 0x80000000) ? "yes" : "no");
        if (this->numCellCaches < 4){ return; }
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache3CellInfo NumEntries=%u persistent=%s",
            (this->bitmapCache3CellInfo & 0x7fffffff),
            (this->bitmapCache3CellInfo & 0x80000000) ? "yes" : "no");
        if (this->numCellCaches < 5){ return; }
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache4CellInfo NumEntries=%u persistent=%s",
            (this->bitmapCache4CellInfo & 0x7fffffff),
            (this->bitmapCache4CellInfo & 0x80000000) ? "yes" : "no");
    }
};
