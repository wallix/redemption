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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/log.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"

namespace RDP {

// [MS-RDPEGDI] - 2.2.2.2.1.3.7 Frame Marker
// =========================================

// The Frame Marker Alternate Secondary Drawing Order is used by the server to
//  indicate to the client the beginning and end of a logical frame of
//  graphics data. Breaking graphics data up into logical frames indicates to
//  the client which orders SHOULD be rendered as a logical unit, hence
//  helping to prevent screen tearing. Support for frame markers is specified
//  in the Order Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     header    |                     action                    |
// +---------------+-----------------------------------------------+
// |      ...      |
// +---------------+

// header (1 byte): An Alternate Secondary Drawing Order Header, as defined
//  in section 2.2.2.2.1.3.1.1. The embedded orderType field MUST be set to
//  TS_ALTSEC_FRAME_MARKER (0x0D).

// action (4 bytes): A 32-bit, unsigned integer. Indicates the start or end
//  of a logical frame.

//  +----------------+---------------------------------------------------------+
//  | Value          | Meaning                                                 |
//  +----------------+---------------------------------------------------------+
//  | TS_FRAME_START | Start of a logical frame of graphics data. All drawing  |
//  | 0x00000000     | orders from this point in the graphics stream are part  |
//  |                | of the same logical frame and can be rendered as one    |
//  |                | cohesive unit to prevent tearing.                       |
//  +----------------+---------------------------------------------------------+
//  | TS_FRAME_END   | End of a logical frame of graphics data.                |
//  | 0x00000001     |                                                         |
//  +----------------+---------------------------------------------------------+

class FrameMarker {
public:
    enum {
          FrameStart = 0x00000000    /* TS_FRAME_START */
        , FrameEnd   = 0x00000001    /* TS_FRAME_END   */
    };

    uint32_t action{FrameStart};

    FrameMarker()  = default;

    explicit FrameMarker(uint32_t action) : action(action) {}

    void emit(OutStream & stream) const {
        uint8_t controlFlags = SECONDARY | (AltsecDrawingOrderHeader::FrameMarker << 2);
        stream.out_uint8(controlFlags);
        stream.out_uint32_le(this->action);
    }

    void receive(InStream & stream, const AltsecDrawingOrderHeader & /*header*/) {
        this->action = stream.in_uint32_le();
    }

    size_t str(char * buffer, size_t sz) const {
        size_t lg  = snprintf( buffer
                             , sz
                             , "RDPFrameMarker(action=%s(%u))\n"
                             , (  (this->action == FrameStart)
                                ? "FrameStart"
                                : (  (this->action == FrameEnd)
                                   ? "FrameEnd"
                                   : "<Unexpected>"
                                  )
                               )
                             , this->action);
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level) const {
        char buffer[1024];
        this->str(buffer, 1024);
        LOG(level, "%s", buffer);
    }
};  // class FrameMarker

}   // namespace RDP

