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

#ifndef _REDEMPTION_TRANSPORT_TESTTRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_TESTTRANSPORT_HPP_

#include "transport.hpp"

class GeneratorTransport : public Transport {

    public:
    RIO rio;


    GeneratorTransport(const char * data, size_t len)
        : Transport()
    {
        RIO_ERROR status = rio_init_generator(&this->rio, data, len);
        if (status != RIO_ERROR_OK){ 
            throw Error(ERR_TRANSPORT, 0);
        }
    }

    ~GeneratorTransport()
    {
        rio_clear(&this->rio);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        ssize_t res = rio_recv(&this->rio, *pbuffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        *pbuffer += res;
        if ((size_t)res < len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        // send perform like a /dev/null and does nothing in generator transport
    }

    virtual bool get_status()
    {
        return rio_get_status(&this->rio) == RIO_ERROR_OK;
    }

};

class CheckTransport : public Transport {

    public:
    RIO rio;

    CheckTransport(const char * data, size_t len, uint32_t verbose = 0)
        : Transport()
    {
        RIO_ERROR res = rio_init_check(&this->rio, data, len);
        if (res != RIO_ERROR_OK){ 
            throw Error(ERR_TRANSPORT, 0);
        }
    }

    ~CheckTransport()
    {
        rio_clear(&this->rio);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        // CheckTransport does never receive anything
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
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

class TestTransport : public Transport {

    public:
    RIO rio;
    bool status;

    TestTransport(const char * name, const char * outdata, size_t outlen, const char * indata, size_t inlen, uint32_t verbose = 0)
    {
        RIO_ERROR res = rio_init_test(&this->rio, indata, inlen, outdata, outlen);
        if (res != RIO_ERROR_OK){ 
            this->status = false;
            throw Error(ERR_TRANSPORT, 0);
        }
    }

    ~TestTransport()
    {
        rio_clear(&this->rio);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        ssize_t res = rio_recv(&this->rio, *pbuffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        *pbuffer += res;
        if ((size_t)res < len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
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

