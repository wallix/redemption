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

   Main entry point file for RIO *Transport library
*/

#include <new>

#include "fdbuf.hpp"
#include "filter/crypto_filter.hpp"

extern "C" {

#include "ccryptofile.h"

/**********************************************
 *                Public API                  *
 **********************************************/

enum Priv_crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

class Priv_crypto_type_base
{
    Priv_crypto_type type;

public:
    Priv_crypto_type_base(Priv_crypto_type t)
    : type(t)
    {}

    bool is_encrypt() const
    { return CRYPTO_ENCRYPT_TYPE == type; }

    bool is_decrypt() const
    { return CRYPTO_DECRYPT_TYPE == type; }
};

struct Priv_crypto_file_decrypt
: Priv_crypto_type_base
{
  transfil::decrypt_filter decrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_decrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_DECRYPT_TYPE)
  , file(fd)
  {}
};

struct Priv_crypto_file_encrypt
: Priv_crypto_type_base
{
  transfil::encrypt_filter encrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_encrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_ENCRYPT_TYPE)
  , file(fd)
  {}
};


crypto_file * crypto_open_read(int systemfd, unsigned char * trace_key,  CryptoContext * cctx)
{
    (void)cctx;
    Priv_crypto_file_decrypt * cf_struct = new (std::nothrow) Priv_crypto_file_decrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->decrypt.open(cf_struct->file, trace_key)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

crypto_file * crypto_open_write(int systemfd, unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
{
    Priv_crypto_file_encrypt * cf_struct = new (std::nothrow) Priv_crypto_file_encrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, cctx, iv)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

/* Flush procedure (compression, encryption, effective file writing)
 * Return 0 on success, -1 on error
 */
int crypto_flush(crypto_file * cf)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.flush(cf_struct->file);
}

/* The actual read method. Read chunks until we reach requested size.
 * Return the actual size read into buf, -1 on error
 */
int crypto_read(crypto_file * cf, char * buf, unsigned int buf_size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        return cf_struct->decrypt.read(cf_struct->file, buf, buf_size);
    }
    return -1;
}

/* Actually appends data to crypto_file buffer, flush if buffer gets full
 * Return the written size, -1 on error
 */
int crypto_write(crypto_file *cf, const char * buf, unsigned int size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.write(cf_struct->file, buf, size);
}

int crypto_close(crypto_file *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
{
    int nResult = 0;

    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        delete cf_struct;
    }
    else {
        Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
        nResult = cf_struct->encrypt.close(cf_struct->file, hash, hmac_key);
        delete cf_struct;
    }

    return nResult;
}

} // extern "C"
