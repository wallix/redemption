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

#if !defined(__RDP_CAPABILITIES_BMPCACHE2_HPP__)
#define __RDP_CAPABILITIES_BMPCACHE2_HPP__

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
    : Capability(CAPSTYPE_BITMAPCACHE_REV2, RDP_CAPLEN_BITMAPCACHE_REV2)
    , cacheFlags(0)
    , pad1(0)
    , bitmapCache0CellInfo(0)
    , bitmapCache1CellInfo(0)
    , bitmapCache2CellInfo(0)
    , bitmapCache3CellInfo(0)
    , bitmapCache4CellInfo(0)
    {
    }

    void emit(Stream & stream){
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
    }

    void recv(Stream & stream, uint16_t len){
        this->len = len;
        this->cacheFlags    = stream.in_uint16_le();
        this->pad1          = stream.in_uint8();
        this->numCellCaches = stream.in_uint8();
        TODO("Check NumCellCaches to know if we must read CellInfo or not ? Check actual windows servers behaviors")
        this->bitmapCache0CellInfo = stream.in_uint32_le();
        this->bitmapCache1CellInfo = stream.in_uint32_le();
        this->bitmapCache2CellInfo = stream.in_uint32_le();
        this->bitmapCache3CellInfo = stream.in_uint32_le();
        this->bitmapCache4CellInfo = stream.in_uint32_le();
      }

    void log(const char * msg){
        LOG(LOG_INFO, "%s BitmapCache2 caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "BitmapCache2 caps::cacheFlags %u", this->cacheFlags);
        LOG(LOG_INFO, "BitmapCache2 caps::pad1 %u", this->pad1);
        LOG(LOG_INFO, "BitmapCache2 caps::numCellCache %u", this->numCellCaches);
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache0CellInfo %u", this->bitmapCache0CellInfo);
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache1CellInfo %u", this->bitmapCache1CellInfo);
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache2CellInfo %u", this->bitmapCache2CellInfo);
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache3CellInfo %u", this->bitmapCache3CellInfo);
        LOG(LOG_INFO, "BitmapCache2 caps::bitampCache4CellInfo %u", this->bitmapCache4CellInfo);
    }
};



static inline void out_bmpcache2_caps(Stream & stream)
{
    stream.out_uint16_le(CAPSTYPE_BITMAPCACHE_REV2);
    stream.out_uint16_le(RDP_CAPLEN_BITMAPCACHE_REV2);

    /* version */
    stream.out_uint16_le(0); // persistant cache disabled
    stream.out_uint16_be(3);	/* number of caches in this set */

    /* Sending bitmap capabilities version 2 */
    stream.out_uint32_le(2000);
    stream.out_uint32_le(2000);
    stream.out_uint32_le(2000);

    stream.out_clear_bytes(20);	/* other bitmap caches not used */
}

#endif
