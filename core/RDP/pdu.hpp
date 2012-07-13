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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Generic RDP PDU Broker

*/

#if !defined(__CORE_RDP_PDU_HPP__)
#define __CORE_RDP_ _HPP__

#include "stream.hpp"
#include "transport.hpp"
#include "x224.hpp"

namespace RDP { 
    class PDU
    {
        public:
        unsigned pduType;

        X224 x224;

        enum {
            UNKNOWN_PDU,
            CONNECTION_REQUEST,
            CONNECTION_CONFIRM,
            DISCONNECT_REQUEST,
            ERROR_TPDU,

            // below are several types of DATA TPDU
            MCS_CONNECT_INITIAL,
            MCS_CONNECT_RESPONSE,
            MCS_DISCONNECT_PROVIDER_ULTIMATUM,
            MCS_ERECT_DOMAIN_REQUEST,
            MCS_ATTACH_USER_REQUEST,
            MCS_ATTACH_USER_CONFIRM,
            MCS_CHANNEL_JOIN_REQUEST,
            MCS_CHANNEL_JOIN_CONFIRM,

            MCS_SEND_DATA_INDICATION,
            MCS_SEND_DATA_REQUEST,

        };

        PDU(Stream & stream) :
            pduType(UNKNOWN_PDU)
        {
        }
        void recv_begin(Transport * t) 
        {
            x224.recv_begin(t);
            switch (x224.tpdu_hdr.code){
            case X224::CR_TPDU:
                this->pduType = CONNECTION_REQUEST;
            break;
            case X224::CC_TPDU:
                this->pduType = CONNECTION_CONFIRM;
            break;
            case X224::DR_TPDU:
                this->pduType = DISCONNECT_REQUEST;
            break;
            case X224::ER_TPDU:
                this->pduType = ERROR_TPDU;
            break;
            default:
            break;
            }
        }
        void recv_end() {
            x224.recv_end();
        }
    };
};
#endif
