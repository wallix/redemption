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

#include <libgen.h>
#include <string.h>

#include "cryptofile.hpp"

extern "C" {

/**********************************************
 *                Public API                  *
 **********************************************/

int compute_hmac(unsigned char * hmac, const unsigned char * key, const unsigned char * derivator)
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

void get_derivator(const char *const_file, unsigned char * derivator, int derivator_len)
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

void * crypto_open_read(int systemfd, unsigned char * trace_key,  struct CryptoContext * cctx)
{
    try
    {
        struct crypto_file * cf_struct = new struct crypto_file();

        if (-1 == cf_struct->open_read_init(systemfd, trace_key, cctx)) {
            delete cf_struct;
            return NULL;
        }

        return cf_struct;
    }
    catch (...)
    {
        return NULL;
    }
}

void * crypto_open_write(int systemfd, unsigned char * trace_key, struct CryptoContext * cctx, const unsigned char * iv)
{
    try
    {
        struct crypto_file * cf_struct = new struct crypto_file();

        if (-1 == cf_struct->open_write_init(systemfd, trace_key, cctx, iv)) {
            delete cf_struct;
            return NULL;
        }

        return cf_struct;
    }
    catch (...)
    {
        return NULL;
    }
}

/* Flush procedure (compression, encryption, effective file writing)
 * Return 0 on success, -1 on error
 */
int crypto_flush(void * cf) {
    struct crypto_file * cf_struct = (struct crypto_file *)cf;

    return cf_struct->flush();
}

/* The actual read method. Read chunks until we reach requested size.
 * Return the actual size read into buf, -1 on error
 */
int crypto_read(void * cf, char * buf, unsigned int buf_size) {
    struct crypto_file * cf_struct = (struct crypto_file *)cf;

    return cf_struct->read(buf, buf_size);
}

/* Actually appends data to crypto_file buffer, flush if buffer gets full
 * Return the written size, -1 on error
 */
int crypto_write(void *cf, const char * buf, unsigned int size)
{
    struct crypto_file * cf_struct = (struct crypto_file *)cf;

    return cf_struct->write(buf, size);
}

int crypto_close(void *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
{
    struct crypto_file * cf_struct = (struct crypto_file *)cf;

    int nResult = cf_struct->close(hash, hmac_key);

    delete cf_struct;

    return nResult;
}

} // extern "C"
