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


class TransportWebSocket :  public Transport
{
    char * buffer;

    void do_send(const char * const buffer, size_t len) override {
        EM_ASM_({ console.log('data_sent_to_serveur'); }, buffer, len);
    }

    void do_recv(char ** pbuffer, size_t len) override {
        //pbuffer[0...len] = read(len)

        this->buffer = *pbuffer;
        int i(0);
        int lenInt(len);
        if (lenInt > 0) {

            for (i = 0; i < lenInt; i++) {
                EM_ASM_({ getDataOctet($0); }, i);
            }

            pbuffer = &(this->buffer) + i;
        } else {
            EM_ASM_({ console.log('Trap len ' + %0); }, lenInt);
        }
    }

public:
    void setBufferValue(char value, int i) {
        this->buffer[i] = value;
    }

};


#endif