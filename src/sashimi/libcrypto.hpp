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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "utils/log.hpp"
#include "libssh/libssh.h"
#include "utils/sugar/cast.hpp"
#include "string.hpp"
#include "openssl_crypto.hpp"
#include "system/ssl_calls.hpp"
#include <arpa/inet.h>

#include <syslog.h>
#include <vector>
#include <memory>

#include "core/error.hpp"

#undef des_cbc_encryt

#include "system/ssl_sha1.hpp"
#include "system/ssl_sha256.hpp"


/* the offsets of methods */
enum ssh_kex_types_e {
    SSH_KEX=0,
    SSH_HOSTKEYS,
    SSH_CRYPT_C_S,
    SSH_CRYPT_S_C,
    SSH_MAC_C_S,
    SSH_MAC_S_C,
    SSH_COMP_C_S,
    SSH_COMP_S_C,
    SSH_LANG_C_S,
    SSH_LANG_S_C
};

#define SSH_CRYPT 2
#define SSH_MAC 3
#define SSH_COMP 4
#define SSH_LANG 5

static inline int find_key(const char * key, std::initializer_list<std::pair<const char *, int>> l, int default_value)
{
    for(auto &p:l){
        if (strcmp(p.first, key) == 0){
            return p.second;
        }
    }
    return default_value;
}

// find_matching gets 2 parameters : a list of available objects (avail)
// and a list of preferred objects (pref)
// it will return a new SSHString object containing the first preferred item
// also found in the available objects list or a zero length SSHString object otherwise.
static inline SSHString find_matching(const char * avail, const char * pref, const char delim)
{
    uint32_t preferred_len = 0;
    uint32_t available_len = 0;

    const char * preferred = pref;
    for (preferred_len = 0 ; preferred[preferred_len] != 0 ; preferred_len++){
        if (preferred[preferred_len] == delim){
            const char * available = avail;
            for (available_len = 0 ; available[available_len] != 0 ; available_len++){
                if (available[available_len] == delim){
                    if (preferred_len == available_len
                    && 0 == memcmp(preferred, available, preferred_len)){
                        // match found
                        SSHString res(preferred, preferred_len);
//                        SSHString res(preferred_len);
//                        memcpy(res.data.get(), preferred, preferred_len);
                        return res;
                    }
                    available += available_len + 1;
                    available_len = 0;
                }
            }
            if (preferred_len == available_len && 0 == memcmp(preferred, available, preferred_len)){
                // match found on last available
                SSHString res(preferred, preferred_len);
//                memcpy(res.data.get(), preferred, preferred_len);
                return res;
            }
            preferred += preferred_len + 1;
            preferred_len = 0;
        }
    }
    // check last preferred on all available
    const char * available = avail;
    for (available_len = 0 ; available[available_len] != 0 ; available_len++){
        if (available[available_len] == delim){
            if (preferred_len == available_len
            && 0 == memcmp(preferred, available, preferred_len)){
                // match found
                SSHString res(preferred, preferred_len);
//                memcpy(res.data.get(), preferred, preferred_len);
                return res;
            }
            available += available_len + 1;
            available_len = 0;
        }
    }
    if (preferred_len == available_len && 0 == memcmp(preferred, available, preferred_len)){
        // match found on last available
        SSHString res(preferred, preferred_len);
//        memcpy(res.data.get(), preferred, preferred_len);
        return res;
    }
    // return string of zero length if no match found
    return SSHString("");
}


#define NISTP256 NID_X9_62_prime256v1
#define NISTP384 NID_secp384r1
#define NISTP521 NID_secp521r1

#define EVP_DIGEST_LEN EVP_MAX_MD_SIZE

#include <openssl/bn.h>
#include <openssl/opensslv.h>

enum ssh_mac_e {
  SSH_MAC_SHA1=1,
  SSH_MAC_SHA256,
  SSH_MAC_SHA384,
  SSH_MAC_SHA512
};

//enum ssh_hmac_e {
//  SSH_HMAC_SHA1 = 1,
//  SSH_HMAC_MD5
//};

enum ssh_des_e {
  SSH_3DES,
  SSH_DES
};

#define CURVE25519_PUBKEY_SIZE 32
#define CURVE25519_PRIVKEY_SIZE 32

struct InternalBignum {
    static void add(unsigned int out[32],const unsigned int a[32],const unsigned int b[32])
    {
      unsigned int j;
      unsigned int u;
      u = 0;
      for (j = 0; j < 31; ++j) { u += a[j] + b[j]; out[j] = u & 255; u >>= 8; }
      u += a[31] + b[31]; out[31] = u;
    }

    static void sub(unsigned int out[32],const unsigned int a[32],const unsigned int b[32])
    {
      unsigned int j;
      unsigned int u;
      u = 218;
      for (j = 0;j < 31;++j) {
        u += a[j] + 65280 - b[j];
        out[j] = u & 255;
        u >>= 8;
      }
      u += a[31] - b[31];
      out[31] = u;
    }

    static void freeze(unsigned int a[32])
    {
        unsigned int aorig[32];
        unsigned int j;
        unsigned int negative;
        const unsigned int minusp[32] = {
            19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128
        } ;

        for (j = 0;j < 32;++j) { aorig[j] = a[j]; }
        add(a, a, minusp);
        negative = -((a[31] >> 7) & 1);
        for (j = 0;j < 32;++j) a[j] ^= negative & (aorig[j] ^ a[j]);
    }

    static void squeeze(unsigned int a[32])
    {
        unsigned int j;
        unsigned int u;
        u = 0;
        for (j = 0;j < 31;++j) { u += a[j]; a[j] = u & 255; u >>= 8; }
        u += a[31]; a[31] = u & 127;
        u = 19 * (u >> 7);
        for (j = 0;j < 31;++j) { u += a[j]; a[j] = u & 255; u >>= 8; }
        u += a[31]; a[31] = u;
    }

    static void mult(unsigned int out[32],const unsigned int a[32],const unsigned int b[32])
    {
        unsigned int i;
        unsigned int j;
        unsigned int u;

        for (i = 0;i < 32;++i) {
            u = 0;
            for (j = 0;j <= i;++j) {
                u += a[j] * b[i - j];
            }
            for (j = i + 1;j < 32;++j) {
                u += 38 * a[j] * b[i + 32 - j];
            }
            out[i] = u;
        }
        squeeze(out);
    }

    static void mult121665(unsigned int out[32],const unsigned int a[32])
    {
        unsigned int j;
        unsigned int u = 0;
        for (j = 0;j < 31;++j) {
            u += 121665 * a[j]; out[j] = u & 255; u >>= 8;
        }
        u += 121665 * a[31]; out[31] = u & 127;
        u = 19 * (u >> 7);
        for (j = 0;j < 31;++j) {
            u += out[j]; out[j] = u & 255; u >>= 8;
        }
        u += out[j]; out[j] = u;
    }

    static void square(unsigned int out[32],const unsigned int a[32])
    {
        unsigned int i;
        unsigned int j;
        unsigned int u;

        for (i = 0;i < 32;++i) {
            u = 0;
            for (j = 0;j < i - j;++j){
                 u += a[j] * a[i - j];
            }
            for (j = i + 1;j < i + 32 - j;++j) {
                u += 38 * a[j] * a[i + 32 - j];
            }
            u *= 2;
            if ((i & 1) == 0) {
              u += a[i / 2] * a[i / 2];
              u += 38 * a[i / 2 + 16] * a[i / 2 + 16];
            }
            out[i] = u;
        }
        squeeze(out);
    }

    static void mainloop(unsigned int work[64], const unsigned char e[32])
    {
        unsigned int xzm1[64];
        unsigned int xzm[64];
        unsigned int xzmb[64];
        unsigned int xzm1b[64];
        unsigned int xznb[64];
        unsigned int xzn1b[64];
        unsigned int a0[64];
        unsigned int a1[64];
        unsigned int b0[64];
        unsigned int b1[64];
        unsigned int c1[64];
        unsigned int r[32];
        unsigned int s[32];
        unsigned int t[32];
        unsigned int u[32];
        unsigned int j;
        unsigned int b;
        int pos;

        for (j = 0;j < 32;++j){
            xzm1[j] = work[j];
        }
        xzm1[32] = 1;
        for (j = 33;j < 64;++j) {
            xzm1[j] = 0;
        }

        xzm[0] = 1;
        for (j = 1;j < 64;++j) {
            xzm[j] = 0;
        }

        for (pos = 254;pos >= 0;--pos) {
            b = e[pos / 8] >> (pos & 7);
            b &= 1;
            c_select(xzmb,xzm1b,xzm,xzm1,b);
            add(a0,xzmb,xzmb + 32);
            sub(a0 + 32,xzmb,xzmb + 32);
            add(a1,xzm1b,xzm1b + 32);
            sub(a1 + 32,xzm1b,xzm1b + 32);
            square(b0,a0);
            square(b0 + 32,a0 + 32);
            mult(b1,a1,a0 + 32);
            mult(b1 + 32,a1 + 32,a0);
            add(c1,b1,b1 + 32);
            sub(c1 + 32,b1,b1 + 32);
            square(r,c1 + 32);
            sub(s,b0,b0 + 32);
            mult121665(t,s);
            add(u,t,b0);
            mult(xznb,b0,b0 + 32);
            mult(xznb + 32,s,u);
            square(xzn1b,c1);
            mult(xzn1b + 32,r,work);
            c_select(xzm,xzm1,xznb,xzn1b,b);
        }

        for (j = 0;j < 64;++j){
            work[j] = xzm[j];
        }
    }

    static void recip(unsigned int out[32],const unsigned int z[32])
    {
        unsigned int z2[32];
        unsigned int z9[32];
        unsigned int z11[32];
        unsigned int z2_5_0[32];
        unsigned int z2_10_0[32];
        unsigned int z2_20_0[32];
        unsigned int z2_50_0[32];
        unsigned int z2_100_0[32];
        unsigned int t0[32];
        unsigned int t1[32];
        int i;

        /* 2 */ square(z2,z);
        /* 4 */ square(t1,z2);
        /* 8 */ square(t0,t1);
        /* 9 */ mult(z9,t0,z);
        /* 11 */ mult(z11,z9,z2);
        /* 22 */ square(t0,z11);
        /* 2^5 - 2^0 = 31 */ mult(z2_5_0,t0,z9);

        /* 2^6 - 2^1 */ square(t0,z2_5_0);
        /* 2^7 - 2^2 */ square(t1,t0);
        /* 2^8 - 2^3 */ square(t0,t1);
        /* 2^9 - 2^4 */ square(t1,t0);
        /* 2^10 - 2^5 */ square(t0,t1);
        /* 2^10 - 2^0 */ mult(z2_10_0,t0,z2_5_0);

        /* 2^11 - 2^1 */ square(t0,z2_10_0);
        /* 2^12 - 2^2 */ square(t1,t0);
        /* 2^20 - 2^10 */ for (i = 2;i < 10;i += 2) { square(t0,t1); square(t1,t0); }
        /* 2^20 - 2^0 */ mult(z2_20_0,t1,z2_10_0);

        /* 2^21 - 2^1 */ square(t0,z2_20_0);
        /* 2^22 - 2^2 */ square(t1,t0);
        /* 2^40 - 2^20 */ for (i = 2;i < 20;i += 2) { square(t0,t1); square(t1,t0); }
        /* 2^40 - 2^0 */ mult(t0,t1,z2_20_0);

        /* 2^41 - 2^1 */ square(t1,t0);
        /* 2^42 - 2^2 */ square(t0,t1);
        /* 2^50 - 2^10 */ for (i = 2;i < 10;i += 2) { square(t1,t0); square(t0,t1); }
        /* 2^50 - 2^0 */ mult(z2_50_0,t0,z2_10_0);

        /* 2^51 - 2^1 */ square(t0,z2_50_0);
        /* 2^52 - 2^2 */ square(t1,t0);
        /* 2^100 - 2^50 */ for (i = 2;i < 50;i += 2) { square(t0,t1); square(t1,t0); }
        /* 2^100 - 2^0 */ mult(z2_100_0,t1,z2_50_0);

        /* 2^101 - 2^1 */ square(t1,z2_100_0);
        /* 2^102 - 2^2 */ square(t0,t1);
        /* 2^200 - 2^100 */ for (i = 2;i < 100;i += 2) { square(t1,t0); square(t0,t1); }
        /* 2^200 - 2^0 */ mult(t1,t0,z2_100_0);

        /* 2^201 - 2^1 */ square(t0,t1);
        /* 2^202 - 2^2 */ square(t1,t0);
        /* 2^250 - 2^50 */ for (i = 2;i < 50;i += 2) { square(t0,t1); square(t1,t0); }
        /* 2^250 - 2^0 */ mult(t0,t1,z2_50_0);

        /* 2^251 - 2^1 */ square(t1,t0);
        /* 2^252 - 2^2 */ square(t0,t1);
        /* 2^253 - 2^3 */ square(t1,t0);
        /* 2^254 - 2^4 */ square(t0,t1);
        /* 2^255 - 2^5 */ square(t1,t0);
        /* 2^255 - 21 */ mult(out,t1,z11);
    }

    int crypto_scalarmult(unsigned char *q, const unsigned char *n, const unsigned char *p)
    {
        unsigned int work[96];
        unsigned char e[32];
        unsigned int i;
        for (i = 0;i < 32;++i) e[i] = n[i];
        e[0] &= 248;
        e[31] &= 127;
        e[31] |= 64;
        for (i = 0;i < 32;++i) work[i] = p[i];
        mainloop(work,e);
        recip(work + 32,work + 32);
        mult(work + 64,work,work + 32);
        freeze(work + 64);
        for (i = 0;i < 32;++i) q[i] = work[64 + i];
        return 0;
    }

    static void c_select(unsigned p[64],unsigned q[64],const unsigned r[64],const unsigned s[64],unsigned b)
    {
        unsigned j;
        unsigned t;
        unsigned bminus1;

        bminus1 = b - 1;
        for (j = 0;j < 64;++j) {
            t = bminus1 & (r[j] ^ s[j]);
            p[j] = s[j] ^ t;
            q[j] = r[j] ^ t;
        }
    }
};

enum ssh_key_exchange_e {
  /* diffie-hellman-group1-sha1 */
  SSH_KEX_DH_GROUP1_SHA1=1,
  /* diffie-hellman-group14-sha1 */
  SSH_KEX_DH_GROUP14_SHA1,
  /* ecdh-sha2-nistp256 */
  SSH_KEX_ECDH_SHA2_NISTP256,
  /* curve25519-sha256@libssh.org */
  SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG
};


#define SSH_KEX_METHODS 10
#define KEX_METHODS_SIZE 10

struct ssh_kex_struct {
    unsigned char cookie[16];
    SSHString methods[SSH_KEX_METHODS];

    ssh_kex_struct()
        : cookie{}
        , methods{"","","","","","","","","",""}
    {
    }
};

#undef cbc_encrypt

struct ssh_cipher_struct {
    const char *name; /* ssh name of the algorithm */
    unsigned int blocksize; /* blocksize of the algo */
    unsigned int keylen; /* length of the key structure */
    void *key; /* a key buffer allocated for the algo */
    void *IV;
    unsigned int keysize; /* bytes of key used. != keylen */
    /* sets the new key for immediate use */
    virtual int set_encrypt_key(void *key, void *IV) = 0;
    virtual int set_decrypt_key(void *key, void *IV) = 0;
    virtual void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) = 0;
    virtual void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) = 0;

    ssh_cipher_struct(const char * name, unsigned blocksize, unsigned keylen, unsigned keysize)
        : name(name) // ssh name of the algorithm
        , blocksize(blocksize) // blocksize of the algo
        , keylen(keylen)
        , key(nullptr)
        , IV(nullptr)
        , keysize(keysize)
    {}

    virtual ~ssh_cipher_struct() {}

    int alloc_key()
    {
        this->key = malloc(this->keylen);
        if (this->key == nullptr) {
          return -1;
        }

        return 0;
    }
};


struct ssh_blowfish_cipher_struct : public ssh_cipher_struct
{

    ssh_blowfish_cipher_struct()
        : ssh_cipher_struct("blowfish-cbc", 8, sizeof(BF_KEY), 128)
    {
    }

    /* the wrapper functions for blowfish */
    int set_encrypt_key(void *key, void *IV) override
    {
//        LOG(LOG_INFO, " ssh_blowfish_cipher_struct::set_encrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        BF_set_key(static_cast<BF_KEY* >(this->key), 16, static_cast<const unsigned char *>(key));
      }
      this->IV = IV;
      return 0;
    }

    int set_decrypt_key(void *key, void *IV) override
    {
//        LOG(LOG_INFO, " ssh_blowfish_cipher_struct::set_decrypt_key");
        return this->set_encrypt_key(key, IV);
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//        LOG(LOG_INFO, " ssh_blowfish_cipher_struct::cbc_encrypt");
      BF_cbc_encrypt(in, out, len,
            static_cast<const BF_KEY*>(this->key),
            static_cast<unsigned char *>(this->IV),
            BF_ENCRYPT);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override {
//      LOG(LOG_INFO, "cbc_decrypt ssh_blowfish_cipher_struct");
      BF_cbc_encrypt(in, out, len,
            static_cast<const BF_KEY*>(this->key),
            static_cast<unsigned char *>(this->IV),
            BF_DECRYPT);
    }
};


struct ssh_aes128_ctr_cipher_struct : public ssh_cipher_struct
{

    ssh_aes128_ctr_cipher_struct()
        : ssh_cipher_struct("aes128-ctr", 16, sizeof(AES_KEY), 128)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes128_ctr_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
              return -1;
            }
            if (AES_set_encrypt_key(static_cast<const unsigned char *>(key),
                 this->keysize, static_cast<AES_KEY*>(this->key)) < 0) {
                free(this->key);
                return -1;
            }
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes128_ctr_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    /** @internal
     * @brief encrypts/decrypts data with stream cipher AES_ctr128. 128 bits is actually
     * the size of the CTR counter and incidentally the blocksize, but not the keysize.
     * @param[in] len must be a multiple of AES128 block size.
     */
    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override {
//      LOG(LOG_INFO, "ssh_aes128_ctr_cipher_struct::cbc_encrypt_key");
      unsigned char tmp_buffer[128/8];
      unsigned int num=0;
      /* Some things are special with ctr128 :
       * In this case, tmp_buffer is not being used, because it is used to store temporary data
       * when an encryption is made on lengths that are not multiple of blocksize.
       * Same for num, which is being used to store the current offset in blocksize in CTR
       * function.
       */
      AES_ctr128_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        tmp_buffer, &num);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//        LOG(LOG_INFO, "ssh_aes128_ctr_cipher_struct::cbc_decrypt_key");
        return this->cbc_encrypt(in, out, len);
    }
};



struct ssh_aes192_ctr_cipher_struct : public ssh_cipher_struct
{

    ssh_aes192_ctr_cipher_struct()
        : ssh_cipher_struct("aes192-ctr", 16, sizeof(AES_KEY), 192)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes192_ctr_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
              return -1;
            }
            if (AES_set_encrypt_key(
                    static_cast<const unsigned char *>(key),
                    this->keysize,
                    static_cast<AES_KEY*>(this->key)) < 0) {
              free(this->key);
              return -1;
            }
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes192_ctr_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    /** @internal
     * @brief encrypts/decrypts data with stream cipher AES_ctr128. 128 bits is actually
     * the size of the CTR counter and incidentally the blocksize, but not the keysize.
     * @param[in] len must be a multiple of AES128 block size.
     */
    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override {
//      LOG(LOG_INFO, "ssh_aes192_ctr_cipher_struct::cbc_encrypt");
      unsigned char tmp_buffer[128/8];
      unsigned int num=0;
      /* Some things are special with ctr128 :
       * In this case, tmp_buffer is not being used, because it is used to store temporary data
       * when an encryption is made on lengths that are not multiple of blocksize.
       * Same for num, which is being used to store the current offset in blocksize in CTR
       * function.
       */
      AES_ctr128_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        tmp_buffer, &num);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//        LOG(LOG_INFO, "ssh_aes192_ctr_cipher_struct::cbc_decrypt");
        return this->cbc_encrypt(in, out, len);
    }
};

struct ssh_aes256_ctr_cipher_struct : public ssh_cipher_struct
{

    ssh_aes256_ctr_cipher_struct()
        : ssh_cipher_struct("aes256-ctr", 16, sizeof(AES_KEY), 256)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes256_ctr_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
                return -1;
            }
            if (AES_set_encrypt_key(
                    static_cast<const unsigned char *>(key),
                    this->keysize,
                    static_cast<AES_KEY*>(this->key)) < 0) {
              free(this->key);
              return -1;
            }
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes256_ctr_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    /** @internal
     * @brief encrypts/decrypts data with stream cipher AES_ctr128. 128 bits is actually
     * the size of the CTR counter and incidentally the blocksize, but not the keysize.
     * @param[in] len must be a multiple of AES128 block size.
     */
    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override {
//      LOG(LOG_INFO, "ssh_aes256_ctr_cipher_struct::cbc_encrypt");
      unsigned char tmp_buffer[128/8];
      unsigned int num=0;
      /* Some things are special with ctr128 :
       * In this case, tmp_buffer is not being used, because it is used to store temporary data
       * when an encryption is made on lengths that are not multiple of blocksize.
       * Same for num, which is being used to store the current offset in blocksize in CTR
       * function.
       */
      AES_ctr128_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        tmp_buffer, &num);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//        LOG(LOG_INFO, "ssh_aes256_ctr_cipher_struct::cbc_decrypt");
        return this->cbc_encrypt(in, out, len);
    }
};

struct ssh_aes128_cbc_cipher_struct : public ssh_cipher_struct
{

    ssh_aes128_cbc_cipher_struct()
        : ssh_cipher_struct("aes128-cbc", 16, sizeof(AES_KEY), 128)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes128_cbc_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
                return -1;
            }
            if (AES_set_encrypt_key(
                    static_cast<const unsigned char *>(key),
                    this->keysize,
                    static_cast<AES_KEY*>(this->key)) < 0) {
                free(this->key);
                return -1;
            }
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes128_cbc_cipher_struct::set_decrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        if (AES_set_decrypt_key(
                static_cast<const unsigned char *>(key),
                this->keysize,
                static_cast<AES_KEY*>(this->key)) < 0) {
          free(this->key);
          return -1;
        }
      }
      this->IV=IV;
      return 0;
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes128_cbc_cipher_struct::cbc_encrypt");
      AES_cbc_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_ENCRYPT);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes128_cbc_cipher_struct::cbc_decrypt");
      AES_cbc_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_DECRYPT);
    }

};

struct ssh_aes192_cbc_cipher_struct : public ssh_cipher_struct
{
    ssh_aes192_cbc_cipher_struct()
        : ssh_cipher_struct("aes192-cbc", 16, sizeof(AES_KEY), 192)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes192_cbc_cipher_struct::set_encrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        if (AES_set_encrypt_key(static_cast<const unsigned char *>(key),
                this->keysize,
                static_cast<AES_KEY*>(this->key)) < 0) {
          free(this->key);
          return -1;
        }
      }
      this->IV=IV;
      return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes192_cbc_cipher_struct::set_decrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        if (AES_set_decrypt_key(static_cast<const unsigned char *>(key),
                this->keysize,
                static_cast<AES_KEY*>(this->key)) < 0) {
          free(this->key);
          return -1;
        }
      }
      this->IV=IV;
      return 0;
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes192_cbc_cipher_struct::cbc_encrypt");
      AES_cbc_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_ENCRYPT);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes192_cbc_cipher_struct::cbc_decrypt");
      AES_cbc_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_DECRYPT);
    }

};

struct ssh_aes256_cbc_cipher_struct : public ssh_cipher_struct
{
    ssh_aes256_cbc_cipher_struct()
        : ssh_cipher_struct("aes256-cbc", 16, sizeof(AES_KEY), 256)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_aes256_cbc_cipher_struct::set_encrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        if (AES_set_encrypt_key(
                static_cast<const unsigned char *>(key),
                this->keysize,
                static_cast<AES_KEY*>(this->key)) < 0) {
          free(this->key);
          return -1;
        }
      }
      this->IV=IV;
      return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_aes256_cbc_cipher_struct::set_decrypt_key");
      if (this->key == nullptr) {
        if (this->alloc_key() < 0) {
          return -1;
        }
        if (AES_set_decrypt_key(
                static_cast<const unsigned char *>(key),
                this->keysize,
                static_cast<AES_KEY*>(this->key)) < 0) {
          free(this->key);
          return -1;
        }
      }
      this->IV=IV;
      return 0;
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes256_cbc_cipher_struct::cbc_encrypt");
      AES_cbc_encrypt(
        in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_ENCRYPT);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_aes256_cbc_cipher_struct::cbc_decrypt");
      AES_cbc_encrypt(in, out, len,
        static_cast<const AES_KEY*>(this->key),
        static_cast<unsigned char *>(this->IV),
        AES_DECRYPT);
    }
};

struct ssh_3des_cbc_cipher_struct : public ssh_cipher_struct
{
    ssh_3des_cbc_cipher_struct()
        : ssh_cipher_struct("3des-cbc", 8, sizeof(DES_key_schedule) * 3, 192)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//        LOG(LOG_INFO, "ssh_3des_cbc_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
              return -1;
            }
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key));
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key) + 1);
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key) + 2);
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key), static_cast<DES_key_schedule*>(this->key));
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key) + 1, static_cast<DES_key_schedule*>(this->key) + 1);
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key) + 2, static_cast<DES_key_schedule*>(this->key) + 2);
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_3des_cbc_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_3des_cbc_cipher_struct::cbc_encrypt");
      DES_ede3_cbc_encrypt(in, out, len,
          static_cast<DES_key_schedule*>(this->key),
          static_cast<DES_key_schedule*>(this->key) + 1,
          static_cast<DES_key_schedule*>(this->key) + 2,
          static_cast<unsigned char (*)[8]>(this->IV), 1);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_3des_cbc_cipher_struct::cbc_decrypt");
      DES_ede3_cbc_encrypt(in, out, len,
          static_cast<DES_key_schedule*>(this->key),
          static_cast<DES_key_schedule*>(this->key) + 1,
          static_cast<DES_key_schedule*>(this->key) + 2,
          static_cast<unsigned char (*)[8]>(this->IV), 0);
    }

};

struct ssh_3des_cbc_ssh1_cipher_struct : public ssh_cipher_struct
{
    ssh_3des_cbc_ssh1_cipher_struct()
        : ssh_cipher_struct("3des-cbc-ssh1", 8, sizeof(DES_key_schedule)*3, 192)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_3des_cbc_ssh1_cipher_struct::set_encrypt_key");
        if (this->key == nullptr) {
            if (this->alloc_key() < 0) {
              return -1;
            }
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key));
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key) + 1);
            DES_set_odd_parity(static_cast<unsigned char (*)[8]>(key) + 2);
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key), static_cast<DES_key_schedule*>(this->key));
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key) + 1, static_cast<DES_key_schedule*>(this->key) + 1);
            DES_set_key_unchecked(static_cast<unsigned char (*)[8]>(key) + 2, static_cast<DES_key_schedule*>(this->key) + 2);
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_3des_cbc_ssh1_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_3des_cbc_ssh1_cipher_struct::cbc_encrypt");
        DES_ncbc_encrypt(in, out, len,
                       static_cast<DES_key_schedule*>(this->key),
                       static_cast<unsigned char (*)[8]>(this->IV), 1);

        // the const_cast here is cheating, but it's either that or weakening prototype for other functions
        // (and we won't call that as it's ssh1 anyway, we should probably just remove class)
        DES_ncbc_encrypt(out, const_cast<unsigned char*>(in), len,
                       static_cast<DES_key_schedule*>(this->key) + 1,
                       static_cast<unsigned char (*)[8]>(this->IV) + 1, 0);

        DES_ncbc_encrypt(in, out, len,
                       static_cast<DES_key_schedule*>(this->key) + 2,
                       static_cast<unsigned char (*)[8]>(this->IV) + 2, 1);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_3des_cbc_ssh1_cipher_struct::cbc_decrypt");
        DES_ncbc_encrypt(in, out, len,
                        static_cast<DES_key_schedule*>(this->key) + 2,
                        static_cast<unsigned char (*)[8]>(this->IV), 0);

        // the const_cast here is cheating, but it's either that or weakening prototype for other functions
        // (and we won't call that as it's ssh1 anyway, we should probably just remove class)
        DES_ncbc_encrypt(out, const_cast<unsigned char*>(in), len,
                        static_cast<DES_key_schedule*>(this->key) + 1,
                        static_cast<unsigned char (*)[8]>(this->IV) + 1,
                        1);

        DES_ncbc_encrypt(in, out, len,
                        static_cast<DES_key_schedule*>(this->key),
                        static_cast<unsigned char (*)[8]>(this->IV) + 2,
                        0);
    }

};

struct ssh_des_cbc_ssh1_cipher_struct : public ssh_cipher_struct
{
    ssh_des_cbc_ssh1_cipher_struct()
        : ssh_cipher_struct("des-cbc-ssh1", 8, sizeof(DES_key_schedule), 64)
    {
    }

    int set_encrypt_key(void *key, void *IV) override
    {
//      LOG(LOG_INFO, "ssh_des_cbc_ssh1_cipher_struct::set_encrypt_key");
        if(!this->key){
            if (this->alloc_key() < 0) {
                return -1;
            }
            DES_set_odd_parity(static_cast<DES_cblock*>(key));
            DES_set_key_unchecked(
                static_cast<unsigned char (*)[8]>(key),
                static_cast<DES_key_schedule*>(this->key));
        }
        this->IV=IV;
        return 0;
    }

    int set_decrypt_key(void *key, void *IV) override {
//      LOG(LOG_INFO, "ssh_des_cbc_ssh1_cipher_struct::set_decrypt_key");
      return this->set_encrypt_key(key, IV);
    }

    void cbc_encrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_des_cbc_ssh1_cipher_struct::cbc_encrypt");
        DES_ncbc_encrypt(in, out, len,
                        static_cast<DES_key_schedule*>(this->key),
                        static_cast<unsigned char (*)[8]>(this->IV),
                        1);
    }

    void cbc_decrypt(const uint8_t *in, uint8_t *out, unsigned long len) override
    {
//      LOG(LOG_INFO, "ssh_des_cbc_ssh1_cipher_struct::cbc_decrypt");
        DES_ncbc_encrypt(in, out, len,
                        static_cast<DES_key_schedule*>(this->key),
                        static_cast<unsigned char (*)[8]>(this->IV),
                        0);
    }
};

struct ssh_crypto_struct {
    BIGNUM * e;
    BIGNUM * f;
    BIGNUM * x;
    BIGNUM * k;
    BIGNUM * y;

    struct ecdh {
        EC_KEY *privkey;
        std::vector<uint8_t> client_pubkey;
        std::vector<uint8_t> server_pubkey;

        ecdh()
            : privkey{}
            , client_pubkey{}
            , server_pubkey{}
        {
        }

        int build_k(BIGNUM * (& k), const std::vector<uint8_t> & ecdh_pubkey) {
          k = BN_new();
          if (k == nullptr) {
            return -1;
          }
          const EC_GROUP *group = EC_KEY_get0_group(this->privkey);
          EC_POINT * pubkey = EC_POINT_new(group);
          if (pubkey == nullptr) {
            return -1;
          }

          BN_CTX* ctx = BN_CTX_new();
          if (ctx == nullptr) {
              EC_POINT_clear_free(pubkey);
            return -1;
          }
          EC_POINT_oct2point(group, pubkey, &ecdh_pubkey[0], ecdh_pubkey.size(), ctx);
          int len = (EC_GROUP_get_degree(group) + 7) / 8;
          // TOOD: Check actual size necessary, Elliptic curve are supposed to used smaller keys than
          // other encryption schemes, but if it force dynamic memory allocation purpose is defeated
          uint8_t buffer[4096];
          int rc = ECDH_compute_key(buffer, len, pubkey, this->privkey, nullptr);
          EC_POINT_clear_free(pubkey);
          if (rc <= 0) {
              BN_CTX_free(ctx);
              return -1;
          }

          BN_bin2bn(buffer, len, k);
          // TODO: see why we deallocate privkey here ? (note done for curve_25519)
          EC_KEY_free(this->privkey);
          this->privkey = nullptr;
          BN_CTX_free(ctx);
          return 0;
        }
    } ecdh;

    struct curve_25519 {
        uint8_t privkey[CURVE25519_PRIVKEY_SIZE];
        uint8_t client_pubkey[CURVE25519_PUBKEY_SIZE];
        uint8_t server_pubkey[CURVE25519_PUBKEY_SIZE];
        curve_25519()
            : privkey{}
            , client_pubkey{}
            , server_pubkey{}
        {}

        void init(uint8_t (&pubkey)[CURVE25519_PUBKEY_SIZE])
        {
            /* Build server's keypair */
            RAND_bytes(this->privkey, CURVE25519_PRIVKEY_SIZE);
            // TODO: raise some exception if random fails
           const unsigned char base[32] = {9};
            InternalBignum BN;
            BN.crypto_scalarmult(pubkey, this->privkey, base);
        }

        void build_k(BIGNUM * (& k), uint8_t (&pubkey)[CURVE25519_PUBKEY_SIZE])
        {
            uint8_t tmpk[CURVE25519_PUBKEY_SIZE];
            InternalBignum BN;
            BN.crypto_scalarmult(tmpk, this->privkey, pubkey);

            k = BN_new();
            BN_bin2bn(tmpk, CURVE25519_PUBKEY_SIZE, k);
        }
    } curve_25519;

    std::vector<uint8_t> dh_server_signature; /* information used by dh_handshake. */
    size_t digest_len; /* len of all the fields below */
     /* Secret hash is same as session id until re-kex */
    uint8_t secret_hash[SHA256_DIGEST_LENGTH]; // max of SHA_DIGEST_LENGTH and SHA256_DIGEST_LENGTH
    unsigned char *encryptIV;
    unsigned char *decryptIV;
    unsigned char *decryptkey;
    unsigned char *encryptkey;
    unsigned char *encryptMAC;
    unsigned char *decryptMAC;
    unsigned char hmacbuf[SslSha1::DIGEST_LENGTH];
    struct ssh_cipher_struct *in_cipher, *out_cipher; /* the cipher structures/objects */
    std::vector<uint8_t> server_pubkey;
    const char *server_pubkey_type;
    int do_compress_out; /* idem */
    int do_compress_in; /* don't set them, set the option instead */
    int delayed_compress_in; /* Use of zlib@openssh.org */
    int delayed_compress_out;
    void *compress_out_ctx; /* don't touch it */
    void *compress_in_ctx; /* really, don't */
    /* kex sent by server, client, and mutually elected methods */
    struct ssh_kex_struct server_kex;
    struct ssh_kex_struct client_kex;
    SSHString kex_methods[SSH_KEX_METHODS];
    enum ssh_key_exchange_e kex_type;
    enum ssh_mac_e mac_type; /* Mac operations to use for key gen */

    ssh_crypto_struct()
        : e(nullptr)
        , f(nullptr)
        , x(nullptr)
        , k(nullptr)
        , y(nullptr)
        , ecdh()
        , curve_25519()
        , dh_server_signature(0) /* information used by dh_handshake. */
        , digest_len(0) /* len of all the fields below */
        , secret_hash{} /* Secret hash is same as session id until re-kex */
        , encryptIV(nullptr)
        , decryptIV(nullptr)
        , decryptkey(nullptr)
        , encryptkey(nullptr)
        , encryptMAC(nullptr)
        , decryptMAC(nullptr)
        , hmacbuf{}
        , in_cipher(nullptr)
        , out_cipher(nullptr) /* the cipher structures/objects */
        , server_pubkey{}
        , server_pubkey_type(nullptr)
        , do_compress_out(0) /* idem */
        , do_compress_in(0) /* don't set them, set the option instead */
        , delayed_compress_in(0) /* Use of zlib@openssh.org */
        , delayed_compress_out(0)
        , compress_out_ctx(nullptr) /* don't touch it */
        , compress_in_ctx(nullptr)  /* really, don't */
        /* kex sent by server, client, and mutually elected methods */
        , server_kex{}
        , client_kex{}
        , kex_methods{"","","","","","","","","",""}
        , kex_type(SSH_KEX_DH_GROUP1_SHA1)
        , mac_type(SSH_MAC_SHA1) /* Mac operations to use for key gen */
    {
    }

    ~ssh_crypto_struct()
    {

        if (this->in_cipher){
            if (this->in_cipher->key){
                memset(this->in_cipher->key, 0, this->in_cipher->keylen);
                free(this->in_cipher->key);
                this->in_cipher->key = nullptr;
            }
            free(this->in_cipher);
            this->in_cipher = nullptr;
        }
        if (this->out_cipher){
            if (this->out_cipher->key){
                memset(this->out_cipher->key, 0, this->out_cipher->keylen);
                free(this->out_cipher->key);
                this->out_cipher->key = nullptr;
            }
            free(this->out_cipher);
            this->out_cipher = nullptr;
        }

        BN_clear_free(this->e);
        BN_clear_free(this->f);
        BN_clear_free(this->x);
        BN_clear_free(this->y);
        BN_clear_free(this->k);

//        if(this->session_id != nullptr){
//            memset(this->session_id, '\0', this->digest_len);
//            free(this->session_id);
//            this->session_id = nullptr;
//        }

        if (this->compress_out_ctx
        && (deflateEnd(static_cast<z_stream_s*>(this->compress_out_ctx)) != 0)) {
            inflateEnd(static_cast<z_stream_s*>(this->compress_out_ctx));
        }

        if (this->compress_in_ctx
        && (deflateEnd(static_cast<z_stream_s*>(this->compress_in_ctx)) != 0)) {
            inflateEnd(static_cast<z_stream_s*>(this->compress_in_ctx));
        }

        free(this->encryptIV);
        this->encryptIV = nullptr;
        free(this->decryptIV);
        this->decryptIV = nullptr;
        free(this->encryptMAC);
        this->encryptMAC = nullptr;
        free(this->decryptMAC);
        this->decryptMAC = nullptr;
        if(this->encryptkey){
            memset(this->encryptkey, 0, this->digest_len);
            free(this->encryptkey);
            this->encryptkey = nullptr;
        }

        if(this->decryptkey){
            memset(this->decryptkey, 0, this->digest_len);
            free(this->decryptkey);
            this->decryptkey = nullptr;
        }

        memset(this, 0, sizeof(struct ssh_crypto_struct));
    }

        /* add data for the final cookie */
    int generate_session_keys_server(unsigned char * session_id, error_struct & error)
    {
        (void)error;
        unsigned int len3 = BN_num_bytes(this->k);
        unsigned int bits3 = BN_num_bits(this->k);
        /* If the first bit is set we have a negative number, padding needed */
        int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->k, bits3 - 1))?1:0;
        std::vector<uint8_t> k_string;
        k_string.resize(len3 + pad3);
//        SSHString k_string(len3 + pad3);
        /* if pad we have a negative number henceforth we need a leading zero */
        k_string[0] = 0;
        BN_bn2bin(this->k, &k_string[pad3]);

        /* IV */
        this->decryptIV = static_cast<uint8_t*>(malloc(this->digest_len));
        this->encryptIV = static_cast<uint8_t*>(malloc(this->digest_len));
        this->decryptkey = static_cast<uint8_t*>(malloc(this->digest_len * 2));
        this->encryptkey = static_cast<uint8_t*>(malloc(this->digest_len * 2));

        this->encryptMAC = static_cast<uint8_t*>(malloc(this->digest_len));
        this->decryptMAC = static_cast<uint8_t*>(malloc(this->digest_len));

        uint32_t k_size_n = htonl(len3+pad3);
        switch(this->mac_type){
        case SSH_MAC_SHA1:
        {
            // this->decryptIV, 'A'
            SslSha1 sha1_decryptIV;
            sha1_decryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptIV.update(&k_string[0], k_string.size());
            sha1_decryptIV.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptIV.update(byte_ptr_cast("A"), 1);
            sha1_decryptIV.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptIV.unchecked_final(this->decryptIV);

            // this->encryptIV, 'B'
            SslSha1 sha1_encryptIV;
            sha1_encryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptIV.update(&k_string[0], k_string.size());
            sha1_encryptIV.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptIV.update(byte_ptr_cast("B"), 1);
            sha1_encryptIV.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptIV.unchecked_final(this->encryptIV);

            // this->decryptkey, 'C'
            SslSha1 sha1_decryptkey;
            sha1_decryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptkey.update(&k_string[0], k_string.size());
            sha1_decryptkey.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptkey.update(byte_ptr_cast("C"), 1);
            sha1_decryptkey.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptkey.unchecked_final(this->decryptkey);

            if (this->in_cipher->keysize > this->digest_len * 8) {
                SslSha1 sha1;
                sha1.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha1.update(&k_string[0], k_string.size());
                sha1.update(this->secret_hash, SHA_DIGEST_LENGTH);
                sha1.update(this->decryptkey, SHA_DIGEST_LENGTH);
                sha1.unchecked_final(this->decryptkey + SHA_DIGEST_LENGTH);
            }

            // this->encryptkey, 'D'
            SslSha1 sha1_encryptkey;
            sha1_encryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptkey.update(&k_string[0], k_string.size());
            sha1_encryptkey.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptkey.update(byte_ptr_cast("D"), 1);
            sha1_encryptkey.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptkey.unchecked_final(this->encryptkey);

            if (this->out_cipher->keysize > this->digest_len * 8) {
                SslSha1 sha1;
                sha1.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha1.update(&k_string[0], k_string.size());
                sha1.update(this->secret_hash, SHA_DIGEST_LENGTH);
                sha1.update(this->encryptkey, SHA_DIGEST_LENGTH);
                sha1.unchecked_final(this->encryptkey + SHA_DIGEST_LENGTH);
            }

            // this->decryptMAC, 'E'
            SslSha1 sha1_decryptMAC;
            sha1_decryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptMAC.update(&k_string[0], k_string.size());
            sha1_decryptMAC.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptMAC.update(byte_ptr_cast("E"), 1);
            sha1_decryptMAC.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptMAC.unchecked_final(this->decryptMAC);

            // this->encryptMAC, 'F'
            SslSha1 sha1_encryptMAC;
            sha1_encryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptMAC.update(&k_string[0], k_string.size());
            sha1_encryptMAC.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptMAC.update(byte_ptr_cast("F"), 1);
            sha1_encryptMAC.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptMAC.unchecked_final(this->encryptMAC);
        }
        break;
        case SSH_MAC_SHA256:
        {
            // this->decryptIV, 'A'
            SslSha256 sha256_decryptIV;
            sha256_decryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptIV.update(&k_string[0], k_string.size());
            sha256_decryptIV.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptIV.update(byte_ptr_cast("A"), 1);
            sha256_decryptIV.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptIV.unchecked_final(this->decryptIV);

            // this->encryptIV, 'B'
            SslSha256 sha256_encryptIV;
            sha256_encryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptIV.update(&k_string[0], k_string.size());
            sha256_encryptIV.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptIV.update(byte_ptr_cast("B"), 1);
            sha256_encryptIV.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptIV.unchecked_final(this->encryptIV);

            // this->decryptkey, 'C'
            SslSha256 sha256_decryptkey;
            sha256_decryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptkey.update(&k_string[0], k_string.size());
            sha256_decryptkey.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptkey.update(byte_ptr_cast("C"), 1);
            sha256_decryptkey.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptkey.unchecked_final(this->decryptkey);

            /* some ciphers need more than DIGEST_LEN bytes of input key */
            if (this->in_cipher->keysize > this->digest_len * 8) {
                SslSha256 sha256;
                sha256.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha256.update(&k_string[0], k_string.size());
                sha256.update(this->secret_hash, SHA256_DIGEST_LENGTH);
                sha256.update(this->decryptkey, SHA256_DIGEST_LENGTH);
                sha256.unchecked_final(this->decryptkey + SHA256_DIGEST_LENGTH);
            }

            // this->encryptkey, 'D'
            SslSha256 sha256_encryptkey;
            sha256_encryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptkey.update(&k_string[0], k_string.size());
            sha256_encryptkey.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptkey.update(byte_ptr_cast("D"), 1);
            sha256_encryptkey.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptkey.unchecked_final(this->encryptkey);

            /* some ciphers need more than DIGEST_LEN bytes of input key */
            if (this->out_cipher->keysize > this->digest_len * 8) {
                SslSha256 sha256;
                sha256.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha256.update(&k_string[0], k_string.size());
                sha256.update(this->secret_hash, SHA256_DIGEST_LENGTH);
                sha256.update(this->encryptkey, SHA256_DIGEST_LENGTH);
                sha256.unchecked_final(this->encryptkey + SHA256_DIGEST_LENGTH);
            }

            // this->decryptMAC, 'E'
            SslSha256 sha256_decryptMAC;
            sha256_decryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptMAC.update(&k_string[0], k_string.size());
            sha256_decryptMAC.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptMAC.update(byte_ptr_cast("E"), 1);
            sha256_decryptMAC.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptMAC.unchecked_final(this->decryptMAC);

            // this->encryptMAC, 'F'
            SslSha256 sha256_encryptMAC;
            sha256_encryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptMAC.update(&k_string[0], k_string.size());
            sha256_encryptMAC.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptMAC.update(byte_ptr_cast("F"), 1);
            sha256_encryptMAC.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptMAC.unchecked_final(this->encryptMAC);
        }
        break;
        case SSH_MAC_SHA384:
        case SSH_MAC_SHA512:
        default:
            return -1;
        }
        return 0;
    }


    int generate_session_keys_client(unsigned char * session_id, error_struct & error)
    {
        (void)error;
        unsigned int len3 = BN_num_bytes(this->k);
        unsigned int bits3 = BN_num_bits(this->k);
        /* If the first bit is set we have a negative number, padding needed */
        int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->k, bits3 - 1))?1:0;
        std::vector<uint8_t> k_string;
        k_string.resize(len3 + pad3);
        /* if pad we have a negative number henceforth we need a leading zero */
        k_string[0] = 0;
        BN_bn2bin(this->k, &k_string[pad3]);

        // TODO: keep static space for these, no need to malloc
        this->encryptIV = static_cast<uint8_t*>(malloc(this->digest_len));
        this->decryptIV = static_cast<uint8_t*>(malloc(this->digest_len));
        this->encryptkey = static_cast<uint8_t*>(malloc(this->digest_len * 2));
        this->decryptkey = static_cast<uint8_t*>(malloc(this->digest_len * 2));
        this->encryptMAC = static_cast<uint8_t*>(malloc(this->digest_len));
        this->decryptMAC = static_cast<uint8_t*>(malloc(this->digest_len));
        // TODO: check memory allocation

        /* IV */
        uint32_t k_size_n = htonl(k_string.size());

        switch (this->mac_type)
        {
        case SSH_MAC_SHA1:
        {
            // this->encryptIV, 'A'
            SslSha1 sha1_encryptIV;
            sha1_encryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptIV.update(&k_string[0], k_string.size());
            sha1_encryptIV.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptIV.update(byte_ptr_cast("A"), 1);
            sha1_encryptIV.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptIV.unchecked_final(this->encryptIV);

            // this->decryptIV, 'B'
            SslSha1 sha1_decryptIV;
            sha1_decryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptIV.update(&k_string[0], k_string.size());
            sha1_decryptIV.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptIV.update(byte_ptr_cast("B"), 1);
            sha1_decryptIV.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptIV.unchecked_final(this->decryptIV);

            // this->encryptkey, 'C'
            SslSha1 sha1_encryptkey;
            sha1_encryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptkey.update(&k_string[0], k_string.size());
            sha1_encryptkey.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptkey.update(byte_ptr_cast("C"), 1);
            sha1_encryptkey.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptkey.unchecked_final(this->encryptkey);

            /* some ciphers need more than DIGEST_LEN bytes of input key */
            // TODO: wont work if need is more than 2 times DIGEST_LEN
            if (this->out_cipher->keysize > this->digest_len * 8) {
                SslSha1 sha1;
                sha1.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha1.update(&k_string[0], k_string.size());
                sha1.update(this->secret_hash, SHA_DIGEST_LENGTH);
                sha1.update(this->encryptkey, SHA_DIGEST_LENGTH);
                sha1.unchecked_final(this->encryptkey + SHA_DIGEST_LENGTH);
            }

            // this->decryptkey, 'D'
            SslSha1 sha1_decryptkey;
            sha1_decryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptkey.update(&k_string[0], k_string.size());
            sha1_decryptkey.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptkey.update(byte_ptr_cast("D"), 1);
            sha1_decryptkey.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptkey.unchecked_final(this->decryptkey);

            if (this->in_cipher->keysize > this->digest_len * 8) {
                SslSha1 sha1;
                sha1.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha1.update(&k_string[0], k_string.size());
                sha1.update(this->secret_hash, SHA_DIGEST_LENGTH);
                sha1.update(this->decryptkey, SHA_DIGEST_LENGTH);
                sha1.unchecked_final(this->decryptkey + SHA_DIGEST_LENGTH);
            }

            // this->encryptMAC, 'E'
            SslSha1 sha1_encryptMAC;
            sha1_encryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_encryptMAC.update(&k_string[0], k_string.size());
            sha1_encryptMAC.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_encryptMAC.update(byte_ptr_cast("E"), 1);
            sha1_encryptMAC.update(session_id, SHA_DIGEST_LENGTH);
            sha1_encryptMAC.unchecked_final(this->encryptMAC);

            // this->decryptMAC, 'F'
            SslSha1 sha1_decryptMAC;
            sha1_decryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha1_decryptMAC.update(&k_string[0], k_string.size());
            sha1_decryptMAC.update(this->secret_hash, SHA_DIGEST_LENGTH);
            sha1_decryptMAC.update(byte_ptr_cast("F"), 1);
            sha1_decryptMAC.update(session_id, SHA_DIGEST_LENGTH);
            sha1_decryptMAC.unchecked_final(this->decryptMAC);
        }
        break;
        case SSH_MAC_SHA256:
        {
            // this->encryptIV, 'A'
            SslSha256 sha256_encryptIV;
            sha256_encryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptIV.update(&k_string[0], k_string.size());
            sha256_encryptIV.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptIV.update(byte_ptr_cast("A"), 1);
            sha256_encryptIV.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptIV.unchecked_final(this->encryptIV);

            // this->decryptIV, 'B'
            SslSha256 sha256_decryptIV;
            sha256_decryptIV.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptIV.update(&k_string[0], k_string.size());
            sha256_decryptIV.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptIV.update(byte_ptr_cast("B"), 1);
            sha256_decryptIV.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptIV.unchecked_final(this->decryptIV);

            // this->encryptkey, 'C'
            SslSha256 sha256_encryptkey;
            sha256_encryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptkey.update(&k_string[0], k_string.size());
            sha256_encryptkey.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptkey.update(byte_ptr_cast("C"), 1);
            sha256_encryptkey.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptkey.unchecked_final(this->encryptkey);

            /* some ciphers need more than DIGEST_LEN bytes of input key */
            if (this->out_cipher->keysize > this->digest_len * 8) {
                SslSha256 sha256;
                sha256.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha256.update(&k_string[0], k_string.size());
                sha256.update(this->secret_hash, SHA256_DIGEST_LENGTH);
                sha256.update(this->encryptkey, SHA256_DIGEST_LENGTH);
                sha256.unchecked_final(this->encryptkey + SHA256_DIGEST_LENGTH);
            }

            // this->decryptkey, 'D'
            SslSha256 sha256_decryptkey;
            sha256_decryptkey.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptkey.update(&k_string[0], k_string.size());
            sha256_decryptkey.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptkey.update(byte_ptr_cast("D"), 1);
            sha256_decryptkey.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptkey.unchecked_final(this->decryptkey);

            if (this->in_cipher->keysize > this->digest_len * 8) {
                SslSha256 sha256;
                sha256.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
                sha256.update(&k_string[0], k_string.size());
                sha256.update(this->secret_hash, SHA256_DIGEST_LENGTH);
                sha256.update(this->decryptkey, SHA256_DIGEST_LENGTH);
                sha256.unchecked_final(this->decryptkey + SHA256_DIGEST_LENGTH);
            }
            // this->encryptMAC, 'E'
            SslSha256 sha256_encryptMAC;
            sha256_encryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_encryptMAC.update(&k_string[0], k_string.size());
            sha256_encryptMAC.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_encryptMAC.update(byte_ptr_cast("E"), 1);
            sha256_encryptMAC.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_encryptMAC.unchecked_final(this->encryptMAC);

            // this->decryptMAC, 'F'
            SslSha256 sha256_decryptMAC;
            sha256_decryptMAC.update(byte_ptr_cast(&k_size_n), sizeof(uint32_t));
            sha256_decryptMAC.update(&k_string[0], k_string.size());
            sha256_decryptMAC.update(this->secret_hash, SHA256_DIGEST_LENGTH);
            sha256_decryptMAC.update(byte_ptr_cast("F"), 1);
            sha256_decryptMAC.update(session_id, SHA256_DIGEST_LENGTH);
            sha256_decryptMAC.unchecked_final(this->decryptMAC);
        }
        break;
        case SSH_MAC_SHA384:
        case SSH_MAC_SHA512:
        default:
            return -1;
        }
        return 0;
    }


    unsigned char * packet_encrypt(uint32_t seq, uint8_t *data, uint32_t len, error_struct & error) {
        auto out = std::make_unique<uint8_t []>(len);

        // TODO: returning nullptr below hides actual errors that should be managed
        if(len % this->in_cipher->blocksize != 0){
            error.eid = SSH_FATAL;
            snprintf(error.error_buffer, sizeof(error.error_buffer),
                "Cryptographic functions must be set on at least one blocksize (received %u)",len);
            return nullptr;
        }

        if (this->out_cipher->set_encrypt_key(this->encryptkey, this->encryptIV) < 0) {
            return nullptr;
        }

        SslHMAC_Sha1 hmac_sha1(this->encryptMAC, SHA_DIGEST_LENGTH);
        hmac_sha1.update(byte_ptr_cast(&seq), sizeof(uint32_t));
        hmac_sha1.update(data, len);
        hmac_sha1.final(this->hmacbuf);

        // hexa("Packet hmac", this->hmacbuf, SHA_DIGEST_LENGTH);

        this->out_cipher->cbc_encrypt(static_cast<const unsigned char *>(data), &out[0], len);

        // TODO: replace with C++ equivalents
        memcpy(data, &out[0], len);

        return this->hmacbuf;
    }
};


static inline struct ssh_cipher_struct *cipher_new_by_name(const SSHString & name)
{
    if (strcmp(name.c_str(), "blowfish-cbc") == 0){
        return new ssh_blowfish_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes128-ctr") == 0){
        return new ssh_aes128_ctr_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes192-ctr") == 0){
        return new ssh_aes192_ctr_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes256-ctr") == 0){
        return new ssh_aes256_ctr_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes128-cbc") == 0){
        return new ssh_aes128_cbc_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes192-cbc") == 0){
        return new ssh_aes192_cbc_cipher_struct();
    }
    if (strcmp(name.c_str(), "aes256-cbc") == 0){
        return new ssh_aes256_cbc_cipher_struct();
    }
    if (strcmp(name.c_str(), "3des-cbc") == 0){
        return new ssh_3des_cbc_cipher_struct();
    }
    if (strcmp(name.c_str(), "3des-cbc-ssh1") == 0){
        return new ssh_3des_cbc_ssh1_cipher_struct();
    }
    if (strcmp(name.c_str(), "des-cbc-ssh1") == 0){
        return new ssh_des_cbc_ssh1_cipher_struct();
    }
    return nullptr;
}
