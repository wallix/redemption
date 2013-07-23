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

   Template for new OutSequence RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_OUTSEQUENCE_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_OUTSEQUENCE_H_

#include "rio.h"

extern "C" {
    struct RIOOutsequence {
        struct SQ * seq;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOOutsequence_constructor(RIOOutsequence * self, SQ * seq)
    {
        self->seq = seq;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOOutsequence_destructor(RIOOutsequence * self)
    {
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOOutsequence_sign(RIOOutsequence * self, unsigned char * buf, size_t size, size_t * len) {
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
    inline ssize_t rio_m_RIOOutsequence_recv(RIOOutsequence * self, void * data, size_t len)
    {
         rio_m_RIOOutsequence_destructor(self);
         return -RIO_ERROR_SEND_ONLY;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOOutsequence_send(RIOOutsequence * self, const void * data, size_t len)
    {
         RIO_ERROR status = RIO_ERROR_OK;
         RIO * trans = sq_get_trans(self->seq, &status);
         if (status == RIO_ERROR_OK){
             ssize_t res = rio_send(trans, data, len);
             if (res < 0){
                rio_m_RIOOutsequence_destructor(self);
             }
             return res;
         }
         else {
            rio_m_RIOOutsequence_destructor(self);
            return -status;
         }
    }

    /* This method flush current chunk and start a new one
       default: do nothing if the current file does not support chunking
    */
    inline RIO_ERROR rio_m_RIOOutsequence_next(RIOOutsequence * self)
    {
         return RIO_ERROR_OK;
    }

    static inline RIO_ERROR rio_m_RIOOutsequence_seek(RIOOutsequence * self, int64_t offset, int whence)
    {
        return RIO_ERROR_SEEK_NOT_AVAILABLE;
    }

    static inline RIO_ERROR rio_m_RIOOutsequence_get_status(RIOOutsequence * self)
    {
        return RIO_ERROR_OK;
    }
};

#endif

