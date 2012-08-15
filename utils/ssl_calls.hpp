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
    static void rc4_set_key(RC4_KEY & rc4, uint8_t * key, int len)
    {
        RC4_set_key(&rc4, len, key);
    }

    static void rc4_crypt(RC4_KEY & rc4, uint8_t * in_data, uint8_t * out_data, int len)
    {
        RC4(&rc4, len, in_data, out_data);
    }

    static void rkey_free(RSA * rkey)
    {
        RSA_free(rkey);
    }

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
        static uint8_t pad_54[40] = { 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
                                     54, 54, 54, 54, 54, 54, 54, 54
                                   };
        static uint8_t pad_92[48] = { 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
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
        BN_CTX* ctx;
        BIGNUM lmod;
        BIGNUM lexp;
        BIGNUM lin;
        BIGNUM lout;
        int rv;
        uint8_t* l_out;
        uint8_t* l_in;
        uint8_t* l_mod;
        uint8_t* l_exp;

        TODO(" replace these fucking new / delete by objects on stack")
        l_out = new uint8_t[out_len];
        memset(l_out, 0, out_len);
        l_in = new uint8_t[in_len];
        memset(l_in, 0, in_len);
        l_mod = new uint8_t[mod_len];
        memset(l_mod, 0, mod_len);
        l_exp = new uint8_t[exp_len];
        memset(l_exp, 0, exp_len);
        memcpy(l_in, in, in_len);
        memcpy(l_mod, mod, mod_len);
        memcpy(l_exp, exp, exp_len);
        reverseit(l_in, in_len);
        reverseit(l_mod, mod_len);
        reverseit(l_exp, exp_len);
        ctx = BN_CTX_new();
        BN_init(&lmod);
        BN_init(&lexp);
        BN_init(&lin);
        BN_init(&lout);
        BN_bin2bn((uint8_t*)l_mod, mod_len, &lmod);
        BN_bin2bn((uint8_t*)l_exp, exp_len, &lexp);
        BN_bin2bn((uint8_t*)l_in, in_len, &lin);
        BN_mod_exp(&lout, &lin, &lexp, &lmod, ctx);
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
        delete [] l_out;
        delete [] l_in;
        delete [] l_mod;
        delete [] l_exp;
        return rv;
    }



    /*****************************************************************************/
    /* returns newly allocated X509 or NULL */

    inline X509 * ssl_cert_read(uint8_t* data, int len)
    {
      /* this will move the data pointer but we don't care, we don't use it again */
      return d2i_X509(NULL, (const unsigned char **) &data, len);
    }

    /*****************************************************************************/

    /* Free an allocated X509 */
    inline void ssl_cert_free(X509 * cert)
    {
      X509_free(cert);
    }

    /*****************************************************************************/

    /* returns boolean */
    inline int ssl_certs_ok(X509 * server_cert, X509 * cacert)
    {
      /* Currently, we don't use the CA Certificate.
      FIXME:
      *) Verify the server certificate (server_cert) with the
      CA certificate.
      *) Store the CA Certificate with the hostname of the
      server we are connecting to as key, and compare it
      when we connect the next time, in order to prevent
      MITM-attacks.
      */
      return 1;
    }

    /*****************************************************************************/

    /* returns newly allocated RSA or NULL */
    inline RSA *ssl_cert_to_rkey(X509* cert, uint32_t & key_len)
    {
      EVP_PKEY *epk = NULL;
      RSA *lkey;
      int nid;

      /* By some reason, Microsoft sets the OID of the Public RSA key to
      the oid for "MD5 with RSA Encryption" instead of "RSA Encryption"

      Kudos to Richard Levitte for the following (. intiutive .)
      lines of code that resets the OID and let's us extract the key. */

      nid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
      if ((nid == NID_md5WithRSAEncryption) || (nid == NID_shaWithRSAEncryption))
      {
        ASN1_OBJECT_free(cert->cert_info->key->algor->algorithm);
        cert->cert_info->key->algor->algorithm = OBJ_nid2obj(NID_rsaEncryption);
      }
      epk = X509_get_pubkey(cert);
      if (NULL == epk){
        printf("Failed to extract public key from certificate\n");
        return 0;
      }

      lkey = RSAPublicKey_dup((RSA *) epk->pkey.ptr);
      EVP_PKEY_free(epk);
      key_len = RSA_size(lkey);
      return lkey;
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
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */

    CryptContext() : use_count(0)
    {
        memset(this->sign_key, 0, 16);
        memset(this->key, 0, 16);
        memset(this->update_key, 0, 16);
        this->rc4_key_len = 0;
        memset(&rc4_info, 0, sizeof(rc4_info));
        this->rc4_key_size = 0;
    }

    void generate_key(uint8_t * key_block, const uint8_t* salt1, const uint8_t* salt2, uint32_t rc4_key_size)
    {
        // 16-byte transformation used to generate export keys (6.2.2).
        ssllib ssl;

        SslMd5 md5;
        md5.update(key_block, 16);
        md5.update(salt1, 32);
        md5.update(salt2, 32);
        md5.final(this->key);

        if (rc4_key_size == 1) {
            // LOG(LOG_DEBUG, "40-bit encryption enabled");
            ssl.sec_make_40bit(this->key);
            this->rc4_key_len = 8;
        }
        else {
            //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled", rc4_key_size);
            this->rc4_key_len = 16;
        }

        /* Save initial RC4 keys as update keys */
        memcpy(this->update_key, this->key, 16);

        ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
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
        LOG(LOG_INFO, "cc.rc4_key_size=%u;", this->rc4_key_size);
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
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, length);
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
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, len);
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
        static uint8_t pad_54[40] = {
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54
        };

        static uint8_t pad_92[48] = {
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

        ssllib ssl;
        RC4_KEY update;
        ssl.rc4_set_key(update, this->key, this->rc4_key_len);
        ssl.rc4_crypt(update, this->key, this->key, this->rc4_key_len);
    }

};


/*****************************************************************************/

#endif
