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
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__SEC_UTILS_HPP__)
#define __SEC_UTILS_HPP__

#include "ssl_calls.hpp"

// 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
// Both SHA1 and MD5 algorithms are used.
static inline void sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
{
    uint8_t shasig[20];
    uint8_t pad[4];
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    ssllib ssl;

    for (int i = 0; i < 3; i++) {
        memset(pad, salt + i, i + 1);

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, pad, i + 1);
        ssl.sha1_update(&sha1, in, 48);
        ssl.sha1_update(&sha1, salt1, 32);
        ssl.sha1_update(&sha1, salt2, 32);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, &out[i * 16]);
    }
}

// 16-byte transformation used to generate export keys (6.2.2).
static inline void sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
{
    SSL_MD5 md5;

    ssllib ssl;

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, in, 16);
    ssl.md5_update(&md5, salt1, 32);
    ssl.md5_update(&md5, salt2, 32);
    ssl.md5_final(&md5, out);
}


inline static void rdp_sec_generate_keys(CryptContext & encrypt, CryptContext & decrypt, uint8_t (& sign_key)[16], uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
{
    uint8_t pre_master_secret[48];
    uint8_t master_secret[48];
    uint8_t key_block[48];

    /* Construct pre-master secret (session key) */
    memcpy(pre_master_secret, client_random, 24);
    memcpy(pre_master_secret + 24, server_random, 24);

    /* Generate master secret and then key material */
    sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
    sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

    /* First 16 bytes of key material is MAC secret */
    memcpy(sign_key, key_block, 16);

    /* Generate export keys from next two blocks of 16 bytes */
    sec_hash_16(decrypt.key, &key_block[16], client_random, server_random);
    sec_hash_16(encrypt.key, &key_block[32], client_random, server_random);

    if (rc4_key_size == 1) {
        // LOG(LOG_DEBUG, "40-bit encryption enabled");
        sec_make_40bit(sign_key);
        sec_make_40bit(decrypt.key);
        sec_make_40bit(encrypt.key);
        decrypt.rc4_key_len = 8;
        encrypt.rc4_key_len = 8;
    }
    else {
        //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled", rc4_key_size);
        decrypt.rc4_key_len = 16;
        encrypt.rc4_key_len = 16;
    }

    /* Save initial RC4 keys as update keys */
    memcpy(decrypt.update_key, decrypt.key, 16);
    memcpy(encrypt.update_key, encrypt.key, 16);

    ssllib ssl;

    ssl.rc4_set_key(decrypt.rc4_info, decrypt.key, decrypt.rc4_key_len);
    ssl.rc4_set_key(encrypt.rc4_info, encrypt.key, encrypt.rc4_key_len);
}

#endif
