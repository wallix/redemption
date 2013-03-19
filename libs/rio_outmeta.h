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

   Template for new Outmeta RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_OUTMETA_H_
#define _REDEMPTION_LIBS_RIO_OUTMETA_H_

#include "rio.h"

extern "C" {

    struct RIOOutmeta {
        int lastcount;
        struct RIO * meta;
        struct SQ * seq;
        struct RIO * out;
    };

    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOOutmeta_constructor(RIOOutmeta * self, SQ ** seq, const char * prefix, const char * extension, 
                                                  const char * header1, const char * header2, const char* header3, timeval * tv)
    {
        char buffer[1024];
        size_t res = snprintf(buffer, sizeof(buffer), "%s%s", prefix, extension);
        if (res >= sizeof(buffer)){
            return RIO_ERROR_FILENAME_TOO_LONG;
        }
        int fd = ::open(buffer, O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
        if (fd < 0){
            return RIO_ERROR_CREAT;
        }
        RIO_ERROR status = RIO_ERROR_OK;
        RIO * meta = rio_new_outfile(&status, fd);
        SQ * sequence = sq_new_outtracker(&status, meta, SQF_PREFIX_COUNT_EXTENSION, prefix, ".wrm", tv, header1, header2, header3);
        if (status != RIO_ERROR_OK){
            rio_delete(meta);
            return status;
        }
        RIO * out = rio_new_outsequence(&status, sequence);
        if (status != RIO_ERROR_OK){
            sq_delete(sequence);
            rio_delete(meta);
            return status;
        }

        self->lastcount = -1;
        *seq = self->seq = sequence;
        self->meta = meta;
        self->out = out;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOOutmeta_destructor(RIOOutmeta * self)
    {
        rio_delete(self->out);
        sq_delete(self->seq);
        rio_delete(self->meta);
        return RIO_ERROR_CLOSED;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOOutmeta_recv(RIOOutmeta * self, void * data, size_t len)
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
    inline ssize_t rio_m_RIOOutmeta_send(RIOOutmeta * self, const void * data, size_t len)
    {
        return rio_send(self->out, data, len);
    }
};

#endif

