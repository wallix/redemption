/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Clement Moroldo

   Transport layer abstraction
*/


#ifndef TRANSPORT_WEB_SOCKET_HPP
#define TRANSPORT_WEB_SOCKET_HPP


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "transport/transport.hpp"
#include "mod/rdp/rdp.hpp"



class TransportWebSocket :  public Transport
{
    uint8_t * buffer;
    uint8_t   pduState = 0;
    size_t    size = 0;
    size_t    pduSize = 0;

    mod_rdp * callback;

    enum : uint8_t {
        PDU_HEADER_FLAG = 0x03,
        PDU_HEADER_SIZE = 4
    };

    enum : uint8_t {
        PUD_HEADER_EMPTY,
        PUD_HEADER_OCT_1,
        PUD_HEADER_OCT_2,
        PUD_HEADER_OCT_3,
        PUD_HEADER_OCT_4,
    };
    

    void setMod(mod_rdp * mod) {
        this->callback = mod;
    }

    void do_send(const char * const buffer, size_t len) override {
        EM_ASM_({ send_to_serveur(HEAPU8.subarray($0, $0 + $1 - 1), $1); }, buffer, len);
    }

    void do_recv(char ** pbuffer, size_t len) override {
        //pbuffer[0...len] = read(len)

        this->buffer = *pbuffer;
        int lenInt(len);
        if (lenInt > 0) {

            for (this->index = 0; this->index < lenInt; this->index++) {
                //EM_ASM_({ var funct = getDataOctet(); funct.next(); }, 0);
                EM_ASM_({ getDataOctet(); }, 0);
            }

            *pbuffer = this->buffer + index;
        } else {
            EM_ASM_({ console.log('No input data from WebSocket'); }, 0);
        }
    }

public:
    void setBufferValue(uint8_t octet) {

        switch (this->pduState) {
            case PUD_HEADER_EMPTY: if (octet == PDU_HEADER_FLAG) {
                                        this->pduState += PUD_HEADER_OCT_1;
                                   }
                break;

            case PUD_HEADER_OCT_1: this->pduState += PUD_HEADER_OCT_2;
                break;

            case PUD_HEADER_OCT_2: this->pduSize += octet << 8;
                                   this->pduState += PUD_HEADER_OCT_3;
                break;

            case PUD_HEADER_OCT_3: this->pduSize += octet;

                                   this->buffer = new uint8_t[this->pduSize + PDU_HEADER_SIZE];
                                   this->buffer[0] = PDU_HEADER_FLAG;
                                   this->buffer[1] = 0x00;  // reserved for further study
                                   this->buffer[2] = this->pduSize >> 8;
                                   this->buffer[3] = this->pduSize;

                                   this->pduSize += PDU_HEADER_SIZE;
                                   this->size = PDU_HEADER_SIZE;
                                   this->pduState += PUD_HEADER_OCT_4;
                break;

            case PUD_HEADER_OCT_4: buffer[size] = octet;
                                   size++;
                                   if (this->size == this->pduSize) {
                                       this->pduState += PUD_HEADER_EMPTY;
                                       pduSize = 0;
                                       this->callback->draw_event(time_t(nullptr));
                                   }
                break;

            default:
                break;
        }
    }


};



#endif