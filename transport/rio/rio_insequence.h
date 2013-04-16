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

   Template for new Insequence RedTransport class
*/

#ifndef _REDEMPTION_LIBS_RIO_INSEQUENCE_H_
#define _REDEMPTION_LIBS_RIO_INSEQUENCE_H_

#include "rio.h"

struct RIOInsequence {
    bool status;
    RIO_ERROR err;
    struct SQ * seq;
};

extern "C" {

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOInsequence_constructor(RIOInsequence * self, SQ * seq)
    {
        self->status = true;
        self->err = RIO_ERROR_OK;
        self->seq = seq;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOInsequence_destructor(RIOInsequence * self)
    {
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOInsequence_sign(RIOInsequence * self, unsigned char * buf, size_t size, size_t & len) {
        memset(buf, 0, size);
        len = 0;
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInsequence_recv(RIOInsequence * self, void * data, size_t len)
    {
        if (!self->status){ 
            if (self->err == RIO_ERROR_EOF){ return 0; }
            return -self->err; 
        }

        RIO_ERROR status = RIO_ERROR_OK;
        RIO * trans = sq_get_trans(self->seq, &status);
        if (status != RIO_ERROR_OK){
            return -status;
        }
        size_t remaining_len = len;
        while (remaining_len > 0){
            int res = rio_recv(trans, &((char*)data)[len-remaining_len], remaining_len);
            if (res == 0){
                RIO_ERROR status = RIO_ERROR_OK;
                sq_next(self->seq);
                trans = sq_get_trans(self->seq, &status);
                if (status != RIO_ERROR_OK){ // this one is the last in the sequence (or other error ?)
                    self->status = false;
                    self->err = RIO_ERROR_EOF; // next read will trigger EOF
                    return len - remaining_len;
                }
                continue;
            }
            if (res < 0){ 
                self->status = false;
                self->err = status;
                if (remaining_len != len) {
                    return len - remaining_len;
                }
                return -status; 
            }
            remaining_len -= res;
        }
        return len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInsequence_send(RIOInsequence * self, const void * data, size_t len)
    {
         return -RIO_ERROR_RECV_ONLY;
    }

    /* This method flush current chunk and start a new one
       default: do nothing if the current file does not support chunking
    */
    inline RIO_ERROR rio_m_RIOInsequence_next(RIOInsequence * self, timeval tv)
    {
         return RIO_ERROR_OUT_ONLY;
    }

    static inline RIO_ERROR rio_m_RIOInsequence_get_status(RIOInsequence * self)
    {
        if (!(self->status)) {
            return self->err;
        }
        return RIO_ERROR_OK;
    }
};

#endif

