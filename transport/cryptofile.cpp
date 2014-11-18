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
#include <cstdio>
#include <unistd.h>

#include "cryptofile.h"

extern "C" {

/*****************************************************************************************************
 *                               Shared memory accessor procedures                                   *
 *****************************************************************************************************/
/* Standard unbase64, store result in buffer. Returns written bytes
 */
size_t unbase64(char *buffer, size_t bufsiz, const char *txt)
{
    const unsigned char _base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int bits = 0;
    int nbits = 0;
    char base64tbl[256];
    char v;
    size_t nbytes = 0;

    memset(base64tbl, -1, sizeof base64tbl);

    for (unsigned i = 0; _base64chars[i]; i++) {
        base64tbl[_base64chars[i]] = i;
    }

    base64tbl['.'] = 62;
    base64tbl['-'] = 62;
    base64tbl['_'] = 63;

    while (*txt) {
        if ((v = base64tbl[(unsigned char)*txt]) >= 0) {
            bits <<= 6;
            bits += v;
            nbits += 6;
            if (nbits >= 8) {
                if (nbytes < bufsiz)
                    *buffer++ = (bits >> (nbits - 8));
                nbytes++;
                nbits -= 8;
            }
        }
        txt++;
    }

    return nbytes;
}

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
        std::printf("[CRYPTO_ERROR][%d]: Could not derivate hash crypto key, SHA256!\n", getpid());
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
        std::printf("[CRYPTO_ERROR][%d]: Could not derivate trace crypto key, SHA256 from=%s!\n", getpid(), file_basename);
        return;
    }
    memcpy(derivator, tmp_derivated, MIN(derivator_len, SHA256_DIGEST_LENGTH));
}

} // extern "C"
