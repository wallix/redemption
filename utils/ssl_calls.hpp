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

class SslRC4
{
    RC4_KEY rc4;

    public:
    SslRC4(){}    

    void set_key(size_t key_len, uint8_t * key)
    {
        RC4_set_key(&this->rc4, key_len, key);
    }

    void crypt(size_t len, uint8_t * data){
        RC4(&this->rc4, len, data, data);
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
        uint8_t lenhdr[4];
        buf_out_uint32(lenhdr, datalen);

        SslSha1 sha1;
        sha1.update(session_key, keylen);
        sha1.update(reinterpret_cast<const uint8_t *>(
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36"
                    ), 40);
        sha1.update(lenhdr, sizeof(lenhdr));
        sha1.update(data, datalen);
        uint8_t shasig[20];
        sha1.final(shasig);

        SslMd5 md5;
        md5.update(session_key, keylen);
        md5.update(reinterpret_cast<const uint8_t *>(
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    ), 48);
        md5.update(shasig, 20);
        uint8_t md5sig[16];
        md5.final(md5sig);

        memcpy(signature, md5sig, siglen);
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
    uint8_t sign_key[16];
    uint8_t key[16];
    uint8_t update_key[16];
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

    CryptContext() : use_count(0)
    {
        memset(this->sign_key, 0, 16);
        memset(this->key, 0, 16);
        memset(this->update_key, 0, 16);
        this->encryptionMethod = 0;
    }

    void generate_key(uint8_t * key_block, const uint8_t* salt1, const uint8_t* salt2, uint32_t encryptionMethod)
    {
        // 16-byte transformation used to generate export keys (6.2.2).
        this->encryptionMethod = encryptionMethod;

        SslMd5 md5;
        md5.update(key_block, 16);
        md5.update(salt1, 32);
        md5.update(salt2, 32);
        md5.final(this->key);

        // 40 bits encryption
        if (encryptionMethod == 1) {
            ssllib ssl;
            ssl.sec_make_40bit(this->key);
            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(8, this->key);
        }
        else {
        // 128 bits encryption
            memcpy(this->update_key, this->key, 16);
            this->rc4.set_key(16, this->key);
        }
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
        this->rc4dump((const char *)(&this->rc4), sizeof(this->rc4));
        LOG(LOG_INFO, "cc.encryptionMethod=%u;", this->encryptionMethod);
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            size_t keylen = (this->encryptionMethod==1)?8:16;

            SslSha1 sha1;
            sha1.update(this->update_key, keylen);
            sha1.update(reinterpret_cast<const uint8_t *>(
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36\x36"
                    "\x36\x36\x36\x36\x36\x36\x36\x36"
                    ), 40);
            sha1.update(this->key, keylen);
            uint8_t shasig[20];
            sha1.final(shasig);

            SslMd5 md5;
            md5.update(this->update_key, keylen);
            md5.update(reinterpret_cast<const uint8_t *>(
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c"
                    ), 48);
            md5.update(shasig, 20);
            md5.final(this->key);

            this->rc4.set_key(keylen, this->key);
            this->rc4.crypt(keylen, this->key);

            if (this->encryptionMethod == 1){
                ssl.sec_make_40bit(this->key);
            }
            this->rc4.set_key(keylen, this->key);
            this->use_count = 0;
        }
        this->rc4.crypt(len, data);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(uint8_t* signature, int siglen, uint8_t* data, int datalen)
    {
        ssllib ssl;
        ssl.sign(signature, siglen, this->sign_key, (this->encryptionMethod==1)?8:16, data, datalen);
    }
};


/*****************************************************************************/

#endif
