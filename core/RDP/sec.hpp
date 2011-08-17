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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common sec layer at core module

*/

#if !defined(__SEC_HPP__)
#define __SEC_HPP__

#warning ssl calls introduce some dependency on ssl system library, injecting it in the sec object would be better.
#include "ssl_calls.hpp"

/* used in sec */
struct mcs_channel_item {
    char name[16];
    int flags;
    int chanid;
    mcs_channel_item(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

struct Sec
{
    struct rdp_lic {
        uint8_t licence_key[16];
        uint8_t licence_sign_key[16];
        int licence_issued;

        rdp_lic(void){
            memset(this->licence_key, 0, 16);
            memset(this->licence_sign_key, 0, 16);
            this->licence_issued = 0;
        }
    } lic_layer;

    uint8_t * licence_data;
    size_t licence_size;

    int userid;
    vector<struct mcs_channel_item *> channel_list;

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];

    int decrypt_use_count;
    int encrypt_use_count;
    uint8_t decrypt_key[16];
    uint8_t encrypt_key[16];
    uint8_t decrypt_update_key[16];
    uint8_t encrypt_update_key[16];

    SSL_RC4 decrypt_rc4_info;
    SSL_RC4 encrypt_rc4_info;

    uint8_t crypt_level;
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */
    int rc4_key_len; /* 8 or 16 */
    uint8_t sign_key[16];

    char hostname[16];
    char username[128];

    Sec(uint8_t crypt_level) :
      licence_data(0),
      licence_size(0),
      decrypt_use_count(0),
      encrypt_use_count(0),
      crypt_level(crypt_level)
    {
        memset(this->decrypt_key, 0, 16);
        memset(this->encrypt_key, 0, 16);
        memset(this->decrypt_update_key, 0, 16);
        memset(this->encrypt_update_key, 0, 16);
        switch (crypt_level) {
        case 1:
        case 2:
            this->rc4_key_size = 1; /* 40 bits */
            this->rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->rc4_key_size = 2; /* 128 bits */
            this->rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }

    }

    ~Sec()
    {
        // clear channel_list
        int count = (int) this->channel_list.size();
        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                delete channel_item;
            }
        }
    }

    // Output a uint32 into a buffer (little-endian)
    static void sec_buf_out_uint32(uint8_t* buffer, int value)
    {
      buffer[0] = value & 0xff;
      buffer[1] = (value >> 8) & 0xff;
      buffer[2] = (value >> 16) & 0xff;
      buffer[3] = (value >> 24) & 0xff;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    static void rdp_sec_sign(uint8_t* signature, int siglen, uint8_t* session_key, int keylen,
                 uint8_t* data, int datalen)
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

      sec_buf_out_uint32(lenhdr, datalen);

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

    void sec_update(uint8_t* key, uint8_t* update_key, uint8_t rc4_key_len)
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
        ssl.sha1_update(&sha1, update_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, key, rc4_key_len);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, update_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, key);

        ssl.rc4_set_key(update, key, rc4_key_len);
        ssl.rc4_crypt(update, key, key, rc4_key_len);

        if (rc4_key_len == 8) {
            this->sec_make_40bit(key);
        }
    }

    void sec_make_40bit(uint8_t* key)
    {
        key[0] = 0xd1;
        key[1] = 0x26;
        key[2] = 0x9e;
    }

    // 16-byte transformation used to generate export keys (6.2.2).
    static void sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
    {
        SSL_MD5 md5;

        ssllib ssl;

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 16);
        ssl.md5_update(&md5, salt1, 32);
        ssl.md5_update(&md5, salt2, 32);
        ssl.md5_final(&md5, out);
    }

    // 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
    // Both SHA1 and MD5 algorithms are used.
    static void sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
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

    /* Encrypt data using RC4 */
    void sec_encrypt(uint8_t* data, int length)
    {
        ssllib ssl;

        if (this->encrypt_use_count == 4096){
            this->sec_update(this->encrypt_key, this->encrypt_update_key, this->rc4_key_len);
            ssl.rc4_set_key(this->encrypt_rc4_info, this->encrypt_key, this->rc4_key_len);
            this->encrypt_use_count = 0;
        }
        ssl.rc4_crypt(this->encrypt_rc4_info, data, data, length);
        this->encrypt_use_count++;
    }

    /* Decrypt data using RC4 */
    void sec_decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->decrypt_use_count == 4096) {
            this->sec_update(this->decrypt_key, this->decrypt_update_key, this->rc4_key_len);
            ssl.rc4_set_key(this->decrypt_rc4_info, this->decrypt_key, this->rc4_key_len);
            this->decrypt_use_count = 0;
        }
        ssl.rc4_crypt(this->decrypt_rc4_info, data, data, len);
        this->decrypt_use_count++;
    }

};

#endif
