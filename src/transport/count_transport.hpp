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


#pragma once

#include "transport/transport.hpp"

class CountTransport
: public Transport
{
    void do_recv(uint8_t **, size_t len) override {
        // TODO move that to base class : accounting_recv(len) (or base class recv could just do accounting)
        this->last_quantum_received += len;
    }

    void do_send(const uint8_t * const, size_t len) override {
        // TODO move that to base class : accounting_send(len) (or base class send could just do accounting)
        this->last_quantum_sent += len;
    }
};

