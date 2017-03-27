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

   RDP Capabilities : Sond

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.7.1.11   Sound Capability Set (TS_SOUND_CAPABILITYSET)

// The TS_SOUND_CAPABILITYSET structure advertises the ability to play a "beep" sound. This
// capability is sent only from client to server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_SOUND (12).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.

// soundFlags (2 bytes): A 16-bit, unsigned integer. Support for sound options.
//   0x0001 SOUND_BEEPS_FLAG    Playing a beep sound is supported.
//   If the client advertises support for beeps, it MUST support the Play Sound PDU (section
//   2.2.9.1.1.5).

// pad2octetsA (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be
//    ignored.


enum {
    SOUND_BEEPS_FLAG = 0x01
};

enum {
    CAPLEN_SOUND = 8
};

struct SoundCaps : public Capability {
    uint16_t soundFlags;
    uint16_t pad2octetsA;
    SoundCaps()
    : Capability(CAPSTYPE_SOUND, CAPLEN_SOUND)
    , soundFlags(SOUND_BEEPS_FLAG)  // True by default
    , pad2octetsA(0)                // MUST be ignored
    {
    }

    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->soundFlags);
        stream.out_uint16_le(this->pad2octetsA);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->soundFlags = stream.in_uint16_le();
        this->pad2octetsA = stream.in_uint16_le();
    }

    void log(const char * msg)
    {
        LOG(LOG_INFO, "%s SoundCaps caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "SoundCaps caps::soundFlags %u", this->soundFlags);
        LOG(LOG_INFO, "SoundCaps caps::pad2octetsA %u", this->pad2octetsA);
    }
};
