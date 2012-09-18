/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#if !defined(SSL_CALLS_H)
#define SSL_CALLS_H
#include "bitfu.hpp"

#include <stdint.h>
#include <stdlib.h> /* needed for openssl headers */
#include <openssl/rc4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

#include <string.h>
#include "constants.hpp"
#include "log.hpp"


class SslSha1
{
    SHA_CTX sha1;

    public:
    SslSha1()
    {
        SHA1_Init(&this->sha1);
    }

    void update(const uint8_t * data, uint32_t len)
    {
        SHA1_Update(&this->sha1, data, len);
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

    void update(const uint8_t * const data, uint32_t len)
    {
        MD5_Update(&this->md5, data, len);
    }

    void final(uint8_t * out_data)
    {
        MD5_Final(out_data, &this->md5);
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
    static void sign(uint8_t* signature, int siglen, uint8_t* session_key, int keylen, uint8_t* data, int datalen)
    {
        const uint8_t pad_54[40] = { 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54
                                   };
        const uint8_t pad_92[48] = { 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                                 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
                                 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
                               };

        uint8_t shasig[20];
        uint8_t md5sig[16];
        uint8_t lenhdr[4];

        buf_out_uint32(lenhdr, datalen);

        SslSha1 sha1;
        sha1.update(session_key, keylen);
        sha1.update(pad_54, 40);
        sha1.update(lenhdr, 4);
        sha1.update(data, datalen);
        sha1.final(shasig);

        SslMd5 md5;
        md5.update(session_key, keylen);
        md5.update(pad_92, 48);
        md5.update(shasig, 20);
        md5.final(md5sig);

        memcpy(signature, md5sig, siglen);
    }

    inline int ssl_mod_exp(uint8_t* out, int out_len, uint8_t* in, int in_len,
                    uint8_t* mod, int mod_len, uint8_t* exp, int exp_len)
    {
        int rv;
        uint8_t l_out[out_len]; memset(l_out, 0, out_len);
        uint8_t l_in[in_len];   rmemcpy(l_in, in, in_len);
        uint8_t l_mod[mod_len]; rmemcpy(l_mod, mod, mod_len);
        uint8_t l_exp[exp_len]; rmemcpy(l_exp, exp, exp_len);

        BN_CTX* ctx;
        ctx = BN_CTX_new();
        BIGNUM lmod; BN_init(&lmod); BN_bin2bn((uint8_t*)l_mod, mod_len, &lmod);
        BIGNUM lexp; BN_init(&lexp); BN_bin2bn((uint8_t*)l_exp, exp_len, &lexp);
        BIGNUM lin; BN_init(&lin);  BN_bin2bn((uint8_t*)l_in, in_len, &lin);
        BIGNUM lout; BN_init(&lout); BN_mod_exp(&lout, &lin, &lexp, &lmod, ctx);

        rv = BN_bn2bin(&lout, (uint8_t*)l_out);
        if (rv <= out_len) {
            reverseit(l_out, rv);
            memcpy(out, l_out, out_len);
        } else {
            rv = 0;
        }
        BN_free(&lin);
        BN_free(&lout);
        BN_free(&lexp);
        BN_free(&lmod);
        BN_CTX_free(ctx);
        return rv;
    }

    static void rdp_sec_generate_keyblock(uint8_t (& key_block)[48], uint8_t *client_random, uint8_t *server_random)
    {
        uint8_t pre_master_secret[48];
        uint8_t master_secret[48];

        /* Construct pre-master secret (session key) */
        memcpy(pre_master_secret, client_random, 24);
        memcpy(pre_master_secret + 24, server_random, 24);

        uint8_t shasig[20];

        // 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
        for (int i = 0; i < 3; i++) {
            uint8_t pad[4];

            memset(pad, 'A' + i, i + 1);

            SslSha1 sha1;
            sha1.update(pad, i + 1);
            sha1.update(pre_master_secret, 48);
            sha1.update(client_random, 32);
            sha1.update(server_random, 32);
            sha1.final(shasig);

            SslMd5 md5;
            md5.update(pre_master_secret, 48);
            md5.update(shasig, 20);
            md5.final(&master_secret[i * 16]);
        }

        // 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
        for (int i = 0; i < 3; i++) {
            uint8_t pad[4];

            memset(pad, 'X' + i, i + 1);

            SslSha1 sha1;
            sha1.update(pad, i + 1);
            sha1.update(master_secret, 48);
            sha1.update(client_random, 32);
            sha1.update(server_random, 32);
            sha1.final(shasig);

            SslMd5 md5;
            md5.update(master_secret, 48);
            md5.update(shasig, 20);
            md5.final(&key_block[i * 16]);
        }
    }
};


struct CryptContext
{
    int use_count;
    uint8_t sign_key[16]; // should I call it session_key ?
    uint8_t key[16];
    uint8_t update_key[16];
    int rc4_key_len;
    RC4_KEY rc4_info;

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


    CryptContext() : use_count(0)
    {
        memset(this->sign_key, 0, 16);
        memset(this->key, 0, 16);
        memset(this->update_key, 0, 16);
        this->rc4_key_len = 0;
        memset(&rc4_info, 0, sizeof(rc4_info));
        this->encryptionMethod = 0;
    }

    void generate_key(uint8_t * key_block, const uint8_t* salt1, const uint8_t* salt2, uint32_t encryptionMethod)
    {
        // 16-byte transformation used to generate export keys (6.2.2).
        ssllib ssl;

        SslMd5 md5;
        md5.update(key_block, 16);
        md5.update(salt1, 32);
        md5.update(salt2, 32);
        md5.final(this->key);

        if (encryptionMethod == 1) {
            // LOG(LOG_DEBUG, "40-bit encryption enabled");
            ssl.sec_make_40bit(this->key);
            this->rc4_key_len = 8;
        }
        else {
            //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled", encryptionMethod);
            this->rc4_key_len = 16;
        }

        /* Save initial RC4 keys as update keys */
        memcpy(this->update_key, this->key, 16);
        RC4_set_key(&this->rc4_info, this->rc4_key_len, this->key);
    }

    void rc4dump(const char * data, size_t size){
        char buffer[16384];
        char * line = buffer;
        line += sprintf(line, "memcpy((void*)(&cc.rc4_info), (void*)\"");
        for (size_t j = 0 ; j < size ; j ++){
            line += sprintf(line, "\\x%.2x", (unsigned char)data[j]);
        }
        line += sprintf(line, "\", %u);", (unsigned)size);
        LOG(LOG_INFO, "%s", buffer);
    }

    void dump(){
        LOG(LOG_INFO, "cc.use_count=%u;", this->use_count);
        LOG(LOG_INFO, "memcpy(cc.sign_key, \""
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\", 16);",
            this->sign_key[0],this->sign_key[1],this->sign_key[2],this->sign_key[3],
            this->sign_key[4],this->sign_key[5],this->sign_key[6],this->sign_key[7],
            this->sign_key[8],this->sign_key[9],this->sign_key[10],this->sign_key[11],
            this->sign_key[12],this->sign_key[13],this->sign_key[14],this->sign_key[15]);
        LOG(LOG_INFO, "memcpy(cc.key, \""
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
            "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\\x%.2x\", 16);",
            this->key[0],this->key[1],this->key[2],this->key[3],
            this->key[4],this->key[5],this->key[6],this->key[7],
            this->key[8],this->key[9],this->key[10],this->key[11],
            this->key[12],this->key[13],this->key[14],this->key[15]);
        LOG(LOG_INFO, "cc.rc4_key_len=%u;", this->rc4_key_len);
        this->rc4dump((const char *)(&this->rc4_info), sizeof(this->rc4_info));
        LOG(LOG_INFO, "cc.encryptionMethod=%u;", this->encryptionMethod);
    }

    /* Encrypt data using RC4 */
    void encrypt(uint8_t* data, int length)
    {
        ssllib ssl;

        if (this->use_count == 4096){
            this->update();
            if (this->rc4_key_len == 8) {
                ssl.sec_make_40bit(this->key);
            }
            RC4_set_key(&this->rc4_info, this->rc4_key_len, this->key);
            this->use_count = 0;
        }
        RC4(&this->rc4_info, length, data, data);
        this->use_count++;
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            this->update();
            if (this->rc4_key_len == 8) {
                ssl.sec_make_40bit(this->key);
            }
            RC4_set_key(&this->rc4_info, this->rc4_key_len, this->key);
            this->use_count = 0;
        }
        RC4(&this->rc4_info, len, data, data);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(uint8_t* signature, int siglen, uint8_t* data, int datalen)
    {
        ssllib ssl;
        ssl.sign(signature, siglen, this->sign_key, this->rc4_key_len, data, datalen);
    }

    void update()
    {
        const uint8_t pad_54[40] = {
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54
        };

        const uint8_t pad_92[48] = {
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
        };

        SslSha1 sha1;
        sha1.update(this->update_key, this->rc4_key_len);
        sha1.update(pad_54, 40);
        sha1.update(this->key, this->rc4_key_len);
        uint8_t shasig[20];
        sha1.final(shasig);

        SslMd5 md5;
        md5.update(this->update_key, this->rc4_key_len);
        md5.update(pad_92, 48);
        md5.update(shasig, 20);
        md5.final(this->key);

        RC4_KEY update;
        RC4_set_key(&update, this->rc4_key_len, this->key);
        RC4(&update, this->rc4_key_len, this->key, this->key);
    }

};


/*****************************************************************************/

#endif
