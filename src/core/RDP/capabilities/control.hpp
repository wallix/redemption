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

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.2     Control Capability Set (TS_CONTROL_CAPABILITYSET)

//  The TS_CONTROL_CAPABILITYSET structure is used by the client to advertise control capabilities
//  and is fully described in [T128] section 8.2.10. This capability is only sent from client to server and
//  the server ignores its contents.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_CONTROL (5).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.

// controlFlags (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to 0.

// remoteDetachFlag (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to FALSE
//    (0x0000).

// controlInterest (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to
//    CONTROLPRIORITY_NEVER (0x0002).

// detachInterest (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to
//    CONTROLPRIORITY_NEVER (0x0002).


enum {
    CONTROLPRIORITY_NEVER = 0x02
};

enum {
    CAPLEN_CONTROL = 12
};

struct ControlCaps : public Capability {
    uint16_t controlFlags{0};
    uint16_t remoteDetachFlag{0};
    uint16_t controlInterest{CONTROLPRIORITY_NEVER};
    uint16_t detachInterest{CONTROLPRIORITY_NEVER};
    ControlCaps()
    : Capability(CAPSTYPE_CONTROL, CAPLEN_CONTROL)
     
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);

        stream.out_uint16_le(this->controlFlags);
        stream.out_uint16_le(this->remoteDetachFlag);
        stream.out_uint16_le(this->controlInterest);
        stream.out_uint16_le(this->detachInterest);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->controlFlags = stream.in_uint16_le();
        this->remoteDetachFlag = stream.in_uint16_le();
        this->controlInterest = stream.in_uint16_le();
        this->detachInterest = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s ControlCaps caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     ControlCaps caps::controlFlags %u", this->controlFlags);
        LOG(LOG_INFO, "     ControlCaps caps::remoteDetachFlag %u", this->remoteDetachFlag);
        LOG(LOG_INFO, "     ControlCaps caps::controlInterest %u", this->controlInterest);
        LOG(LOG_INFO, "     ControlCaps caps::detachInterest %u", this->detachInterest);
    }
};
