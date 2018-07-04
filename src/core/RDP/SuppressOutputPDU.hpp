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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include <cstdint>


namespace RDP {

// [MS-RDPBCGR] - 2.2.11.1 Inclusive Rectangle (TS_RECTANGLE16)
// ============================================================

// The TS_RECTANGLE16 structure describes a rectangle expressed in inclusive
//  coordinates (the right and bottom coordinates are included in the
//  rectangle bounds).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |              left             |              top              |
// +-------------------------------+-------------------------------+
// |             right             |             bottom            |
// +-------------------------------+-------------------------------+

// left (2 bytes): A 16-bit, unsigned integer. The leftmost bound of the
//  rectangle.

// top (2 bytes): A 16-bit, unsigned integer. The upper bound of the
//  rectangle.

// right (2 bytes): A 16-bit, unsigned integer. The rightmost bound of the
//  rectangle.

// bottom (2 bytes): A 16-bit, unsigned integer. The lower bound of the
//  rectangle.


// [MS-RDPBCGR] - 2.2.11.3.1 Suppress Output PDU Data
//  (TS_SUPPRESS_OUTPUT_PDU)
// ===================================================

// The TS_SUPPRESS_OUTPUT_PDU structure contains the contents of the Suppress
//  Output PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) and two
//  fields.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   shareDataHeader (18 bytes)                  |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+---------------+---------------+
// |              ...              |  allowDisplay |   pad3Octets  |
// |                               |     Updates   |               |
// +-------------------------------+---------------+---------------+
// |              ...              |          desktopRect          |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |
// +-------------------------------+

// shareDataHeader (18 bytes): A Share Data Header containing information
//  about the packet (section 2.2.8.1.1.1.2). The type subfield of the
//  pduType field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be
//  set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header
//  MUST be set to PDUTYPE2_SUPPRESS_OUTPUT (35).

// allowDisplayUpdates (1 byte): An 8-bit, unsigned integer. Indicates
//  whether the client wants to receive display updates from the server.

//  +--------------------------+-------------------------------------------+
//  | Value                    | Meaning                                   |
//  +--------------------------+-------------------------------------------+
//  | SUPPRESS_DISPLAY_UPDATES | Turn off display updates from the server. |
//  [ 0x00                     |                                           |
//  +--------------------------+-------------------------------------------+
//  [ ALLOW_DISPLAY_UPDATES    | Turn on display updates from the server.  |
//  [ 0x01                     |                                           |
//  +--------------------------+-------------------------------------------+

enum {
      SUPPRESS_DISPLAY_UPDATES = 0x00
    , ALLOW_DISPLAY_UPDATES    = 0x01
};

// pad3Octets (3 bytes): A 3-element array of 8-bit, unsigned integer values.
//  Padding. Values in this field MUST be ignored.

// desktopRect (8 bytes): An Inclusive Rectangle (section 2.2.11.1) which
//  contains the coordinates of the desktop rectangle if the
//  allowDisplayUpdates field is set to ALLOW_DISPLAY_UPDATES (1). If the
//  allowDisplayUpdates field is set to SUPPRESS_DISPLAY_UPDATES (0), this
//  field MUST NOT be included in the PDU.

class SuppressOutputPDUData {
    uint8_t allowDisplayUpdates_ = SUPPRESS_DISPLAY_UPDATES;

    uint16_t left_   = 0;
    uint16_t top_    = 0;
    uint16_t right_  = 0;
    uint16_t bottom_ = 0;

public:
    SuppressOutputPDUData() = default;

    SuppressOutputPDUData(uint16_t left,
        uint16_t top, uint16_t right, uint16_t bottom)
    : allowDisplayUpdates_(ALLOW_DISPLAY_UPDATES)
    , left_(left)
    , top_(top)
    , right_(right)
    , bottom_(bottom) {}

    void emit(OutStream & stream) const {
        stream.out_uint8(this->allowDisplayUpdates_);
        stream.out_clear_bytes(3);  // Padding(3)

        if (ALLOW_DISPLAY_UPDATES == this->allowDisplayUpdates_) {
            stream.out_uint16_le(this->left_);
            stream.out_uint16_le(this->top_);
            stream.out_uint16_le(this->right_);
            stream.out_uint16_le(this->bottom_);
        }
    }   // void emit(OutStream & stream) const

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // allowDisplayUpdates(1) + Padding(3)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated SuppressOutputPDUData: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        this->allowDisplayUpdates_ = stream.in_uint8();

        stream.in_skip_bytes(3);    // Padding(3)

        if (ALLOW_DISPLAY_UPDATES == this->allowDisplayUpdates_) {
            {
                const unsigned expected = 8;    // left(2) + top(2) + right(2) + bottom(2)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated SuppressOutputPDUData(2): expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            this->left_   = stream.in_uint16_le();
            this->top_    = stream.in_uint16_le();
            this->right_  = stream.in_uint16_le();
            this->bottom_ = stream.in_uint16_le();
        }
    }   // void receive(InStream & stream)

    static const char * get_allowDisplayUpdates_name(int allowDisplayUpdates) {
        switch (allowDisplayUpdates) {
            case SUPPRESS_DISPLAY_UPDATES: return "SUPPRESS_DISPLAY_UPDATES";
            case ALLOW_DISPLAY_UPDATES:  return "ALLOW_DISPLAY_UPDATES";
        }

        return "<unknown>";
    }   // const char * get_allowDisplayUpdates_name(int allowDisplayUpdates)

    uint8_t  get_allowDisplayUpdates() const {
        return this->allowDisplayUpdates_;
    }

    uint16_t get_left() const { return this->left_; }
    uint16_t get_top() const { return this->top_; }
    uint16_t get_right() const { return this->right_; }
    uint16_t get_bottom() const { return this->bottom_; }

    size_t size() const {
        return 4 +  // allowDisplayUpdates(1) + Padding(3)
            ((ALLOW_DISPLAY_UPDATES == this->allowDisplayUpdates_) ?
             8 :    // left(2) + top(2) + right(2) + bottom(2)
             0);
    }   // size_t size() const

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;
        length += ::snprintf(buffer + length, size - length,
            "SuppressOutputPDUData: allowDisplayUpdates=%s(0x%X)",
            this->get_allowDisplayUpdates_name(this->allowDisplayUpdates_),
            this->allowDisplayUpdates_);

        if (ALLOW_DISPLAY_UPDATES == this->allowDisplayUpdates_) {
            length += ::snprintf(buffer + length, size - length,
                " (%u, %u, %u, %u)",
                this->left_, this->top_, this->right_, this->bottom_);
        }

        return ((length < size) ? length : size - 1);
    }   // size_t str(char * buffer, size_t size) const

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }   // inline void log(int level)
};  // class SuppressOutputPDUData

}   // namespace RDP

