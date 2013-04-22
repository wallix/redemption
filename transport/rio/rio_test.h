/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   new Test RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_TEST_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_TEST_H_

#include "rio.h"

struct RIOTest {
    RIO * check;
    RIO * generator;
};

extern "C" {
    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOTest_constructor(RIOTest * self,
                                            const void * data_check, size_t len_check,
                                            const void * data_gen, size_t len_gen)
    {
        RIO_ERROR status = RIO_ERROR_OK;
        self->check = rio_new_check(&status, data_check, len_check);
        if (status != RIO_ERROR_OK){
            return status;
        }
        self->generator = rio_new_generator(&status, data_gen, len_gen);
        if (status != RIO_ERROR_OK){
            rio_delete(self->check);
            return status;
        }
        return status;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOTest_destructor(RIOTest * self)
    {
        rio_delete(self->check);
        rio_delete(self->generator);
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOTest_sign(RIOTest * self, unsigned char * buf, size_t size, size_t * len) {
        memset(buf, 0, (size>=32)?32:size);
        *len = (size>=32)?32:size;
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOTest_recv(RIOTest * self, void * data, size_t len)
    {
        RIO_ERROR err_gen = rio_get_status(self->generator);
        if (err_gen != RIO_ERROR_OK){
            rio_m_RIOTest_destructor(self);
            return -err_gen;
        }
        RIO_ERROR err_check = rio_get_status(self->check);
        if (err_check != RIO_ERROR_OK){
            rio_m_RIOTest_destructor(self);
            return -err_check;
        }
        ssize_t res = rio_recv(self->generator, data, len);
        if (res < 0){
            rio_m_RIOTest_destructor(self);
            return res;
        }
        return res;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOTest_send(RIOTest * self, const void * data, size_t len)
    {
        RIO_ERROR err_gen = rio_get_status(self->generator);
        if (err_gen != RIO_ERROR_OK){
            rio_m_RIOTest_destructor(self);
            return -err_gen;
        }
        RIO_ERROR err_check = rio_get_status(self->check);
        if (err_check != RIO_ERROR_OK){
            rio_m_RIOTest_destructor(self);
            return -err_check;
        }
        return rio_send(self->check, data, len);
    }

    static inline RIO_ERROR rio_m_RIOTest_get_status(RIOTest * self)
    {
        return rio_get_status(self->check);
    }
};

#endif

