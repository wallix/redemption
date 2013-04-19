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

   Template for new Outmeta RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_CRYPTOOUTMETA_H_
#define _REDEMPTION_TRANSPORT_RIO_CRYPTOOUTMETA_H_

#include "rio.h"

extern "C" {

    /*******************
    * RIOCryptoOutmeta *
    *******************/

    struct RIOCryptoOutmeta {
        int lastcount;
        struct RIO * meta;
        struct SQ * seq;
        struct RIO * out;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOCryptoOutmeta_constructor(RIOCryptoOutmeta * self, SQ ** seq,
                                                  const char * path, const char * filename, const char * extension,
                                                  const char * header1, const char * header2, const char* header3, timeval * tv,
                                                  const int groupid)
    {
        TODO("use system constants for size")
        char buffer[2048];
        size_t res = snprintf(buffer, sizeof(buffer), "%s%s%s", path, filename, extension);
        if (res >= sizeof(buffer)){
            return RIO_ERROR_FILENAME_TOO_LONG;
        }
        RIO_ERROR status = RIO_ERROR_OK;
        RIO * meta = rio_new_crypto(&status, buffer, O_WRONLY);
        SQ * sequence = sq_new_cryptoouttracker(&status, meta, SQF_PATH_FILE_COUNT_EXTENSION, path, filename, ".wrm", tv, header1, header2, header3, groupid);
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
    inline RIO_ERROR rio_m_RIOCryptoOutmeta_destructor(RIOCryptoOutmeta * self)
    {
        rio_delete(self->out);
        sq_delete(self->seq);
        rio_delete(self->meta);
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOCryptoOutmeta_sign(RIOCryptoOutmeta * self, unsigned char * buf, size_t size, size_t & len) {
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
    inline ssize_t rio_m_RIOCryptoOutmeta_recv(RIOCryptoOutmeta * self, void * data, size_t len)
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
    inline ssize_t rio_m_RIOCryptoOutmeta_send(RIOCryptoOutmeta * self, const void * data, size_t len)
    {
        return rio_send(self->out, data, len);
    }

    static inline RIO_ERROR rio_m_RIOCryptoOutmeta_get_status(RIOCryptoOutmeta * self)
    {
        return rio_get_status(self->out);
    }
};

#endif

