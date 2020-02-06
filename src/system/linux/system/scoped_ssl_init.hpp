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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>


struct ScopedSslInit
{
    ScopedSslInit()
    {
        // init TLS

        // -------- Start of system wide SSL_Ctx option ------------------------------

        // ERR_load_crypto_strings() registers the error strings for all libcrypto
        // functions. SSL_load_error_strings() does the same, but also registers the
        // libssl error strings.

        // One of these functions should be called before generating textual error
        // messages. However, this is not required when memory usage is an issue.

        // ERR_free_strings() frees all previously loaded error strings.

        SSL_load_error_strings();

        // SSL_library_init() registers the available SSL/TLS ciphers and digests.
        // OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms
        // for SSL_library_init().

        // - SSL_library_init() must be called before any other action takes place.
        // - SSL_library_init() is not reentrant.
        // - SSL_library_init() always returns "1", so it is safe to discard the return
        // value.

        // Note: OpenSSL 0.9.8o and 1.0.0a and later added SHA2 algorithms to
        // SSL_library_init(). Applications which need to use SHA2 in earlier versions
        // of OpenSSL should call OpenSSL_add_all_algorithms() as well.

        SSL_library_init();
    }

    ~ScopedSslInit()
    {
        ERR_free_strings();
    }
};
