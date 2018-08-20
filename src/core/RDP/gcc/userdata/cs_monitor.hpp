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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/rect.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {


// 2.2.1.3.6 Client Monitor Data (TS_UD_CS_MONITOR)
// ------------------------------------------------
// The TS_UD_CS_MONITOR packet describes the client-side display monitor
// layout. This packet is an Extended Client Data Block and MUST NOT be sent
// to a server which does not advertise support for Extended Client Data
// Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as
// described in section 2.2.1.2.1.

// header (4 bytes): GCC user data block header, as specified in User Data
//                   Header (section 2.2.1.3.1). The User Data Header type
//                   field MUST be set to CS_MONITOR (0xC005).

// flags (4 bytes): A 32-bit, unsigned integer. This field is unused and
//                  reserved for future use.

// monitorCount (4 bytes): A 32-bit, unsigned integer. The number of display
//                         monitor definitions in the monitorDefArray field
//                        (the maximum allowed is 16).

// monitorDefArray (variable): A variable-length array containing a series
//                             of TS_MONITOR_DEF structures (section
//                             2.2.1.3.6.1) which describe the display
//                             monitor layout of the client. The number of
//                             TS_MONITOR_DEF structures is given by the
//                             monitorCount field.


// 2.2.1.3.6.1 Monitor Definition (TS_MONITOR_DEF)
// -----------------------------------------------
// The TS_MONITOR_DEF packet describes the configuration of a client-side
// display monitor. The x and y coordinates used to describe the monitor
// position MUST be relative to the upper-left corner of the monitor
// designated as the "primary display monitor" (the upper-left corner of the
// primary monitor is always (0, 0)).

// left (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate of
//                 the upper-left corner of the display monitor.

// top (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate of
//                the upper-left corner of the display monitor.

// right (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate
//                  of the lower-right corner of the display monitor.

// bottom (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate
//                   of the lower-right corner of the display monitor.

// flags (4 bytes): A 32-bit, unsigned integer. Monitor configuration flags.

//        Value                          Meaning
// -------------------------------------------------------------------------
// TS_MONITOR_PRIMARY            The top, left, right and bottom fields
//      0x00000001               describe the position of the primary
//                               monitor.
// -------------------------------------------------------------------------

struct CSMonitor {
    uint16_t userDataType{CS_MONITOR};
    uint16_t length{0};

    uint32_t flags{0};
    uint32_t monitorCount{0};

    enum {
        MAX_MONITOR_COUNT = 16
    };
    enum {
        TS_MONITOR_PRIMARY = 0x00000001
    };
    struct {
        int32_t  left;
        int32_t  top;
        int32_t  right;
        int32_t  bottom;
        uint32_t flags;
    } monitorDefArray[MAX_MONITOR_COUNT] {};

    bool permissive{false};

    CSMonitor() = default;

    void emit(OutStream & stream) /* TODO const*/ {
        assert((this->monitorCount > 0) && (this->monitorCount <= MAX_MONITOR_COUNT));

        stream.out_uint16_le(this->userDataType);
        this->length = 4 + 4 + 4 + this->monitorCount * 20; // header(4) + flags(4) + monitorCount(4) + monitorCount * monitorDefArray(20)
        stream.out_uint16_le(this->length);

        stream.out_uint32_le(this->flags);
        stream.out_uint32_le(this->monitorCount);

        for (uint32_t i = 0; i < this->monitorCount; i++) {
            stream.out_sint32_le(this->monitorDefArray[i].left);
            stream.out_sint32_le(this->monitorDefArray[i].top);
            stream.out_sint32_le(this->monitorDefArray[i].right);
            stream.out_sint32_le(this->monitorDefArray[i].bottom);
            stream.out_uint32_le(this->monitorDefArray[i].flags);
        }
    }

    void recv(InStream & stream) {
        if (!stream.in_check_rem(4)) {
            LOG(LOG_ERR, "CSMonitor::recv short header, need=4 remains=%zu",
                stream.in_remain());
            throw Error(ERR_GCC);
        }

        this->userDataType = stream.in_uint16_le();
        this->length       = stream.in_uint16_le();

        if (!stream.in_check_rem(8)) {
            LOG(LOG_ERR, "GCC User Data CS_MONITOR truncated, need=8 remains=%zu",
                stream.in_remain());
            throw Error(ERR_GCC);
        }

        this->flags        = stream.in_uint32_le();
        this->monitorCount = stream.in_uint32_le();

        if ((this->monitorCount < 1) || (this->monitorCount > MAX_MONITOR_COUNT)) {
            LOG(LOG_ERR, "CSMonitor::recv monitor count out of range (%u)", this->monitorCount);
            this->monitorCount = 0;
            if (this->permissive) {
                stream.in_skip_bytes(this->length - 12 /*header(4) + flags(4) + monitorCount(4)*/);
                return;
            }
            throw Error(ERR_GCC);
        }

        unsigned expected = 4 + 4 + 4 + this->monitorCount * 20;    // header(4) + flags(4) + monitorCount(4) + monitorCount * monitorDefArray(20)

        if ((this->length != expected) && (!this->permissive || (this->length < expected))) {
            LOG(LOG_ERR, "CSMonitor::recv bad header length, expecting=%u got=%u",
                expected, this->length);
            throw Error(ERR_GCC);
        }

        expected = this->monitorCount * 20;    // monitorCount * monitorDefArray(20)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "GCC User Data CS_MONITOR truncated, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_GCC);
        }

        for (uint32_t i = 0; i < this->monitorCount; i++) {
            this->monitorDefArray[i].left   = stream.in_sint32_le();
            this->monitorDefArray[i].top    = stream.in_sint32_le();
            this->monitorDefArray[i].right  = stream.in_sint32_le();
            this->monitorDefArray[i].bottom = stream.in_sint32_le();
            this->monitorDefArray[i].flags  = stream.in_uint32_le();
        }

        expected = 4 + 4 + 4 + this->monitorCount * 20;    // header(4) + flags(4) + monitorCount(4) + monitorCount * monitorDefArray(20)
        if ((this->length > expected) && this->permissive) {
            stream.in_skip_bytes(this->length - expected);
        }
    }

    void log(const char * msg) const
    {
        char buffer[2048];

        size_t lg = 0;
        // --------------------- Base Fields ---------------------------------------
        lg += snprintf(buffer + lg, sizeof(buffer) - lg, "%s GCC User Data CS_MONITOR (%u bytes) ", msg, unsigned(this->length));
        lg += snprintf(buffer + lg, sizeof(buffer) - lg, "flags=0x%X monitorCount=%u (", this->flags, this->monitorCount);

        for (uint32_t i = 0; i < this->monitorCount; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sizeof(buffer) - lg, " ");
            }
            lg += snprintf(buffer + lg, sizeof(buffer) - lg, "(left=%d, top=%d, right=%d, bottom=%d, primary=%s(0x%X))",
                this->monitorDefArray[i].left, this->monitorDefArray[i].top, this->monitorDefArray[i].right,
                this->monitorDefArray[i].bottom, ((this->monitorDefArray[i].flags & TS_MONITOR_PRIMARY) ? "yes" : "no"),
                this->monitorDefArray[i].flags);
        }
        snprintf(buffer + lg, sizeof(buffer) - lg, ")");

        buffer[sizeof(buffer) - 1] = 0;
        LOG(LOG_INFO, "%s", buffer);
    }

    Rect get_rect() const {
        int32_t left   = 0;
        int32_t top    = 0;
        int32_t right  = 0;
        int32_t bottom = 0;
        for (uint32_t i = 0; i < this->monitorCount; i++) {
            if (left   > this->monitorDefArray[i].left) {
                left   = this->monitorDefArray[i].left;
            }
            if (top    > this->monitorDefArray[i].top) {
                top    = this->monitorDefArray[i].top;
            }
            if (right  < this->monitorDefArray[i].right) {
                right  = this->monitorDefArray[i].right;
            }
            if (bottom < this->monitorDefArray[i].bottom) {
                bottom = this->monitorDefArray[i].bottom;
            }
        }

        return Rect(left, top, right - left, bottom - top);
    }

    Rect get_primary_monitor_rect() const {
        for (uint32_t i = 0; i < this->monitorCount; i++) {
            if (this->monitorDefArray[i].flags & TS_MONITOR_PRIMARY) {
                return Rect(this->monitorDefArray[i].left,
                            this->monitorDefArray[i].top,
                            this->monitorDefArray[i].right -
                                this->monitorDefArray[i].left,
                            this->monitorDefArray[i].bottom -
                                this->monitorDefArray[i].top);
            }
        }

        return Rect(0, 0, 0, 0);
    }
};

} // namespace UserData
} // namespace GCC
