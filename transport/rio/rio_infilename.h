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
   Author(s): Christophe Grosjean, Raphael Zhou

   New Infilename RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_INFILENAME_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_INFILENAME_H_

#include "rio.h"

extern "C" {
    /**********************
    * RIOInfilename *
    **********************/

    struct RIOInfilename {
        RIO *trans;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOInfilename_constructor(RIOInfilename * self, const CryptoContext * crypto_ctx, const char * filename)
    {
        RIO_ERROR error;
        self->trans = rio_new_crypto(&error, crypto_ctx, filename, O_RDONLY);
        return error;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOInfilename_destructor(RIOInfilename * self)
    {
        rio_delete(self->trans);
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOInfilename_sign(RIOInfilename * self, unsigned char * buf, size_t size, size_t * len) {
         rio_m_RIOInfilename_destructor(self);
         return RIO_ERROR_RECV_ONLY;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInfilename_recv(RIOInfilename * self, void * data, size_t len)
    {
        return rio_recv(self->trans, data, len);
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOInfilename_send(RIOInfilename * self, const void * data, size_t len)
    {
         rio_m_RIOInfilename_destructor(self);
         return -RIO_ERROR_RECV_ONLY;
    }

    static inline RIO_ERROR rio_m_RIOInfilename_seek(RIOInfilename * self, int64_t offset, int whence)
    {
        return RIO_ERROR_SEEK_NOT_AVAILABLE;
    }

    static inline RIO_ERROR rio_m_RIOInfilename_get_status(RIOInfilename * self)
    {
        // either OK, or error has already been intercepted
        return RIO_ERROR_OK;
    }
};

#endif

