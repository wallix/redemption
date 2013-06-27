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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

   openssl headers

   Based on xrdp and rdesktop
   Copyright (C) Jay Sorg 2004-2010
   Copyright (C) Matthew Chapman 1999-2007
*/

#ifndef _REDEMPTION_UTILS_SSL_CALLS_HPP_
#define _REDEMPTION_UTILS_SSL_CALLS_HPP_
#include "bitfu.hpp"

#include <stdint.h>
#include <stdlib.h> /* needed for openssl headers */
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

#include <string.h>
#include "constants.hpp"
#include "log.hpp"
#include "stream.hpp"

enum {
    SEC_RANDOM_SIZE   = 32,
    SEC_MODULUS_SIZE  = 64,
    SEC_MAX_MODULUS_SIZE  = 256,
    SEC_PADDING_SIZE  =  8,
    SEC_EXPONENT_SIZE =  4
};

class SslSha1
{
    SHA_CTX sha1;

    public:
    SslSha1()
    {
        SHA1_Init(&this->sha1);
    }

    void update(const Stream & stream)
    {
        SHA1_Update(&this->sha1, stream.get_data(), stream.size());
    }

    void final(uint8_t * out_data)
    {
        SHA1_Final(out_data, &this->sha1);
    }
};

class SslMd5
{
    MD5_CTX md5;

    public:
    SslMd5()
    {
        MD5_Init(&this->md5);
    }

    void update(const Stream & stream)
    {
        MD5_Update(&this->md5, stream.get_data(), stream.size());
    }

    void final(uint8_t * out_data)
    {
        MD5_Final(out_data, &this->md5);
    }
};

class SslRC4
{
    RC4_KEY rc4;

    public:
    SslRC4(){}

    void set_key(const Stream & stream)
    {
        RC4_set_key(&this->rc4, stream.size(), stream.get_data());
    }

    void crypt(Stream & stream){
        RC4(&this->rc4, stream.size(), stream.get_data(), stream.get_data());
    }
};

class SslHMAC
{
    HMAC_CTX hmac;

    public:
    SslHMAC(const Stream & key, const EVP_MD *md = EVP_sha256())
    {
        HMAC_Init(&this->hmac, key.get_data(), key.size(), md);
    }

    ~SslHMAC()
    {
        HMAC_cleanup(&this->hmac);
    }

    void update(const Stream & stream)
    {
        HMAC_Update(&this->hmac, stream.get_data(), stream.size());
    }

    void final(Stream & stream)
    {
        unsigned int len = 0;

        HMAC_Final(&this->hmac, stream.get_data(), &len);

        stream.p = stream.end = stream.get_data() + len;
    }
};

class ssllib
{
    public:
    static void rsa_encrypt(uint8_t * out, uint8_t * in, int len, uint32_t modulus_size, uint8_t * modulus, uint8_t * exponent)
    {
        BN_CTX *ctx;
        BIGNUM mod, exp, x, y;
        uint8_t inr[SEC_MAX_MODULUS_SIZE];
        int outlen;

        reverseit(modulus, modulus_size);
        reverseit(exponent, SEC_EXPONENT_SIZE);
        rmemcpy(inr, in, len);

        ctx = BN_CTX_new();
        BN_init(&mod);
        BN_init(&exp);
        BN_init(&x);
        BN_init(&y);

        BN_bin2bn(modulus, modulus_size, &mod);
        BN_bin2bn(exponent, SEC_EXPONENT_SIZE, &exp);
        BN_bin2bn(inr, len, &x);
        BN_mod_exp(&y, &x, &exp, &mod, ctx);
        outlen = BN_bn2bin(&y, out);
        reverseit(out, outlen);
        if (outlen < (int)modulus_size){
            memset(out + outlen, 0, modulus_size - outlen);
        }
        BN_free(&y);
        BN_clear_free(&x);
        BN_free(&exp);
        BN_free(&mod);
        BN_CTX_free(ctx);
    }

    static void sec_make_40bit(uint8_t* key)
    {
        key[0] = 0xd1;
        key[1] = 0x26;
        key[2] = 0x9e;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    static void sign(Stream & signature, const Stream & key, const Stream & data)
    {
        uint8_t lenhdr[4];
        buf_out_uint32(lenhdr, data.size());

        SslSha1 sha1;
        sha1.update(key);
        sha1.update(StaticStream(
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36"
                  , 40));
        // Data we are signing
        TODO("pass in a stream and factorize with code in decrypt")
        sha1.update(FixedSizeStream(lenhdr, sizeof(lenhdr)));
        sha1.update(data);

        uint8_t shasig[20];
        sha1.final(shasig);

        SslMd5 md5;
        md5.update(key);
        md5.update(StaticStream(
                   "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                   "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                   "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                 , 48));
        md5.update(FixedSizeStream(shasig, sizeof(shasig)));
        uint8_t md5sig[MD5_DIGEST_LENGTH];
        md5.final(md5sig);

        memcpy(signature.get_data(), md5sig, signature.capacity);
    }
};


struct CryptContext
{
    int use_count;
    uint8_t sign_key[MD5_DIGEST_LENGTH];
    uint8_t key[MD5_DIGEST_LENGTH];
    uint8_t update_key[MD5_DIGEST_LENGTH];
    SslRC4 rc4;

    // encryptionMethod (4 bytes): A 32-bit, unsigned integer. The selected
    // cryptographic method to use for the session. When Enhanced RDP Security
    // (section 5.4) is being used, this field MUST be set to ENCRYPTION_METHOD_NONE
    // (0).

    // +-------------------------------------+-------------------------------------+
    // | 0x00000000 ENCRYPTION_METHOD_NONE   | No encryption or Message            |
    // |                                     | Authentication Codes (MACs) will be |
    // |                                     | used.                               |
    // +-------------------------------------+-------------------------------------+
    // | 0x00000001 ENCRYPTION_METHOD_40BIT  | 40-bit session keys will be used to |
    // |                                     | encrypt data (with RC4) and generate|
    // |                                     | MACs.                               |
    // +-------------------------------------+-------------------------------------+
    // | 0x00000002 ENCRYPTION_METHOD_128BIT | 128-bit session keys will be used   |
    // |                                     | to encrypt data (with RC4) and      |
    // |                                     | generate MACs.                      |
    // +-------------------------------------+-------------------------------------+
    // | 0x00000008 ENCRYPTION_METHOD_56BIT  | 56-bit session keys will be used to |
    // |                                     | encrypt data (with RC4) and generate|
    // |                                     | MACs.                               |
    // +-------------------------------------+-------------------------------------+
    // | 0x00000010 ENCRYPTION_METHOD_FIPS   | All encryption and Message          |
    // |                                     | Authentication Code                 |
    // |                                     | generation routines will            |
    // |                                     | be FIPS 140-1 compliant.            |
    // +-------------------------------------+-------------------------------------+
    uint32_t encryptionMethod;

    CryptContext()
        : use_count(0)
        , encryptionMethod(0)
    {
        memset(this->sign_key, 0, MD5_DIGEST_LENGTH);
        memset(this->key, 0, MD5_DIGEST_LENGTH);
        memset(this->update_key, 0, MD5_DIGEST_LENGTH);
    }

    void generate_key(uint8_t * keyblob, uint32_t encryptionMethod)
    {
        // 16-byte transformation used to generate export keys (6.2.2).
        this->encryptionMethod = encryptionMethod;
        memcpy(this->key, keyblob, 16);

        if (encryptionMethod == 1) {
            // 40 bits encryption
            ssllib ssl;
            ssl.sec_make_40bit(this->key);
            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(FixedSizeStream(this->key, 8));
        }
        else {
            // 128 bits encryption

            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(FixedSizeStream(this->key, 16));
        }
    }

    /* Decrypt data using RC4 */
    void decrypt(Stream & stream)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            size_t keylen = (this->encryptionMethod==1)?8:16;

            SslSha1 sha1;
            sha1.update(FixedSizeStream(this->update_key, keylen));
            sha1.update(StaticStream(
                        "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                        "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                        "\x36\x36\x36\x36\x36\x36\x36\x36"
                      , 40));

            // Data we are signing
            TODO("pass in a stream and factorize with code in sign")
            sha1.update(FixedSizeStream(this->key, keylen));

            uint8_t shasig[20];
            sha1.final(shasig);

            SslMd5 md5;
            md5.update(FixedSizeStream(this->update_key, keylen));
            md5.update(StaticStream(
                       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                     , 48));
            md5.update(FixedSizeStream(shasig, sizeof(shasig)));
            md5.final(this->key);

            this->rc4.set_key(FixedSizeStream(this->key, keylen));

            FixedSizeStream key(this->key, keylen);
            this->rc4.crypt(key);

            if (this->encryptionMethod == 1){
                ssl.sec_make_40bit(this->key);
            }
            this->rc4.set_key(FixedSizeStream(this->key, keylen));
            this->use_count = 0;
        }
        this->rc4.crypt(stream);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(Stream & signature, Stream & data)
    {
        ssllib ssl;

        FixedSizeStream key(this->sign_key, (this->encryptionMethod==1)?8:16);

        ssl.sign(signature, key, data);
    }
};


/*****************************************************************************/

#endif
