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
   Author(s): Christophe Grosjean

   Early Transport Protocol Security Negotiation stage

*/

#ifndef __CORE_RDP_NEGO_HPP__
#define __CORE_RDP_NEGO_HPP__

struct rdp_nego
{

    // Protocol Security Negotiation Protocols
    enum RDP_NEG_PROTOCOLS
    {
        PROTOCOL_RDP = 0x00000000,
        PROTOCOL_TLS = 0x00000001,
        PROTOCOL_NLA = 0x00000002
    };

//    int port;
    uint32_t flags;
//    char* hostname;
//    char* cookie;

    enum _NEGO_STATE
    {
        NEGO_STATE_INITIAL,
        NEGO_STATE_NLA,  // Network Level Authentication (TLS implicit)
        NEGO_STATE_TLS,  // TLS Encryption without NLA
        NEGO_STATE_RDP,  // Standard Legacy RDP Encryption
        NEGO_STATE_FAIL, // Negotiation failure */
        NEGO_STATE_FINAL
    } state;

//    int tcp_connected;
//    struct rdp_blob
//    {
//        void* data;
//        int length;
//    } * routing_token;
//    uint32_t selected_protocol;
    uint32_t requested_protocols;
//    uint8_t enabled_protocols[3];
//    Transport * transport;

    rdp_nego()
    : flags(0)
    , state(NEGO_STATE_INITIAL)
    , requested_protocols(PROTOCOL_RDP)
    {
    }
};


#endif
