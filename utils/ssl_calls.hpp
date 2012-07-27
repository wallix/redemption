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

#include <stdint.h>
#define SSL_CERT X509
#define SSL_RKEY RSA

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

#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x0090800f)
#undef OLD_RSA_GEN1
#define D2I_X509_CONST const
#else
#define OLD_RSA_GEN1
#define D2I_X509_CONST
#endif

#define SSL_RC4 RC4_KEY
#define SSL_SHA1 SHA_CTX
#define SSL_MD5 MD5_CTX
#define SSL_CERT X509
#define SSL_RKEY RSA

class ssllib
{
    public:
    static void sha1_init(SSL_SHA1 * sha1)
    {
        SHA1_Init(sha1);
    }

    static void sha1_update(SSL_SHA1 * sha1, const uint8_t * data, uint32_t len)
    {
        SHA1_Update(sha1, data, len);
    }

    static void sha1_final(SSL_SHA1 * sha1, uint8_t * out_data)
    {
        SHA1_Final(out_data, sha1);
    }

    static void md5_init(SSL_MD5 * md5)
    {
        MD5_Init(md5);
    }

    static void md5_update(SSL_MD5 * md5, const uint8_t * data, uint32_t len)
    {
        MD5_Update(md5, data, len);
    }

    static void md5_final(SSL_MD5 * md5, uint8_t * out_data)
    {
        MD5_Final(out_data, md5);
    }

    static void rc4_set_key(SSL_RC4 & rc4, uint8_t * key, int len)
    {
        RC4_set_key(&rc4, len, key);
    }

    static void rc4_crypt(SSL_RC4 & rc4, uint8_t * in_data, uint8_t * out_data, int len)
    {
        RC4(&rc4, len, in_data, out_data);
    }

    static void rkey_free(SSL_RKEY * rkey)
    {
        RSA_free(rkey);
    }

    static void rsa_encrypt(uint8_t * out, uint8_t * in, int len, uint32_t modulus_size, uint8_t * modulus, uint8_t * exponent)
    {
        BN_CTX *ctx;
        BIGNUM mod, exp, x, y;
        uint8_t inr[SEC_MAX_MODULUS_SIZE];
        int outlen;

        reverse(modulus, modulus_size);
        reverse(exponent, SEC_EXPONENT_SIZE);
        memcpy(inr, in, len);
        reverse(inr, len);

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
        reverse(out, outlen);
        if (outlen < (int) modulus_size){
            memset(out + outlen, 0, modulus_size - outlen);
        }
        BN_free(&y);
        BN_clear_free(&x);
        BN_free(&exp);
        BN_free(&mod);
        BN_CTX_free(ctx);
    }

    static void reverse(uint8_t * p, int len)
    {
        int i, j;
        uint8_t temp;

        for (i = 0, j = len - 1; i < j; i++, j--){
            temp = p[i];
            p[i] = p[j];
            p[j] = temp;
        }
    }

};

inline static void sec_make_40bit(uint8_t* key)
{
    key[0] = 0xd1;
    key[1] = 0x26;
    key[2] = 0x9e;
}

// Output a uint32 into a buffer (little-endian)
inline static void buf_out_uint32(uint8_t* buffer, int value)
{
  buffer[0] = value & 0xff;
  buffer[1] = (value >> 8) & 0xff;
  buffer[2] = (value >> 16) & 0xff;
  buffer[3] = (value >> 24) & 0xff;
}


TODO(" method used by licence  common with basic crypto support code should be made common. pad are also common to several functions.")
/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
inline static void sec_sign(uint8_t* signature, int siglen, uint8_t* session_key, int keylen, uint8_t* data, int datalen)
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
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    buf_out_uint32(lenhdr, datalen);

    ssllib ssl;

    ssl.sha1_init(&sha1);
    ssl.sha1_update(&sha1, session_key, keylen);
    ssl.sha1_update(&sha1, pad_54, 40);
    ssl.sha1_update(&sha1, lenhdr, 4);
    ssl.sha1_update(&sha1, data, datalen);
    ssl.sha1_final(&sha1, shasig);

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, session_key, keylen);
    ssl.md5_update(&md5, pad_92, 48);
    ssl.md5_update(&md5, shasig, 20);
    ssl.md5_final(&md5, md5sig);

    memcpy(signature, md5sig, siglen);
}




struct CryptContext
{
    int use_count;
    uint8_t sign_key[16]; // should I call it session_key ?
    uint8_t key[16];
    uint8_t update_key[16];
    int rc4_key_len;
    SSL_RC4 rc4_info;
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
                sec_make_40bit(this->key);
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
                sec_make_40bit(this->key);
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
        SSL_SHA1 sha1;
        SSL_MD5 md5;

        buf_out_uint32(lenhdr, datalen);

        ssllib ssl;

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, this->sign_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, lenhdr, 4);
        ssl.sha1_update(&sha1, data, datalen);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->sign_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, md5sig);

        memcpy(signature, md5sig, siglen);
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

        uint8_t shasig[20];
        SSL_SHA1 sha1;
        SSL_MD5 md5;
        SSL_RC4 update;

        ssllib ssl;

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, this->update_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, this->key, this->rc4_key_len);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->update_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, key);

        ssl.rc4_set_key(update, this->key, this->rc4_key_len);
        ssl.rc4_crypt(update, this->key, this->key, this->rc4_key_len);
    }

};


/*****************************************************************************/
static inline void ssl_reverse_it(uint8_t* p, int len)
{
    char temp;

    int i = 0;
    int j = len - 1;
    while (i < j) {
        temp = p[i];
        p[i] = p[j];
        p[j] = temp;
        i++;
        j--;
    }
}

/*****************************************************************************/
static inline int ssl_mod_exp(uint8_t* out, int out_len, uint8_t* in, int in_len,
                uint8_t* mod, int mod_len, uint8_t* exp, int exp_len)
{
    BN_CTX* ctx;
    BIGNUM lmod;
    BIGNUM lexp;
    BIGNUM lin;
    BIGNUM lout;

    int rv;
    ctx = BN_CTX_new();

    uint8_t l_out[1024];
    memset(l_out, 0, out_len);
    BN_init(&lout);

    uint8_t l_in[1024];
    ssl_reverse_it(l_in, in_len);
    memcpy(l_in, in, in_len);
    BN_init(&lin);
    BN_bin2bn((uint8_t*)l_in, in_len, &lin);

    uint8_t l_mod[1024];
    memcpy(l_mod, mod, mod_len);
    ssl_reverse_it(l_mod, mod_len);
    BN_init(&lmod);
    BN_bin2bn((uint8_t*)l_mod, mod_len, &lmod);

    uint8_t l_exp[1024];
    memcpy(l_exp, exp, exp_len);
    ssl_reverse_it(l_exp, exp_len);
    BN_init(&lexp);
    BN_bin2bn((uint8_t*)l_exp, exp_len, &lexp);

    BN_mod_exp(&lout, &lin, &lexp, &lmod, ctx);
    rv = BN_bn2bin(&lout, (uint8_t*)l_out);
    if (rv <= out_len) {
        ssl_reverse_it(l_out, rv);
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



/*****************************************************************************/
/* returns newly allocated SSL_CERT or NULL */

static inline SSL_CERT * ssl_cert_read(uint8_t* data, int len)
{
  /* this will move the data pointer but we don't care, we don't use it again */
  return d2i_X509(NULL, (D2I_X509_CONST unsigned char **) &data, len);
}

/*****************************************************************************/

/* Free an allocated SSL_CERT */
static inline void ssl_cert_free(SSL_CERT * cert)
{
  X509_free(cert);
}

/*****************************************************************************/

/* returns boolean */
static inline int ssl_certs_ok(SSL_CERT * server_cert, SSL_CERT * cacert)
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

/* returns newly allocated SSL_RKEY or NULL */
static inline SSL_RKEY *ssl_cert_to_rkey(SSL_CERT* cert, uint32_t & key_len)
{
  EVP_PKEY *epk = NULL;
  SSL_RKEY *lkey;
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


/*****************************************************************************/
static void reverse(uint8_t* p, int len)
{
  int i;
  int j;
  char temp;

  for (i = 0, j = len - 1; i < j; i++, j--)
  {
    temp = p[i];
    p[i] = p[j];
    p[j] = temp;
  }
}

/* returns error */
static inline int ssl_rkey_get_exp_mod(SSL_RKEY * rkey, uint8_t* exponent, int max_exp_len,
                     uint8_t* modulus, int max_mod_len)
{
  int len;

  if ((BN_num_bytes(rkey->e) > (int) max_exp_len) ||
     (BN_num_bytes(rkey->n) > (int) max_mod_len))
  {
    return 1;
  }
  len = BN_bn2bin(rkey->e, (unsigned char*)exponent);
  reverse(exponent, len);
  len = BN_bn2bin(rkey->n, (unsigned char*)modulus);
  reverse(modulus, len);
  return 0;
}


#endif
