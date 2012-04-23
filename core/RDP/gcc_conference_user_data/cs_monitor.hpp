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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   MCS Connect Initial PDU with GCC Conference User Data


*/

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_MONITOR_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_MONITOR_HPP__


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

struct CSMonitorGccUserData {
    uint16_t userDataType;
    uint16_t length;
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
    enum {
        TS_MONITOR_PRIMARY = 0x00000001,
    };

    uint32_t flags;

    CSMonitorGccUserData()
    : userDataType(CS_MONITOR)
    , length(12) // default: everything except serverSelectedProtocol
    , left(0)
    , top(0)
    , right(0)
    , bottom(0)
    , flags(0)
    {
    }


    void emit(Stream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->left);
        stream.out_uint32_le(this->top);
        stream.out_uint32_le(this->right);
        stream.out_uint32_le(this->bottom);
        stream.out_uint32_le(this->flags);
    }

    void recv(Stream & stream, uint16_t length)
    {
        this->length = length;
        this->left   = stream.in_uint32_le();
        this->top    = stream.in_uint32_le();
        this->right  = stream.in_uint32_le();
        this->bottom = stream.in_uint32_le();
        this->flags  = stream.in_uint32_le();
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data CS_MONITOR (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "cs_monitor::left   = %u", this->left);
        LOG(LOG_INFO, "cs_monitor::top    = %u", this->top);
        LOG(LOG_INFO, "cs_monitor::right  = %u", this->right);
        LOG(LOG_INFO, "cs_monitor::bottom = %u", this->bottom);
        LOG(LOG_INFO, "cs_monitor::flags [%04X]", this->flags);
        if (this->flags & TS_MONITOR_PRIMARY){
            LOG(LOG_INFO, "cs_monitor::flags::TS_MONITOR_PRIMARY");
        }
    }
};

#endif
