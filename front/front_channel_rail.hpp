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

#ifndef _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_
#define _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_

void Front::process_rail_event(Callback & cb, uint8_t const * chunk, size_t chunk_size, uint16_t length, uint16_t flags) {
    StaticStream stream(chunk, chunk_size);

    uint16_t orderType   = stream.in_uint16_le();
    uint16_t orderLength = stream.in_uint16_le();

    LOG(LOG_INFO, "Front::process_rail_event: orderType=%u orderLength=%u", orderType, orderLength);
    hexdump_d(stream.get_data(), stream.size());

    if (orderType == TS_RAIL_ORDER_CLIENTSTATUS) {
        ClientInformationPDU_Recv cipdur(stream);

        LOG(LOG_INFO, "Front::process_rail_event: Client Information PDU Flags=0x%08X", cipdur.Flags());
    }
    else if (orderType == TS_RAIL_ORDER_HANDSHAKE) {
        HandshakePDU_Recv hpdur(stream);

        LOG(LOG_INFO, "Front::process_rail_event: Handshake PDU buildNumber=%u", hpdur.buildNumber());
    }
    else {
        stream.rewind();

        cb.send_to_mod_channel(channel_names::rail, stream, length, flags);
    }
}

#endif  // #ifndef _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_
