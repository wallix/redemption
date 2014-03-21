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
   Copyright (C) Wallix 2010-2014
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan

   openssl headers

   Based on xrdp and rdesktop
   Copyright (C) Jay Sorg 2004-2010
   Copyright (C) Matthew Chapman 1999-2007
*/

#ifndef _REDEMPTION_UTILS_SSL_CALLS_HPP_
#define _REDEMPTION_UTILS_SSL_CALLS_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "openssl_crypto.hpp"
#include "bitfu.hpp"

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
        int res = 0;
        res = SHA1_Init(&this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_INIT_FAILED);
        }
    }

    void update(const uint8_t * const data,  size_t data_size)
    {
        int res = 0;
        res = SHA1_Update(&this->sha1, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(SHA_DIGEST_LENGTH == out_data_size);
        int res = 0;
        res = SHA1_Final(out_data, &this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_FINAL_FAILED);
        }
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

    void update(const uint8_t * const data, size_t data_size)
    {
        MD5_Update(&this->md5, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        if (MD5_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD5_DIGEST_LENGTH];
            MD5_Final(tmp, &this->md5);
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        MD5_Final(out_data, &this->md5);
    }
};

class SslMd4
{
    MD4_CTX md4;

    public:
    SslMd4()
    {
        MD4_Init(&this->md4);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        MD4_Update(&this->md4, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        if (MD4_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD4_DIGEST_LENGTH];
            MD4_Final(tmp, &this->md4);
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        MD4_Final(out_data, &this->md4);
    }
};


class SslRC4
{
    RC4_KEY rc4;

    public:
    SslRC4(){}

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        RC4_set_key(&this->rc4, key_size, key);
    }

    void crypt(size_t data_size, const uint8_t * const indata, uint8_t * const outdata){
        RC4(&this->rc4, data_size, indata, outdata);
    }
};

class SslAES
{
    AES_KEY e_key;
    AES_KEY d_key;

    uint8_t iv;

    public:
    SslAES(){}

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        if ((key_size != 16) &&
            (key_size != 24) &&
            (key_size != 32)) {
            LOG(LOG_ERR, "Unexpected AES Key size");
            return;
        }

        AES_set_encrypt_key(key, key_size * 8, &(this->e_key));
        AES_set_decrypt_key(key, key_size * 8, &(this->d_key));
    }

    void crypt_cbc(size_t data_size, uint8_t * ivec,
                   const uint8_t * const indata, uint8_t * const outdata) {
        AES_cbc_encrypt(indata, outdata, data_size, &(this->e_key), ivec, AES_ENCRYPT);
    }

    void decrypt_cbc(size_t data_size, uint8_t * ivec,
                     const uint8_t * const indata, uint8_t * const outdata) {
        AES_cbc_encrypt(indata, outdata, data_size, &(this->d_key), ivec, AES_DECRYPT);
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

class SslHMAC_Md5
{
    HMAC_CTX hmac;

    public:
    SslHMAC_Md5(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_md5(), NULL);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC_Md5()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (MD5_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD5_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};

/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
class Sign
{
    SslSha1 sha1;
    const uint8_t * const key;
    size_t key_size;

    public:
    Sign(const uint8_t * const key, size_t key_size)
        : key(key)
        , key_size(key_size)
    {
        this->sha1.update(this->key, this->key_size);
        const uint8_t sha1const[40] = {
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
        };
        sha1.update(sha1const, 40);
    }

    void update(const uint8_t * const data, size_t data_size) {
        this->sha1.update(data, data_size);
    }

    void final(uint8_t * out, size_t out_size) {
        uint8_t shasig[20];
        this->sha1.final(shasig, 20);

        SslMd5 md5;
        md5.update(this->key, this->key_size);
        const uint8_t sigconst[48] = {
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c
        };
        md5.update(sigconst, sizeof(sigconst));
        md5.update(shasig, sizeof(shasig));
        md5.final(out, out_size);
    }
};


class ssllib
{
    public:
    static void rsa_encrypt(uint8_t * out, uint8_t * in, int len, uint32_t modulus_size, uint8_t * modulus, uint8_t * exponent)
    {
        uint8_t inr[SEC_MAX_MODULUS_SIZE];

        reverseit(modulus, modulus_size);
        reverseit(exponent, SEC_EXPONENT_SIZE);
        rmemcpy(inr, in, len);

        BN_CTX *ctx = BN_CTX_new();
        BIGNUM mod; BN_init(&mod); BN_bin2bn(modulus, modulus_size, &mod);
        BIGNUM exp; BN_init(&exp); BN_bin2bn(exponent, SEC_EXPONENT_SIZE, &exp);
        BIGNUM x; BN_init(&x); BN_bin2bn(inr, len, &x);
        BIGNUM y; BN_init(&y); BN_mod_exp(&y, &x, &exp, &mod, ctx);

        int outlen = BN_bn2bin(&y, out);
        reverseit(out, outlen);
        if (outlen < static_cast<int>(modulus_size)){
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
            this->rc4.set_key(this->key, 8);
        }
        else {
            // 128 bits encryption

            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(this->key, 16);
        }
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t * data, size_t data_size)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            size_t keylen = (this->encryptionMethod==1)?8:16;

            Sign sign(this->update_key, keylen);
            sign.update(this->key, keylen);
            sign.final(this->key, sizeof(key));

            this->rc4.set_key(this->key, keylen);

            // size, in, out
            this->rc4.crypt(keylen, this->key, this->key);

            if (this->encryptionMethod == 1){
                ssl.sec_make_40bit(this->key);
            }
            this->rc4.set_key(this->key, keylen);
            this->use_count = 0;
        }
        // size, in, out
        this->rc4.crypt(data_size, data, data);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(const uint8_t * data, size_t data_size, uint8_t * signature, size_t signature_size)
    {
        uint8_t lenhdr[4];
        buf_out_uint32(lenhdr, data_size);

        Sign sign(this->sign_key, (this->encryptionMethod==1)?8:16);
        sign.update(lenhdr, sizeof(lenhdr));
        sign.update(data, data_size);
        sign.final(signature, 8);
    }
};


/*****************************************************************************/

#endif
