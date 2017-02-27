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

#include "core/RDP/gcc.hpp"

// [MS-RDPBCGR] - 2.2.1.3.6.1 Monitor Definition (TS_MONITOR_DEF)
// ==============================================================

// The TS_MONITOR_DEF packet describes the configuration of a client-side
//  display monitor. The x and y coordinates used to describe the monitor
//  position MUST be relative to the upper-left corner of the monitor
//  designated as the "primary display monitor" (the upper-left corner of the
//  primary monitor is always (0, 0)).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              left                             |
// +---------------------------------------------------------------+
// |                              top                              |
// +---------------------------------------------------------------+
// |                             right                             |
// +---------------------------------------------------------------+
// |                             bottom                            |
// +---------------------------------------------------------------+
// |                             flags                             |
// +---------------------------------------------------------------+

// left (4 bytes): A 32-bit, signed integer. Specifies the x-coordinate of
//  the upper-left corner of the display monitor.

// top (4 bytes): A 32-bit, signed integer. Specifies the y-coordinate of the
//  upper-left corner of the display monitor.

// right (4 bytes): A 32-bit, signed integer. Specifies the inclusive
//  x-coordinate of the lower-right corner of the display monitor.

// bottom (4 bytes): A 32-bit, signed integer. Specifies the inclusive
//  y-coordinate of the lower-right corner of the display monitor.

// flags (4 bytes): A 32-bit, unsigned integer. Monitor configuration flags.

//  +--------------------+--------------------------------------------------+
//  | Flag               | Meaning                                          |
//  +--------------------+--------------------------------------------------+
//  | TS_MONITOR_PRIMARY | The top, left, right, and bottom fields describe |
//  | 0x00000001         | the position of the primary monitor.             |
//  +--------------------+--------------------------------------------------+

// [MS-RDPBCGR] - 2.2.12.1 Monitor Layout PDU
// ==========================================

// The Monitor Layout PDU is used by the server to notify the client of the
//  monitor layout in the session on the remote server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           tpktHeader                          |
// +-----------------------------------------------+---------------+
// |                    x224Data                   |    mcsSDin    |
// |                                               |   (variable)  |
// +-----------------------------------------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   securityHeader (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   shareDataHeader (18 bytes)                  |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |          monitorCount         |
// +-------------------------------+-------------------------------+
// |              ...              |   monitorDefArray (variable)  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//  section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU
//  (DomainMCSPDU) which encapsulates an MCS Send Data Indication structure
//  (SDin, choice 26 from DomainMCSPDU), as specified in [T125] section 11.33
//  (the ASN.1 structure definitions are given in [T125] section 7, parts 7
//  and 10). The userData field of the MCS Send Data Indication contains a
//  Security Header,Share Data Header, monitor count, and a monitor
//  definition array.

// securityHeader (variable): Optional security header. The presence and
//  format of the security header depends on the Encryption Level and
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).
//  If the Encryption Level selected by the server is greater than
//  ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the
//  server is greater than ENCRYPTION_METHOD_NONE (0), then this field MUST
//  contain one of the following headers:

//  * Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//    selected by the server is ENCRYPTION_LEVEL_LOW (1).

//  * Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//    Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
//    ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
//    (0x00000002).

//  * FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
//    selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010).

//  If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE
//   (0) and the Encryption Method selected by the server is
//   ENCRYPTION_METHOD_NONE (0), then this header MUST NOT be included in the
//   PDU.

// shareDataHeader (18 bytes): A Share Data Header containing information
//  about the packet. The type subfield of the pduType field of the Share
//  Control Header (section 2.2.8.1.1.1.1) MUST be set to PDUTYPE_DATAPDU
//  (7). The pduType2 field of the Share Data Header MUST be set to
//  PDUTYPE2_MONITOR_LAYOUT_PDU (55), and the pduSource field MUST be set to
//  zero.

// monitorCount (4 bytes): A 32-bit, unsigned integer. The number of display
//  monitor definitions in the monitorDefArray field.

// monitorDefArray (variable): A variable-length array containing a series of
//  TS_MONITOR_DEF structures (section 2.2.1.3.6.1), which describe the
//  display monitor layout of the session on the remote server. The number of
//  TS_MONITOR_DEF structures that follows is given by the monitorCount
//  field.

class MonitorLayoutPDU {
    uint32_t monitorCount;

    struct {
        int32_t  left;
        int32_t  top;
        int32_t  right;
        int32_t  bottom;
        uint32_t flags;
    } monitorDefArray[GCC::UserData::CSMonitor::MAX_MONITOR_COUNT];

public:
    MonitorLayoutPDU()
    : monitorCount(0)
    , monitorDefArray() {}

    void set(GCC::UserData::CSMonitor const & cs_monitor) {
        this->monitorCount = cs_monitor.monitorCount;

        for (uint32_t monitorIndex = 0; monitorIndex < cs_monitor.monitorCount; ++monitorIndex) {
            this->monitorDefArray[monitorIndex].left   = cs_monitor.monitorDefArray[monitorIndex].left;
            this->monitorDefArray[monitorIndex].top    = cs_monitor.monitorDefArray[monitorIndex].top;
            this->monitorDefArray[monitorIndex].right  = cs_monitor.monitorDefArray[monitorIndex].right;
            this->monitorDefArray[monitorIndex].bottom = cs_monitor.monitorDefArray[monitorIndex].bottom;
            this->monitorDefArray[monitorIndex].flags  = cs_monitor.monitorDefArray[monitorIndex].flags;
        }
    }

    void get(GCC::UserData::CSMonitor & cs_monitor) const {
        cs_monitor.monitorCount = this->monitorCount;

        for (uint32_t monitorIndex = 0; monitorIndex < cs_monitor.monitorCount; ++monitorIndex) {
            cs_monitor.monitorDefArray[monitorIndex].left   = this->monitorDefArray[monitorIndex].left;
            cs_monitor.monitorDefArray[monitorIndex].top    = this->monitorDefArray[monitorIndex].top;
            cs_monitor.monitorDefArray[monitorIndex].right  = this->monitorDefArray[monitorIndex].right;
            cs_monitor.monitorDefArray[monitorIndex].bottom = this->monitorDefArray[monitorIndex].bottom;
            cs_monitor.monitorDefArray[monitorIndex].flags  = this->monitorDefArray[monitorIndex].flags;
        }
    }

    void emit(OutStream & stream) const {
        REDASSERT((this->monitorCount > 0) &&
            (this->monitorCount <=
             GCC::UserData::CSMonitor::MAX_MONITOR_COUNT));

        stream.out_uint32_le(this->monitorCount);

        for (uint32_t monitorIndex = 0; monitorIndex < this->monitorCount; ++monitorIndex) {
            stream.out_sint32_le(this->monitorDefArray[monitorIndex].left);
            stream.out_sint32_le(this->monitorDefArray[monitorIndex].top);
            stream.out_sint32_le(this->monitorDefArray[monitorIndex].right);
            stream.out_sint32_le(this->monitorDefArray[monitorIndex].bottom);
            stream.out_uint32_le(this->monitorDefArray[monitorIndex].flags);
        }
    }

    void recv(InStream & stream) {
        if (!stream.in_check_rem(4)) {
            LOG(LOG_ERR,
                "MonitorLayoutPDU::recv: Truncated data, need=4, remains=%zu",
                stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->monitorCount = stream.in_uint32_le();

        if ((this->monitorCount < 1) ||
            (this->monitorCount > GCC::UserData::CSMonitor::MAX_MONITOR_COUNT)) {
            LOG(LOG_ERR,
                "MonitorLayoutPDU::recv: Monitor count out of range (%u)",
                this->monitorCount);
            this->monitorCount = 0;
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        const unsigned expected = this->monitorCount * 20;  // monitorCount * monitorDefArray(20)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "MonitorLayoutPDU::recv: Truncated data, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        for (uint32_t monitorIndex = 0; monitorIndex < this->monitorCount; ++monitorIndex) {
            this->monitorDefArray[monitorIndex].left   = stream.in_sint32_le();
            this->monitorDefArray[monitorIndex].top    = stream.in_sint32_le();
            this->monitorDefArray[monitorIndex].right  = stream.in_sint32_le();
            this->monitorDefArray[monitorIndex].bottom = stream.in_sint32_le();
            this->monitorDefArray[monitorIndex].flags  = stream.in_uint32_le();
        }
    }

    uint32_t get_monitorCount() const {
        return this->monitorCount;
    }

    void log(const char * msg)
    {
        char buffer[2048];

        size_t lg = 0;
        // --------------------- Base Fields ---------------------------------------
        lg += snprintf(buffer + lg, sizeof(buffer) - lg,
            "%s MonitorLayoutPDU ", msg);
        lg += snprintf(buffer + lg, sizeof(buffer) - lg,
            "monitorCount=%u (", this->monitorCount);

        for (uint32_t monitorIndex = 0; monitorIndex < this->monitorCount; ++monitorIndex) {
            if (monitorIndex) {
                lg += snprintf(buffer + lg, sizeof(buffer) - lg, " ");
            }
            lg += snprintf(buffer + lg, sizeof(buffer) - lg,
                "(left=%d, top=%d, right=%d, bottom=%d, primary=%s(0x%X))",
                this->monitorDefArray[monitorIndex].left,
                this->monitorDefArray[monitorIndex].top,
                this->monitorDefArray[monitorIndex].right,
                this->monitorDefArray[monitorIndex].bottom,
                ((this->monitorDefArray[monitorIndex].flags & GCC::UserData::CSMonitor::TS_MONITOR_PRIMARY) ?
                 "yes" : "no"),
                this->monitorDefArray[monitorIndex].flags);
        }
        snprintf(buffer + lg, sizeof(buffer) - lg, ")");

        buffer[sizeof(buffer) - 1] = 0;
        LOG(LOG_INFO, "%s", buffer);
    }
};
