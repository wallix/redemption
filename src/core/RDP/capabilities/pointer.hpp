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

#include "common.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.1.5 Pointer Capability Set (TS_POINTER_CAPABILITYSET)

// The TS_POINTER_CAPABILITYSET structure advertises pointer cache sizes and
// flags and is based on the capability set specified in [T128] section 8.2.11.
// This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//    capability set. This field MUST be set to CAPSTYPE_POINTER (8).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//    of the capability data, including the size of the capabilitySetType and
//    lengthCapability fields.

// colorPointerFlag (2 bytes): A 16-bit, unsigned integer. Indicates support for
//    color pointers. Since RDP supports monochrome cursors by using Color Pointer
//    Updates and New Pointer Updates (sections 2.2.9.1.1.4.4 and 2.2.9.1.1.4.5
//    respectively), the value of this field is ignored and is always assumed to be
//    TRUE (at a minimum the Color Pointer Update MUST be supported by an RDP
//    client).
//    +---------------+-----------------------------------------+
//    | 0x0000 FALSE  | Monochrome mouse cursors are supported. |
//    +---------------+-----------------------------------------+
//    | 0x0001 TRUE   | Color mouse cursors are supported.      |
//    +---------------+-----------------------------------------+

// colorPointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of
//    available slots in the 24 bpp color pointer cache used to store data received
//    in the Color Pointer Update (section 2.2.9.1.1.4.4).

// pointerCacheSize (2 bytes): A 16-bit, unsigned integer. The number of
//    available slots in the pointer cache used to store pointer data of arbitrary
//    bit depth received in the New Pointer Update (section 2.2.9.1.1.4.5).

//    If the value contained in this field is zero or the Pointer Capability Set
//    sent from the client does not include "his field, the server will not use
//    the New Pointer Update.

enum {
    CAPLEN_POINTER = 10,
    CAPLEN_POINTER_MONO = 8
};

struct PointerCaps : public Capability {
    uint16_t colorPointerFlag;
    uint16_t colorPointerCacheSize;
    uint16_t pointerCacheSize;
    PointerCaps()
    : Capability(CAPSTYPE_POINTER, CAPLEN_POINTER)
    , colorPointerFlag(1)       // True = color supported ; False = only mono supported
    , colorPointerCacheSize(25) // Number of available slots in color pointer cache
    , pointerCacheSize(25)       // Optionnal ; Number of available slots in new update pointer cache
    {
    }

    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->colorPointerFlag);
        stream.out_uint16_le(this->colorPointerCacheSize);
        if (this->len  < 10) return;
        stream.out_uint16_le(this->pointerCacheSize);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;

        unsigned int expected = 2 + 2 + ((this->len < 10) ? 0 : 2); /* colorPointerFlag(2) + colorPointerCacheSize(2) + pointerCacheSize*/
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated CAPSTYPE_POINTER, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->colorPointerFlag = stream.in_uint16_le();
        this->colorPointerCacheSize = stream.in_uint16_le();
        if (this->len < 10) return;
        this->pointerCacheSize = stream.in_uint16_le();
    }

    void log(const char * msg)
    {
        LOG(LOG_INFO, "%s Pointer caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "Pointer caps::colorPointerFlag %u", this->colorPointerFlag);
        LOG(LOG_INFO, "Pointer caps::colorPointerCacheSize %u", this->colorPointerCacheSize);
        if (this->len  < 10 ) return;
        LOG(LOG_INFO, "Pointer caps::pointerCacheSize %u", this->pointerCacheSize);
    }
};

