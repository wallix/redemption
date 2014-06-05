/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_PUBLIC_TRANSPORT_CRYPTO_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_CRYPTO_TRANSPORT_HPP

#include "crypto_file.hpp"
#include "urandom_read.hpp"
#include "error.hpp"
#include "log.hpp"

#define HASH_LEN 64

namespace detail {
    bool noexcept_init_crypto(crypto_file & crypto, CryptoContext & ctx, int fd, const char * filename, bool is_read_mode)
    /*noexcept*/
    {
        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(filename, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (-1 == compute_hmac(trace_key, ctx.crypto_key, derivator)) {
            return false;
        }

        if (is_read_mode) {
            if (-1 == crypto.open_read_init(fd, trace_key, &ctx)) {
                return false;
            }
        }
        else {
            unsigned char iv[32];
            if (urandom_read(iv, 32) == -1) {
                LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
                return false;
            }

            if (-1 == crypto.open_write_init(fd, trace_key, &ctx, iv)) {
                return false;
            }
        }

        return true;
    }
}

bool noexcept_init_crypto_read(crypto_file & crypto, CryptoContext & ctx, int fd, const char * filename)
/*noexcept*/
{ return detail::noexcept_init_crypto(crypto, ctx, fd, filename, true); }

bool noexcept_init_crypto_write(crypto_file & crypto, CryptoContext & ctx, int fd, const char * filename)
/*noexcept*/
{ return detail::noexcept_init_crypto(crypto, ctx, fd, filename, false); }

void init_crypto_read(crypto_file & crypto, CryptoContext & ctx, int fd, const char * filename, int err)
{
    if ( ! detail::noexcept_init_crypto(crypto, ctx, fd, filename, true)) {
        throw Error(err, errno);
    }
}

void init_crypto_write(crypto_file & crypto, CryptoContext & ctx, int fd, const char * filename, int err)
{
    if ( ! detail::noexcept_init_crypto(crypto, ctx, fd, filename, false)) {
        throw Error(err, errno);
    }
}

#endif
