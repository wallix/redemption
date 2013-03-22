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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common slowpath layer at core module
*/

#ifndef _REDEMPTION_CORE_RDP_SLOWPATH_HPP_
#define _REDEMPTION_CORE_RDP_SLOWPATH_HPP_

namespace SlowPath {

    struct ClientInputEventPDU_Recv {
        uint16_t  numEvents;
        SubStream payload;

        ClientInputEventPDU_Recv(Stream & stream)
        : numEvents(0)
        , payload(stream) {
            if (!stream.in_check_rem(2)) {
                LOG(LOG_ERR, "SlowPath::ClientInputEventPDU: data truncated (numEvents)");
                throw Error(ERR_RDP_SLOWPATH);
            }

            numEvents = stream.in_uint16_le();

            const unsigned expected =
                  2              // pad(2)
                + numEvents * 12 // (time(4) + mes_type(2) + device_flags(2) + param1(2) + param2(2)) * 12
                ;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "SlowPath::ClientInputEventPDU: data truncated, expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDP_SLOWPATH);
            }

            stream.in_skip_bytes(2); // pad

            payload.resize(stream,
                numEvents * 12); // (time(4) + mes_type(2) + device_flags(2) + param1(2) + param2(2)) * 12
        }
    };

// 2.2.8.1.1.3.1.1 Slow-Path Input Event (TS_INPUT_EVENT)
// ======================================================

// The TS_INPUT_EVENT structure is used to wrap event-specific information for
//  all slow-path input events.

// messageType (2 bytes): A 16-bit, unsigned integer. The input event type.

// +-----------------------------+---------------------------------------------+
// | Value                       | Meaning                                     |
// +-----------------------------+---------------------------------------------+
// | 0x0000 INPUT_EVENT_SYNC     | Indicates a Synchronize Event (section      |
// |                             | 2.2.8.1.1.3.1.1.5).                         |
// +-----------------------------+---------------------------------------------+
// | 0x0002 INPUT_EVENT_UNUSED   | Indicates an Unused Event (section          |
// |                             | 2.2.8.1.1.3.1.1.6).                         |
// +-----------------------------+---------------------------------------------+
// | 0x0004 INPUT_EVENT_SCANCODE | Indicates a Keyboard Event (section         |
// |                             | 2.2.8.1.1.3.1.1.1).                         |
// +-----------------------------+---------------------------------------------+
// | 0x0005 INPUT_EVENT_UNICODE  | Indicates a Unicode Keyboard Event (section |
// |                             | 2.2.8.1.1.3.1.1.2).                         |
// +-----------------------------+---------------------------------------------+
// | 0x8001 INPUT_EVENT_MOUSE    | Indicates a Mouse Event (section            |
// |                             | 2.2.8.1.1.3.1.1.3).                         |
// +-----------------------------+---------------------------------------------+
// | 0x8002 INPUT_EVENT_MOUSEX   | Indicates an Extended Mouse Event (section  |
// |                             | 2.2.8.1.1.3.1.1.4).                         |
// +-----------------------------+---------------------------------------------+

    enum {
          INPUT_EVENT_SYNC     = 0x0000
        , INPUT_EVENT_UNUSED   = 0x0002
        , INPUT_EVENT_SCANCODE = 0x0004
        , INPUT_EVENT_UNICODE  = 0x0005
        , INPUT_EVENT_MOUSE    = 0x8001
        , INPUT_EVENT_MOUSEX   = 0x8002
    };

    struct InputEvent_Recv {
        uint32_t  eventTime;
        uint16_t  messageType;
        SubStream payload;

        InputEvent_Recv(Stream & stream)
        : eventTime(0)
        , messageType(0)
        , payload(stream) {
            const unsigned expected =
                12; // time(4) + mes_type(2) + device_flags(2) + param1(2) + param2(2)
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "SlowPath::InputEvent: data truncated, expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDP_SLOWPATH);
            }

            eventTime   = stream.in_uint32_le();
            messageType = stream.in_uint16_le();

            payload.resize(stream, 6); // device_flags(2) + param1(2) + param2(2)

            stream.in_skip_bytes(6);
        }
    };

} // namespace SlowPath

#endif // #ifndef _REDEMPTION_CORE_RDP_SLOWPATH_HPP_
