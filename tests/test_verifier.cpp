/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT

#include <fcntl.h>

#include <iostream>

#include "ssl_calls.hpp"
#include "utils/apps/app_verifier.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>

#include <new>

#include "fdbuf.hpp"
#include "filter/crypto_filter.hpp"

#include "cryptofile.hpp"

extern "C" {

struct crypto_file;

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

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

int libc_read(int fd, char *buf, unsigned int count)
{
   return read(fd, buf, count);
}   /* int libc_read(int fd, char *buf, unsigned int count) */

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
    const char * test_file_name = "./TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/
    LCGRandom rnd(0);

    CryptoContext cctx(rnd);
    uint8_t crypto_key[sizeof(CryptoContext::crypto_key)] = {
         0,  1,  2,  3,  4,  5,  6,  7,
         8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31
    };
    uint8_t hmac_key[sizeof(CryptoContext::hmac_key)] = {};
    memcpy(cctx.crypto_key, crypto_key, sizeof(CryptoContext::crypto_key));
    memcpy(cctx.hmac_key, hmac_key, sizeof(CryptoContext::hmac_key));


    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    BOOST_CHECK(0 == compute_hmac(hmac_key, crypto_key, HASH_DERIVATOR));
    OpenSSL_add_all_digests();

    // Any iv key would do, we are checking round trip
    unsigned char iv[32] = {
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
    };

    unsigned char derivator[DERIVATOR_LENGTH];
    cctx.get_derivator(test_file_name, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    if (compute_hmac(trace_key, crypto_key, derivator) == -1){
        BOOST_CHECK(false);
    }

    int system_fd = open(test_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", test_file_name);
        BOOST_CHECK(false);
    }

    crypto_file * cf_struct = crypto_open_write(system_fd, trace_key, &cctx, iv);
    if (!cf_struct){
        close(system_fd);
    }

    BOOST_CHECK(cf_struct);

    const char    * data     = "Indentation Settings determine the size of the tab stops, "
                               "and control whether the tab key should insert tabs or spaces.";
    const size_t    data_len = strlen(data);
    int             res;
    unsigned char   hash[HASH_LEN];


    for (int i = 0; i < 256; i ++) {
        res = crypto_write(cf_struct, const_cast<char *>(data), data_len);

        BOOST_CHECK_EQUAL(data_len, res);
    }

    res = crypto_close(cf_struct, hash, cctx.hmac_key);

    BOOST_CHECK_EQUAL(0, res);

    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.hmac_key, sizeof(cctx.hmac_key),
                                                   hash, HASH_LEN / 2, 4096));
    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.hmac_key, sizeof(cctx.hmac_key),
                                                   hash + (HASH_LEN / 2), HASH_LEN / 2, 0));

    unlink(test_file_name);
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */
