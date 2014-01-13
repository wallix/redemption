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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio/rio.h"

#define HASH_LEN 64

#include "rio/cryptofile.hpp"

extern "C" {

    struct RIOCrypto {
        void * cf_struct;
        CryptoContext * crypto_ctx;
    };

    /* This method does not allocate space for object itself,
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    static inline RIO_ERROR rio_m_RIOCrypto_constructor(RIOCrypto * self, CryptoContext * crypto_ctx, const char * file, int oflag) {
        self->crypto_ctx = crypto_ctx;

        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(file, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (compute_hmac(trace_key, self->crypto_ctx->crypto_key, derivator) == -1){
            return RIO_ERROR_OPEN;
        }

        if (oflag != O_RDONLY){
            unsigned char iv[32];
            if (dev_urandom_read(iv, 32) == -1){
                printf("iv randomization failed for crypto file=%s\n", file);
                return RIO_ERROR_OPEN;
            }

            printf("crypto open write... file=%s oflag=%u\n", file, oflag);
            int system_fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
            if (system_fd == -1){
                printf("failed opening=%s\n", file);
                return RIO_ERROR_OPEN;
            }

            self->cf_struct = crypto_open_write(system_fd, trace_key, self->crypto_ctx, iv);
            if (!self->cf_struct) {
                close(system_fd);
                printf("open failed for crypto file=%s\n", file);
                return RIO_ERROR_OPEN;
            }
        }
        else {
            printf("crypto open read... file=%s\n", file);

            int system_fd = open(file, O_RDONLY, 0600);
            if (system_fd == -1){
                printf("failed opening=%s\n", file);
                return RIO_ERROR_OPEN;
            }

            self->cf_struct = crypto_open_read(system_fd, trace_key, self->crypto_ctx);
            if (!self->cf_struct) {
                close(system_fd);
                printf("open failed for crypto file=%s\n", file);
                return RIO_ERROR_OPEN;
            }
        }

        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    static inline RIO_ERROR rio_m_RIOCrypto_destructor(RIOCrypto * self) {
        unsigned char hash[HASH_LEN];

        int error = crypto_close(self->cf_struct, hash, self->crypto_ctx->hmac_key);
        if (error == -1) { return RIO_ERROR_CLOSE_FAILED; }

        self->cf_struct = NULL;

        return RIO_ERROR_CLOSED;
    }

    /* This method return a signature based on the data written
    */
    static inline RIO_ERROR rio_m_RIOCrypto_sign(RIOCrypto * self, unsigned char * buf, size_t size, size_t * len) {
        memset(buf, 0, size);
        *len = HASH_LEN;
        if (size < HASH_LEN) {
            rio_m_RIOCrypto_destructor(self);
            return RIO_ERROR_MORE_DATA;
        }

        int error = crypto_close(self->cf_struct, buf, self->crypto_ctx->hmac_key);
        if (error == -1) { return RIO_ERROR_CLOSE_FAILED; }

        self->cf_struct = NULL;

        return RIO_ERROR_CLOSED;
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data the amount read will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOCrypto_recv(RIOCrypto * self, void * data, size_t len) {
        ssize_t res = crypto_read(self->cf_struct, (char *)data, len);
        if (res == -1) {
            rio_m_RIOCrypto_destructor(self);
            return -RIO_ERROR_ANY;
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
    static inline ssize_t rio_m_RIOCrypto_send(RIOCrypto * self, const void * data, size_t len) {
        ssize_t res = crypto_write(self->cf_struct, (char *)data, len);
        if (res != (ssize_t)len) {
            rio_m_RIOCrypto_destructor(self);
            return -RIO_ERROR_ANY;
        }

        return res;
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

