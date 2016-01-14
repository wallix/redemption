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
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>

#include "utils/genrandom.hpp"
#include "openssl_crypto.hpp"
#include "openssl_evp.hpp"
#include "core/config.hpp"

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

class CryptoContext {
    private:
    bool crypto_key_loaded;
    unsigned char crypto_key[CRYPTO_KEY_LENGTH];

    public:
    Random & gen;
    const Inifile & ini;
    int key_source; // 0: key from shm, 1: key from Ini file, 2: key in place
    private:
    unsigned char hmac_key[HMAC_KEY_LENGTH];
    public:

    auto get_hmac_key() -> unsigned char (&)[HMAC_KEY_LENGTH]
    {
        return hmac_key;
    }

    void reset_mode(int key_source)
    {
        this->key_source = key_source;
        this->crypto_key_loaded = false;
    }

    CryptoContext(Random & gen, const Inifile & ini, int key_source) 
        : crypto_key_loaded(false)
        , crypto_key{}
        , gen(gen)
        , ini(ini)
        , key_source(key_source)
        , hmac_key{}
        {
            memcpy(this->crypto_key,
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

    int get_crypto_key_from_shm()
    {
        char tmp_buf[512] = {0};
        int shmid = shmget(2242, 512, 0600);
        if (shmid == -1){
            printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmget! error=%s\n", getpid(), strerror(errno));
            return 1;
        }
        char *shm = (char*)shmat(shmid, nullptr, 0);
        if (shm == (char *)-1){
            printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmat! error=%s\n", getpid(), strerror(errno));
            return 1;
        }
        this->unbase64(tmp_buf, 512, shm);
        if (shmdt(shm) == -1){
            printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmdt! error=%s\n", getpid(), strerror(errno));
            return 1;
        }

        /* Extract the effective master key component, and check its control signature.
         * Returns 0 on success, -1 on error.
         */
        char sha256_computed[SHA256_DIGEST_LENGTH];

        if (SHA256((unsigned char *)(tmp_buf + SHA256_DIGEST_LENGTH+1),
            MKSALT_LEN+CRYPTO_KEY_LENGTH, (unsigned char *)sha256_computed) == nullptr)
        {
            printf("[CRYPTO_ERROR][%d]: Could not check crypto key, SHA256!\n", getpid());
            return 1;
        }

        if (strncmp(tmp_buf + 1, sha256_computed, SHA256_DIGEST_LENGTH)){
            printf("[CRYPTO_ERROR][%d]: Crypto key integrity check failed!\n", getpid());
            return 1;
        }
        memcpy(this->crypto_key, tmp_buf + SHA256_DIGEST_LENGTH+MKSALT_LEN+1, CRYPTO_KEY_LENGTH);
        this->crypto_key_loaded = true;
        return 0;
    }

    int get_crypto_key_from_ini()
    {
        memcpy(this->crypto_key, this->ini.get<cfg::crypto::key0>(), sizeof(this->crypto_key));
        memcpy(this->hmac_key, ini.get<cfg::crypto::key1>(), sizeof(this->hmac_key));
        return 0;
    }

    int get_crypto_key_from_ini_derivated_hmac()
    {
        memcpy(this->crypto_key, this->ini.get<cfg::crypto::key0>(), sizeof(this->crypto_key));
        this->crypto_key_loaded = true;
        const unsigned char HASH_DERIVATOR[] = {
             0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b
        };                
        this->compute_hmac(this->hmac_key, HASH_DERIVATOR);
        return 0;
    }

    void set_crypto_key(const char * key)
    {
        memcpy(this->crypto_key, key, sizeof(this->crypto_key));
    }

    void set_hmac_key(const char * key)
    {
        memcpy(this->hmac_key, key, sizeof(this->hmac_key));
    }

    const unsigned char * get_crypto_key()
    {
        if (not this->crypto_key_loaded)
        {
            switch (key_source){
            case 0:
            {
                this->get_crypto_key_from_shm();
                const unsigned char HASH_DERIVATOR[] = {
                     0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b
                };                
                this->compute_hmac(this->hmac_key, HASH_DERIVATOR);
            }
            break;
            case 1:
                this->get_crypto_key_from_ini();
            break;
            case 2:
                this->get_crypto_key_from_ini_derivated_hmac();
            break;
            default:
            {
                LOG(LOG_ERR, "Failed to get cryptographic key, using default key\n");
//                "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
//                "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
            }
            }
            this->crypto_key_loaded = true;
        }
        return &(this->crypto_key[0]);
    }

    int compute_hmac(unsigned char * hmac, const unsigned char * derivator)
    {
        unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
        unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do

        memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
        memcpy(tmp_derivation + DERIVATOR_LENGTH, this->get_crypto_key(), CRYPTO_KEY_LENGTH);
        if (SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated) == nullptr){
            std::printf("[CRYPTO_ERROR][%d]: Could not derivate hash crypto key, SHA256!\n", getpid());
            return -1;
        }
        memcpy(hmac, derivated, HMAC_KEY_LENGTH);
        return 0;
    }
};

#endif
