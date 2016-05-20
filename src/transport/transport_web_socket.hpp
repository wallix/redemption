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



#include "transport/transport.hpp"


class TransportWebSocket :  public Transport
{
    void send(const char * const buffer, size_t len) 
    {
        EM_ASM_({ console.log('data_sent_to_serveur'); }, buffer, len);
    }
};


#endif