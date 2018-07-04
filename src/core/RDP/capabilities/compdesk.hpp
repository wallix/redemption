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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Desktop Composition Extension Capability Set ([MS-RDPBCGR] section 2.2.7.2.8)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.2.8 Desktop Composition Capability Set (TS_COMPDESK_CAPABILITYSET)
// ========================================================================
// The TS_COMPDESK_CAPABILITYSET structure is used to support desktop composition. This
// capability is sent by both client and server.

// CompDeskSupportLevel (2 bytes): A 16-bit, unsigned integer. The desktop composition
//    support level.
//  +-------------------------+-------------------------------------------------+
//  | COMPDESK_NOT_SUPPORTED  | Desktop composition services are not supported. |
//  | 0x0000                  |                                                 |
//  +-------------------------+-------------------------------------------------+
//  | COMPDESK_SUPPORTED      | Desktop composition services are supported.     |
//  | 0x0001                  |                                                 |
//  +-------------------------+-------------------------------------------------+

enum {
      COMPDESK_NOT_SUPPORTED
    , COMPDESK_SUPPORTED
};

enum {
    CAPLEN_COMPDESK = 8
};


struct CompDeskCaps : public Capability {
    uint16_t CompDeskSupportLevel{COMPDESK_NOT_SUPPORTED};

    CompDeskCaps()
    : Capability(CAPSETTYPE_COMPDESK, CAPLEN_COMPDESK)
    
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->CompDeskSupportLevel);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;

        if (!stream.in_check_rem(2)){
            LOG(LOG_ERR, "Truncated CompDeskCaps, need=2 remains=%zu",
                stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->CompDeskSupportLevel = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s CompDesk caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     CompDeskCaps caps::CompDeskSupportLevel %u", this->CompDeskSupportLevel);
    }
};

