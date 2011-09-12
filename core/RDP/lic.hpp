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

   Licence Management

*/

#if !defined(__CORE_RDP_LIC_HPP__)
#define __CORE_RDP_LIC_HPP__

#include "RDP/sec.hpp"

static inline void send_lic_initial(Transport * trans, int userid) throw (Error)
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
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    McsOut sdin_out(stream, MCS_SDIN, userid, MCS_GLOBAL_CHANNEL);
    stream.out_copy_bytes((char*)lic1, 322);
    sdin_out.end();
    tpdu.end();
    tpdu.send(trans);
}


static inline void send_lic_response(Transport * trans, int userid) throw (Error)
{
    /* some compilers need unsigned char to avoid warnings */
    static uint8_t lic2[20] = { 0x80, 0x00, 0x10, 0x00, 0xff, 0x02, 0x10, 0x00,
                             0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0x28, 0x14, 0x00, 0x00
                           };

    Stream stream(8192);
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    McsOut sdin_out(stream, MCS_SDIN, userid, MCS_GLOBAL_CHANNEL);
    stream.out_copy_bytes((char*)lic2, 20);
    sdin_out.end();
    tpdu.end();
    tpdu.send(trans);
}

static inline void send_media_lic_response(Transport * trans, int userid) throw (Error)
{
    /* mce */
    /* some compilers need unsigned char to avoid warnings */
    static uint8_t lic3[20] = { 0x80, 0x02, 0x10, 0x00, 0xff, 0x03, 0x10, 0x00,
                             0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0xf3, 0x99, 0x00, 0x00
                             };

    Stream stream(8192);
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    McsOut sdin_out(stream, MCS_SDIN, userid, MCS_GLOBAL_CHANNEL);
    stream.out_copy_bytes((char*)lic3, 20);
    sdin_out.end();
    tpdu.end();
    tpdu.send(trans);
}


struct RdpLicence {
    uint8_t licence_key[16];
    uint8_t licence_sign_key[16];
    int licence_issued;
    uint8_t * licence_data;
    size_t licence_size;

    RdpLicence(const char * hostname) : licence_issued(0) {
        memset(this->licence_key, 0, 16);
        memset(this->licence_sign_key, 0, 16);
        #warning licence loading should be done before creating protocol layers
        struct stat st;
        char path[256];
        sprintf(path, "/etc/xrdp/.xrdp/licence.%s", hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1 && fstat(fd, &st) != 0){
            this->licence_data = (uint8_t *)malloc(this->licence_size);
            #warning check error code here
            if (this->licence_size != read(fd, this->licence_data, this->licence_size)){
                #warning throwing an error would be better
                return;
            }
            close(fd);
        }
    }

    void rdp_lic_process_authreq(Transport * trans, Stream & stream, const char * hostname, int userid, int licence_issued)
    {

        ssllib ssl;

        const uint8_t* in_token;
        const uint8_t* in_sig;
        uint8_t out_token[LICENCE_TOKEN_SIZE];
        uint8_t decrypt_token[LICENCE_TOKEN_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];
        uint8_t crypt_hwid[LICENCE_HWID_SIZE];
        uint8_t sealed_buffer[LICENCE_TOKEN_SIZE + LICENCE_HWID_SIZE];
        uint8_t out_sig[LICENCE_SIGNATURE_SIZE];

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
        SSL_RC4 crypt_key;
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        memcpy(decrypt_token, in_token, LICENCE_TOKEN_SIZE);
        ssl.rc4_crypt(crypt_key, decrypt_token, decrypt_token, LICENCE_TOKEN_SIZE);
        /* Generate a signature for a buffer of token and HWID */
        this->rdp_lic_generate_hwid(hwid, hostname);
        memcpy(sealed_buffer, decrypt_token, LICENCE_TOKEN_SIZE);
        memcpy(sealed_buffer + LICENCE_TOKEN_SIZE, hwid, LICENCE_HWID_SIZE);
        sec_sign(out_sig, 16, this->licence_sign_key, 16, sealed_buffer, sizeof(sealed_buffer));
        /* Now encrypt the HWID */
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        memcpy(crypt_hwid, hwid, LICENCE_HWID_SIZE);
        ssl.rc4_crypt(crypt_key, crypt_hwid, crypt_hwid, LICENCE_HWID_SIZE);

        rdp_lic_send_authresp(trans, out_token, crypt_hwid, out_sig, userid, licence_issued);
    }

    void rdp_lic_send_authresp(Transport * trans, uint8_t* token, uint8_t* crypt_hwid, uint8_t* signature, int userid, int licence_issued)
    {
        int length = 58;

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        stream.out_uint8(licence_issued?LICENCE_TAG_AUTHRESP:SEC_LICENCE_NEG);

        stream.out_uint8(2); /* version */
        stream.out_uint16_le(length);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_TOKEN_SIZE);
        stream.out_copy_bytes(token, LICENCE_TOKEN_SIZE);
        stream.out_uint16_le(1);
        stream.out_uint16_le(LICENCE_HWID_SIZE);
        stream.out_copy_bytes(crypt_hwid, LICENCE_HWID_SIZE);
        stream.out_copy_bytes(signature, LICENCE_SIGNATURE_SIZE);

        sdrq_out.end();
        tpdu.end();

        tpdu.send(trans);
    }


    void rdp_lic_process_demand(Transport * trans, Stream & stream, const char * hostname, const char * username, int userid, const int licence_issued)
    {
        uint8_t null_data[SEC_MODULUS_SIZE];
        uint8_t signature[LICENCE_SIGNATURE_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];

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
            sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 65);
            sec_hash_48(key_block, master_secret, server_random, client_random, 65);
            /* Store first 16 bytes of session key as MAC secret */
            memcpy(this->licence_sign_key, key_block, 16);
            /* Generate RC4 key from next 16 bytes */
            sec_hash_16(this->licence_key, key_block + 16, client_random, server_random);
        }

        if (this->licence_size > 0) {
            /* Generate a signature for the HWID buffer */
            this->rdp_lic_generate_hwid(hwid, hostname);
            sec_sign(signature, 16, this->licence_sign_key, 16, hwid, sizeof(hwid));
            /* Now encrypt the HWID */
            ssllib ssl;

            SSL_RC4 crypt_key;
            ssl.rc4_set_key(crypt_key, this->licence_key, 16);
            ssl.rc4_crypt(crypt_key, hwid, hwid, sizeof(hwid));

            this->rdp_lic_present(trans, null_data, null_data,
                                  this->licence_data,
                                  this->licence_size,
                                  hwid, signature, userid, licence_issued);
        }
        else {
            this->rdp_lic_send_request(trans, null_data, null_data, hostname, username, userid, licence_issued);
        }
    }

    int rdp_lic_process_issue(Stream & stream, const char * hostname, int & licence_issued)
    {
        stream.skip_uint8(2); /* 3d 45 - unknown */
        int length = stream.in_uint16_le();
        if (!stream.check_rem(length)) {
            #warning use exception
            return 0;
        }
        ssllib ssl;
        SSL_RC4 crypt_key;
        ssl.rc4_set_key(crypt_key, this->licence_key, 16);
        ssl.rc4_crypt(crypt_key, stream.p, stream.p, length);
        int check = stream.in_uint16_le();
        if (check != 0) {
            #warning use exception
            return 0;
        }
        licence_issued = 1;
        stream.skip_uint8(2); /* pad */
        /* advance to fourth string */
        length = 0;
        for (int i = 0; i < 4; i++) {
            stream.skip_uint8(length);
            length = stream.in_uint32_le();
            if (!stream.check_rem(length)) {
            #warning use exception
                return 0;
            }
        }
        /* todo save_licence(stream.p, length); */
        this->rdp_save_licence(stream.p, length, hostname);
        return 1;
    }

    #warning this is not supported yet, but using rdp_save_licence we would keep a local copy of the licence of a remote server thus avoiding to ask it every time we connect. Anyway the use of files to stoe licences should be abstracted.
    void rdp_save_licence(uint8_t *data, int length, const char * hostname)
    {
        int fd;
        char* path = NULL;
        char* tmppath = NULL;

        path = new char[256];
        /* TODO: verify if location that we've stablished is right or not */
        sprintf(path, "/etc/xrdp./xrdp/licence.%s", hostname);

        if ((mkdir(path, 0700) == -1))
        {
            if (errno != EEXIST){
                perror(path);
                return;
            }
        }

        /* write licence to licence.hostname.new and after rename to licence.hostname */

        sprintf(path, "/etc/xrdp./xrdp/licence.%s", hostname);
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

    void rdp_lic_generate_hwid(uint8_t* hwid, const char * hostname)
    {
        buf_out_uint32(hwid, 2);
        memcpy(hwid + 4, hostname, LICENCE_HWID_SIZE - 4);
    }

    void rdp_lic_send_request(Transport * trans, uint8_t* client_random, uint8_t* rsa_data, const char * hostname, const char * username, int userid, int licence_issued)
    {
        int userlen = strlen(username) + 1;
        int hostlen = strlen(hostname) + 1;
        int length = 128 + userlen + hostlen;

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        #warning if we are performing licence request doesn't it mean that licence has not been issued ?
        stream.out_uint8(licence_issued?LICENCE_TAG_REQUEST:SEC_LICENCE_NEG);
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
        stream.out_copy_bytes(username, userlen);

        stream.out_uint16_le(LICENCE_TAG_HOST);
        stream.out_uint16_le(hostlen);
        stream.out_copy_bytes(hostname, hostlen);

        sdrq_out.end();
        tpdu.end();
        tpdu.send(trans);
    }

    void rdp_lic_present(Transport * trans, uint8_t* client_random, uint8_t* rsa_data,
                uint8_t* licence_data, int licence_size, uint8_t* hwid,
                uint8_t* signature, int userid, const int licence_issued)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        int length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                 licence_size + LICENCE_HWID_SIZE + LICENCE_SIGNATURE_SIZE;

        stream.out_uint8(licence_issued?LICENCE_TAG_PRESENT:SEC_LICENCE_NEG);
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

        sdrq_out.end();
        tpdu.end();
        tpdu.send(trans);
    }

};



#endif
