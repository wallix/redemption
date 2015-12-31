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

#ifndef REDEMPTION_TRANSPORT_CRYPTOFILE_HPP
#define REDEMPTION_TRANSPORT_CRYPTOFILE_HPP

#include <libgen.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>

#include "utils/genrandom.hpp"
#include "openssl_crypto.hpp"
#include "openssl_evp.hpp"

enum crypto_file_state {
    CF_EOF = 1
};

#define MIN(x, y)               (((x) > (y) ? (y) : (x)))
#define AES_BLOCK_SIZE          16
#define WABCRYPTOFILE_MAGIC     0x4D464357
#define WABCRYPTOFILE_EOF_MAGIC 0x5743464D
#define WABCRYPTOFILE_VERSION   0x00000001

enum {
    DERIVATOR_LENGTH = 8
};

/* size of salt to protect master key */
#define MKSALT_LEN 8

/* for HMAC calculations */
#define MD_HASH_FUNC   SHA256
#define MD_HASH_NAME   "SHA256"
#define MD_HASH_LENGTH SHA256_DIGEST_LENGTH

#define CRYPTO_BUFFER_SIZE ((4096 * 4))

/* 256 bits key size */
#define CRYPTO_KEY_LENGTH 32
#define HMAC_KEY_LENGTH   CRYPTO_KEY_LENGTH

struct CryptoContext {
    Random & gen;

    CryptoContext(Random & gen) : gen(gen) {}

    unsigned char hmac_key[HMAC_KEY_LENGTH];
    unsigned char crypto_key[CRYPTO_KEY_LENGTH];

    void get_derivator(const char *const_file, unsigned char * derivator, int derivator_len)
    {
         // generate key derivator as SHA256(basename)
        char * file = strdupa(const_file);
        char * file_basename = basename(file);
        char tmp_derivated[SHA256_DIGEST_LENGTH];
        if (SHA256((unsigned char *)file_basename, 
            strlen(file_basename), 
            (unsigned char *)tmp_derivated) == nullptr)
        {
            std::printf("[CRYPTO_ERROR][%d]: Could not derivate trace crypto key, SHA256 from=%s!\n", 
                getpid(), file_basename);
            return;
        }
        memcpy(derivator, tmp_derivated, MIN(derivator_len, SHA256_DIGEST_LENGTH));
    }

    void random(void * dest, size_t size) 
    {
        this->gen.random(dest, size);
    }

    size_t unbase64(char *buffer, size_t bufsiz, const char *txt)
    {
        const unsigned char _base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned int bits = 0;
        int nbits = 0;
        char base64tbl[256];
        char v;
        size_t nbytes = 0;

        memset(base64tbl, -1, sizeof base64tbl);

        for (unsigned i = 0; _base64chars[i]; i++) {
            base64tbl[_base64chars[i]] = i;
        }

        base64tbl['.'] = 62;
        base64tbl['-'] = 62;
        base64tbl['_'] = 63;

        while (*txt) {
            if ((v = base64tbl[(unsigned char)*txt]) >= 0) {
                bits <<= 6;
                bits += v;
                nbits += 6;
                if (nbits >= 8) {
                    if (nbytes < bufsiz)
                        *buffer++ = (bits >> (nbits - 8));
                    nbytes++;
                    nbits -= 8;
                }
            }
            txt++;
        }

        return nbytes;
    }

    int compute_hmac(unsigned char * hmac, const unsigned char * key, const unsigned char * derivator)
    {
        unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
        unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do

        memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
        memcpy(tmp_derivation + DERIVATOR_LENGTH, key, CRYPTO_KEY_LENGTH);
        if (SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated) == nullptr){
            std::printf("[CRYPTO_ERROR][%d]: Could not derivate hash crypto key, SHA256!\n", getpid());
            return -1;
        }
        memcpy(hmac, derivated, HMAC_KEY_LENGTH);
        return 0;
    }
};


#endif
