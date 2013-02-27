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

   Template for new Insequence RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RT_INSEQUENCE_H_
#define _REDEMPTION_LIBS_RT_INSEQUENCE_H_

#include "rt_constants.h"

struct RTInsequence {
    bool status;
    RT_ERROR err;
    struct SQ * seq;
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RT_ERROR rt_m_RTInsequence_constructor(RTInsequence * self, SQ * seq)
    {
        self->status = true;
        self->err = RT_ERROR_OK;
        self->seq = seq;
        return RT_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RT_ERROR rt_m_RTInsequence_destructor(RTInsequence * self)
    {
        return RT_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rt_m_RTInsequence_recv(RTInsequence * self, void * data, size_t len)
    {
        if (!self->status){ 
            if (self->err == RT_ERROR_EOF){
                return 0;
            }
            return -self->err; 
        }

        RT_ERROR status = RT_ERROR_OK;
        RT * trans = sq_get_trans(self->seq, &status);
        if (status != RT_ERROR_OK){
            return -status;
        }
        int remaining_len = len;
        while (remaining_len > 0){
            int res = rt_recv(trans, &((char*)data)[len-remaining_len], remaining_len);
            if (res == 0){
                RT_ERROR status = RT_ERROR_OK;
                sq_next(self->seq);
                trans = sq_get_trans(self->seq, &status);
                if (status != RT_ERROR_OK){ // this one is the last in the sequence (or other error ?)
                    self->status = false;
                    self->err = RT_ERROR_EOF; // next read will trigger EOF
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
    inline ssize_t rt_m_RTInsequence_send(RTInsequence * self, const void * data, size_t len)
    {
         return -RT_ERROR_RECV_ONLY;
    }

    /* This method flush current chunk and start a new one
       default: do nothing if the current file does not support chunking
    */
    inline RT_ERROR rt_m_RTInsequence_next(RTInsequence * self)
    {
         return RT_ERROR_OK;
    }

    /* Set Timestamp for next chunk
       default : do nothing if the current file does not support timestamped chunks 
    */
    inline RT_ERROR rt_m_RTInsequence_timestamp(RTInsequence * self, uint32_t tv_sec, uint32_t tv_usec)
    {
         return RT_ERROR_OK;
    }

    /* Get Timestamp for current chunk
       tv_usec can be a NULL pointer. In this case usec won't be returned
       tv_sec is mandatory.
       default : do nothing if the current file does not support timestamped chunks 
    */
    inline RT_ERROR rt_m_RTInsequence_get_timestamp(RTInsequence * self, uint32_t * tv_sec, uint32_t * tv_usec)
    {
         return RT_ERROR_OK;
    }

    /* Set metadata for next chunk (when writing to transport) 
       this method can be called any number of times
       meta is some UTF-8 zero terminated string and can't be larger than 1024 bytes (arbitrary limit)
       Metadata can be added until the chunk is terminated (by calling next)
       default : do nothing if the current file does not support timestamped chunks
    */
    inline RT_ERROR rt_m_RTInsequence_add_meta(RTInsequence * self, const char * meta)
    {
         return RT_ERROR_OK;
    }

    /* Get metadata for current chunk (when reading from transport)
       this method can be called any number of time to get all metadata blocks relevant to current chunk.
       default : do nothing if the current file does not support timestamped chunks
    */
    inline RT_ERROR rt_m_RTInsequence_get_meta(RTInsequence * self, char * meta)
    {
         return RT_ERROR_OK;
    }
};

#endif

