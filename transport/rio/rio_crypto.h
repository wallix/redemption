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

   new Crypto RedTransport class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_CRYPTO_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_CRYPTO_H_

#include "rio.h"

#define HASH_LEN 64

extern "C" {
    /* gl_crypto_key is a copy of the master key
     */
    char gl_crypto_key[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    TODO("this is a place holder implementation without cryptography...")
    struct RIOCrypto {
        int fd;
        RIO *trans;
        int oflag;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    static inline RIO_ERROR rio_m_RIOCrypto_constructor(RIOCrypto * self, const char * filename, int oflag) {
        RIO_ERROR error = RIO_ERROR_OK;
        int       _oflag;

        if (oflag & O_WRONLY) {
            _oflag = oflag | O_CREAT;
        }
        else {
            _oflag = oflag;
        }

        self->fd = ::open(filename, _oflag, S_IRUSR);
        if (self->fd < 0) {
            return RIO_ERROR_CREAT;
        }
        if (oflag & O_WRONLY) {
            self->trans = rio_new_outfile(&error, self->fd);
        }
        else {
            self->trans = rio_new_infile(&error, self->fd);
        }
        self->oflag = _oflag;
        return error;
    }

    /* This method deallocate any space used for subfields if any
    */
    static inline RIO_ERROR rio_m_RIOCrypto_destructor(RIOCrypto * self) {
        rio_delete(self->trans);
        int res = close(self->fd);
        if (res < 0){
            LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
            return RIO_ERROR_CLOSE_FAILED;
        }
        self->trans = NULL;
        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOCrypto_sign(RIOCrypto * self, unsigned char * buf, size_t size, size_t * len) {
        memset(buf, 1, (size>=HASH_LEN)?HASH_LEN:size);
        *len = (size>=HASH_LEN)?HASH_LEN:size;
        return RIO_ERROR_OK;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOCrypto_recv(RIOCrypto * self, void * data, size_t len) {
        if (self->oflag & O_WRONLY) {
            rio_m_RIOCrypto_destructor(self);
            return -RIO_ERROR_SEND_ONLY;
        }

        return rio_recv(self->trans, data, len);
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOCrypto_send(RIOCrypto * self, const void * data, size_t len) {
        if (!(self->oflag & O_WRONLY)) {
            rio_m_RIOCrypto_destructor(self);
            return -RIO_ERROR_RECV_ONLY;
        }

        return rio_send(self->trans, data, len);
    }

    static inline RIO_ERROR rio_m_RIOCrypto_seek(RIOCrypto * self, int64_t offset, int whence)
    {
        return RIO_ERROR_SEEK_NOT_AVAILABLE;
    }

    static inline RIO_ERROR rio_m_RIOCrypto_get_status(RIOCrypto * self) {
        return RIO_ERROR_OK;
    }
    
};

#endif // #ifndef _REDEMPTION_LIBS_RIO_CRYPTO_H_

