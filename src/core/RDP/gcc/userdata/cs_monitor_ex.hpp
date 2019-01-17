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
   Author(s): Christophe Grosjean, Jennifer Inthavong

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {
// 2.2.1.3.9 Client Monitor Extended Data (TS_UD_CS_MONITOR_EX)
// ============================================================

// The TS_UD_CS_MONITOR_EX packet describes extended attributes of the client-side display
// monitor layout defined by the Client Monitor Data block (section 2.2.1.3.6). This packet
// is an Extended Client Data Block and MUST NOT be sent to a server which does not advertise
// support for Extended Client Data Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag
// (0x00000001) as described in section 2.2.1.2.1.

//    header (4 bytes): A GCC user data block header, as specified in User Data
//                      Header (section 2.2.1.3.1). The User Data Header type
//                      field MUST be set to CS_MONITOR_EX (0xC008).

//    flags (4 bytes): A 32-bit, unsigned integer. This field is unused and reserved for
//                     future use. It MUST be set to zero.

//    monitorAttributeSize (4 bytes): A 32-bit, unsigned integer. The size, in bytes, of a
//                                    single element in the monitorAttributesArray field.
//                                    This field MUST be set to 20 bytes, which is the size of
//                                    the Monitor Attributes structure (section 2.2.1.3.9.1).

//    monitorCount (4 bytes): A 32-bit, unsigned integer. The number of elements in the
//                            monitorAttributesArray field. This value MUST be the same
//                            as the monitorCount field specified in the Client Monitor
//                            Data (section 2.2.1.3.6 block (section).

//    monitorAttributesArray (variable): A variable-length array containing a series of
//                                       TS_MONITOR_ATTRIBUTES structures (section
//                                       2.2.1.3.9.1) which describe extended attributes
//                                       of each display monitor specified in the Client
//                                       Monitor Data block. The number of TS_MONITOR_ATTRIBUTES
//                                       structures is specified by the monitorCount field.


// 2.2.1.3.9.1 Monitor Attributes (TS_MONITOR_ATTRIBUTES)
// -----------------------------------------------

// The TS_MONITOR_ATTRIBUTES packet describes extended attributes of a client-side display
// monitor.

// physicalWidth (4 bytes): A 32-bit, unsigned integer. The physical width of the monitor,
// in millimeters (mm). This value MUST be ignored if it is less than 10 mm or greater than
// 10,000 mm or physicalHeight is less than 10 mm or greater than 10,000 mm.

// physicalHeight (4 bytes): A 32-bit, unsigned integer. The physical height of the monitor,
// in millimeters. This value MUST be ignored if it is less than 10 mm or greater than
// 10,000 mm or physicalWidth is less than 10 mm or greater than 10,000 mm.

// orientation (4 bytes): A 32-bit, unsigned integer. The orientation of the monitor,
// in degrees. This value MUST be ignored if it is invalid.

//        Value                                Meaning
// -------------------------------------------------------------------------
// ORIENTATION_LANDSCAPE            The desktop is not rotated.
//       0
// ORIENTATION_PORTRAIT             The desktop is rotated clockwise by 90 degrees.
//       90
// ORIENTATION_LANDSCAPE_FLIPPED    The desktop is rotated clockwise by 180 degrees.
//       180
// ORIENTATION_PORTRAIT_FLIPPED     The desktop is rotated clockwise by 270 degrees.
//       270
// -------------------------------------------------------------------------

// desktopScaleFactor (4 bytes): A 32-bit, unsigned integer. The desktop scale factor of the monitor. This value MUST be ignored if it is less than 100% or greater than 500% or deviceScaleFactor is not 100%, 140% or 180%.

// deviceScaleFactor (4 bytes): A 32-bit, unsigned integer. The device scale factor of the monitor. This value MUST be ignored if it is not set to 100%, 140%, or 180% or desktopScaleFactor is less than 100% or greater than 500%


struct CSMonitorEx {
    uint16_t userDataType{CS_MONITOR_EX};
    uint16_t length{0};

    uint32_t flags{0};
    uint32_t monitorAttributeSize{20};
    uint32_t monitorCount{0};

    enum {
        MAX_MONITOR_COUNT = 16
    };
    enum {
      ORIENTATION_LANDSCAPE = 0,
      ORIENTATION_PORTRAIT = 90,
      ORIENTATION_LANDSCAPE_FLIPPED = 180,
      ORIENTATION_PORTRAIT_FLIPPED = 270
    };

   struct {
        uint32_t  physicalWidth;
        uint32_t  physicalHeight;
        uint32_t  orientation;
        uint32_t  desktopScaleFactor;
        uint32_t deviceScaleFactor;
    } monitorAttributesArray[MAX_MONITOR_COUNT];

    CSMonitorEx()
    :
     monitorAttributesArray() {}

    void emit(OutStream & stream) /* TODO const*/ {
        assert((this->monitorCount > 0) && (this->monitorCount <= MAX_MONITOR_COUNT));

        stream.out_uint16_le(this->userDataType);
        this->length = 4 + 4 + 4 + this->monitorCount * 20; // header(4) + flags(4) + monitorAttributeSize(4) + monitorCount(4) + monitorCount * monitorAttributesArray(20)
        stream.out_uint16_le(this->length);

        stream.out_uint32_le(this->flags);
        stream.out_uint32_le(this->monitorAttributeSize);
        stream.out_uint32_le(this->monitorCount);


        for (uint32_t i = 0; i < this->monitorCount; i++) {
            stream.out_uint32_le(this->monitorAttributesArray[i].physicalWidth);
            stream.out_uint32_le(this->monitorAttributesArray[i].physicalHeight);
            stream.out_uint32_le(this->monitorAttributesArray[i].orientation);
            stream.out_uint32_le(this->monitorAttributesArray[i].desktopScaleFactor);
            stream.out_uint32_le(this->monitorAttributesArray[i].deviceScaleFactor);
        }
    }

    void recv(InStream & stream) {
        if (!stream.in_check_rem(4)) {
            LOG(LOG_ERR, "CSMonitorEx::recv short header, need=4 remains=%zu",
                stream.in_remain());
            throw Error(ERR_GCC);
        }

        this->userDataType = stream.in_uint16_le();
        this->length       = stream.in_uint16_le();

        if (!stream.in_check_rem(12)) {
            LOG(LOG_ERR, "GCC User Data CS_MONITOR_EX truncated, remains=%zu",
                stream.in_remain());
            throw Error(ERR_GCC);
        }

        this->flags        = stream.in_uint32_le();
        this->monitorAttributeSize = stream.in_uint32_le();
        this->monitorCount = stream.in_uint32_le();


        if (this->monitorAttributeSize != 20) {
            LOG(LOG_ERR, "CSMonitorEx::recv monitorAttributeSize not as expected (%u)", this->monitorAttributeSize);
            this->monitorAttributeSize = 0;
            throw Error(ERR_GCC);
        }

        if ((this->monitorCount < 1) || (this->monitorCount > MAX_MONITOR_COUNT)) {
            LOG(LOG_ERR, "CSMonitorEx::recv monitor count out of range (%u)", this->monitorCount);
            this->monitorCount = 0;
            throw Error(ERR_GCC);
        }

        unsigned expected = 4 + 4 + 4 + 4 + this->monitorCount * 20;
        // = header(4) + flags(4) + monitorAttributeSize(4) + monitorCount(4) + monitorCount * monitorAttributesArray(20)

        if (this->length != expected) {
            LOG(LOG_ERR, "CSMonitorEx::recv bad header length, expecting=%u got=%u",
                expected, this->length);
            throw Error(ERR_GCC);
        }

        expected = this->monitorCount * 20;    // monitorCount * monitorAttributesArray(20)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "GCC User Data CS_MONITOR_EX truncated, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_GCC);
        }

        for (uint32_t i = 0; i < this->monitorCount; i++) {
            this->monitorAttributesArray[i].physicalWidth      = stream.in_uint32_le();
            this->monitorAttributesArray[i].physicalHeight     = stream.in_uint32_le();
            this->monitorAttributesArray[i].orientation        = stream.in_uint32_le();
            this->monitorAttributesArray[i].desktopScaleFactor = stream.in_uint32_le();
            this->monitorAttributesArray[i].deviceScaleFactor  = stream.in_uint32_le();

            if ((this->monitorAttributesArray[i].physicalWidth < 10) ||
                (this->monitorAttributesArray[i].physicalWidth > 10000))
            {
               LOG(LOG_INFO, "GCC User Data CS_MONITOR_EX physicalWidth out of range [10,10000], ignored");
               this->monitorAttributesArray[i].physicalWidth = 0;
            }

            if ((this->monitorAttributesArray[i].physicalHeight < 10) ||
                (this->monitorAttributesArray[i].physicalHeight > 10000))
            {
               LOG(LOG_INFO, "GCC User Data CS_MONITOR_EX physicalHeight out of range [10,10000], ignored");
               this->monitorAttributesArray[i].physicalHeight = 0;
            }

            if((this->monitorAttributesArray[i].orientation != ORIENTATION_LANDSCAPE)         &&
               (this->monitorAttributesArray[i].orientation != ORIENTATION_PORTRAIT)          &&
               (this->monitorAttributesArray[i].orientation != ORIENTATION_LANDSCAPE_FLIPPED) &&
               (this->monitorAttributesArray[i].orientation != ORIENTATION_PORTRAIT_FLIPPED))
            {
               LOG(LOG_INFO, "GCC User Data CS_MONITOR_EX orientation out of scope, ignored");
               this->monitorAttributesArray[i].orientation = ORIENTATION_LANDSCAPE; // ignored
            }

            if (  ( (this->monitorAttributesArray[i].desktopScaleFactor < 100) ||
                    (this->monitorAttributesArray[i].desktopScaleFactor > 500) )
                 ||
                  (  (this->monitorAttributesArray[i].deviceScaleFactor != 100) &&
                    (this->monitorAttributesArray[i].deviceScaleFactor != 140) &&
                    (this->monitorAttributesArray[i].deviceScaleFactor != 180) )
               )
            {
               LOG(LOG_INFO, "GCC User Data CS_MONITOR_EX desktopScaleFactor and deviceScaleFactor out of scope, ignored");
               this->monitorAttributesArray[i].desktopScaleFactor = 100; // ignored
               this->monitorAttributesArray[i].deviceScaleFactor = 100;
            }
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
            lg += snprintf(buffer + lg, sizeof(buffer) - lg,
                "(physicalWidth=%u"
                ", physicalHeight=%u"
                ", orientation=%u"
                ", desktopScaleFactor=%u"
                ", deviceScaleFactor=%u)",
                this->monitorAttributesArray[i].physicalWidth,
                this->monitorAttributesArray[i].physicalHeight,
                this->monitorAttributesArray[i].orientation,
                this->monitorAttributesArray[i].desktopScaleFactor,
                this->monitorAttributesArray[i].deviceScaleFactor);
        }
        snprintf(buffer + lg, sizeof(buffer) - lg, ")");

        buffer[sizeof(buffer) - 1] = 0;
        LOG(LOG_INFO, "%s", buffer);
    }
};
} // namespace UserData
} // namespace GCC
