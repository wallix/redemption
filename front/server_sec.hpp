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

   header file, secure layer at core module, in charge of
   encryption / decryption methods

*/

#if !defined(__SEC_HPP__)
#define __SEC_HPP__

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "ssl_calls.hpp"
#include "client_info.hpp"
#include "rsa_keys.hpp"
#include "constants.hpp"


#include <assert.h>
#include <stdint.h>

#include <iostream>
using namespace std;

/* sec */
struct server_sec {
    struct ClientInfo * client_info;
    struct Mcs mcs_layer;
    uint8_t server_random[32];
    uint8_t client_random[64];
    uint8_t client_crypt_random[72];
    Stream client_mcs_data;
    int decrypt_use_count;
    int encrypt_use_count;
    uint8_t decrypt_key[16];
    uint8_t encrypt_key[16];
    uint8_t decrypt_update_key[16];
    uint8_t encrypt_update_key[16];
    int rc4_key_size; /* 1 = 40 bit, 2 = 128 bit */
    int rc4_key_len; /* 8 = 40 bit, 16 = 128 bit */
    uint8_t sign_key[16];
    uint8_t* decrypt_rc4_info;
    uint8_t* encrypt_rc4_info;
    uint8_t pub_exp[4];
    uint8_t pub_mod[64];
    uint8_t pub_sig[64];
    uint8_t pri_exp[64];
    Stream data;
    Transport * trans;

    /*****************************************************************************/

    server_sec(ClientInfo * client_info, Transport * trans) :
        client_info(client_info),
        mcs_layer(),
        trans(trans)
    {
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);
        memset(this->client_crypt_random, 0, 72);
        this->decrypt_use_count = 0;
        this->encrypt_use_count = 0;
        memset(this->decrypt_key, 0, 16);
        memset(this->encrypt_key, 0, 16);
        memset(this->decrypt_update_key, 0, 16);
        memset(this->encrypt_update_key, 0, 16);

        memset(this->sign_key, 0, 16);
        memset(this->pub_exp, 0, 4);
        memset(this->pub_mod, 0, 64);
        memset(this->pub_sig, 0, 64);
        memset(this->pri_exp, 0, 64);
        switch (client_info->crypt_level) {
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
        this->decrypt_rc4_info = ssl_rc4_info_create();
        this->encrypt_rc4_info = ssl_rc4_info_create();
    }


    ~server_sec()
    {
        ssl_rc4_info_delete(this->decrypt_rc4_info);
        ssl_rc4_info_delete(this->encrypt_rc4_info);
    }

    /* Reduce key entropy from 64 to 40 bits */
    void server_sec_make_40bit(uint8_t* key)
    {
        key[0] = 0xd1;
        key[1] = 0x26;
        key[2] = 0x9e;
    }

    void server_sec_decrypt(uint8_t* data, int len) throw (Error)
    {
        if (this->decrypt_use_count == 4096) {
            this->server_sec_update(
                this->decrypt_key, this->decrypt_update_key, this->rc4_key_len);
            ssl_rc4_set_key(
                this->decrypt_rc4_info, this->decrypt_key, this->rc4_key_len);
            this->decrypt_use_count = 0;
        }
        ssl_rc4_crypt(this->decrypt_rc4_info, data, len);
        this->decrypt_use_count++;
    }

    /* update an encryption key */
    void server_sec_update(uint8_t* key, uint8_t* update_key, int key_len) throw (Error)
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
        uint8_t* sha1_info;
        uint8_t* md5_info;
        uint8_t* rc4_info;

        sha1_info = ssl_sha1_info_create();
        md5_info = ssl_md5_info_create();
        rc4_info = ssl_rc4_info_create();
        ssl_sha1_clear(sha1_info);
        ssl_sha1_transform(sha1_info, update_key, key_len);
        ssl_sha1_transform(sha1_info, pad_54, 40);
        ssl_sha1_transform(sha1_info, key, key_len);
        ssl_sha1_complete(sha1_info, shasig);
        ssl_md5_clear(md5_info);
        ssl_md5_transform(md5_info, update_key, key_len);
        ssl_md5_transform(md5_info, pad_92, 48);
        ssl_md5_transform(md5_info, shasig, 20);
        ssl_md5_complete(md5_info, key);
        ssl_rc4_set_key(rc4_info, key, key_len);
        ssl_rc4_crypt(rc4_info, key, key_len);
        if (key_len == 8) {
            this->server_sec_make_40bit(key);
        }
        ssl_sha1_info_delete(sha1_info);
        ssl_md5_info_delete(md5_info);
        ssl_rc4_info_delete(rc4_info);
    }

    /* process the mcs client data we received from the mcs layer */
    void server_sec_process_mcs_data(Stream & stream) throw (Error)
    {
        stream.p = stream.data;
        stream.skip_uint8(23);

// 2.2.1.3.1 User Data Header (TS_UD_HEADER)
// =========================================

// type (2 bytes): A 16-bit, unsigned integer. The type of the data
//                 block that this header precedes.

// +-------------------+-------------------------------------------------------+
// | CS_CORE 0xC001 : The data block that follows contains Client Core
//                 Data (section 2.2.1.3.2).
// +-------------------+-------------------------------------------------------+
// | CS_SECURITY 0xC002 : The data block that follows contains Client
//                  Security Data (section 2.2.1.3.3).
// +-------------------+-------------------------------------------------------+
// | CS_NET 0xC003 : The data block that follows contains Client Network
//                 Data (section 2.2.1.3.4).
// +-------------------+-------------------------------------------------------+
// | CS_CLUSTER 0xC004 | The data block that follows contains Client Cluster   |
// |                   | Data (section 2.2.1.3.5).                             |
// +-------------------+-------------------------------------------------------+
// | CS_MONITOR 0xC005 | The data block that follows contains Client
//                 Monitor Data (section 2.2.1.3.6).
// +-------------------+-------------------------------------------------------+
// | SC_CORE 0x0C01 : The data block that follows contains Server Core
//                 Data (section 2.2.1.4.2)
// +-------------------+-------------------------------------------------------+
// | SC_SECURITY 0x0C02 : The data block that follows contains Server
//                 Security Data (section 2.2.1.4.3).
// +-------------------+-------------------------------------------------------+
// | SC_NET 0x0C03 : The data block that follows contains Server Network
//                 Data (section 2.2.1.4.4)
// +-------------------+-------------------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. The size in bytes of the data
//   block, including this header.



        while (stream.check_rem(4)) {
            uint8_t * current_header = stream.p;
            uint16_t tag = stream.in_uint16_le();
            uint16_t length = stream.in_uint16_le();
            if (length < 4 || !stream.check_rem(length - 4)) {
                LOG(LOG_ERR,
                    "error reading block tag %d size %d\n",
                    tag, length);
                break;
            }

            switch (tag){
                case CS_CORE:
                    #warning we should check length to call the two variants of core_data (or begin by reading the common part then the extended part)
                    this->server_sec_parse_mcs_data_cs_core(stream);
                break;
                case CS_SECURITY:
                    this->server_sec_parse_mcs_data_cs_security(stream);
                break;
                case CS_NET:
                    this->server_sec_parse_mcs_data_cs_net(stream);
                break;
                case CS_CLUSTER:
                    this->server_sec_parse_mcs_data_cs_cluster(stream);
                break;
                case CS_MONITOR:
                    this->server_sec_parse_mcs_data_cs_monitor(stream);
                break;
                case SC_CORE:
                    this->server_sec_parse_mcs_data_sc_core(stream);
                break;
                case SC_SECURITY:
                    this->server_sec_parse_mcs_data_sc_security(stream);
                break;
                case SC_NET:
                    this->server_sec_parse_mcs_data_sc_net(stream);
                break;
                default:
                    LOG(LOG_INFO, "Unknown data block tag\n");
                break;
            }
            stream.p = current_header + length;
        }
    }

    void server_sec_encrypt(uint8_t* data, int len) throw (Error)
    {
        if (this->encrypt_use_count == 4096) {
            this->server_sec_update(this->encrypt_key, this->encrypt_update_key,
                            this->rc4_key_len);
            ssl_rc4_set_key(this->encrypt_rc4_info, this->encrypt_key,
                            this->rc4_key_len);
            this->encrypt_use_count = 0;
        }
        ssl_rc4_crypt(this->encrypt_rc4_info, data, len);
        this->encrypt_use_count++;
    }

    void unicode_in(Stream & stream, int uni_len, uint8_t* dst, int dst_len) throw (Error)
    {
        int dst_index = 0;
        int src_index = 0;
        while (src_index < uni_len) {
            if (dst_index >= dst_len || src_index > 512) {
                break;
            }
            dst[dst_index] = stream.in_uint8();
            stream.skip_uint8(1);
            dst_index++;
            src_index += 2;
        }
        stream.skip_uint8(2);
    }


    void server_sec_send_lic_initial() throw (Error)
    {
        /* some compilers need unsigned char to avoid warnings */
        static uint8_t lic1[322] = {
            0x80, 0x00, 0x3e, 0x01, 0x01, 0x02, 0x3e, 0x01,
            0x7b, 0x3c, 0x31, 0xa6, 0xae, 0xe8, 0x74, 0xf6,
            0xb4, 0xa5, 0x03, 0x90, 0xe7, 0xc2, 0xc7, 0x39,
            0xba, 0x53, 0x1c, 0x30, 0x54, 0x6e, 0x90, 0x05,
            0xd0, 0x05, 0xce, 0x44, 0x18, 0x91, 0x83, 0x81,
            0x00, 0x00, 0x04, 0x00, 0x2c, 0x00, 0x00, 0x00,
            0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00,
            0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00,
            0x74, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00,
            0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00,
            0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00,
            0x6e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
            0x32, 0x00, 0x33, 0x00, 0x36, 0x00, 0x00, 0x00,
            0x0d, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x03, 0x00, 0xb8, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x06, 0x00, 0x5c, 0x00, 0x52, 0x53, 0x41, 0x31,
            0x48, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
            0x3f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
            0x01, 0xc7, 0xc9, 0xf7, 0x8e, 0x5a, 0x38, 0xe4,
            0x29, 0xc3, 0x00, 0x95, 0x2d, 0xdd, 0x4c, 0x3e,
            0x50, 0x45, 0x0b, 0x0d, 0x9e, 0x2a, 0x5d, 0x18,
            0x63, 0x64, 0xc4, 0x2c, 0xf7, 0x8f, 0x29, 0xd5,
            0x3f, 0xc5, 0x35, 0x22, 0x34, 0xff, 0xad, 0x3a,
            0xe6, 0xe3, 0x95, 0x06, 0xae, 0x55, 0x82, 0xe3,
            0xc8, 0xc7, 0xb4, 0xa8, 0x47, 0xc8, 0x50, 0x71,
            0x74, 0x29, 0x53, 0x89, 0x6d, 0x9c, 0xed, 0x70,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x08, 0x00, 0x48, 0x00, 0xa8, 0xf4, 0x31, 0xb9,
            0xab, 0x4b, 0xe6, 0xb4, 0xf4, 0x39, 0x89, 0xd6,
            0xb1, 0xda, 0xf6, 0x1e, 0xec, 0xb1, 0xf0, 0x54,
            0x3b, 0x5e, 0x3e, 0x6a, 0x71, 0xb4, 0xf7, 0x75,
            0xc8, 0x16, 0x2f, 0x24, 0x00, 0xde, 0xe9, 0x82,
            0x99, 0x5f, 0x33, 0x0b, 0xa9, 0xa6, 0x94, 0xaf,
            0xcb, 0x11, 0xc3, 0xf2, 0xdb, 0x09, 0x42, 0x68,
            0x29, 0x56, 0x58, 0x01, 0x56, 0xdb, 0x59, 0x03,
            0x69, 0xdb, 0x7d, 0x37, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x0e, 0x00, 0x0e, 0x00, 0x6d, 0x69, 0x63, 0x72,
            0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f,
            0x6d, 0x00
       };

        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->mcs_layer.userid);
        stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
        stream.out_uint8(0x70);
        stream.out_uint16_be((8+322)|0x8000);
        stream.out_copy_bytes((char*)lic1, 322);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void server_sec_send_lic_response() throw (Error)
    {
        /* some compilers need unsigned char to avoid warnings */
        static uint8_t lic2[20] = { 0x80, 0x00, 0x10, 0x00, 0xff, 0x02, 0x10, 0x00,
                                 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                                 0x28, 0x14, 0x00, 0x00
                               };

        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->mcs_layer.userid);
        stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
        stream.out_uint8(0x70);
        stream.out_uint8(8+20);
        stream.out_copy_bytes((char*)lic2, 20);

        tpdu.end();
        tpdu.send(this->trans);
// ----------------------------
    }

    void server_sec_send_media_lic_response() throw (Error)
    {
        /* mce */
        /* some compilers need unsigned char to avoid warnings */
        static uint8_t lic3[20] = { 0x80, 0x02, 0x10, 0x00, 0xff, 0x03, 0x10, 0x00,
                                 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                                 0xf3, 0x99, 0x00, 0x00
                                 };

        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->mcs_layer.userid);
        stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
        stream.out_uint8(0x70);
        stream.out_uint8(8+20);
        stream.out_copy_bytes((char*)lic3, 20);

        tpdu.end();
        tpdu.send(this->trans);
// ----------------------------

    }

    void server_sec_rsa_op()
    {
        ssl_mod_exp(this->client_random, 64,
                    this->client_crypt_random, 64,
                    this->pub_mod, 64,
                    this->pri_exp, 64);
    }

    void server_sec_hash_48(uint8_t* out, uint8_t* in, uint8_t* salt1, uint8_t* salt2, uint8_t salt)
    {
        int i;
        uint8_t* sha1_info;
        uint8_t* md5_info;
        uint8_t pad[4];
        uint8_t sha1_sig[20];
        uint8_t md5_sig[16];

        sha1_info = ssl_sha1_info_create();
        md5_info = ssl_md5_info_create();
        for (i = 0; i < 3; i++) {
            memset(pad, salt + i, 4);
            ssl_sha1_clear(sha1_info);
            ssl_sha1_transform(sha1_info, pad, i + 1);
            ssl_sha1_transform(sha1_info, in, 48);
            ssl_sha1_transform(sha1_info, salt1, 32);
            ssl_sha1_transform(sha1_info, salt2, 32);
            ssl_sha1_complete(sha1_info, sha1_sig);
            ssl_md5_clear(md5_info);
            ssl_md5_transform(md5_info, in, 48);
            ssl_md5_transform(md5_info, sha1_sig, 20);
            ssl_md5_complete(md5_info, md5_sig);
            memcpy(out + i * 16, md5_sig, 16);
        }
        ssl_sha1_info_delete(sha1_info);
        ssl_md5_info_delete(md5_info);
    }

    void server_sec_hash_16(uint8_t* out, uint8_t* in, uint8_t* salt1, uint8_t* salt2)
    {
        uint8_t* md5_info = ssl_md5_info_create();
        ssl_md5_clear(md5_info);
        ssl_md5_transform(md5_info, in, 16);
        ssl_md5_transform(md5_info, salt1, 32);
        ssl_md5_transform(md5_info, salt2, 32);
        ssl_md5_complete(md5_info, out);
        ssl_md5_info_delete(md5_info);
    }

    /*****************************************************************************/
    void server_sec_establish_keys()
    {
        uint8_t session_key[48];
        uint8_t temp_hash[48];
        uint8_t input[48];

        memcpy(input, this->client_random, 24);
        memcpy(input + 24, this->server_random, 24);
        server_sec_hash_48(temp_hash, input, this->client_random,
                         this->server_random, 65);
        server_sec_hash_48(session_key, temp_hash, this->client_random,
                         this->server_random, 88);
        memcpy(this->sign_key, session_key, 16);
        server_sec_hash_16(this->encrypt_key, session_key + 16, this->client_random,
                         this->server_random);
        server_sec_hash_16(this->decrypt_key, session_key + 32, this->client_random,
                         this->server_random);
        if (this->rc4_key_size == 1) {
            server_sec_make_40bit(this->sign_key);
            server_sec_make_40bit(this->encrypt_key);
            server_sec_make_40bit(this->decrypt_key);
            this->rc4_key_len = 8;
        } else {
            this->rc4_key_len = 16;
        }
        memcpy(this->decrypt_update_key, this->decrypt_key, 16);
        memcpy(this->encrypt_update_key, this->encrypt_key, 16);
        ssl_rc4_set_key(this->decrypt_rc4_info, this->decrypt_key, this->rc4_key_len);
        ssl_rc4_set_key(this->encrypt_rc4_info, this->encrypt_key, this->rc4_key_len);
    }


    /*****************************************************************************/
    /* Output a uint32 into a buffer (little-endian) */
    void buf_out_uint32(uint8_t* buffer, int value)
    {
        buffer[0] = (value) & 0xff;
        buffer[1] = (value >> 8) & 0xff;
        buffer[2] = (value >> 16) & 0xff;
        buffer[3] = (value >> 24) & 0xff;
    }

    /*****************************************************************************/
    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void server_sec_sign(uint8_t* out, int out_len, uint8_t* data, int data_len)
    {
        /* some compilers need unsigned char to avoid warnings */
        static uint8_t pad_54[40] = {
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
            54, 54, 54, 54, 54, 54, 54, 54
        };

        /* some compilers need unsigned char to avoid warnings */
        static uint8_t pad_92[48] = {
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
            92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92
        };

        uint8_t shasig[20];
        uint8_t md5sig[16];
        uint8_t lenhdr[4];
        uint8_t* sha1_info;
        uint8_t* md5_info;

        buf_out_uint32(lenhdr, data_len);
        sha1_info = ssl_sha1_info_create();
        md5_info = ssl_md5_info_create();
        ssl_sha1_clear(sha1_info);
        ssl_sha1_transform(sha1_info, this->sign_key, this->rc4_key_len);
        ssl_sha1_transform(sha1_info, pad_54, 40);
        ssl_sha1_transform(sha1_info, lenhdr, 4);
        ssl_sha1_transform(sha1_info, data, data_len);
        ssl_sha1_complete(sha1_info, shasig);
        ssl_md5_clear(md5_info);
        ssl_md5_transform(md5_info, this->sign_key, this->rc4_key_len);
        ssl_md5_transform(md5_info, pad_92, 48);
        ssl_md5_transform(md5_info, shasig, 20);
        ssl_md5_complete(md5_info, md5sig);
        memcpy(out, md5sig, out_len);
        ssl_sha1_info_delete(sha1_info);
        ssl_md5_info_delete(md5_info);
    }

    void server_sec_process_logon_info(Stream & stream) throw (Error)
    {
        // LOG(LOG_DEBUG, "server_sec_process_logon_info\n");
        stream.skip_uint8(4);
        int flags = stream.in_uint32_le();
        /* this is the first test that the decrypt is working */
        if ((flags & RDP_LOGON_NORMAL) != RDP_LOGON_NORMAL) /* 0x33 */
        {                                                   /* must be or error */
            throw Error(ERR_SEC_PROCESS_LOGON_UNKNOWN_FLAGS);
        }
        if (flags & RDP_LOGON_LEAVE_AUDIO) {
            this->client_info->sound_code = 1;
        }
        if ((flags & RDP_LOGON_AUTO) && (!this->client_info->is_mce))
            /* todo, for now not allowing autologon and mce both */
        {
            this->client_info->rdp_autologin = 1;
        }
        if (flags & RDP_COMPRESSION) {
            this->client_info->rdp_compression = 1;
        }
        unsigned len_domain = stream.in_uint16_le();
        unsigned len_user = stream.in_uint16_le();
        unsigned len_password = stream.in_uint16_le();
        unsigned len_program = stream.in_uint16_le();
        unsigned len_directory = stream.in_uint16_le();
        /* todo, we should error out in any of the above lengths are > 512 */
        /* to avoid buffer overruns */
        unicode_in(stream, len_domain, (uint8_t*)this->client_info->domain, 255);
        unicode_in(stream, len_user, (uint8_t*)this->client_info->username, 255);
        // LOG(LOG_DEBUG, "setting username to %s\n", this->client_info->username);

        if (flags & RDP_LOGON_AUTO) {
            unicode_in(stream, len_password, (uint8_t*)this->client_info->password, 255);
        } else {
            stream.skip_uint8(len_password + 2);
        }
        unicode_in(stream, len_program, (uint8_t*)this->client_info->program, 255);
        unicode_in(stream, len_directory, (uint8_t*)this->client_info->directory, 255);
        if (flags & RDP_LOGON_BLOB) {
            stream.skip_uint8(2);                                    /* unknown */
            unsigned len_ip = stream.in_uint16_le();
            uint8_t tmpdata[256];
            unicode_in(stream, len_ip - 2, tmpdata, 255);
            unsigned len_dll = stream.in_uint16_le();
            unicode_in(stream, len_dll - 2, tmpdata, 255);
            stream.in_uint32_le(); /* len of timetone */
            stream.skip_uint8(62); /* skip */
            stream.skip_uint8(22); /* skip misc. */
            stream.skip_uint8(62); /* skip */
            stream.skip_uint8(26); /* skip stuff */
            this->client_info->rdp5_performanceflags = stream.in_uint32_le();
        }
    }

    void server_sec_send(Stream & stream, int chan)
    {
        uint8_t * oldp = stream.p;
        stream.p = stream.sec_hdr;
        if (this->client_info->crypt_level > 1) {
            stream.out_uint32_le(SEC_ENCRYPT);
            int datalen = (int)((stream.end - stream.p) - 8);
            this->server_sec_sign(stream.p, 8, stream.p + 8, datalen);
            this->server_sec_encrypt(stream.p + 8, datalen);
        } else {
            stream.out_uint32_le(0);
        }

        stream.p = oldp;

        uint8_t * oldp2 = stream.p;
        stream.p = stream.mcs_hdr;
        int len = (stream.end - stream.p) - 8;
        if (len > 8192 * 2) {
            LOG(LOG_ERR,
                "error in.mcs_send, size too long, its %d (buffer=%d)\n",
                len, stream.capacity);
        }
        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->mcs_layer.userid);
        stream.out_uint16_be(chan);
        stream.out_uint8(0x70);
        if (len >= 128) {
            len = len | 0x8000;
            stream.out_uint16_be(len);
            stream.p = oldp2;
        }
        else {
            stream.out_uint8(len);
            #warning this is ugly isn't there a way to avoid moving the whole buffer
            /* move everything up one byte */
            uint8_t *lp = stream.p;
            while (lp < stream.end) {
                lp[0] = lp[1];
                lp++;
            }
            stream.end--;
            stream.p = oldp2-1;
        }
    }

    // 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
    // -------------------------------------
    // Below relevant quotes from MS-RDPBCGR v20100601 (2.2.1.3.2)

    // header (4 bytes): GCC user data block header, as specified in section
    //                   2.2.1.3.1. The User Data Header type field MUST be
    //                   set to CS_CORE (0xC001).

    // version (4 bytes): A 32-bit, unsigned integer. Client version number
    //                    for the RDP. The major version number is stored in
    //                    the high 2 bytes, while the minor version number
    //                    is stored in the low 2 bytes.
    //
    //         Value Meaning
    //         0x00080001 RDP 4.0 clients
    //         0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 clients

    // desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested
    //                         desktop width in pixels (up to a maximum
    //                         value of 4096 pixels).

    // desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested
    //                         desktop height in pixels (up to a maximum
    //                         value of 2048 pixels).

    // colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
    //                       depth. Values in this field MUST be ignored if
    //                       the postBeta2ColorDepth field is present.
    //          Value Meaning
    //          RNS_UD_COLOR_4BPP 0xCA00 4 bits-per-pixel (bpp)
    //          RNS_UD_COLOR_8BPP 0xCA01 8 bpp

    // SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access
    //                        sequence. This field SHOULD be set to
    //                        RNS_UD_SAS_DEL (0xAA03).

    // keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout
    //                           (active input locale identifier). For a
    //                           list of possible input locales, see
    //                           [MSDN-MUI].

    // clientBuild (4 bytes): A 32-bit, unsigned integer. The build number
    // of the client.

    // clientName (32 bytes): Name of the client computer. This field
    //                        contains up to 15 Unicode characters plus a
    //                        null terminator.

    // keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
    //              Value Meaning
    //              0x00000001 IBM PC/XT or compatible (83-key) keyboard
    //              0x00000002 Olivetti "ICO" (102-key) keyboard
    //              0x00000003 IBM PC/AT (84-key) and similar keyboards
    //              0x00000004 IBM enhanced (101-key or 102-key) keyboard
    //              0x00000005 Nokia 1050 and similar keyboards
    //              0x00000006 Nokia 9140 and similar keyboards
    //              0x00000007 Japanese keyboard

    // keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard
    //                        subtype (an original equipment manufacturer-
    //                        -dependent value).

    // keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number
    //                        of function keys on the keyboard.

    // imeFileName (64 bytes): A 64-byte field. The Input Method Editor
    //                        (IME) file name associated with the input
    //                        locale. This field contains up to 31 Unicode
    //                        characters plus a null terminator.

    // --> Note By CGR How do we know that the following fields are
    //     present of Not ? The only rational method I see is to look
    //     at the length field in the preceding User Data Header
    //     120 bytes without optional data
    //     216 bytes with optional data present

    // postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The
    //                        requested color depth. Values in this field
    //                        MUST be ignored if the highColorDepth field
    //                        is present.
    //       Value Meaning
    //       RNS_UD_COLOR_4BPP 0xCA00        : 4 bits-per-pixel (bpp)
    //       RNS_UD_COLOR_8BPP 0xCA01        : 8 bpp
    //       RNS_UD_COLOR_16BPP_555 0xCA02   : 15-bit 555 RGB mask
    //                                         (5 bits for red, 5 bits for
    //                                         green, and 5 bits for blue)
    //       RNS_UD_COLOR_16BPP_565 0xCA03   : 16-bit 565 RGB mask
    //                                         (5 bits for red, 6 bits for
    //                                         green, and 5 bits for blue)
    //       RNS_UD_COLOR_24BPP 0xCA04       : 24-bit RGB mask
    //                                         (8 bits for red, 8 bits for
    //                                         green, and 8 bits for blue)
    // If this field is present, all of the preceding fields MUST also be
    // present. If this field is not present, all of the subsequent fields
    // MUST NOT be present.

    // clientProductId (2 bytes): A 16-bit, unsigned integer. The client
    //                          product ID. This field SHOULD be initialized
    //                          to 1. If this field is present, all of the
    //                          preceding fields MUST also be present. If
    //                          this field is not present, all of the
    //                          subsequent fields MUST NOT be present.

    // serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number.
    //                         This field SHOULD be initialized to 0. If
    //                         this field is present, all of the preceding
    //                         fields MUST also be present. If this field
    //                         is not present, all of the subsequent fields
    //                         MUST NOT be present.

    // highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
    //                         color depth.
    //          Value Meaning
    // HIGH_COLOR_4BPP  0x0004             : 4 bpp
    // HIGH_COLOR_8BPP  0x0008             : 8 bpp
    // HIGH_COLOR_15BPP 0x000F             : 15-bit 555 RGB mask
    //                                       (5 bits for red, 5 bits for
    //                                       green, and 5 bits for blue)
    // HIGH_COLOR_16BPP 0x0010             : 16-bit 565 RGB mask
    //                                       (5 bits for red, 6 bits for
    //                                       green, and 5 bits for blue)
    // HIGH_COLOR_24BPP 0x0018             : 24-bit RGB mask
    //                                       (8 bits for red, 8 bits for
    //                                       green, and 8 bits for blue)
    //
    // If this field is present, all of the preceding fields MUST also be
    // present. If this field is not present, all of the subsequent fields
    // MUST NOT be present.

    // supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies
    //                                 the high color depths that the client
    //                                 is capable of supporting.
    //
    //         Flag Meaning
    //   RNS_UD_24BPP_SUPPORT 0x0001       : 24-bit RGB mask
    //                                       (8 bits for red, 8 bits for
    //                                       green, and 8 bits for blue)
    //   RNS_UD_16BPP_SUPPORT 0x0002       : 16-bit 565 RGB mask
    //                                       (5 bits for red, 6 bits for
    //                                       green, and 5 bits for blue)
    //   RNS_UD_15BPP_SUPPORT 0x0004       : 15-bit 555 RGB mask
    //                                       (5 bits for red, 5 bits for
    //                                       green, and 5 bits for blue)
    //   RNS_UD_32BPP_SUPPORT 0x0008       : 32-bit RGB mask
    //                                       (8 bits for the alpha channel,
    //                                       8 bits for red, 8 bits for
    //                                       green, and 8 bits for blue)
    // If this field is present, all of the preceding fields MUST also be
    // present. If this field is not present, all of the subsequent fields
    // MUST NOT be present.

    // earlyCapabilityFlags (2 bytes)      : A 16-bit, unsigned integer. It
    //                                       specifies capabilities early in
    //                                       the connection sequence.
    //        Flag                        Meaning
    //  RNS_UD_CS_SUPPORT_ERRINFO_PDU Indicates that the client supports
    //    0x0001                        the Set Error Info PDU
    //                                 (section 2.2.5.1).
    //
    //  RNS_UD_CS_WANT_32BPP_SESSION Indicates that the client is requesting
    //    0x0002                     a session color depth of 32 bpp. This
    //                               flag is necessary because the
    //                               highColorDepth field does not support a
    //                               value of 32. If this flag is set, the
    //                               highColorDepth field SHOULD be set to
    //                               24 to provide an acceptable fallback
    //                               for the scenario where the server does
    //                               not support 32 bpp color.
    //
    //  RNS_UD_CS_SUPPORT_STATUSINFO_PDU  Indicates that the client supports
    //    0x0004                          the Server Status Info PDU
    //                                    (section 2.2.5.2).
    //
    //  RNS_UD_CS_STRONG_ASYMMETRIC_KEYS  Indicates that the client supports
    //    0x0008                          asymmetric keys larger than
    //                                    512 bits for use with the Server
    //                                    Certificate (section 2.2.1.4.3.1)
    //                                    sent in the Server Security Data
    //                                    block (section 2.2.1.4.3).
    //
    //  RNS_UD_CS_VALID_CONNECTION_TYPE Indicates that the connectionType
    //     0x0020                       field contains valid data.
    //
    //  RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU Indicates that the client
    //     0x0040                            supports the Monitor Layout PDU
    //                                       (section 2.2.12.1).
    //
    // If this field is present, all of the preceding fields MUST also be
    // present. If this field is not present, all of the subsequent fields
    // MUST NOT be present.

    // clientDigProductId (64 bytes): Contains a value that uniquely
    //                                identifies the client. If this field
    //                                is present, all of the preceding
    //                                fields MUST also be present. If this
    //                                field is not present, all of the
    //                                subsequent fields MUST NOT be present.

    // connectionType (1 byte): An 8-bit unsigned integer. Hints at the type
    //                      of network connection being used by the client.
    //                      This field only contains valid data if the
    //                      RNS_UD_CS_VALID_CONNECTION_TYPE (0x0020) flag
    //                      is present in the earlyCapabilityFlags field.
    //
    //    Value                          Meaning
    //  CONNECTION_TYPE_MODEM 0x01 : Modem (56 Kbps)
    //  CONNECTION_TYPE_BROADBAND_LOW 0x02 : Low-speed broadband
    //                                 (256 Kbps - 2 Mbps)
    //  CONNECTION_TYPE_SATELLITE 0x03 : Satellite
    //                                 (2 Mbps - 16 Mbps with high latency)
    //  CONNECTION_TYPE_BROADBAND_HIGH 0x04 : High-speed broadband
    //                                 (2 Mbps - 10 Mbps)
    //  CONNECTION_TYPE_WAN 0x05 : WAN (10 Mbps or higher with high latency)
    //  CONNECTION_TYPE_LAN 0x06 : LAN (10 Mbps or higher)

    // If this field is present, all of the preceding fields MUST also be
    // present. If this field is not present, all of the subsequent fields
    // MUST NOT be present.

    // pad1octet (1 byte): An 8-bit, unsigned integer. Padding to align the
    //   serverSelectedProtocol field on the correct byte boundary. If this
    //   field is present, all of the preceding fields MUST also be present.
    //   If this field is not present, all of the subsequent fields MUST NOT
    //   be present.

    // serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer that
    //   contains the value returned by the server in the selectedProtocol
    //   field of the RDP Negotiation Response (section 2.2.1.2.1). In the
    //   event that an RDP Negotiation Response was not received from the
    //   server, this field MUST be initialized to PROTOCOL_RDP (0). This
    //   field MUST be present if an RDP Negotiation Request (section
    //   2.2.1.1.1) was sent to the server. If this field is present,
    //   then all of the preceding fields MUST also be present.

    #warning use official field names from MS-RDPBCGR
    void server_sec_parse_mcs_data_cs_core(Stream & stream)
    {
        LOG(LOG_INFO, "PARSE CS_CORE\n");
        uint16_t rdp_version = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: rdp_version (1=RDP1, 4=RDP5) %u\n", rdp_version);
        uint16_t dummy1 = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: ?? = %u\n", dummy1);
        this->client_info->width = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: width = %u\n", this->client_info->width);
        this->client_info->height = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: height = %u\n", this->client_info->height);
        uint16_t bpp_code = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: bpp_code = %x\n", bpp_code);
        uint16_t dummy2 = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: ?? = %x\n", dummy2);
        /* get keylayout */
        this->client_info->keylayout = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: layout = %x\n", this->client_info->keylayout);
        /* get build : windows build */
        this->client_info->build = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: build = %x\n", this->client_info->build);

        /* get hostname (it is UTF16, windows flavored widechars) */
        /* Unicode name of client is padded to 32 bytes */
        stream.in_uni_to_ascii_str(this->client_info->hostname, 32);
        LOG(LOG_INFO, "core_data: hostname = %s\n", this->client_info->hostname);

        uint32_t keyboard_type = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: keyboard_type = %x\n", keyboard_type);
        uint32_t keyboard_subtype = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: keyboard_subtype = %x\n", keyboard_subtype);
        uint32_t keyboard_functionkeys = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: keyboard_functionkeys = %x\n", keyboard_functionkeys);
        stream.skip_uint8(64);

        this->client_info->bpp = 8;
        int i = stream.in_uint16_le();
        switch (i) {
        case 0xca01:
        {
            uint16_t clientProductId = stream.in_uint16_le();
            uint32_t serialNumber = stream.in_uint32_le();
            uint16_t rdp_bpp = stream.in_uint16_le();
            uint16_t supportedColorDepths = stream.in_uint16_le();
            if (rdp_bpp <= 24){
                this->client_info->bpp = rdp_bpp;
            }
            else {
                this->client_info->bpp = 24;
            }
        }
            break;
        case 0xca02:
            this->client_info->bpp = 15;
            break;
        case 0xca03:
            this->client_info->bpp = 16;
            break;
        case 0xca04:
            this->client_info->bpp = 24;
            break;
        }
        LOG(LOG_INFO, "core_data: bpp = %u\n", this->client_info->bpp);
    }

    // 2.2.1.3.3 Client Security Data (TS_UD_CS_SEC)
    // ---------------------------------------------
    // The TS_UD_CS_SEC data block contains security-related information used to
    // advertise client cryptographic support. This information is only relevant
    // when Standard RDP Security mechanisms (section 5.3) will be used. See
    // sections 3 and 5.3.2 for a detailed discussion of how this information is
    // used.

    // header (4 bytes): GCC user data block header as described in User Data
    //                   Header (section 2.2.1.3.1). The User Data Header type
    //                   field MUST be set to CS_SECURITY (0xC002).

    // encryptionMethods (4 bytes): A 32-bit, unsigned integer. Cryptographic
    //                              encryption methods supported by the client
    //                              and used in conjunction with Standard RDP
    //                              Security The server MUST select one of these
    //                              methods. Section 5.3.2 describes how the
    //                              client and server negotiate the security
    //                              parameters for a given connection.
    //
    //           Value                           Meaning
    // -------------------------------------------------------------------------
    //    40BIT_ENCRYPTION_FLAG   40-bit session keys MUST be used to encrypt
    //       0x00000001           data (with RC4) and generate Message
    //                            Authentication Codes (MAC).
    // -------------------------------------------------------------------------
    //    128BIT_ENCRYPTION_FLAG  128-bit session keys MUST be used to encrypt
    //       0x00000002           data (with RC4) and generate MACs.
    // -------------------------------------------------------------------------
    //    56BIT_ENCRYPTION_FLAG   56-bit session keys MUST be used to encrypt
    //       0x00000008           data (with RC4) and generate MACs.
    // -------------------------------------------------------------------------
    //   FIPS_ENCRYPTION_FLAG All encryption and Message Authentication Code
    //                            generation routines MUST be Federal
    //       0x00000010           Information Processing Standard (FIPS) 140-1
    //                            compliant.

    // extEncryptionMethods (4 bytes): A 32-bit, unsigned integer. This field is
    //                               used exclusively for the French locale.
    //                               In French locale clients, encryptionMethods
    //                               MUST be set to 0 and extEncryptionMethods
    //                               MUST be set to the value to which
    //                               encryptionMethods would have been set.
    //                               For non-French locale clients, this field
    //                               MUST be set to 0

    void server_sec_parse_mcs_data_cs_security(Stream & stream)
    {
        LOG(LOG_INFO, "CS_SECURITY\n");
    }

    // 2.2.1.3.4 Client Network Data (TS_UD_CS_NET)
    // --------------------------------------------
    // The TS_UD_CS_NET packet contains a list of requested virtual channels.

    // header (4 bytes): A 32-bit, unsigned integer. GCC user data block header,
    //                   as specified in User Data Header (section 2.2.1.3.1).
    //                   The User Data Header type field MUST be set to CS_NET
    //                   (0xC003).

    // channelCount (4 bytes): A 32-bit, unsigned integer. The number of
    //                         requested static virtual channels (the maximum
    //                         allowed is 31).

    // channelDefArray (variable): A variable-length array containing the
    //                             information for requested static virtual
    //                             channels encapsulated in CHANNEL_DEF
    //                             structures (section 2.2.1.3.4.1). The number
    //                             of CHANNEL_DEF structures which follows is
    //                             given by the channelCount field.

    // 2.2.1.3.4.1 Channel Definition Structure (CHANNEL_DEF)
    // ------------------------------------------------------
    // The CHANNEL_DEF packet contains information for a particular static
    // virtual channel.

    // name (8 bytes): An 8-byte array containing a null-terminated collection
    //                 of seven ANSI characters that uniquely identify the
    //                 channel.

    // options (4 bytes): A 32-bit, unsigned integer. Channel option flags.
    //
    //           Flag                             Meaning
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_INITIALIZED   Absence of this flag indicates that this
    //        0x80000000            channel is a placeholder and that the
    //                              server MUST NOT set it up.
    // ------------------------------------------------------------------------
    // CHANNEL_OPTION_ENCRYPT_RDP   This flag is unused and its value MUST be
    //        0x40000000            ignored by the server.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_ENCRYPT_SC    This flag is unused and its value MUST be
    //        0x20000000            ignored by the server.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_ENCRYPT_CS    This flag is unused and its value MUST be
    //        0x10000000            ignored by the server.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_PRI_HIGH      Channel data MUST be sent with high MCS
    //        0x08000000            priority.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_PRI_MED       Channel data MUST be sent with medium
    //        0x04000000            MCS priority.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_PRI_LOW       Channel data MUST be sent with low MCS
    //        0x02000000            priority.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_COMPRESS_RDP  Virtual channel data MUST be compressed
    //        0x00800000            if RDP data is being compressed.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_COMPRESS      Virtual channel data MUST be compressed,
    //        0x00400000            regardless of RDP compression settings.
    // -------------------------------------------------------------------------
    // CHANNEL_OPTION_SHOW_PROTOCOL The value of this flag MUST be ignored by
    //        0x00200000            the server. The visibility of the Channel
    //                              PDU Header (section 2.2.6.1.1) is
    //                              determined by the CHANNEL_FLAG_SHOW_PROTOCOL
    //                              (0x00000010) flag as defined in the flags
    //                              field (section 2.2.6.1.1).
    // -------------------------------------------------------------------------
    //REMOTE_CONTROL_PERSISTENT     Channel MUST be persistent across remote
    //                              control 0x00100000 transactions.

    // this adds the mcs channels in the list of channels to be used when
    // creating the server mcs data
    void server_sec_parse_mcs_data_cs_net(Stream & stream)
    {
        LOG(LOG_INFO, "CS_NET\n");
        // this is an option set in rdpproxy.ini
        // to disable all channels (no clipboard, no device redirection, etc)
        if (this->client_info->channel_code != 1) { /* are channels on? */
            return;
        }
        uint32_t channelCount = stream.in_uint32_le();

        #warning make an object with the channel list and let it manage creation of channels
        for (uint32_t index = 0; index < channelCount; index++) {
            struct mcs_channel_item *channel_item = new mcs_channel_item; /* zeroed */
            memcpy(channel_item->name, stream.in_uint8p(8), 8);
            channel_item->flags = stream.in_uint32_be();
            channel_item->chanid = MCS_GLOBAL_CHANNEL + (index + 1);
            this->mcs_layer.channel_list.push_back(channel_item);
        }
    }


    // 2.2.1.3.5 Client Cluster Data (TS_UD_CS_CLUSTER)
    // ------------------------------------------------
    // The TS_UD_CS_CLUSTER data block is sent by the client to the server either to advertise that it can
    // support the Server Redirection PDUs (sections 2.2.13.2 and 2.2.13.3) or to request a connection to
    // a given session identifier.

    // header (4 bytes): GCC user data block header, as specified in User Data
    //                   Header (section 2.2.1.3.1). The User Data Header type
    //                   field MUST be set to CS_CLUSTER (0xC004).

    // Flags (4 bytes): A 32-bit, unsigned integer. Cluster information flags.

    //           Flag                            Meaning
    // -------------------------------------------------------------------------
    // REDIRECTION_SUPPORTED               The client can receive server session
    //       0x00000001                    redirection packets. If this flag is
    //                                     set, the
    //                                     ServerSessionRedirectionVersionMask
    //                                     MUST contain the server session
    //                                     redirection version that the client
    //                                     supports.
    // -------------------------------------------------------------------------
    // ServerSessionRedirectionVersionMask The server session redirection
    //       0x0000003C                    version that the client supports.
    //                                     See the discussion which follows
    //                                     this table for more information.
    // -------------------------------------------------------------------------
    // REDIRECTED_SESSIONID_FIELD_VALID    The RedirectedSessionID field
    //       0x00000002                    contains an ID that identifies a
    //                                     session on the server to associate
    //                                     with the connection.
    // -------------------------------------------------------------------------
    // REDIRECTED_SMARTCARD                The client logged on with a smart
    //       0x00000040                    card.
    // -------------------------------------------------------------------------

    // The ServerSessionRedirectionVersionMask is a 4-bit enumerated value
    // containing the server session redirection version supported by the
    // client. The following are possible version values.

    //          Value                              Meaning
    // -------------------------------------------------------------------------
    // REDIRECTION_VERSION3                If REDIRECTION_SUPPORTED is set,
    //          0x02                       server session redirection version 3
    //                                     is supported by the client.
    // -------------------------------------------------------------------------
    // REDIRECTION_VERSION4                If REDIRECTION_SUPPORTED is set,
    //          0x03                       server session redirection version 4
    //                                     is supported by the client.
    // -------------------------------------------------------------------------
    // REDIRECTION_VERSION5                If REDIRECTION_SUPPORTED is set,
    //          0x04                       server session redirection version 5
    //                                     is supported by the client.
    // -------------------------------------------------------------------------

    // The version values cannot be combined; only one value MUST be specified
    // if the REDIRECTED_SESSIONID_FIELD_VALID (0x00000002) flag is present in
    // the Flags field.

    // RedirectedSessionID (4 bytes): A 32-bit unsigned integer. If the
    //                                REDIRECTED_SESSIONID_FIELD_VALID flag is
    //                                set in the Flags field, then the
    //                                RedirectedSessionID field contains a valid
    //                                session identifier to which the client
    //                                requests to connect.

    // This is this header that contains the console flag (undocumented ?)
    void server_sec_parse_mcs_data_cs_cluster(Stream & stream)
    {
        LOG(LOG_INFO, "CS_CLUSTER\n");
        uint32_t flags = stream.in_uint32_le();
        LOG(LOG_INFO, "cluster_data: flags = %x\n", flags);
        this->client_info->console_session = (flags & 0x2) != 0;
    }

    // 2.2.1.3.6 Client Monitor Data (TS_UD_CS_MONITOR)
    // ------------------------------------------------
    // The TS_UD_CS_MONITOR packet describes the client-side display monitor
    // layout. This packet is an Extended Client Data Block and MUST NOT be sent
    // to a server which does not advertise support for Extended Client Data
    // Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as
    // described in section 2.2.1.2.1.

    // header (4 bytes): GCC user data block header, as specified in User Data
    //                   Header (section 2.2.1.3.1). The User Data Header type
    //                   field MUST be set to CS_MONITOR (0xC005).

    // flags (4 bytes): A 32-bit, unsigned integer. This field is unused and
    //                  reserved for future use.

    // monitorCount (4 bytes): A 32-bit, unsigned integer. The number of display
    //                         monitor definitions in the monitorDefArray field
    //                        (the maximum allowed is 16).

    // monitorDefArray (variable): A variable-length array containing a series
    //                             of TS_MONITOR_DEF structures (section
    //                             2.2.1.3.6.1) which describe the display
    //                             monitor layout of the client. The number of
    //                             TS_MONITOR_DEF structures is given by the
    //                             monitorCount field.


    // 2.2.1.3.6.1 Monitor Definition (TS_MONITOR_DEF)
    // -----------------------------------------------
    // The TS_MONITOR_DEF packet describes the configuration of a client-side
    // display monitor. The x and y coordinates used to describe the monitor
    // position MUST be relative to the upper-left corner of the monitor
    // designated as the "primary display monitor" (the upper-left corner of the
    // primary monitor is always (0, 0)).

    // left (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate of
    //                 the upper-left corner of the display monitor.

    // top (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate of
    //                the upper-left corner of the display monitor.

    // right (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate
    //                  of the lower-right corner of the display monitor.

    // bottom (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate
    //                   of the lower-right corner of the display monitor.

    // flags (4 bytes): A 32-bit, unsigned integer. Monitor configuration flags.

    //        Value                          Meaning
    // -------------------------------------------------------------------------
    // TS_MONITOR_PRIMARY            The top, left, right and bottom fields
    //      0x00000001               describe the position of the primary
    //                               monitor.
    // -------------------------------------------------------------------------

    void server_sec_parse_mcs_data_cs_monitor(Stream & stream)
    {
        LOG(LOG_INFO, "CS_MONITOR\n");
    }

    // 2.2.1.4.2 Server Core Data (TS_UD_SC_CORE)
    void server_sec_parse_mcs_data_sc_core(Stream & stream)
    {
        LOG(LOG_INFO, "SC_CORE\n");
    }

    // 2.2.1.4.3 Server Security Data (TS_UD_SC_SEC1)
    void server_sec_parse_mcs_data_sc_security(Stream & stream)
    {
        LOG(LOG_INFO, "SC_SECURITY\n");
    }

    // 2.2.1.4.4 Server Network Data (TS_UD_SC_NET)
    void server_sec_parse_mcs_data_sc_net(Stream & stream)
    {
        LOG(LOG_INFO, "SC_NET\n");
    }

    /*****************************************************************************/
    void server_sec_disconnect()
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void server_sec_init_client_crypt_random(Stream & stream)
    {
        memcpy(this->client_crypt_random, stream.in_uint8p(64), 64);
    }

    void server_sec_init_server_random()
    {
        memset(this->server_random, 0x44, 32);
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, this->server_random, 32) != 32) {
            }
            close(fd);
        }
    }

// 2.2.1.3 Client MCS Connect Initial PDU with GCC Conference Create Request
// =========================================================================

// The MCS Connect Initial PDU is an RDP Connection Sequence PDU sent from
// client to server during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent after receiving the X.224 Connection Confirm PDU
// (section 2.2.1.2). The MCS Connect Initial PDU encapsulates a GCC Conference
// Create Request, which encapsulates concatenated blocks of settings data. A
// basic high-level overview of the nested structure for the Client MCS Connect
// Initial PDU is illustrated in section 1.3.1.1, in the figure specifying MCS
// Connect Initial PDU. Note that the order of the settings data blocks is
// allowed to vary from that shown in the previously mentioned figure and the
// message syntax layout that follows. This is possible because each data block
// is identified by a User Data Header structure (section 2.2.1.3.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//   section 13.7.

// mcsCi (variable): Variable-length BER-encoded MCS Connect Initial structure
//   (using definite-length encoding) as described in [T125] (the ASN.1
//   structure definition is detailed in [T125] section 7, part 2). The userData
//   field of the MCS Connect Initial encapsulates the GCC Conference Create
//   Request data (contained in the gccCCrq and subsequent fields). The maximum
//   allowed size of this user data is 1024 bytes, which implies that the
//   combined size of the gccCCrq and subsequent fields MUST be less than 1024
//   bytes.

// gccCCrq (variable): Variable-length Packed Encoding Rule encoded
//   (PER-encoded) GCC Connect Data structure, which encapsulates a Connect GCC
//   PDU that contains a GCC Conference Create Request structure as described in
//   [T124] (the ASN.1 structure definitions are detailed in [T124] section 8.7)
//   appended as user data to the MCS Connect Initial (using the format
//   described in [T124] sections 9.5 and 9.6). The userData field of the GCC
//   Conference Create Request contains one user data set consisting of
//   concatenated client data blocks.

// clientCoreData (216 bytes): Client Core Data structure (section 2.2.1.3.2).

// clientSecurityData (12 bytes): Client Security Data structure (section
//   2.2.1.3.3).

// clientNetworkData (variable): Optional and variable-length Client Network
//   Data structure (section 2.2.1.3.4).

// clientClusterData (12 bytes): Optional Client Cluster Data structure (section
//   2.2.1.3.5).

// clientMonitorData (variable): Optional Client Monitor Data structure (section
//   2.2.1.3.6). This field MUST NOT be included if the server does not
//   advertise support for extended client data blocks by using the
//   EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section
//   2.2.1.2.1.

    void recv_connection_initial(Stream & data)
    {
        Stream stream(8192);
        X224In(this->trans, stream);

        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();
        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);
        if (stream.in_uint8() != BER_TAG_BOOLEAN) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        /* make a copy of client mcs data */
        data.init(len);
        data.out_copy_bytes(stream.p, len);
        data.mark_end();
        stream.skip_uint8(len);
    }



    void send_connect_response(Stream & data, Transport * trans) throw(Error)
    {
//        LOG(LOG_INFO, send_connect_response");
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = data.end - data.data;
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        stream.out_ber_len(data_len + 38);

        stream.out_uint8(BER_TAG_RESULT);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_uint8(26);
        stream.out_ber_int8(22); // max_channels
        stream.out_ber_int8(3); // max_users
        stream.out_ber_int8(0); // max_tokens
        stream.out_ber_int8(1);
        stream.out_ber_int8(0);
        stream.out_ber_int8(1);
        stream.out_ber_int24(0xfff8); // max_pdu_size
        stream.out_ber_int8(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(data_len);
        /* mcs data */
        stream.out_copy_bytes(data.data, data_len);

        tpdu.end();
        tpdu.send(this->trans);
    }



    void server_sec_incoming() throw (Error)
    {

        Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);

        this->server_sec_init_server_random();

        memcpy(this->pub_exp, rsa_keys.pub_exp, 4);
        memcpy(this->pub_mod, rsa_keys.pub_mod, 64);
        memcpy(this->pub_sig, rsa_keys.pub_sig, 64);
        memcpy(this->pri_exp, rsa_keys.pri_exp, 64);

        Stream in(8192);
        X224In crtpdu(this->trans, in);
        if (crtpdu.tpdu_hdr.code != ISO_PDU_CR) {
            throw Error(ERR_ISO_INCOMING_CODE_NOT_PDU_CR);
        }

        Stream out(11);
        X224Out cctpdu(X224Packet::CC_TPDU, out);
        cctpdu.end();
        cctpdu.send(this->trans);

        this->recv_connection_initial(this->client_mcs_data);
        #warning we should fully decode Client MCS Connect Initial PDU with GCC Conference Create Request instead of just calling the function below to extract the fields, that is quite dirty
        this->server_sec_process_mcs_data(this->client_mcs_data);
        this->server_sec_out_mcs_data(this->data);
        this->send_connect_response(this->data, this->trans);

        //   2.2.1.5 Client MCS Erect Domain Request PDU
        //   -------------------------------------------
        //   The MCS Erect Domain Request PDU is an RDP Connection Sequence PDU sent
        //   from client to server during the Channel Connection phase (see section
        //   1.3.1.1). It is sent after receiving the MCS Connect Response PDU (section
        //   2.2.1.4).

        //   tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

        //   x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
        //      section 13.7.

        // See description of tpktHeader and x224 Data TPDU in cheat sheet

        //   mcsEDrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an MCS
        //      Erect Domain Request structure, as specified in [T125] (the ASN.1
        //      structure definitions are given in [T125] section 7, parts 3 and 10).

        {
            Stream stream(8192);
            X224In(this->trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_EDRQ) {
                throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
            }
            stream.skip_uint8(2);
            stream.skip_uint8(2);
            if (opcode & 2) {
                this->mcs_layer.userid = stream.in_uint16_be();
            }
            if (!stream.check_end()) {
                throw Error(ERR_MCS_RECV_EDQR_TRUNCATED);
            }
        }


        // 2.2.1.6 Client MCS Attach User Request PDU
        // ------------------------------------------
        // The MCS Attach User Request PDU is an RDP Connection Sequence PDU
        // sent from client to server during the Channel Connection phase (see
        // section 1.3.1.1) to request a user channel ID. It is sent after
        // transmitting the MCS Erect Domain Request PDU (section 2.2.1.5).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //   [X224] section 13.7.

        // See description of tpktHeader and x224 Data TPDU in cheat sheet

        // mcsAUrq (1 byte): PER-encoded MCS Domain PDU which encapsulates an
        //  MCS Attach User Request structure, as specified in [T125] (the ASN.1
        //  structure definitions are given in [T125] section 7, parts 5 and 10).

        {
            Stream stream(8192);
            X224In(this->trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_AURQ) {
                throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
            }
            if (opcode & 2) {
                this->mcs_layer.userid = stream.in_uint16_be();
            }
            if (!stream.check_end()) {
                throw Error(ERR_MCS_RECV_AURQ_TRUNCATED);
            }
        }

        // 2.2.1.7 Server MCS Attach User Confirm PDU
        // ------------------------------------------
        // The MCS Attach User Confirm PDU is an RDP Connection Sequence
        // PDU sent from server to client during the Channel Connection
        // phase (see section 1.3.1.1). It is sent as a response to the MCS
        // Attach User Request PDU (section 2.2.1.6).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
        //   section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //   section [X224] 13.7.

        // mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
        //   an MCS Attach User Confirm structure, as specified in [T125]
        //   (the ASN.1 structure definitions are given in [T125] section 7,
        // parts 5 and 10).

        {
            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            stream.out_uint8(((MCS_AUCF << 2) | 2));
            stream.out_uint8(0);
            stream.out_uint16_be(this->mcs_layer.userid);
            tpdu.end();
            tpdu.send(this->trans);
        }

        {
            {
                Stream stream(8192);
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                X224In(this->trans, stream);

                int opcode = stream.in_uint8();
                if ((opcode >> 2) != MCS_CJRQ) {
                    throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
                }
                stream.skip_uint8(4);
                if (opcode & 2) {
                    stream.skip_uint8(2);
                }
            }

            // 2.2.1.9 Server MCS Channel Join Confirm PDU
            // -------------------------------------------
            // The MCS Channel Join Confirm PDU is an RDP Connection Sequence
            // PDU sent from server to client during the Channel Connection
            // phase (see section 1.3.1.1). It is sent as a response to the MCS
            // Channel Join Request PDU (section 2.2.1.8).

            // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
            //   section 8.

            // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
            //  [X224] section 13.7.

            // mcsCJcf (8 bytes): PER-encoded MCS Domain PDU which encapsulates
            //  an MCS Channel Join Confirm PDU structure, as specified in
            //  [T125] (the ASN.1 structure definitions are given in [T125]
            //  section 7, parts 6 and 10).

            {
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                stream.out_uint8((MCS_CJCF << 2) | 2);
                stream.out_uint8(0);
                stream.out_uint16_be(this->mcs_layer.userid);
                stream.out_uint16_be(this->mcs_layer.userid + MCS_USERCHANNEL_BASE);
                stream.out_uint16_be(this->mcs_layer.userid + MCS_USERCHANNEL_BASE);
                tpdu.end();
                tpdu.send(this->trans);
            }
        }

        {
            {
                Stream stream(8192);
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                X224In(this->trans, stream);

                int opcode = stream.in_uint8();
                if ((opcode >> 2) != MCS_CJRQ) {
                    throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
                }
                stream.skip_uint8(4);
                if (opcode & 2) {
                    stream.skip_uint8(2);
                }
            }

            {
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                stream.out_uint8((MCS_CJCF << 2) | 2);
                stream.out_uint8(0);
                stream.out_uint16_be(this->mcs_layer.userid);
                stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
                stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
                tpdu.end();
                tpdu.send(this->trans);
            }
        }

    }


    /*****************************************************************************/
    /* prepare server mcs data to send in mcs layer */
    void server_sec_out_mcs_data(Stream  & stream)
    {
        /* Same code above using list_test */
        int num_channels = (int) this->mcs_layer.channel_list.size();
        int num_channels_even = num_channels + (num_channels & 1);

        stream.init(512);
        stream.out_uint16_be(5);
        stream.out_uint16_be(0x14);
        stream.out_uint8(0x7c);
        stream.out_uint16_be(1);
        stream.out_uint8(0x2a);
        stream.out_uint8(0x14);
        stream.out_uint8(0x76);
        stream.out_uint8(0x0a);
        stream.out_uint8(1);
        stream.out_uint8(1);
        stream.out_uint8(0);
        stream.out_uint16_le(0xc001);
        stream.out_uint8(0);
        stream.out_uint8(0x4d); /* M */
        stream.out_uint8(0x63); /* c */
        stream.out_uint8(0x44); /* D */
        stream.out_uint8(0x6e); /* n */
        stream.out_uint16_be(0x80fc + (num_channels_even * 2));
        stream.out_uint16_le(SEC_TAG_SRV_INFO);
        stream.out_uint16_le(8); /* len */
        stream.out_uint8(4); /* 4 = rdp5 1 = rdp4 */
        stream.out_uint8(0);
        stream.out_uint8(8);
        stream.out_uint8(0);
        stream.out_uint16_le(SEC_TAG_SRV_CHANNELS);
        stream.out_uint16_le(8 + (num_channels_even * 2)); /* len */
        stream.out_uint16_le(MCS_GLOBAL_CHANNEL); /* 1003, 0x03eb main channel */
        stream.out_uint16_le(num_channels); /* number of other channels */

        for (int index = 0; index < num_channels_even; index++) {
            if (index < num_channels) {
                stream.out_uint16_le(MCS_GLOBAL_CHANNEL + (index + 1));
            } else {
                stream.out_uint16_le( 0);
            }
        }
        stream.out_uint16_le(SEC_TAG_SRV_CRYPT);
        stream.out_uint16_le(0x00ec); /* len is 236 */
        stream.out_uint32_le(this->rc4_key_size); /* key len 1 = 40 bit 2 = 128 bit */
        stream.out_uint32_le(this->client_info->crypt_level); /* crypt level 1 = low 2 = medium */
        /* 3 = high */
        stream.out_uint32_le(32);     /* 32 bytes random len */
        stream.out_uint32_le(0xb8);   /* 184 bytes rsa info(certificate) len */
        stream.out_copy_bytes(this->server_random, 32);
        /* here to end is certificate */
        /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
        /* TermService\Parameters\Certificate */
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);
        stream.out_uint16_le(SEC_TAG_PUBKEY);
        stream.out_uint16_le(0x005c); /* 92 bytes length of SEC_TAG_PUBKEY */
        stream.out_uint32_le(SEC_RSA_MAGIC);
        stream.out_uint32_le(0x48); /* 72 bytes modulus len */
        stream.out_uint32_be(0x00020000);
        stream.out_uint32_be(0x3f000000);
        stream.out_copy_bytes(this->pub_exp, 4); /* pub exp */
        stream.out_copy_bytes(this->pub_mod, 64); /* pub mod */
        stream.out_clear_bytes(8); /* pad */
        stream.out_uint16_le(SEC_TAG_KEYSIG);
        stream.out_uint16_le(72); /* len */
        stream.out_copy_bytes(this->pub_sig, 64); /* pub sig */
        stream.out_clear_bytes(8); /* pad */
        /* end certificate */
        stream.mark_end();
    }

};


#endif
