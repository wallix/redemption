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

   Transport layer abstraction
*/

#ifndef _REDEMPTION_TRANSPORT_COUNTTRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_COUNTTRANSPORT_HPP_

#include "transport.hpp"

class CountTransport : public Transport {
    public:

    CountTransport() {}

    ~CountTransport() {}

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        TODO("move that to base class : accounting_recv(len) (or base class recv could just do accounting)");
        this->total_received += len;
        this->last_quantum_received += len;
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        TODO("move that to base class : accounting_send(len) (or base class send could just do accounting)");
        this->total_sent += len;
        this->last_quantum_sent += len;
    }

    virtual bool get_status()
    {
        return true;
    }
};

#endif

