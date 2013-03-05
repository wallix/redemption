/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   new Generator RedTransport class
*/

#ifndef _REDEMPTION_LIBS_RIO_GENERATOR_H_
#define _REDEMPTION_LIBS_RIO_GENERATOR_H_

#include "rio_constants.h"

extern "C" {

    struct RIOGenerator {
        size_t current;
        uint8_t * data;
        size_t len;
        bool status;
        RIO_ERROR err;
    };

    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    static inline RIO_ERROR rio_m_RIOGenerator_constructor(RIOGenerator * self, const void * data, size_t len)
    {
        self->data = (uint8_t *)malloc(len);
        if (!self->data) { return RIO_ERROR_MALLOC; }
        self->len = len;
        self->current = 0;
        self->status = true;
        self->err = RIO_ERROR_OK;
        memcpy(self->data, data, len);
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    static inline RIO_ERROR rio_m_RIOGenerator_destructor(RIOGenerator * self)
    {
        free(self->data);
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOGenerator_recv(RIOGenerator * self, void * data, size_t len)
    {
        if (!self->status){ 
            if (self->err == RIO_ERROR_EOF){
                return 0;
            }
            return -self->err; 
        }
        if (self->current + len > self->len){
            size_t available_len = self->len - self->current;
            memcpy(data, (char*)self->data + self->current, available_len);
            self->current += available_len;
            self->status = false; // next read will trigger EOF
            self->err = RIO_ERROR_EOF;
            return available_len;
        }
        memcpy(data, (char*)self->data + self->current, len);
        self->current += len;
        return len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOGenerator_send(RIOGenerator * self, const void * data, size_t len)
    {
         self->status = false;
         self->err = RIO_ERROR_RECV_ONLY;
         return -self->err;
    }


};

#endif

