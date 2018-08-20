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

#include <openssl/evp.h>
#include <openssl/err.h>

struct ScopedCryptoInit
{
    ScopedCryptoInit()
    {
        OpenSSL_add_all_digests();
    }

    ~ScopedCryptoInit()
    {
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();
#if OPENSSL_VERSION_NUMBER < 0x10000000L
        ERR_remove_state(0);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_remove_thread_state(nullptr);
#endif
    }
};
