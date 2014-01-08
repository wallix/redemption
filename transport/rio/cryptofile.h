/*
 * Copyright (c) 2013 WALLIX, SARL. All rights reserved.
 * Licensed computer software. Property of WALLIX.
 * Product Name : Wallix Admin Bastion
 * Author(s)    : Tristan de Cacqueray <tdc@wallix.com>
 * Id           : $Id$
 * URL          : $URL$
 */

#ifndef WABCRYPTOFILE_H
#define WABCRYPTOFILE_H

#include "openssl_crypto.hpp"
#include "openssl_evp.hpp"

enum crypto_file_state {
    CF_EOF = 1,
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

struct CryptoContext {
    unsigned char hmac_key[HMAC_KEY_LENGTH];
    unsigned char crypto_key[CRYPTO_KEY_LENGTH];
};

void get_derivator(const char * file, unsigned char * derivator, int derivator_len);
int compute_hmac(unsigned char * hmac, const unsigned char * key, const unsigned char * derivator);

//int dev_urandom_read(unsigned char * buf, int sz);

void * crypto_open_read (int systemfd, unsigned char * trace_key, struct CryptoContext * cctx);
void * crypto_open_write(int systemfd, unsigned char * trace_key, struct CryptoContext * cctx, const unsigned char * iv);

int    crypto_flush(void * cf);
int    crypto_read (void * cf, char * buf, unsigned int size);
int    crypto_write(void * cf, const char * buf, unsigned int size);
int    crypto_close(void * cf, unsigned char buf[MD_HASH_LENGTH << 1], unsigned char * hmac_key);

#endif
