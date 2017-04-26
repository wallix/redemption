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
    Copyright (C) Wallix 2015
    Author(s): Clément Moroldo
*/


#pragma once

#include "utils/stream.hpp"

namespace rdpsnd {


// 2.2.2.1 Server Audio Formats and Version PDU (SERVER_AUDIO_VERSION_AND_FORMATS)
//
// The Server Audio Formats and Version PDU is a PDU used by the server to send version
// information and a list of supported audio formats to the client. This PDU MUST be sent
// using virtual channels.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Header                           |
// +---------------------------------------------------------------+
// |                             dwFlags                           |
// +---------------------------------------------------------------+
// |                             dwVolume                          |
// +---------------------------------------------------------------+
// |                            b dwPitch                          |
// +-------------------------------+-------------------------------+
// |          wDGramPort           |        wNumberOfFormats       |
// +---------------+---------------+---------------+---------------+
// |cLastBlockConf |            wVersion           |               |
// +---------------+-------------------------------+---------------+
// |                       sndFormats (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): A RDPSND PDU Header (section 2.2.1). The msgType field of the RDPSND PDU
//                   Header MUST be set to SNDC_FORMATS (0x07).
//
// dwFlags (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                    and MUST be ignored on receipt.
//
// dwVolume (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                     and MUST be ignored on receipt.
//
// dwPitch (4 bytes): A 32-bit unsigned integer. This field is unused. The value is arbitrary
//                    and MUST be ignored on receipt.
//
// wDGramPort (2 bytes): A 16-bit unsigned integer. This field is unused. The value is arbitrary
//                       and MUST be ignored on receipt.
//
// wNumberOfFormats (2 bytes): A 16-bit unsigned integer. Number of AUDIO_FORMAT structures
//                             contained in the sndFormats array.
//
// cLastBlockConfirmed (1 byte): An 8-bit unsigned integer specifying the initial value for
//                               the cBlockNo counter used by the WaveInfo PDU, Wave2 PDU, Wave
//                               Encrypt PDU, UDP Wave PDU, and UDP Wave Last PDU. The value sent
//                               by the server is arbitrary. See section 3.3.5.2.1.1 for more
//                               information about the cBlockNo counter.
//
// wVersion (2 bytes): A 16-bit unsigned integer that contains the version of the protocol
//                     supported by the server.<3>
//
// bPad (1 byte): An 8-bit unsigned integer. This field is unused. The value is arbitrary and MUST
//                be ignored on receipt.
//
// sndFormats (variable): A variable-sized array of audio formats supported by the server, each
//                        conforming in structure to the AUDIO_FORMAT structure. The number of
//                        formats in the array is wNumberOfFormats.

}