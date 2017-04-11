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

//  2.2.7.2.5 Font Capability Set (TS_FONT_CAPABILITYSET)

//  The TS_FONT_CAPABILITYSET structure is used to advertise font support options. This capability is
//  sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_FONT (14).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//     data, including the size of the capabilitySetType and lengthCapability fields.

// fontSupportFlags (2 bytes): A 16-bit, unsigned integer. The font support options. This field
//     SHOULD be set to FONTSUPPORT_FONTLIST (0x0001).

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be
//    ignored.

enum {
    FONTSUPPORT_FONTLIST = 0x1
};

enum {
    CAPLEN_FONT = 8
};

struct FontCaps : public Capability {
    uint16_t fontSupportFlags;
    uint16_t pad2octets;
    FontCaps()
    : Capability(CAPSTYPE_FONT, CAPLEN_FONT)
    , fontSupportFlags(FONTSUPPORT_FONTLIST)
    , pad2octets(0)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->fontSupportFlags);
        stream.out_uint16_le(this->pad2octets);

    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->fontSupportFlags = stream.in_uint16_le();
        this->pad2octets = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s Font caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "Font caps::fontSupportFlags %u", this->fontSupportFlags);
        LOG(LOG_INFO, "Font caps::pad2octets %u", this->pad2octets);
    }
};
