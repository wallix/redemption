/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Raphaël Zhou
*/

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

#include <cstddef>
#include <cstdio>


namespace drdynvc {

enum {
    CMD_CREATE                = 0x01,
    CMD_DATA_FIRST            = 0x02,
    CMD_DATA                  = 0x03,
    CMD_CLOSE                 = 0x04,
    CMD_CAPABILITIES          = 0x05,
    CMD_DATA_FIRST_COMPRESSED = 0x06,
    CMD_DATA_COMPRESSED       = 0x07,
    CMD_SOFTSYNC_REQUEST      = 0x08,
    CMD_SOFTSYNC_RESPONSE     = 0x09
};

static inline
const char * get_DCVC_Cmd_name(uint8_t Cmd) {
    switch (Cmd) {
        case CMD_CREATE:                return "CMD_CREATE";
        case CMD_DATA_FIRST:            return "CMD_DATA_FIRST";
        case CMD_DATA:                  return "CMD_DATA";
        case CMD_CLOSE:                 return "CMD_CLOSE";
        case CMD_CAPABILITIES:          return "CMD_CAPABILITIES";
        case CMD_DATA_FIRST_COMPRESSED: return "CMD_DATA_FIRST_COMPRESSED";
        case CMD_DATA_COMPRESSED:       return "CMD_DATA_COMPRESSED";
        case CMD_SOFTSYNC_REQUEST:      return "CMD_SOFTSYNC_REQUEST";
        case CMD_SOFTSYNC_RESPONSE:     return "CMD_SOFTSYNC_RESPONSE";
        default:                        return "<unknown>";
    }   // switch (Cmd)
}   // get_DCVC_Cmd_name

// [MS-RDPEDYC] - 2.2.1.1 DVC Capabilities Request PDU

// [MS-RDPEDYC] - 2.2.1.1.1 Version 1 (DYNVC_CAPS_VERSION1)

// The DYNVC_CAPS_VERSION1 PDU is sent by the DVC server manager to indicate
//  that it supports version 1 of the Remote Desktop Protocol: Dynamic
//  Channel Virtual Channel Extension.<1>

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | cb| Sp|  Cmd  |      Pad      |            Version            |
// | Id|   |       |               |                               |
// +---+---+-------+---------------+-------------------------------+

// cbId (2 bits): Unused. MUST be set to 0x00.

// Sp (2 bits): Unused. SHOULD be initialized to 0x00.<2>

// <2> Section 2.2.1.1.1: Windows implementations initialize Sp to 0x02.

// Cmd (4 bits): MUST be set to 0x05 (Capabilities).

// Pad (1 byte): An 8-bit unsigned integer. Unused. MUST be set to 0x00.

// Version (2 bytes): A 16-bit unsigned integer. MUST be set to 0x0001.

// [MS-RDPEDYC] - 2.2.1.1.2 Version 2 (DYNVC_CAPS_VERSION2)

// The DYNVC_CAPS_VERSION2 PDU is sent by the DVC server manager to indicate
//  that it supports version 2 of the Remote Desktop Protocol: Dynamic
//  Virtual Channel Extension.<3>

// <3> Section 2.2.1.1.2: The DYNVC_CAPS_VERSION2 PDU is supported in Windows
//  Vista operating system with Service Pack 1 (SP1), Windows Server 2008,
//  Windows 7, and Windows Server 2008 R2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | cb| Sp|  Cmd  |      Pad      |            Version            |
// | Id|   |       |               |                               |
// +---+---+-------+---------------+-------------------------------+
// |        PriorityCharge0        |        PriorityCharge1        |
// +-------------------------------+-------------------------------+
// |        PriorityCharge2        |        PriorityCharge3        |
// +-------------------------------+-------------------------------+

// cbId (2 bits): Unused. MUST be set to 0x00.

// Sp (2 bits): Unused. SHOULD be set to 0x00.<4>

// <4> Section 2.2.1.1.2: Windows implementations initialize Sp to 0x02.

// Cmd (4 bits): MUST be set to 0x05 (Capabilities).

// Pad (1 byte): An 8-bit unsigned integer. Unused. MUST be set to 0x00.

// Version (2 bytes): A 16-bit unsigned integer. MUST be set to 0x0002.

// PriorityCharge0 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the following algorithm.

// PriorityCharge1 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the following algorithm.

// PriorityCharge2 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the following algorithm.

// PriorityCharge3 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the following algorithm.

// The PriorityCharge fields determine how much bandwidth is allocated for
//  each priority class. The percentage is calculated using the following
//  formula.

//  Base = PriorityCharge0 * PriorityCharge1 *
//      PriorityCharge2 * PriorityCharge3 /
//      (PriorityCharge1 * PriorityCharge2 *
//      PriorityCharge3 + PriorityCharge0 *
//      PriorityCharge2 * PriorityCharge3 +
//      PriorityCharge0 * PriorityCharge1 * PriorityCharge3 +
//      PriorityCharge0 * PriorityCharge1 *
//      PriorityCharge2)

//  BandwidthPriority0 = Base / PriorityCharge0
//  BandwidthPriority1 = Base / PriorityCharge1
//  BandwidthPriority2 = Base / PriorityCharge2
//  BandwidthPriority3 = Base / PriorityCharge3

// Where BandwidthPriorityX is a number between 0 and 1, and the total sum of
//  all BandwidthPriorityX values is equal to 1. If PriorityChargeX is set to
//  0, the priority formula is not used and the data is sent immediately;
//  remaining bandwidth is shared among channels with non-zero
//  PriorityChargeX values, as described in the formula.

// To calculate priority charges from given priorities the formula is as
//  follows.

//  PriorityCharge0 = 65536 / (BandwidthPriority0 * 100)
//  PriorityCharge1 = 65536 / (BandwidthPriority1 * 100)
//  PriorityCharge2 = 65536 / (BandwidthPriority2 * 100)
//  PriorityCharge3 = 65536 / (BandwidthPriority3 * 100)

// Where BandwidthPriorityX is a number between 0 and 1, and the total sum of
//  all BandwidthPriorityX values is equal to 1.

// For example, to have distribution for priority 0 to 3 be 70%, 20%, 7%, and
//  3%, the priority charges numbers are as follows.

//  PriorityCharge0 = 65536 / (0.70*100) = 936
//  PriorityCharge1 = 65536 / (0.20*100) = 3276
//  PriorityCharge2 = 65536 / (0.07*100) = 9362
//  PriorityCharge3 = 65536 / (0.03*100) = 21845

// Calculating the priority from priority charges, as follows.

//  Base = 936 * 3276 * 9362 * 21845 /
//      (3276 * 9362 * 21845 + 936 * 9362 *
//      21845 + 936 * 3276 * 21845 + 936 *
//      3276 * 9362) = 655

//  BandwidthPriority0 = 655/936 = 70%
//  BandwidthPriority1 = 655/3276 = 20%
//  BandwidthPriority2 = 655/9362 = 7%
//  BandwidthPriority3 = 655/21845 = 3%

// [MS-RDPEDYC] - 2.2.1.1.3 Version 3 (DYNVC_CAPS_VERSION3)

// The DYNVC_CAPS_VERSION3 PDU is sent by the DVC server manager to indicate
//  that it supports version 3 of the Remote Desktop Protocol: Dynamic
//  Virtual Channel Extension.<5>

// <5> Section 2.2.1.1.3: The DYNVC_CAPS_VERSION3 PDU is not supported in
//  Windows Vista, Windows Server 2008, Windows 7, and Windows Server 2008
//  R2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | cb| Sp|  Cmd  |      Pad      |            Version            |
// | Id|   |       |               |                               |
// +---+---+-------+---------------+-------------------------------+
// |        PriorityCharge0        |        PriorityCharge1        |
// +-------------------------------+-------------------------------+
// |        PriorityCharge2        |        PriorityCharge3        |
// +-------------------------------+-------------------------------+

// cbId (2 bits): Unused. MUST be set to 0x00.

// Sp (2 bits): Unused. SHOULD be set to 0x00.<6>

// <6> Section 2.2.1.1.3: Windows implementations initialize Sp to 0x02.

// Cmd (4 bits): MUST be set to 0x05 (Capabilities).

// Pad (1 byte): An 8-bit unsigned integer. Unused. MUST be set to 0x00.

// Version (2 bytes): A 16-bit unsigned integer. MUST be set to 0x0003.

// PriorityCharge0 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the algorithm specified in section 2.2.1.1.2.

// PriorityCharge1 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the algorithm specified in section 2.2.1.1.2.

// PriorityCharge2 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the algorithm specified in section 2.2.1.1.2.

// PriorityCharge3 (2 bytes): A 16-bit unsigned integer. Specifies the amount
//  of bandwidth that is allotted for each priority class, in accordance with
//  the algorithm specified in section 2.2.1.1.2.

enum {
    CAPS_VERSION1 = 0x0001,
    CAPS_VERSION2 = 0x0002,
    CAPS_VERSION3 = 0x0003
};


class DVCCapabilitiesRequestPDU
{
    uint8_t  cdId            = 0x00;
    uint8_t  Sp              = 0x00;
    uint8_t  Cmd             = CMD_CAPABILITIES;
    uint16_t Version         = CAPS_VERSION1;
    uint16_t PriorityCharge0 = 0;
    uint16_t PriorityCharge1 = 0;
    uint16_t PriorityCharge2 = 0;
    uint16_t PriorityCharge3 = 0;

public:
    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // cbId(:2) + Sp(:2) + Cmd(:4) +
                                            //     Pad(1) + Version(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DVCCapabilitiesRequestPDU (1): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        uint8_t const tmp = stream.in_uint8();

        this->cdId =  (tmp & 0x03);
        this->Sp   = ((tmp & 0x0C) >> 2);
        this->Cmd  = ((tmp & 0xF0) >> 4);

        stream.in_skip_bytes(1);    // Pad(1)

        this->Version = stream.in_uint16_le();

        if ((this->Version == CAPS_VERSION2) ||
            (this->Version == CAPS_VERSION3)) {
            {
                const unsigned expected = 8;    // PriorityCharge0(2) +
                                                //  PriorityCharge1(2) +
                                                //  PriorityCharge2(2) +
                                                //  PriorityCharge3(2)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated DVCCapabilitiesRequestPDU (2): expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDPDR_PDU_TRUNCATED);
                }
            }

            this->PriorityCharge0 = stream.in_uint16_le();
            this->PriorityCharge1 = stream.in_uint16_le();
            this->PriorityCharge2 = stream.in_uint16_le();
            this->PriorityCharge3 = stream.in_uint16_le();
        }
    }

    void emit(OutStream & stream) const {
        uint8_t const tmp = (
                 (this->cdId & 0x03)       |
                ((this->Sp   & 0x03) << 2) |
                ((this->Cmd  & 0x0F) << 4)
            );
        stream.out_uint8(tmp);

        stream.out_clear_bytes(1);  // Pad(1)

        stream.out_uint16_le(this->Version);

        if ((this->Version == CAPS_VERSION2) ||
            (this->Version == CAPS_VERSION3)) {
            stream.out_uint16_le(this->PriorityCharge0);
            stream.out_uint16_le(this->PriorityCharge1);
            stream.out_uint16_le(this->PriorityCharge2);
            stream.out_uint16_le(this->PriorityCharge3);
        }
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "DVCCapabilitiesRequestPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "cdId=0x%02X Sp=0x%02X Cmd=0x%02X Version=0x%04X",
            static_cast<unsigned int>(this->cdId),
            static_cast<unsigned int>(this->Sp),
            static_cast<unsigned int>(this->Cmd),
            static_cast<unsigned int>(this->Version));
        length += ((result < size - length) ? result : (size - length - 1));

        if ((this->Version == CAPS_VERSION2) ||
            (this->Version == CAPS_VERSION3)) {
            result = ::snprintf(buffer + length, size - length,
                " PriorityCharge0=0x%u PriorityCharge1=0x%u "
                    "PriorityCharge2=0x%u PriorityCharge3=0x%u",
                static_cast<unsigned int>(this->PriorityCharge0),
                static_cast<unsigned int>(this->PriorityCharge1),
                static_cast<unsigned int>(this->PriorityCharge2),
                static_cast<unsigned int>(this->PriorityCharge3));
            length += ((result < size - length) ? result : (size - length - 1));
        }

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

}   // namespace drdynvc
