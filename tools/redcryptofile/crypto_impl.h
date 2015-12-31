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
   Copyright (C) Wallix 2010-2014
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_CRYPTO_IMPL_H_
#define _REDEMPTION_TRANSPORT_RIO_CRYPTO_IMPL_H_

#include "cryptofile.h"

#ifdef __cplusplus
extern "C" {
# if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
# endif
#endif

#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>

/* File format V1:  ([...] represent an uint32_t)
 *
 * Header:
 *  [WABCRYPTOFILE_MAGIC][FILE_VERSION][Crypto IV]
 *
 * Chunk:
 *  [ciphered chunk size (size it takes on disk)][data]
 *
 * Footer:
 *  [WABCRYPTOFILE_EOF_MAGIC][raw_file_size]
 *
 */

/*****************************************************************************************************
 *                             WAB shared memory accessor procedures                                 *
 *****************************************************************************************************/
/* Attach, read and detach wab shared memory. Returns 0 on success, -1 on error.
 */
static inline int get_crypto_key(char * crypto_key)
{
    char tmp_buf[512] = {0};

    int shmid = shmget(2242, 512, 0600);
    if (shmid == -1){
        printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmget! error=%s\n", getpid(), strerror(errno));
        return -1;
    }
    char *shm = (char*)shmat(shmid, 0, 0);
    if (shm == (char *)-1){
        printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmat! error=%s\n", getpid(), strerror(errno));
        return -1;
    }
    unbase64(tmp_buf, 512, shm);
    if (shmdt(shm) == -1){
        printf("[CRYPTO_ERROR][%d]: Could not initialize crypto, shmdt! error=%s\n", getpid(), strerror(errno));
        return -1;
    }

    /* Extract the effective master key component, and check its control signature.
     * Returns 0 on success, -1 on error.
     */
    char sha256_computed[SHA256_DIGEST_LENGTH];

    if (SHA256((unsigned char *)(tmp_buf + SHA256_DIGEST_LENGTH+1),
        MKSALT_LEN+CRYPTO_KEY_LENGTH, (unsigned char *)sha256_computed) == 0)
    {
        printf("[CRYPTO_ERROR][%d]: Could not check crypto key, SHA256!\n", getpid());
        return -1;
    }

    if (strncmp(tmp_buf + 1, sha256_computed, SHA256_DIGEST_LENGTH)){
        printf("[CRYPTO_ERROR][%d]: Crypto key integrity check failed!\n", getpid());
        return -1;
    }

    memcpy(crypto_key, tmp_buf + SHA256_DIGEST_LENGTH+MKSALT_LEN+1, CRYPTO_KEY_LENGTH);
    return 0;
}

static inline int derivate_crypto_key(const char * crypto_key, char * hmac_key)
{
    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    return compute_hmac((unsigned char *)hmac_key, (const unsigned char *)crypto_key, HASH_DERIVATOR);
}

#ifdef __cplusplus
# if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
# endif
} //extern "C"
#endif

#endif
