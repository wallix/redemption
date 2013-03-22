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

    struct ClientInputEventPDU_Recv
    {
        uint16_t  numEvents;
        SubStream payload;

        ClientInputEventPDU_Recv(Stream & stream)
        : numEvents(0)
        , payload(stream) {
            if (!stream.in_check_rem(2)) {
                LOG(LOG_ERR, "SlowPathClientInputEventPDU: data truncated (numEvents)");
                throw Error(ERR_RDP_SLOWPATH);
            }

            numEvents = stream.in_uint16_le();

            const unsigned expected =
                  2              // pad(2)
                + numEvents * 12 // time(4) + mes_type(2) + device_flags(2) + param1(2) + param2(2)
                ;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "SlowPathClientInputEventPDU: data truncated, expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_MCS_PDU_TRUNCATED);
            }

            stream.in_skip_bytes(2); // pad

            payload.resize(stream, numEvents * 12);
        } // ClientInputEventPDU_Recv(Stream & stream)
    }; // struct ClientInputEventPDU_Recv

} // namespace SlowPath

#endif // #ifndef _REDEMPTION_CORE_RDP_SLOWPATH_HPP_
