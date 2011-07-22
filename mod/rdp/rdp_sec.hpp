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

   rdp secure layer header

*/

#if !defined(__RDP_SEC_HPP__)
#define __RDP_SEC_HPP__

#include "x224.hpp"
#include "rdp_mcs.hpp"
#include "client_mod.hpp"

#include "constants.hpp"
#include "ssl_calls.hpp"

#include <stdint.h>

/* sec */
struct rdp_sec {
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

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];
    Stream client_mcs_data;
    struct rdp_mcs mcs_layer;
    int decrypt_use_count;
    int encrypt_use_count;
    uint8_t decrypt_key[16];
    uint8_t encrypt_key[16];
    uint8_t decrypt_update_key[16];
    uint8_t encrypt_update_key[16];
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */
    int rc4_key_len; /* 8 or 16 */
    int crypt_level; /* 1 = low, 2 = medium, 3 = high */
    uint8_t sign_key[16];
    SSL_RC4 rc4_decrypt_key;
    SSL_RC4 rc4_encrypt_key;
    int channel_code;
    int server_public_key_len;
    uint16_t server_rdp_version;
    char hostname[16];
    char username[128];
    int & use_rdp5;

    rdp_sec(Transport * t, int & use_rdp5, const char * hostname, const char * username)
        : mcs_layer(t),
          decrypt_use_count(0),
          encrypt_use_count(0),
          rc4_key_size(0), /* 1 = 40-bit, 2 = 128-bit */
          rc4_key_len(0),  /* 8 or 16 */
          crypt_level(0),  /* 1 = low, 2 = medium, 3 = high */
          channel_code(1),
          server_public_key_len(0),  /* static virtual channels accepted bu default*/
          server_rdp_version(0),
          use_rdp5(use_rdp5) {
        #warning and if hostname is really larger, what happens ? We should at least emit a warning log
        strncpy(this->hostname, hostname, 15);
        this->hostname[15] = 0;
        #warning and if username is really larger, what happens ? We should at least emit a warning log
        strncpy(this->username, username, 127);
        this->username[127] = 0;
        memset(this->client_crypt_random, 0, 512);
        memset(this->decrypt_key, 0, 16);
        memset(this->encrypt_key, 0, 16);
        memset(this->decrypt_update_key, 0, 16);
        memset(this->encrypt_update_key, 0, 16);
        memset(this->sign_key, 0, 16);
    }

    ~rdp_sec()
    {
    }

    void rdp_lic_generate_hwid(uint8_t* hwid)
    {
        this->rdp_sec_buf_out_uint32(hwid, 2);
        memcpy(hwid + 4, this->hostname, LICENCE_HWID_SIZE - 4);
    }

    void rdp_lic_process_authreq(Stream & stream)
    {
        const uint8_t* in_token;
        const uint8_t* in_sig;
        uint8_t out_token[LICENCE_TOKEN_SIZE];
        uint8_t decrypt_token[LICENCE_TOKEN_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];
        uint8_t crypt_hwid[LICENCE_HWID_SIZE];
        uint8_t sealed_buffer[LICENCE_TOKEN_SIZE + LICENCE_HWID_SIZE];
        uint8_t out_sig[LICENCE_SIGNATURE_SIZE];
        uint8_t* crypt_key;

        in_token = 0;
        in_sig = 0;
        /* Parse incoming packet and save the encrypted token */
        stream.skip_uint8(6); /* unknown: f8 3d 15 00 04 f6 */

        int tokenlen = stream.in_uint16_le();
        if (tokenlen != LICENCE_TOKEN_SIZE) {
            LOG(LOG_ERR, "token len = %d, expected %d\n", tokenlen, LICENCE_TOKEN_SIZE);
        }
        else{
            in_token = stream.in_uint8p(tokenlen);
            in_sig = stream.in_uint8p(LICENCE_SIGNATURE_SIZE);
            stream.check_end();
        }

        memcpy(out_token, in_token, LICENCE_TOKEN_SIZE);
        /* Decrypt the token. It should read TEST in Unicode. */
        crypt_key = ssl_rc4_info_create();
        ssl_rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        memcpy(decrypt_token, in_token, LICENCE_TOKEN_SIZE);
        ssl_rc4_crypt(crypt_key, decrypt_token, LICENCE_TOKEN_SIZE);
        /* Generate a signature for a buffer of token and HWID */
        this->rdp_lic_generate_hwid(hwid);
        memcpy(sealed_buffer, decrypt_token, LICENCE_TOKEN_SIZE);
        memcpy(sealed_buffer + LICENCE_TOKEN_SIZE, hwid, LICENCE_HWID_SIZE);
        this->rdp_sec_sign(out_sig, 16,
                           this->lic_layer.licence_sign_key,
                           16, sealed_buffer,
                           sizeof(sealed_buffer));
        /* Now encrypt the HWID */
        ssl_rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        memcpy(crypt_hwid, hwid, LICENCE_HWID_SIZE);
        ssl_rc4_crypt(crypt_key, crypt_hwid, LICENCE_HWID_SIZE);
        this->rdp_lic_send_authresp(out_token, crypt_hwid, out_sig);
        ssl_rc4_info_delete(crypt_key);
    }

    void rdp_lic_send_authresp(uint8_t* token, uint8_t* crypt_hwid, uint8_t* signature)
    {
        int sec_flags = SEC_LICENCE_NEG;
        int length = 58;
        Stream stream(8192);
        this->rdp_sec_init(stream, sec_flags);
        stream.out_uint8(LICENCE_TAG_AUTHRESP);
        stream.out_uint8(2); /* version */
        stream.out_uint16_le(length);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_TOKEN_SIZE);
        stream.out_copy_bytes(token, LICENCE_TOKEN_SIZE);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_HWID_SIZE);
        stream.out_copy_bytes(crypt_hwid, LICENCE_HWID_SIZE);
        stream.out_copy_bytes(signature, LICENCE_SIGNATURE_SIZE);
        stream.mark_end();
        this->rdp_sec_send(stream, sec_flags);
    }

    void rdp_lic_process_demand(Stream & stream)
    {
        uint8_t null_data[SEC_MODULUS_SIZE];
        uint8_t signature[LICENCE_SIGNATURE_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];
        uint8_t* licence_data;
        int licence_size;
        uint8_t* crypt_key;

        licence_data = 0;
        /* Retrieve the server random from the incoming packet */
        const uint8_t * server_random = stream.in_uint8p(SEC_RANDOM_SIZE);

        // RDP licence generate key
        {
            /* We currently use null client keys. This is a bit naughty but, hey,
               the security of licence negotiation isn't exactly paramount. */
            memset(null_data, 0, sizeof(null_data));
            uint8_t* client_random = null_data;
            uint8_t* pre_master_secret = null_data;
            uint8_t master_secret[48];
            uint8_t key_block[48];

            /* Generate master secret and then key material */
            this->rdp_sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 65);
            this->rdp_sec_hash_48(key_block, master_secret, server_random, client_random, 65);
            /* Store first 16 bytes of session key as MAC secret */
            memcpy(this->lic_layer.licence_sign_key, key_block, 16);
            /* Generate RC4 key from next 16 bytes */
            this->rdp_sec_hash_16(this->lic_layer.licence_key, key_block + 16, client_random, server_random);
        }

        licence_size = 0;

        licence_size = this->rdp_load_licence(&licence_data);
        if (licence_size > 0) {
            /* Generate a signature for the HWID buffer */
            this->rdp_lic_generate_hwid(hwid);
            this->rdp_sec_sign(signature, 16, this->lic_layer.licence_sign_key, 16,
                         hwid, sizeof(hwid));
            /* Now encrypt the HWID */
            crypt_key = ssl_rc4_info_create();
            ssl_rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
            ssl_rc4_crypt(crypt_key, hwid, sizeof(hwid));
            ssl_rc4_info_delete(crypt_key);
            this->rdp_lic_present(null_data, null_data, licence_data,
                            licence_size, hwid, signature);
            delete(licence_data);
            return;
        }
        this->rdp_lic_send_request(null_data, null_data);
    }

    void rdp_lic_send_request(uint8_t* client_random, uint8_t* rsa_data)
    {
        int sec_flags = SEC_LICENCE_NEG;
        int userlen = strlen(this->username) + 1;
        int hostlen = strlen(this->hostname) + 1;
        int length = 128 + userlen + hostlen;
        Stream stream(8192);
        this->rdp_sec_init(stream, sec_flags);
        stream.out_uint8(LICENCE_TAG_REQUEST);
        stream.out_uint8(2); /* version */
        stream.out_uint16_le(length);
        stream.out_uint32_le(1);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0xff01);
        stream.out_copy_bytes(client_random, SEC_RANDOM_SIZE);
        stream.out_uint16_le(0);
        stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
        stream.out_copy_bytes(rsa_data, SEC_MODULUS_SIZE);
        stream.out_clear_bytes(SEC_PADDING_SIZE);
        stream.out_uint16_le(LICENCE_TAG_USER);
        stream.out_uint16_le(userlen);
        stream.out_copy_bytes(this->username, userlen);
        stream.out_uint16_le(LICENCE_TAG_HOST);
        stream.out_uint16_le(hostlen);
        stream.out_copy_bytes(this->hostname, hostlen);
        stream.mark_end();
        this->rdp_sec_send(stream, sec_flags);
    }

    void rdp_lic_present(uint8_t* client_random, uint8_t* rsa_data,
                uint8_t* licence_data, int licence_size, uint8_t* hwid,
                uint8_t* signature)
    {
        int sec_flags = SEC_LICENCE_NEG;
        int length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                 licence_size + LICENCE_HWID_SIZE + LICENCE_SIGNATURE_SIZE;
        Stream stream(8192);
        this->rdp_sec_init(stream, sec_flags);
        stream.out_uint8(LICENCE_TAG_PRESENT);
        stream.out_uint8(2); /* version */
        stream.out_uint16_le(length);
        stream.out_uint32_le(1);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0x0201);
        stream.out_copy_bytes(client_random, SEC_RANDOM_SIZE);
        stream.out_uint16_le(0);
        stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
        stream.out_copy_bytes(rsa_data, SEC_MODULUS_SIZE);
        stream.out_clear_bytes( SEC_PADDING_SIZE);
        stream.out_uint16_le(1);
        stream.out_uint16_le(licence_size);
        stream.out_copy_bytes(licence_data, licence_size);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_HWID_SIZE);
        stream.out_copy_bytes(hwid, LICENCE_HWID_SIZE);
        stream.out_copy_bytes(signature, LICENCE_SIGNATURE_SIZE);
        stream.mark_end();
        this->rdp_sec_send(stream, sec_flags);
    }

    #warning see string management for load_licence and save_licence
    int rdp_load_licence(uint8_t **data)
    {
      struct stat st;

      #warning beware of memory allocation here
      /* TODO: verify if location that we've stablished is right or not */
      char * path = new char [256];
      sprintf(path, "etc/xrdp/.xrdp/licence.%s", this->hostname);

      int fd = open(path, O_RDONLY);
      if (fd == -1){
        return -1;
      }
      if (fstat(fd, &st)){
        return -1;
      }
      #warning beware of memory allocation here
      *data = new uint8_t[st.st_size];
      int length = read(fd, *data, st.st_size);
      close(fd);
      delete [] path;

      return length;
    }

    #warning see what's the use of rdp_save_licence below. Probably useless obsolete code, to remove
    void rdp_save_licence(uint8_t *data, int length)
    {
      int fd;
      char* path = NULL;
      char* tmppath = NULL;

      path = new char[256];
      /* TODO: verify if location that we've stablished is right or not */
      sprintf(path, "/etc/xrdp./xrdp/licence.%s", this->hostname);

      if ((mkdir(path, 0700) == -1))
      {
        if (errno != EEXIST){
          perror(path);
          return;
        }
      }

      /* write licence to licence.hostname.new and after rename to licence.hostname */

      sprintf(path, "/etc/xrdp./xrdp/licence.%s", this->hostname);
      tmppath = new char[256];
      strcpy(tmppath, path);
      strcat(tmppath, ".new");

      fd = open(tmppath, O_WRONLY | O_CREAT | O_TRUNC, 0600);

      if (fd == -1){
        perror(tmppath);
        return;
      }
      if (write(fd, data, length) != length){
        perror(tmppath);
        unlink(tmppath);
      }
      else if (rename(tmppath, path) == -1){
        printf("Error renaming licence file\n");
        unlink(tmppath);
      }
      close(fd);
      delete [] tmppath;
      delete [] path;
    }

    void rdp_lic_process_issue(Stream & stream)
    {
        uint8_t* crypt_key;
        int length;
        int check;
        int i;

        stream.skip_uint8(2); /* 3d 45 - unknown */
        length = stream.in_uint16_le();
        if (!stream.check_rem(length)) {
            return;
        }
        crypt_key = ssl_rc4_info_create();
        ssl_rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        ssl_rc4_crypt(crypt_key, stream.p, length);
        ssl_rc4_info_delete(crypt_key);
        check = stream.in_uint16_le();
        if (check != 0) {
            return;
        }
        this->lic_layer.licence_issued = 1;
        stream.skip_uint8(2); /* pad */
        /* advance to fourth string */
        length = 0;
        for (i = 0; i < 4; i++) {
            stream.skip_uint8(length);
            length = stream.in_uint32_le();
            if (!stream.check_rem(length)) {
                return;
            }
        }
        /* todo save_licence(stream.p, length); */
        this->rdp_save_licence(stream.p, length);
    }

    void rdp_lic_process(Stream & stream)
    {
        uint8_t tag = stream.in_uint8();
        stream.skip_uint8(3); /* version, length */
        switch (tag) {
        case LICENCE_TAG_DEMAND:
            this->rdp_lic_process_demand(stream);
            break;
        case LICENCE_TAG_AUTHREQ:
            this->rdp_lic_process_authreq(stream);
            break;
        case LICENCE_TAG_ISSUE:
            this->rdp_lic_process_issue(stream);
            break;
        case LICENCE_TAG_REISSUE:
        case LICENCE_TAG_RESULT:
            break;
        default:
            break;
            /* todo unimpl("licence tag 0x%x\n", tag); */
        }
    }

    // 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
    // Both SHA1 and MD5 algorithms are used.
    static void rdp_sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
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
    static void rdp_sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
    {
        SSL_MD5 md5;

        ssllib ssl;

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 16);
        ssl.md5_update(&md5, salt1, 32);
        ssl.md5_update(&md5, salt2, 32);
        ssl.md5_final(&md5, out);
    }


    /* Reduce key entropy from 64 to 40 bits */
    void rdp_sec_make_40bit(uint8_t* key)
    {
        key[0] = 0xd1;
        key[1] = 0x26;
        key[2] = 0x9e;
    }

    /*****************************************************************************/
    void rdp_sec_generate_keys(uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
    {
        uint8_t pre_master_secret[48];
        uint8_t master_secret[48];
        uint8_t key_block[48];

        /* Construct pre-master secret */
        memcpy(pre_master_secret, client_random, 24);
        memcpy(pre_master_secret + 24, server_random, 24);

        /* Generate master secret and then key material */
        this->rdp_sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
        this->rdp_sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

        /* First 16 bytes of key material is MAC secret */
        memcpy(this->sign_key, key_block, 16);

        /* Generate export keys from next two blocks of 16 bytes */
        this->rdp_sec_hash_16(this->decrypt_key, &key_block[16], client_random, server_random);
        this->rdp_sec_hash_16(this->encrypt_key, &key_block[32], client_random, server_random);

        if (rc4_key_size == 1) {
            // LOG(LOG_DEBUG, "40-bit encryption enabled\n");
            this->rdp_sec_make_40bit(this->sign_key);
            this->rdp_sec_make_40bit(this->decrypt_key);
            this->rdp_sec_make_40bit(this->encrypt_key);
            this->rc4_key_len = 8;
        }
        else {
            //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled\n", rc4_key_size);
            this->rc4_key_len = 16;
        }

        /* Save initial RC4 keys as update keys */
        memcpy(this->decrypt_update_key, this->decrypt_key, 16);
        memcpy(this->encrypt_update_key, this->encrypt_key, 16);

        ssllib ssl;

        ssl.rc4_set_key(&(this->rc4_decrypt_key), this->decrypt_key, this->rc4_key_len);
        ssl.rc4_set_key(&(this->rc4_encrypt_key), this->encrypt_key, this->rc4_key_len);
    }

    // Output a uint32 into a buffer (little-endian)
    static void rdp_sec_buf_out_uint32(uint8_t* buffer, int value)
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

      rdp_sec_buf_out_uint32(lenhdr, datalen);

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

    /* update an encryption key */
    void rdp_sec_update(uint8_t* key, uint8_t* update_key)
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
        ssl.sha1_update(&sha1, key, this->rc4_key_len);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, update_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, key);

        ssl.rc4_set_key(&update, key, this->rc4_key_len);
        ssl.rc4_crypt(&update, key, key, this->rc4_key_len);

        if (this->rc4_key_len == 8) {
            this->rdp_sec_make_40bit(key);
        }
    }

    /* Encrypt data using RC4 */
    void rdp_sec_encrypt(uint8_t* data, int length)
    {
        ssllib ssl;

        if (this->encrypt_use_count == 4096){
            this->rdp_sec_update(this->encrypt_key, this->encrypt_update_key);
            ssl.rc4_set_key(&(this->rc4_encrypt_key), this->encrypt_key, this->rc4_key_len);
            this->encrypt_use_count = 0;
        }
        ssl.rc4_crypt(&(this->rc4_encrypt_key), data, data, length);
        this->encrypt_use_count++;
    }

    /* Decrypt data using RC4 */
    void rdp_sec_decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->decrypt_use_count == 4096) {
            this->rdp_sec_update(this->decrypt_key, this->decrypt_update_key);
            ssl.rc4_set_key(&(this->rc4_decrypt_key), this->decrypt_key, this->rc4_key_len);
            this->decrypt_use_count = 0;
        }
        ssl.rc4_crypt(&(this->rc4_decrypt_key), data, data, len);
        this->decrypt_use_count++;
    }

    /* Perform an RSA public key encryption operation */
    static void rdp_sec_rsa_encrypt(uint8_t * out, uint8_t * in, uint8_t len, uint32_t modulus_size, uint8_t * modulus, uint8_t * exponent)
    {
        ssllib ssl;

        ssl.rsa_encrypt(out, in, len, modulus_size, modulus, exponent);
    }

    /* Initialise secure transport packet */
    void rdp_sec_init(Stream & stream, uint32_t flags)
    {
      this->mcs_layer.rdp_mcs_init(stream);

      int hdrlen = (flags & SEC_ENCRYPT)           ? 12
                 : this->lic_layer.licence_issued ? 0
                 : 4 ;

      stream.sec_hdr = stream.p;
      stream.p += hdrlen;
    }

    /* Transmit secure transport packet over specified channel */
    void rdp_sec_send_to_channel(Stream & stream, uint32_t flags, uint16_t channel)
    {
        stream.p = stream.sec_hdr;
        if (!this->lic_layer.licence_issued || (flags & SEC_ENCRYPT)){
                stream.out_uint32_le(flags);
        }

        if (flags & SEC_ENCRYPT){
            flags &= ~SEC_ENCRYPT;
            int datalen = stream.end - stream.p - 8;

            this->rdp_sec_sign(stream.p, 8, this->sign_key, this->rc4_key_len, stream.p + 8, datalen);
            this->rdp_sec_encrypt(stream.p + 8, datalen);
        }

        this->mcs_layer.rdp_mcs_send_to_channel(stream, channel);
    }

    /* Transmit secure transport packet */
    void rdp_sec_send(Stream & stream, uint32_t flags)
    {
        this->rdp_sec_send_to_channel(stream, flags, MCS_GLOBAL_CHANNEL);
    }


    /* Transfer the client random to the server */
    void rdp_sec_establish_key()
    {
        Stream stream(8192);

        int length = this->server_public_key_len + SEC_PADDING_SIZE;
        int flags = SEC_CLIENT_RANDOM;

        this->rdp_sec_init(stream, flags);

        stream.out_uint32_le(length);
        LOG(LOG_INFO, "Server public key is %d bytes long", this->server_public_key_len);
        stream.out_copy_bytes(this->client_crypt_random, this->server_public_key_len);
        stream.out_clear_bytes(SEC_PADDING_SIZE);

        stream.mark_end();
        this->rdp_sec_send(stream, flags);
    }

    /* Parse a public key structure */
    void rdp_sec_parse_public_key(Stream & stream, uint8_t* modulus, uint8_t* exponent)
    {
        uint32_t magic;
        uint32_t modulus_len;

        magic = stream.in_uint32_le();
        if (magic != SEC_RSA_MAGIC) {
            LOG(LOG_WARNING, "RSA magic 0x%x\n", magic);
            throw Error(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK);
        }
        modulus_len = stream.in_uint32_le();
        modulus_len -= SEC_PADDING_SIZE;

        if ((modulus_len < SEC_MODULUS_SIZE)
        ||  (modulus_len > SEC_MAX_MODULUS_SIZE)) {
            LOG(LOG_WARNING, "Bad server public key size (%u bits)\n", modulus_len * 8);
            throw Error(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK);
        }
        stream.skip_uint8(8); /* modulus_bits, unknown */
        memcpy(exponent, stream.in_uint8p(SEC_EXPONENT_SIZE), SEC_EXPONENT_SIZE);
        memcpy(modulus, stream.in_uint8p(modulus_len), modulus_len);
        stream.skip_uint8(SEC_PADDING_SIZE);
        this->server_public_key_len = modulus_len;

        if (!stream.check()){
            throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
        }
    }


    /* Parse a public key structure */
    int rdp_sec_parse_public_sig(Stream & stream, int len, uint8_t* modulus, uint8_t* exponent)
    {
        uint8_t signature[SEC_MAX_MODULUS_SIZE];
        uint32_t sig_len;

        #warning check that. Why is it ok if signature len is not of the right size ?
        #warning Use Exception instead of return value for error cases.
        if (len != 72){
            return 1;
        }
        memset(signature, 0, sizeof(signature));
        sig_len = len - 8;
        memcpy(signature, stream.in_uint8p(sig_len), sig_len);
        return ssl_sig_ok(exponent, SEC_EXPONENT_SIZE,
                        modulus, this->server_public_key_len,
                        signature, sig_len);
    }


    /* Parse a crypto information structure */
    int rdp_sec_parse_crypt_info(Stream & stream, uint32_t *rc4_key_size,
                                  uint8_t * server_random,
                                  uint8_t* modulus, uint8_t* exponent)
    {
        uint32_t random_len;
        uint32_t rsa_info_len;
        uint32_t cacert_len;
        uint32_t cert_len;
        uint32_t flags;
        SSL_CERT *cacert;
        SSL_CERT *server_cert;
        SSL_RKEY *server_public_key;
        uint16_t tag;
        uint16_t length;
        uint8_t* next_tag;
        uint8_t* end;

        *rc4_key_size = stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
        this->crypt_level = stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
        if (this->crypt_level == 0) { /* no encryption */
            LOG(LOG_INFO, "No encryption");
            return 0;
        }
        random_len = stream.in_uint32_le();
        rsa_info_len = stream.in_uint32_le();
        if (random_len != SEC_RANDOM_SIZE) {
            LOG(LOG_ERR,
                "parse_crypt_info_error: random len %d, expected %d\n",
                random_len, SEC_RANDOM_SIZE);
            return 0;
        }
        memcpy(server_random, stream.in_uint8p(random_len), random_len);

        /* RSA info */
        end = stream.p + rsa_info_len;
        if (end > stream.end) {
            return 0;
        }

        flags = stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
        LOG(LOG_INFO, "crypt flags %x\n", flags);
        if (flags & 1) {

            LOG(LOG_DEBUG, "We're going for the RDP4-style encryption\n");
            stream.skip_uint8(8); /* unknown */

            while (stream.p < end) {
                tag = stream.in_uint16_le();
                length = stream.in_uint16_le();

                next_tag = stream.p + length;

                switch (tag) {
                case SEC_TAG_PUBKEY:
                    #warning exception style should be used throughout the code, not an horrible mixup as below
                    try {
                        this->rdp_sec_parse_public_key(stream, modulus, exponent);
                    }
                    catch (...) {
                        return 0;
                    }
                    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
                    break;
                case SEC_TAG_KEYSIG:
                    LOG(LOG_DEBUG, "SEC_TAG_KEYSIG RDP4-style\n");
                    //if (!this->rdp_sec_parse_public_sig(stream, length, modulus, exponent)){
                    //    return 0;
                    //}
                    break;
                default:
                    LOG(LOG_DEBUG, "unimplemented: crypt tag 0x%x\n", tag);
                    return 0;
                    break;
                }
                stream.p = next_tag;
            }
        }
        else {
            try {
                LOG(LOG_DEBUG, "We're going for the RDP5-style encryption\n");
                LOG(LOG_DEBUG, "RDP5-style encryption with certificates not available\n");
                uint32_t certcount = stream.in_uint32_le();
                if (certcount < 2){
                    LOG(LOG_DEBUG, "Server didn't send enough X509 certificates\n");
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
                }
                for (; certcount > 2; certcount--){
                    /* ignore all the certificates between the root and the signing CA */
                    LOG(LOG_WARNING, " Ignored certs left: %d\n", certcount);
                    uint32_t ignorelen = stream.in_uint32_le();
                    LOG(LOG_WARNING, "Ignored Certificate length is %d\n", ignorelen);
                    SSL_CERT *ignorecert = ssl_cert_read(stream.p, ignorelen);
                    stream.skip_uint8(ignorelen);
                    if (ignorecert == NULL){
                        LOG(LOG_WARNING,
                            "got a bad cert: this will probably screw up"
                            " the rest of the communication\n");
                    }
                }

                /* Do da funky X.509 stuffy

               "How did I find out about this?  I looked up and saw a
               bright light and when I came to I had a scar on my forehead
               and knew about X.500"
               - Peter Gutman in a early version of
               http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
               */

                /* Loading CA_Certificate from server*/
                cacert_len = stream.in_uint32_le();
                LOG(LOG_DEBUG, "CA Certificate length is %d\n", cacert_len);
                cacert = ssl_cert_read(stream.p, cacert_len);
                stream.skip_uint8(cacert_len);
                if (NULL == cacert){
                    LOG(LOG_DEBUG, "Couldn't load CA Certificate from server\n");
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
                }

                ssllib ssl;

                /* Loading Certificate from server*/
                cert_len = stream.in_uint32_le();
                LOG(LOG_DEBUG, "Certificate length is %d\n", cert_len);
                server_cert = ssl_cert_read(stream.p, cert_len);
                stream.skip_uint8(cert_len);
                if (NULL == server_cert){
                    ssl_cert_free(cacert);
                    LOG(LOG_DEBUG, "Couldn't load Certificate from server\n");
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED);
                }
                /* Matching certificates */
                if (!ssl_certs_ok(server_cert,cacert)){
                    ssl_cert_free(server_cert);
                    ssl_cert_free(cacert);
                    LOG(LOG_DEBUG, "Security error CA Certificate invalid\n");
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH);
                }
                ssl_cert_free(cacert);
                stream.skip_uint8(16); /* Padding */
                server_public_key = ssl_cert_to_rkey(server_cert, this->server_public_key_len);
                if (NULL == server_public_key){
                    LOG(LOG_DEBUG, "Didn't parse X509 correctly\n");
                    ssl_cert_free(server_cert);
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

                }
                ssl_cert_free(server_cert);
                LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d\n", this->server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);
                if ((this->server_public_key_len < SEC_MODULUS_SIZE) ||
                    (this->server_public_key_len > SEC_MAX_MODULUS_SIZE)){
                    LOG(LOG_DEBUG, "Bad server public key size (%u bits)\n",
                        this->server_public_key_len * 8);
                    ssl.rkey_free(server_public_key);
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
                }
                if (ssl_rkey_get_exp_mod(server_public_key, exponent, SEC_EXPONENT_SIZE,
                    modulus, SEC_MAX_MODULUS_SIZE) != 0){
                    LOG(LOG_DEBUG, "Problem extracting RSA exponent, modulus");
                    ssl.rkey_free(server_public_key);
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);

                }
                ssl.rkey_free(server_public_key);
                return 1; /* There's some garbage here we don't care about */
            }
            catch (...){
                return 0;
            };
        }
        if (!stream.check_end()) {
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_ERROR_CHECKING_STREAM);
        }
        return 1;
    }

    /*****************************************************************************/
    /* Process crypto information blob */
    void rdp_sec_process_crypt_info(Stream & stream)
    {
        uint8_t server_random[SEC_RANDOM_SIZE];
        uint8_t client_random[SEC_RANDOM_SIZE];
        uint8_t modulus[SEC_MAX_MODULUS_SIZE];
        uint8_t exponent[SEC_EXPONENT_SIZE];
        uint32_t rc4_key_size;

        memset(modulus, 0, sizeof(modulus));
        memset(exponent, 0, sizeof(exponent));
        memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));
        #warning check for the true size
        memset(server_random, 0, SEC_RANDOM_SIZE);
        if (!this->rdp_sec_parse_crypt_info(stream, &rc4_key_size, server_random, modulus, exponent)){
            return;
        }
        /* Generate a client random, and determine encryption keys */
        int fd;

        memset(client_random, 0x44, SEC_RANDOM_SIZE);
        fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, client_random, SEC_RANDOM_SIZE) != SEC_RANDOM_SIZE) {
                LOG(LOG_WARNING, "random source failed to provide random data\n");
            }
            close(fd);
        }
        else {
            LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
        }
        #warning see order of parameters, (always buffer len of len, buffer, but not both)
        this->rdp_sec_rsa_encrypt(this->client_crypt_random, client_random, SEC_RANDOM_SIZE, this->server_public_key_len, modulus, exponent);
        this->rdp_sec_generate_keys(client_random, server_random, rc4_key_size);
    }




//    /*****************************************************************************/
//    static void rdp_sec_rsa_op(uint8_t* out, uint8_t* in, uint8_t* mod, uint8_t* exp)
//    {
//        ssl_mod_exp(out, SEC_MODULUS_SIZE, /* 64 */
//                    in, SEC_RANDOM_SIZE, /* 32 */
//                    mod, SEC_MODULUS_SIZE, /* 64 */
//                    exp, SEC_EXPONENT_SIZE); /* 4 */
//    }



    /* Process connect response data blob */
    void rdp_sec_process_mcs_data(Stream & stream, vector<mcs_channel_item*> channel_list)
    {
        stream.skip_uint8(21); /* header (T.124 ConferenceCreateResponse) */
        uint8_t len = stream.in_uint8();

        if (len & 0x80) {
            len = stream.in_uint8();
        }
        while (stream.p < stream.end) {
            uint16_t tag = stream.in_uint16_le();
            uint16_t length = stream.in_uint16_le();
            if (length <= 4) {
                return;
            }
            uint8_t *next_tag = (stream.p + length) - 4;
            switch (tag) {
            case SEC_TAG_SRV_INFO:
                this->rdp_sec_process_srv_info(stream);
                break;
            case SEC_TAG_SRV_CRYPT:
                this->rdp_sec_process_crypt_info(stream);
                break;
            case SEC_TAG_SRV_CHANNELS:
            /*  This is what rdesktop says in comment:
                FIXME: We should parse this information and
                use it to map RDP5 channels to MCS
                channels
                rdesktop does not call the function below
            */
                #warning rdesktop does not call the function below
                 this->rdp_sec_process_srv_channels(stream, channel_list);
                break;
            default:
                LOG(LOG_WARNING, "response tag 0x%x\n", tag);
                break;
            }
            stream.p = next_tag;
        }
    }

    int rdp_process_redirect_pdu(Stream & stream, int flags, int length, int chanid, client_mod * mod)
    {
        /* We need to recover the name of the channel linked with this
         channel_id in order to match it with the same channel on the
         first channel_list created by the RDP client at initialization
         process*/

    //    LOG(LOG_DEBUG, "rdp_process_redirect_pdu()\n");

        int num_channels_src = (int) this->mcs_layer.channel_list.size();
        mcs_channel_item *channel_item = NULL;
        for (int index = 0; index < num_channels_src; index++){
            channel_item = this->mcs_layer.channel_list[index];
            if (chanid == channel_item->chanid){
                break;
            }
        }

        if (!channel_item || (chanid != channel_item->chanid)){
            LOG(LOG_ERR, "failed to recover name of linked channel\n");
            return 0;
        }
        char * name = channel_item->name;

        /* Here, we're going to search the correct channel in order to send
        information throughout this channel to RDP client*/

        #warning remove dependency to mod
        int num_channels_dst = (int) mod->channel_list.size();
        for (int index = 0; index < num_channels_dst; index++){
            channel_item = mod->channel_list[index];
            if (strcmp(name, channel_item->name) == 0){
                break;
            }
        }
        if (strcmp(name, channel_item->name) != 0){
            LOG(LOG_ERR, "failed to recover channel id\n");
            return 0;
        }
        int channel_id = channel_item->chanid;


        int size = (int)(stream.end - stream.p);

        /* TODO: create new function in order to activate / deactivate copy-paste
        sequence from server to client */

        #warning remove dependency to mod
        if(this->clipboard_check(name, mod->clipboard_enable) == 1){
            /* Clipboard deactivation required */
            return 0;
        }

        if (channel_id < 0){
            LOG(LOG_ERR, "Error sending information, wrong channel id");
            return 0;
        }

        #warning remove dependency to mod
        mod->server_send_to_channel_mod(channel_id, stream.p, size, length, flags);
        return 0;
    }

    void rdp_sec_recv(Stream & stream, int & channel, int & rdpver, client_mod * mod)
    {
        #warning this loop is ugly, the only true reason is we are waiting for the licence
        while (1){
            rdpver = 3;
            this->mcs_layer.rdp_mcs_recv(stream, channel);

            uint32_t sec_flags = stream.in_uint32_le();
            if (sec_flags & SEC_ENCRYPT) { /* 0x08 */
                stream.skip_uint8(8); /* signature */
                this->rdp_sec_decrypt(stream.p, stream.end - stream.p);
            }

            if (sec_flags & SEC_LICENCE_NEG) { /* 0x80 */
                this->rdp_lic_process(stream);
                // read again until licence is processed
                continue;
            }

            if (sec_flags & 0x0400){ /* SEC_REDIRECT_ENCRYPT */
                stream.skip_uint8(8); /* signature */
                this->rdp_sec_decrypt(stream.p, stream.end - stream.p);

                /* Check for a redirect packet, starts with 00 04 */
                if (stream.p[0] == 0 && stream.p[1] == 4){
                /* for some reason the PDU and the length seem to be swapped.
                   This isn't good, but we're going to do a byte for byte
                   swap.  So the first four value appear as: 00 04 XX YY,
                   where XX YY is the little endian length. We're going to
                   use 04 00 as the PDU type, so after our swap this will look
                   like: XX YY 04 00 */

                    uint8_t swapbyte1 = stream.p[0];
                    stream.p[0] = stream.p[2];
                    stream.p[2] = swapbyte1;

                    uint8_t swapbyte2 = stream.p[1];
                    stream.p[1] = stream.p[3];
                    stream.p[3] = swapbyte2;

                    uint8_t swapbyte3 = stream.p[2];
                    stream.p[2] = stream.p[3];
                    stream.p[3] = swapbyte3;
                }
            }
            if (channel != MCS_GLOBAL_CHANNEL){
                if (channel > 0){
                  uint32_t length = stream.in_uint32_le();
                  int channel_flags = stream.in_uint32_le();
                  this->rdp_process_redirect_pdu(stream, channel_flags, length, channel, mod);
                }
                rdpver = 0xff;
            }
            return;
        }
    }


// 2.2.1.4  Server MCS Connect Response PDU with GCC Conference Create Response
// ----------------------------------------------------------------------------
// The MCS Connect Response PDU is an RDP Connection Sequence PDU sent from
// server to client during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent as a response to the MCS Connect Initial PDU (section
// 2.2.1.3). The MCS Connect Response PDU encapsulates a GCC Conference Create
// Response, which encapsulates concatenated blocks of settings data.

// A basic high-level overview of the nested structure for the Server MCS
// Connect Response PDU is illustrated in section 1.3.1.1, in the figure
// specifying MCS Connect Response PDU. Note that the order of the settings
// data blocks is allowed to vary from that shown in the previously mentioned
// figure and the message syntax layout that follows. This is possible because
// each data block is identified by a User Data Header structure (section
// 2.2.1.4.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
// section 13.7.

// mcsCrsp (variable): Variable-length BER-encoded MCS Connect Response
//   structure (using definite-length encoding) as described in [T125]
//   (the ASN.1 structure definition is detailed in [T125] section 7, part 2).
//   The userData field of the MCS Connect Response encapsulates the GCC
//   Conference Create Response data (contained in the gccCCrsp and subsequent
//   fields).

// gccCCrsp (variable): Variable-length PER-encoded GCC Connect Data structure
//   which encapsulates a Connect GCC PDU that contains a GCC Conference Create
//   Response structure as described in [T124] (the ASN.1 structure definitions
//   are specified in [T124] section 8.7) appended as user data to the MCS
//   Connect Response (using the format specified in [T124] sections 9.5 and
//   9.6). The userData field of the GCC Conference Create Response contains
//   one user data set consisting of concatenated server data blocks.

// serverCoreData (12 bytes): Server Core Data structure (section 2.2.1.4.2).

// serverSecurityData (variable): Variable-length Server Security Data structure
//   (section 2.2.1.4.3).

// serverNetworkData (variable): Variable-length Server Network Data structure
//   (section 2.2.1.4.4).

    /*****************************************************************************/
    /* Establish a secure connection */
    void rdp_sec_connect(vector<mcs_channel_item*> channel_list,
                        int width, int height,
                        int rdp_bpp, int keylayout,
                        bool console_session) throw(Error)
    {
        Stream client_mcs_data(512);

        this->rdp_sec_out_mcs_data(
                            client_mcs_data,
                            channel_list,
                            width, height, rdp_bpp,
                            keylayout, console_session);

        LOG(LOG_INFO, "Iso Layer : connect %s\n", this->username);

        try {
            Stream out;
            X224Out crtpdu(X224Packet::CR_TPDU, out);

            #warning looks like this strange cookie thing is in fact useless...
            // USER DATA
//            out.out_concat("Cookie: mstshash=");
//            out.out_concat(this->username);
//            out.out_concat("\r\n");
//            crtpdu.extend_tpdu_hdr();
            crtpdu.end();

            crtpdu.send(this->mcs_layer.trans);

            Stream in;
            X224In cctpdu(this->mcs_layer.trans, in);
            if (cctpdu.tpkt.version != 3){
                throw Error(ERR_T123_EXPECTED_TPKT_VERSION_3);
            }
            if (cctpdu.tpdu_hdr.code != X224Packet::CC_TPDU){
                throw Error(ERR_X224_EXPECTED_CONNECTION_CONFIRM);
            }
        } catch (Error) {
            try {
                this->mcs_layer.trans->disconnect();
            } catch (Error){
                // rethrow the first error, not the error we could get disconnecting
            }
            throw;
        }

        try{
            this->mcs_layer.rdp_mcs_send_connection_initial(client_mcs_data);

            int len = 0;
            Stream stream(8192);

            this->mcs_layer.iso_layer.iso_recv(this->mcs_layer.trans, stream);
            len = this->mcs_layer.ber_parse_header(stream, MCS_CONNECT_RESPONSE);
            len = this->mcs_layer.ber_parse_header(stream, BER_TAG_RESULT);

            int res = stream.in_uint8();

            if (res != 0) {
                throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
            }
            len = this->mcs_layer.ber_parse_header(stream, BER_TAG_INTEGER);
            stream.skip_uint8(len); /* connect id */

            len = this->mcs_layer.ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
            if (!stream.check_rem(len)) {
                throw Error(ERR_MCS_PARSE_DOMAIN_PARAMS_ERROR_CHECKING_STREAM);
            }
            stream.skip_uint8(len);

            len = this->mcs_layer.ber_parse_header(stream, BER_TAG_OCTET_STRING);

            this->rdp_sec_process_mcs_data(stream, channel_list);

            this->mcs_layer.rdp_mcs_send_edrq();
            this->mcs_layer.rdp_mcs_send_aurq();
            this->mcs_layer.rdp_mcs_recv_aucf();
            this->mcs_layer.rdp_mcs_send_cjrq(this->mcs_layer.userid + 1001);
            this->mcs_layer.rdp_mcs_recv_cjcf();
            this->mcs_layer.rdp_mcs_send_cjrq(MCS_GLOBAL_CHANNEL);
            this->mcs_layer.rdp_mcs_recv_cjcf();

            int num_channels = (int)this->mcs_layer.channel_list.size();
            for (int index = 0; index < num_channels; index++){
                const mcs_channel_item* channel_item = this->mcs_layer.channel_list[index];
                this->mcs_layer.rdp_mcs_send_cjrq(channel_item->chanid);
                this->mcs_layer.rdp_mcs_recv_cjcf();
            }
        }
        catch(...){
            this->mcs_layer.iso_layer.iso_disconnect(this->mcs_layer.trans);
            throw;
        }

        LOG(LOG_INFO, "Iso Layer : setting encryption\n");
//        if (this->encryption){
            this->rdp_sec_establish_key();
//        }
    }



    /* this adds the mcs channels in the list of channels to be used when
       creating the server mcs data */
    void rdp_sec_process_srv_channels(Stream & stream, vector<mcs_channel_item*> channel_list)
    {
        /* this is an option set in xrdp.ini, use 1 by default, static virtual
        channels accepted */
        if (1 != this->channel_code) /* are channels on? */{
            return;
        }

        int base_channel = stream.in_uint16_le();
        size_t num_channels = stream.in_uint16_le();

        /* We assume that the channel_id array is confirmed in the same order
        that it has been sent. If there are any channels not confirmed, they're
        going to be the last channels on the array sent in MCS Connect Initial */
        for (size_t index = 0; index < num_channels; index++){
            mcs_channel_item *channel_item_cli = channel_list[index];
            #warning check matching delete, valgrind say memory leak
            mcs_channel_item *channel_item_srv = new mcs_channel_item;
            int chanid = stream.in_uint16_le();
            channel_item_srv->chanid = chanid;
            strcpy(channel_item_srv->name, channel_item_cli->name);
            channel_item_srv->flags = channel_item_cli->flags;
            this->mcs_layer.channel_list.push_back(channel_item_srv);
        }
    }

    /******************************************************************************/

    /* TODO: this function is not working well because it is stopping copy / paste
       what is required is to stop data from server to client. What we need to do is
       to recover clip_flags, send it to rdp_process_redirect_pdu. After that, we
       need to pass this flags to session_send_to_channel and before doing the
       stream.out_uint8a(data, data_len), we need to do stream.out_uint16_le(clip_flags)*/

    int clipboard_check(char* name, bool clipboard)
    {
      if (!clipboard)
      {
        if (strcmp("cliprdr", name) == 0)
        {
          return 1;
        }
      }
      return 0;
    }

// 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
// ------------------------------------------

//The TS_UD_CS_CORE data block contains core client connection-related
// information.

//header (4 bytes): GCC user data block header, as specified in section
//                  2.2.1.3.1. The User Data Header type field MUST be set to
//                  CS_CORE (0xC001).

// version (4 bytes): A 32-bit, unsigned integer. Client version number for the
//                    RDP. The major version number is stored in the high 2
//                    bytes, while the minor version number is stored in the
//                    low 2 bytes.
// +------------+------------------------------------+
// |   Value    |    Meaning                         |
// +------------+------------------------------------+
// | 0x00080001 | RDP 4.0 clients                    |
// +------------+------------------------------------+
// | 0x00080004 | RDP 5.0, 5.1, 5.2, and 6.0 clients |
// +------------+------------------------------------+

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                         width in pixels (up to a maximum value of 4096
//                         pixels).

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                          height in pixels (up to a maximum value of 2048
//                          pixels).

// colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color depth.
//                       Values in this field MUST be ignored if the
//                       postBeta2ColorDepth field is present.
// +--------------------------+-------------------------+
// |     Value                |        Meaning          |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+

// SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access sequence.
//                        This field SHOULD be set to RNS_UD_SAS_DEL (0xAA03).

// keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout (active
//                           input locale identifier). For a list of possible
//                           input locales, see [MSDN-MUI].

// clientBuild (4 bytes): A 32-bit, unsigned integer. The build number of the
//                        client.

// clientName (32 bytes): Name of the client computer. This field contains up to
//                        15 Unicode characters plus a null terminator.

// keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
// +-------+--------------------------------------------+
// | Value |              Meaning                       |
// +-------+--------------------------------------------+
// |   1   | IBM PC/XT or compatible (83-key) keyboard  |
// +-------+--------------------------------------------+
// |   2   | Olivetti "ICO" (102-key) keyboard          |
// +-------+--------------------------------------------+
// |   3   | IBM PC/AT (84-key) and similar keyboards   |
// +-------+--------------------------------------------+
// |   4   | IBM enhanced (101- or 102-key) keyboard    |
// +-------+--------------------------------------------+
// |   5   | Nokia 1050 and similar keyboards           |
// +-------+--------------------------------------------+
// |   6   | Nokia 9140 and similar keyboards           |
// +-------+--------------------------------------------+
// |   7   | Japanese keyboard                          |
// +-------+--------------------------------------------+

// keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard subtype
//                            (an original equipment manufacturer-dependent
//                            value).

// keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number of
//                                function keys on the keyboard.

// imeFileName (64 bytes): A 64-byte field. The Input Method Editor (IME) file
//                         name associated with the input locale. This field
//                         contains up to 31 Unicode characters plus a null
//                         terminator.

// postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
//                                color depth. Values in this field MUST be
//                                ignored if the highColorDepth field is
//                                present.
// +--------------------------+-------------------------+
// |      Value               |         Meaning         |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientProductId (2 bytes): A 16-bit, unsigned integer. The client product ID.
//                            This field SHOULD be initialized to 1. If this
//                            field is present, then all of the preceding fields
//                            MUST also be present. If this field is not
//                            present, then none of the subsequent fields MUST
//                            be present.

// serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number. This field
//                         SHOULD be initialized to 0. If this field is present,
//                         then all of the preceding fields MUST also be
//                         present. If this field is not present, then none of
//                         the subsequent fields MUST be present.

// highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
//                           depth.
// +-------+-------------------------------------------------------------------+
// | Value |                      Meaning                                      |
// +-------+-------------------------------------------------------------------+
// |     4 |   4 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |     8 |   8 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |    15 |  15-bit 555 RGB mask                                              |
// |       |  (5 bits for red, 5 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    16 |  16-bit 565 RGB mask                                              |
// |       |  (5 bits for red, 6 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    24 |  24-bit RGB mask                                                  |
// |       |  (8 bits for red, 8 bits for green, and 8 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies the
//                                 high color depths that the client is capable
//                                 of supporting.
// +-----------------------------+---------------------------------------------+
// |          Flag               |                Meaning                      |
// +-----------------------------+---------------------------------------------+
// | 0x0001 RNS_UD_24BPP_SUPPORT | 24-bit RGB mask                             |
// |                             | (8 bits for red, 8 bits for green,          |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0002 RNS_UD_16BPP_SUPPORT | 16-bit 565 RGB mask                         |
// |                             | (5 bits for red, 6 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0004 RNS_UD_15BPP_SUPPORT | 15-bit 555 RGB mask                         |
// |                             | (5 bits for red, 5 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0008 RNS_UD_32BPP_SUPPORT | 32-bit RGB mask                             |
// |                             | (8 bits for the alpha channel,              |
// |                             | 8 bits for red, 8 bits for green,           |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// earlyCapabilityFlags (2 bytes): A 16-bit, unsigned integer. It specifies
// capabilities early in the connection sequence.
// +---------------------------------------------+-----------------------------|
// |                Flag                         |              Meaning        |
// +---------------------------------------------+-----------------------------|
// | 0x0001 RNS_UD_CS_SUPPORT_ERRINFO_PDU        | Indicates that the client   |
// |                                             | supports the Set Error Info |
// |                                             | PDU (section 2.2.5.1).      |
// +---------------------------------------------+-----------------------------|
// | 0x0002 RNS_UD_CS_WANT_32BPP_SESSION         | Indicates that the client is|
// |                                             | requesting a session color  |
// |                                             | depth of 32 bpp. This flag  |
// |                                             | is necessary because the    |
// |                                             | highColorDepth field does   |
// |                                             | not support a value of 32.  |
// |                                             | If this flag is set, the    |
// |                                             | highColorDepth field SHOULD |
// |                                             | be set to 24 to provide an  |
// |                                             | acceptable fallback for the |
// |                                             | scenario where the server   |
// |                                             | does not support 32 bpp     |
// |                                             | color.                      |
// +---------------------------------------------+-----------------------------|
// | 0x0004 RNS_UD_CS_SUPPORT_STATUSINFO_PDU     | Indicates that the client   |
// |                                             | supports the Server Status  |
// |                                             | Info PDU (section 2.2.5.2). |
// +---------------------------------------------+-----------------------------|
// | 0x0008 RNS_UD_CS_STRONG_ASYMMETRIC_KEYS     | Indicates that the client   |
// |                                             | supports asymmetric keys    |
// |                                             | larger than 512 bits for use|
// |                                             | with the Server Certificate |
// |                                             | (section 2.2.1.4.3.1) sent  |
// |                                             | in the Server Security Data |
// |                                             | block (section 2.2.1.4.3).  |
// +---------------------------------------------+-----------------------------|
// | 0x0020 RNS_UD_CS_RESERVED1                  | Reserved for future use.    |
// |                                             | This flag is ignored by the |
// |                                             | server.                     |
// +---------------------------------------------+-----------------------------+
// | 0x0040 RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU | Indicates that the client   |
// |                                             | supports the Monitor Layout |
// |                                             | PDU (section 2.2.12.1).     |
// +---------------------------------------------+-----------------------------|
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientDigProductId (64 bytes): Contains a value that uniquely identifies the
//                                client. If this field is present, then all of
//                                the preceding fields MUST also be present. If
//                                this field is not present, then none of the
//                                subsequent fields MUST be present.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding to align the
//   serverSelectedProtocol field on the correct byte boundary.
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer. It contains the value returned
//   by the server in the selectedProtocol field of the RDP Negotiation Response structure
//   (section 2.2.1.2.1). In the event that an RDP Negotiation Response structure was not sent,
//   this field MUST be initialized to PROTOCOL_RDP (0). If this field is present, then all of the
//   preceding fields MUST also be present.


    /* Output connect initial data blob */
    void rdp_sec_out_mcs_data(
            Stream & client_mcs_data,
            vector<mcs_channel_item*> channel_list,
            int width, int height, int rdp_bpp, int keylayout, bool console_session)
    {
        int length = 158 + 76 + 12 + 4;

        if (channel_list.size() > 0){
            length += channel_list.size() * 12 + 8;
        }

        /* Generic Conference Control (T.124) ConferenceCreateRequest */
        client_mcs_data.out_uint16_be(5);
        client_mcs_data.out_uint16_be(0x14);
        client_mcs_data.out_uint8(0x7c);
        client_mcs_data.out_uint16_be(1);

        client_mcs_data.out_uint16_be((length | 0x8000)); /* remaining length */

        client_mcs_data.out_uint16_be(8); /* length? */
        client_mcs_data.out_uint16_be(16);
        client_mcs_data.out_uint8(0);
        client_mcs_data.out_uint16_le(0xc001);
        client_mcs_data.out_uint8(0);

        client_mcs_data.out_uint32_le(0x61637544); /* OEM ID: "Duca", as in Ducati. */
        client_mcs_data.out_uint16_be(((length - 14) | 0x8000)); /* remaining length */

        /* Client information */
        client_mcs_data.out_uint16_le(CS_CORE);
        LOG(LOG_INFO, "Sending Client Core Data to remote server\n");
        client_mcs_data.out_uint16_le(212); /* length */
        LOG(LOG_INFO, "core::header::length = %u\n", 212);
        client_mcs_data.out_uint32_le(0x00080004); // RDP version. 1 == RDP4, 4 == RDP5.
        LOG(LOG_INFO, "core::header::version (0x00080004 = RDP 5.0, 5.1, 5.2, and 6.0 clients)");
        client_mcs_data.out_uint16_le(width);
        LOG(LOG_INFO, "core::desktopWidth = %u\n", width);
        client_mcs_data.out_uint16_le(height);
        LOG(LOG_INFO, "core::desktopHeight = %u\n", height);
        client_mcs_data.out_uint16_le(0xca01);
        LOG(LOG_INFO, "core::colorDepth = RNS_UD_COLOR_8BPP (superseded by postBeta2ColorDepth)");
        client_mcs_data.out_uint16_le(0xaa03);
        LOG(LOG_INFO, "core::SASSequence = RNS_UD_SAS_DEL");
        client_mcs_data.out_uint32_le(keylayout);
        LOG(LOG_INFO, "core::keyboardLayout = %x", keylayout);
        client_mcs_data.out_uint32_le(2600); /* Client build. We are now 2600 compatible :-) */
        LOG(LOG_INFO, "core::clientBuild = 2600");
        LOG(LOG_INFO, "core::clientName=%s\n", this->hostname);

        /* Added in order to limit hostlen and hostname size */
        int hostlen = 2 * strlen(this->hostname);
        if (hostlen > 30){
            hostlen = 30;
        }
        /* Unicode name of client, padded to 30 bytes */
        client_mcs_data.out_unistr(this->hostname);
        client_mcs_data.out_clear_bytes(30 - hostlen);

    /* See
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceddk40/html/cxtsksupportingremotedesktopprotocol.asp */
    #warning code should be updated to take care of keyboard type
        client_mcs_data.out_uint32_le(4); // g_keyboard_type
        LOG(LOG_INFO, "core::keyboardType = IBM enhanced (101- or 102-key) keyboard");
        client_mcs_data.out_uint32_le(0); // g_keyboard_subtype
        LOG(LOG_INFO, "core::keyboardSubType = 0");
        client_mcs_data.out_uint32_le(12); // g_keyboard_functionkeys
        LOG(LOG_INFO, "core::keyboardFunctionKey = 12 function keys");
        client_mcs_data.out_clear_bytes(64); /* imeFileName */
        LOG(LOG_INFO, "core::imeFileName = \"\"");
        client_mcs_data.out_uint16_le(0xca01); /* color depth 8bpp */
        LOG(LOG_INFO, "core::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (superseded by highColorDepth)");
        client_mcs_data.out_uint16_le(1);
        LOG(LOG_INFO, "core::clientProductId = 1");
        client_mcs_data.out_uint32_le(0);
        LOG(LOG_INFO, "core::serialNumber = 0");
        client_mcs_data.out_uint16_le(rdp_bpp);
        LOG(LOG_INFO, "core::highColorDepth = %u", rdp_bpp);
        client_mcs_data.out_uint16_le(0x0007);
        LOG(LOG_INFO, "core::supportedColorDepths = 24/16/15");
        client_mcs_data.out_uint16_le(1);
        LOG(LOG_INFO, "core::earlyCapabilityFlags = RNS_UD_CS_SUPPORT_ERRINFO_PDU");
        client_mcs_data.out_clear_bytes(64);
        LOG(LOG_INFO, "core::clientDigProductId = \"\"");
        client_mcs_data.out_clear_bytes(2);
        LOG(LOG_INFO, "core::pad2octets");
//        client_mcs_data.out_uint32_le(0); // optional
//        LOG(LOG_INFO, "core::serverSelectedProtocol = 0");
        /* End of client info */

        client_mcs_data.out_uint16_le(CS_CLUSTER);
        client_mcs_data.out_uint16_le(12);
        #warning check that should depend on g_console_session
        client_mcs_data.out_uint32_le(console_session ? 0xb : 9);
        client_mcs_data.out_uint32_le(0);

        /* Client encryption settings */
        client_mcs_data.out_uint16_le(CS_SECURITY);
        client_mcs_data.out_uint16_le(12); /* length */
        #warning check that, should depend on g_encryption
        /* encryption supported, 128-bit supported */
        client_mcs_data.out_uint32_le(0x3);
        client_mcs_data.out_uint32_le(0); /* Unknown */

        /* Here we need to put channel information in order to redirect channel data
        from client to server passing through the "proxy" */
        size_t num_channels = channel_list.size();

        if (num_channels > 0) {
            client_mcs_data.out_uint16_le(CS_NET);
            client_mcs_data.out_uint16_le(num_channels * 12 + 8); /* length */
            client_mcs_data.out_uint32_le(num_channels); /* number of virtual channels */
            for (size_t i = 0; i < num_channels; i++){
                const mcs_channel_item* channel_item = channel_list[i];

                LOG(LOG_DEBUG, "Requesting channel %s\n", channel_item->name);
                memcpy(client_mcs_data.p, channel_item->name, 8);
                client_mcs_data.p += 8;

                client_mcs_data.out_uint32_be(channel_item->flags);
            }
        }
        client_mcs_data.mark_end();
    }

    /* Process SRV_INFO, find RDP version supported by server */
    void rdp_sec_process_srv_info(Stream & stream)
    {
        this->server_rdp_version = stream.in_uint16_le();
        LOG(LOG_DEBUG, "Server RDP version is %d\n", this->server_rdp_version);
        if (1 == this->server_rdp_version){
            this->use_rdp5 = 0;
        #warning why caring of server_depth here ? Quite strange
        //        this->server_depth = 8;
        }
    }

};

#endif
