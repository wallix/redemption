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

uint8_t* ssl_rc4_info_create(void);
void ssl_rc4_info_delete(uint8_t* rc4_info);
void ssl_rc4_set_key(uint8_t* rc4_info, uint8_t* key, int len);
void ssl_rc4_crypt(uint8_t* rc4_info, uint8_t* data, int len);
uint8_t* ssl_sha1_info_create(void);
void ssl_sha1_info_delete(uint8_t* sha1_info);
uint8_t* ssl_md5_info_create(void);
void ssl_md5_info_delete(uint8_t* md5_info);
int ssl_mod_exp(uint8_t* out, int out_len, uint8_t* in, int in_len,
            uint8_t* mod, int mod_len, uint8_t* exp, int exp_len);
int ssl_gen_key_xrdp1(int key_size_in_bits, uint8_t* exp, int exp_len,
                  uint8_t* mod, int mod_len, uint8_t* pri, int pri_len);
SSL_CERT * ssl_cert_read(uint8_t* data, int len);

void ssl_cert_free(SSL_CERT * cert);

int ssl_certs_ok(SSL_CERT * server_cert, SSL_CERT * cacert);

SSL_RKEY * ssl_cert_to_rkey(SSL_CERT* cert, int & key_len);

int ssl_rkey_get_exp_mod(SSL_RKEY * rkey,
                         uint8_t* exponent, int max_exp_len,
                         uint8_t* modulus, int max_mod_len);

void reverse(uint8_t* p, int len);

int ssl_sig_ok(uint8_t* exponent, int exp_len, uint8_t* modulus, int mod_len,
   uint8_t* signature, int sig_len);

#endif
