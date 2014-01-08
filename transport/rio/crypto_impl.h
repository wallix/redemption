/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Main entry point file for RIO *Transport library
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_CRYPTO_IMPL_H_
#define _REDEMPTION_TRANSPORT_RIO_CRYPTO_IMPL_H_

#include <libgen.h>
#include <string.h>

/* 256 bits key size */
#define CRYPTO_KEY_LENGTH 32
#define HMAC_KEY_LENGTH   CRYPTO_KEY_LENGTH

struct CryptoContext {
    unsigned char hmac_key[HMAC_KEY_LENGTH];
    unsigned char crypto_key[CRYPTO_KEY_LENGTH];
};

#define MIN(x, y) (((x) > (y) ? (y) : (x)))

enum {
    DERIVATOR_LENGTH = 8
};

static inline int compute_hmac(unsigned char * hmac, const unsigned char * key, const unsigned char * derivator)
{
    unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do

    memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
    memcpy(tmp_derivation + DERIVATOR_LENGTH, key, CRYPTO_KEY_LENGTH);
    if (SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated) == NULL){
        printf("[CRYPTO_ERROR][%d]: Could not derivate hash crypto key, SHA256!\n", getpid());
        return -1;
    }
    memcpy(hmac, derivated, HMAC_KEY_LENGTH);
    return 0;
}

static inline void get_derivator(const char *const_file, unsigned char * derivator, int derivator_len)
{
     // generate key derivator as SHA256(basename)
    char * file = strdupa(const_file);
    char * file_basename = basename(file);
    char tmp_derivated[SHA256_DIGEST_LENGTH];
    if (SHA256((unsigned char *)file_basename, strlen(file_basename), (unsigned char *)tmp_derivated) == (void *) 0){
        printf("[CRYPTO_ERROR][%d]: Could not derivate trace crypto key, SHA256 from=%s!\n", getpid(), file_basename);
        return;
    }
    memcpy(derivator, tmp_derivated, MIN(DERIVATOR_LENGTH, SHA256_DIGEST_LENGTH));
}

#endif
