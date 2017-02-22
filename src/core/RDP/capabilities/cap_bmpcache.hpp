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

   RDP Capabilities : BMPCACHE

*/


#pragma once

#include <cinttypes>

#include "common.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.1.4 Bitmap Cache Capability Set

// 2.2.7.1.4.1 Revision 1 (TS_BITMAPCACHE_CAPABILITYSET)

// The TS_BITMAPCACHE_CAPABILITYSET structure is used to advertise support for Revision 1 bitmap
// caches (see [MS-RDPEGDI] section 3.1.1.1.1). This capability is only sent from client to server.

// In addition to specifying bitmap caching parameters in the Revision 1 Bitmap Cache Capability Set,
// a client MUST also support the MemBlt and Mem3Blt Primary Drawing Orders (see [MS-RDPEGDI]
// sections 2.2.2.2.1.1.2.9 and 2.2.2.2.1.1.2.10, respectively) in order to receive the Cache Bitmap
// (Revision 1) Secondary Drawing Order (see [MS-RDPEGDI] section 2.2.2.2.1.2.2).

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//   field MUST be set to CAPSTYPE_BITMAPCACHE (4).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//   data, including the size of the capabilitySetType and lengthCapability fields.

// pad1 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.
// pad2 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.
// pad3 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.
// pad4 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.
// pad5 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.
// pad6 (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// Cache0Entries (2 bytes): A 16-bit, unsigned integer. The number of entries in Bitmap Cache 0
//   (maximum allowed value is 200 entries).

// Cache0MaximumCellSize (2 bytes): A 16-bit, unsigned integer. The maximum cell size in
//   Bitmap Cache 0.

// Cache1Entries (2 bytes): A 16-bit, unsigned integer. The number of entries in Bitmap Cache 1
//   maximum allowed value is 600 entries).

// Cache1MaximumCellSize (2 bytes): A 16-bit, unsigned integer. The maximum cell size in
//   Bitmap Cache 1.

// Cache2Entries (2 bytes): A 16-bit, unsigned integer. The number of entries in Bitmap Cache 2
//   (maximum allowed value is 65535 entries).

// Cache2MaximumCellSize (2 bytes): A 16-bit, unsigned integer. The maximum cell size in
//   Bitmap Cache 2.

enum {
    CAPLEN_BITMAPCACHE = 40
};

struct BmpCacheCaps : public Capability {
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
    uint32_t pad4;
    uint32_t pad5;
    uint32_t pad6;
    uint16_t cache0Entries;
    uint16_t cache0MaximumCellSize;
    uint16_t cache1Entries;
    uint16_t cache1MaximumCellSize;
    uint16_t cache2Entries;
    uint16_t cache2MaximumCellSize;
    BmpCacheCaps()
    : Capability(CAPSTYPE_BITMAPCACHE, CAPLEN_BITMAPCACHE)
    , pad1(0)
    , pad2(0)
    , pad3(0)
    , pad4(0)
    , pad5(0)
    , pad6(0)
    , cache0Entries(0xC8)
    , cache0MaximumCellSize(0)
    , cache1Entries(0x258)
    , cache1MaximumCellSize(0)
    , cache2Entries(0x3E8)
    , cache2MaximumCellSize(0)
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->pad1);
        stream.out_uint32_le(this->pad2);
        stream.out_uint32_le(this->pad3);
        stream.out_uint32_le(this->pad4);
        stream.out_uint32_le(this->pad5);
        stream.out_uint32_le(this->pad6);
        stream.out_uint16_le(this->cache0Entries);
        stream.out_uint16_le(this->cache0MaximumCellSize);
        stream.out_uint16_le(this->cache1Entries);
        stream.out_uint16_le(this->cache1MaximumCellSize);
        stream.out_uint16_le(this->cache2Entries);
        stream.out_uint16_le(this->cache2MaximumCellSize);
    }

    void recv(InStream & stream, uint16_t len) override {
        this->len = len;

        /* pad1(4) + pad2(4) + pad3(4) + pad4(4) + pad5(4) + pad6(4) + cache0Entries(2) + cache0MaximumCellSize(2) +
         * cache1Entries(2) + cache1MaximumCellSize(2) + cache2Entries(2) + cache2MaximumCellSize(2)
         */
        const unsigned expected = 36;
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated BmpCacheCaps, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->pad1 = stream.in_uint32_le();
        this->pad2 = stream.in_uint32_le();
        this->pad3 = stream.in_uint32_le();
        this->pad4 = stream.in_uint32_le();
        this->pad5 = stream.in_uint32_le();
        this->pad6 = stream.in_uint32_le();
        this->cache0Entries = stream.in_uint16_le();
        this->cache0MaximumCellSize = stream.in_uint16_le();
        this->cache1Entries = stream.in_uint16_le();
        this->cache1MaximumCellSize = stream.in_uint16_le();
        this->cache2Entries = stream.in_uint16_le();
        this->cache2MaximumCellSize = stream.in_uint16_le();
      }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s BitmapCache caps (%" PRIu16 " bytes)", msg, this->len);
        LOG(LOG_INFO, "BitmapCache caps::pad1 %" PRIu32, this->pad1);
        LOG(LOG_INFO, "BitmapCache caps::pad2 %" PRIu32, this->pad2);
        LOG(LOG_INFO, "BitmapCache caps::pad3 %" PRIu32, this->pad3);
        LOG(LOG_INFO, "BitmapCache caps::pad4 %" PRIu32, this->pad4);
        LOG(LOG_INFO, "BitmapCache caps::pad5 %" PRIu32, this->pad5);
        LOG(LOG_INFO, "BitmapCache caps::pad6 %" PRIu32, this->pad6);
        LOG(LOG_INFO, "BitmapCache caps::cache0Entries %" PRIu16, this->cache0Entries);
        LOG(LOG_INFO, "BitmapCache caps::cache0MaximumCellSize %" PRIu16, this->cache0MaximumCellSize);
        LOG(LOG_INFO, "BitmapCache caps::cache1Entries %" PRIu16, this->cache1Entries);
        LOG(LOG_INFO, "BitmapCache caps::cache1MaximumCellSize %" PRIu16, this->cache1MaximumCellSize);
        LOG(LOG_INFO, "BitmapCache caps::cache2Entries %" PRIu16, this->cache2Entries);
        LOG(LOG_INFO, "BitmapCache caps::cache2MaximumCellSize %" PRIu16, this->cache2MaximumCellSize);
    }
};

