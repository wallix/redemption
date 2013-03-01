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

   Template for new XXX RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_OUTSEQUENCE_H_
#define _REDEMPTION_LIBS_RIO_OUTSEQUENCE_H_

#include "rio_constants.h"

struct RIOOutsequence {
    struct SQ * seq;
};

extern "C" {
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
             return rio_send(trans, data, len);
         }
         else {
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

    /* Set Timestamp for next chunk
       default : do nothing if the current file does not support timestamped chunks 
    */
    inline RIO_ERROR rio_m_RIOOutsequence_timestamp(RIOOutsequence * self, uint32_t tv_sec, uint32_t tv_usec)
    {
         return RIO_ERROR_OK;
    }

    /* Get Timestamp for current chunk
       tv_usec can be a NULL pointer. In this case usec won't be returned
       tv_sec is mandatory.
       default : do nothing if the current file does not support timestamped chunks 
    */
    inline RIO_ERROR rio_m_RIOOutsequence_get_timestamp(RIOOutsequence * self, uint32_t * tv_sec, uint32_t * tv_usec)
    {
         return RIO_ERROR_OK;
    }

    /* Set metadata for next chunk (when writing to transport) 
       this method can be called any number of times
       meta is some UTF-8 zero terminated string and can't be larger than 1024 bytes (arbitrary limit)
       Metadata can be added until the chunk is terminated (by calling next)
       default : do nothing if the current file does not support timestamped chunks
    */
    inline RIO_ERROR rio_m_RIOOutsequence_add_meta(RIOOutsequence * self, const char * meta)
    {
         return RIO_ERROR_OK;
    }

    /* Get metadata for current chunk (when reading from transport)
       this method can be called any number of time to get all metadata blocks relevant to current chunk.
       default : do nothing if the current file does not support timestamped chunks
    */
    inline RIO_ERROR rio_m_RIOOutsequence_get_meta(RIOOutsequence * self, char * meta)
    {
         return RIO_ERROR_OK;
    }
};

#endif

