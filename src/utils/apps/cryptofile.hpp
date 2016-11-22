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


#pragma once

/* for HMAC calculations */
#define MD_HASH_FUNC   SHA256
#define MD_HASH_NAME   "SHA256"
#define MD_HASH_LENGTH SHA256_DIGEST_LENGTH

#include <libgen.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


#include "system/ssl_sha256.hpp"

#include "openssl_crypto.hpp"
//#include "openssl_evp.hpp"
#include "configs/config.hpp"

enum crypto_file_state {
    CF_READY = 0, // Crypto File Reading
    CF_EOF = 1,   // Crypto File reached EOF
    CF_INIT = 2,  // Crypto File nor yet initialised
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


#define CRYPTO_BUFFER_SIZE ((4096 * 4))

extern "C" {
    typedef int get_hmac_key_prototype(char * buffer);
    typedef int get_trace_key_prototype(char * base, int len, char * buffer, unsigned oldscheme);
}



/* 256 bits key size */
#define CRYPTO_KEY_LENGTH 32
#define HMAC_KEY_LENGTH   CRYPTO_KEY_LENGTH


class CryptoContext
{
    bool master_key_loaded;
    bool hmac_key_loaded;
    unsigned char master_key[CRYPTO_KEY_LENGTH];
    public:
    bool old_encryption_scheme;
    get_hmac_key_prototype * get_hmac_key_cb;
    get_trace_key_prototype * get_trace_key_cb;

    // TODO only for cfg::crypto::key0 and key1
    const Inifile & ini;
private:
    unsigned char hmac_key[HMAC_KEY_LENGTH];
    static_assert(cfg::crypto::key1::type().size() == HMAC_KEY_LENGTH, "");
public:

    auto get_hmac_key() -> unsigned char (&)[HMAC_KEY_LENGTH]
    {
        if (!this->hmac_key_loaded){
            if (this->get_hmac_key_cb != nullptr){
                // if we have a callback ask key
                this->get_hmac_key_cb(reinterpret_cast<char*>(this->hmac_key));
            }
            else {
                memcpy(this->hmac_key, ini.get<cfg::crypto::key1>().data(), sizeof(this->hmac_key));
            }
            this->hmac_key_loaded = true;
        }
        return this->hmac_key;
    }

    void get_derived_key(uint8_t (& trace_key)[CRYPTO_KEY_LENGTH], const uint8_t * derivator, size_t derivator_len)
    {
        if (this->old_encryption_scheme){
            //LOG(LOG_INFO, "old encryption scheme derivator %.*s", static_cast<unsigned>(derivator_len), derivator);
            if (this->get_trace_key_cb != nullptr){
                // if we have a callback ask key
                uint8_t tmp[SHA256_DIGEST_LENGTH];
                this->get_trace_key_cb(
                      reinterpret_cast<char*>(const_cast<uint8_t*>(derivator))
                    , static_cast<int>(derivator_len)
                    , reinterpret_cast<char*>(tmp)
                    , this->old_encryption_scheme?1:0
                    );
                memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
                return;
            }
        }

        //LOG(LOG_INFO, "new encryption scheme derivator %.*s", static_cast<unsigned>(derivator_len), derivator);
        if (!this->master_key_loaded){
            //LOG(LOG_INFO, "first call, loading master key");
            if (this->get_trace_key_cb != nullptr){
                // if we have a callback ask key
                this->get_trace_key_cb(
                      reinterpret_cast<char*>(const_cast<uint8_t*>(derivator))
                    , static_cast<int>(derivator_len)
                    , reinterpret_cast<char*>(this->master_key)
                    , this->old_encryption_scheme?1:0
                    );
                this->master_key_loaded = true;
            }
            else {
                memcpy(this->master_key, this->ini.get<cfg::crypto::key0>().data(), sizeof(this->master_key));
                this->master_key_loaded = true;
            }
        }

        uint8_t tmp[SHA256_DIGEST_LENGTH];
        {
            SslSha256 sha256;
            sha256.update(derivator, derivator_len);
            sha256.final(tmp);
        }
        {
            SslSha256 sha256;
            sha256.update(tmp, DERIVATOR_LENGTH);
            sha256.update(this->master_key, CRYPTO_KEY_LENGTH);
            sha256.final(tmp);
        }
        memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
    }

    void reset_mode()
    {
        this->master_key_loaded = false;
        this->hmac_key_loaded = false;
    }

    CryptoContext(const Inifile & ini)
	: master_key_loaded(false)
	, hmac_key_loaded(false)
	, master_key{}
	, old_encryption_scheme(false)
	, get_hmac_key_cb(nullptr)
	, get_trace_key_cb(nullptr)
	, ini(ini)
	, hmac_key{}
    {
        memcpy(this->master_key,
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08",
            CRYPTO_KEY_LENGTH);
        memcpy(this->hmac_key,
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08",
            HMAC_KEY_LENGTH);
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

        base64tbl[int('.')] = 62;
        base64tbl[int('-')] = 62;
        base64tbl[int('_')] = 63;

        while (*txt) {
            if ((v = base64tbl[static_cast<unsigned char>(*txt)]) >= 0) {
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

    void set_master_key(const uint8_t * key)
    {
        memcpy(this->master_key, key, sizeof(this->master_key));
        this->master_key_loaded = true;
    }

    void set_hmac_key(const uint8_t * key)
    {
        memcpy(this->hmac_key, key, sizeof(this->hmac_key));
        this->hmac_key_loaded = true;
    }

    void set_get_hmac_key_cb(get_hmac_key_prototype * get_hmac_key_cb)
    {
        this->get_hmac_key_cb = get_hmac_key_cb;
    }

    void set_get_trace_key_cb(get_trace_key_prototype * get_trace_key_cb)
    {
        this->get_trace_key_cb = get_trace_key_cb;
    }

    const unsigned char * get_master_key()
    {
        if (not this->master_key_loaded)
        {
            memcpy(this->master_key, this->ini.get<cfg::crypto::key0>().data(), sizeof(this->master_key));
            this->master_key_loaded = true;
        }
        return &(this->master_key[0]);
    }
};

