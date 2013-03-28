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
   Author(s): Christophe Grosjean

   Transport layer abstraction, socket implementation with TLS support
*/

#ifndef _REDEMPTION_TRANSPORT_SOCKETTRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_SOCKETTRANSPORT_HPP_

#include "transport.hpp"
#include "rio/rio.h"

class SocketTransport : public Transport {
    public:
        RIO rio;
        int sck;
        int sck_closed;
        const char * name;
        uint32_t verbose;

    SocketTransport(const char * name, int sck, uint32_t verbose)
        : Transport(), name(name), verbose(verbose)
    {
        RIO_ERROR res = rio_init_socket_tls(&this->rio, sck);
        this->sck = sck;
        this->sck_closed = 0;
        if (res != RIO_ERROR_OK){ 
            throw Error(ERR_TRANSPORT, 0);
        }
    }

    virtual ~SocketTransport(){
        rio_clear(&this->rio);        
        if (!this->sck_closed){
            this->disconnect();
        }
    }


    virtual void enable_tls() throw (Error)
    {
        rio_m_RIOSocketTLS_enable_TLS_client(&this->rio.u.socket_tls);
    }

    void disconnect(){
        LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
        if (this->rio.u.socket_tls.sck != 0) {
            shutdown(this->rio.u.socket_tls.sck, 2);
            close(this->rio.u.socket_tls.sck);
        }
        this->rio.u.socket_tls.sck = this->sck = 0;
        this->sck_closed = 1;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
//        if (this->verbose & 0x100){
//            LOG(LOG_INFO, "Socket %s (%u) receiving %u bytes", this->name, this->sck, total_len);
//        }

        ssize_t res = rio_recv(&this->rio, *pbuffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        *pbuffer += res;
        if ((size_t)res < len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }

//        if (this->verbose & 0x100){
//            LOG(LOG_INFO, "Recv done on %s (%u) %u bytes", this->name, this->sck, total_len);
//            hexdump_c(start, total_len);
//            LOG(LOG_INFO, "Dump done on %s (%u) %u bytes", this->name, this->sck, total_len);
//        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0) {
            throw Error(ERR_TRANSPORT_DIFFERS);
        }
        if (res < (ssize_t)len) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        return;
    }

    virtual bool get_status()
    {
        return rio_get_status(&this->rio) == RIO_ERROR_OK;
    }
};

#endif
