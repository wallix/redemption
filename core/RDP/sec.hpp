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

#include <assert.h>
#include <stdint.h>

#include <iostream>

#include "RDP/x224.hpp"
#include "RDP/rdp.hpp"
#include "client_info.hpp"
#include "rsa_keys.hpp"
#include "constants.hpp"


#warning ssl calls introduce some dependency on ssl system library, injecting it in the sec object would be better.
#include "ssl_calls.hpp"


inline static void sec_make_40bit(uint8_t* key)
{
    key[0] = 0xd1;
    key[1] = 0x26;
    key[2] = 0x9e;
}

// Output a uint32 into a buffer (little-endian)
inline static void buf_out_uint32(uint8_t* buffer, int value)
{
  buffer[0] = value & 0xff;
  buffer[1] = (value >> 8) & 0xff;
  buffer[2] = (value >> 16) & 0xff;
  buffer[3] = (value >> 24) & 0xff;
}


#warning method used by licence, common with basic crypto support code should be made common. pad are also common to several functions.
/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
inline static void sec_sign(uint8_t* signature, int siglen, uint8_t* session_key, int keylen, uint8_t* data, int datalen)
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

    buf_out_uint32(lenhdr, datalen);

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




struct CryptContext
{
    int use_count;
    uint8_t sign_key[16]; // should I call it session_key ?
    uint8_t key[16];
    uint8_t update_key[16];
    int rc4_key_len;
    SSL_RC4 rc4_info;
    CryptContext() : use_count(0)
    {
        memset(this->sign_key, 0, 16);
    }

    /* Encrypt data using RC4 */
    void encrypt(uint8_t* data, int length)
    {
        ssllib ssl;

        if (this->use_count == 4096){
            this->update();
            if (this->rc4_key_len == 8) {
                sec_make_40bit(this->key);
            }
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, length);
        this->use_count++;
    }

    /* Decrypt data using RC4 */
    void decrypt(uint8_t* data, int len)
    {
        ssllib ssl;

        if (this->use_count == 4096) {
            this->update();
            if (this->rc4_key_len == 8) {
                sec_make_40bit(this->key);
            }
            ssl.rc4_set_key(this->rc4_info, this->key, this->rc4_key_len);
            this->use_count = 0;
        }
        ssl.rc4_crypt(this->rc4_info, data, data, len);
        this->use_count++;
    }

    /* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
    void sign(uint8_t* signature, int siglen, uint8_t* data, int datalen)
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

        buf_out_uint32(lenhdr, datalen);

        ssllib ssl;

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, this->sign_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, lenhdr, 4);
        ssl.sha1_update(&sha1, data, datalen);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->sign_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, md5sig);

        memcpy(signature, md5sig, siglen);
    }

    void update()
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
        ssl.sha1_update(&sha1, this->update_key, this->rc4_key_len);
        ssl.sha1_update(&sha1, pad_54, 40);
        ssl.sha1_update(&sha1, this->key, this->rc4_key_len);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, this->update_key, this->rc4_key_len);
        ssl.md5_update(&md5, pad_92, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, key);

        ssl.rc4_set_key(update, this->key, this->rc4_key_len);
        ssl.rc4_crypt(update, this->key, this->key, this->rc4_key_len);
    }

};




// 2.2.1.1.1   RDP Negotiation Request (RDP_NEG_REQ)
// =================================================
//  The RDP Negotiation Request structure is used by a client to advertise the
//  security protocols which it supports.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x01 (TYPE_RDP_NEG_REQ) to indicate that the packet
//   is a Negotiation Request.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size.
//   This field MUST be set to 0x0008 (8 bytes).

// requestedProtocols (4 bytes): A 32-bit, unsigned integer. Flags indicating
//   the supported security protocols.

// +---------------------------------+-----------------------------------------+
// | 0x00000000 PROTOCOL_RDP_FLAG    |  Legacy RDP encryption.                 |
// +---------------------------------+-----------------------------------------+
// | 0x00000001 PROTOCOL_SSL_FLAG    | TLS 1.0 (section 5.4.5.1).              |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID_FLAG | Credential Security Support Provider    |
// |                                 | protocol (CredSSP) (section 5.4.5.2).   |
// |                                 | If this flag is set, then the           |
// |                                 | PROTOCOL_SSL_FLAG (0x00000001) SHOULD   |
// |                                 | also be set because Transport Layer     |
// |                                 | Security (TLS) is a subset of CredSSP.  |
// +---------------------------------+-----------------------------------------+

// 2.2.1.2.1   RDP Negotiation Response (RDP_NEG_RSP)
// ==================================================

//  The RDP Negotiation Response structure is used by a server to inform the
//  client of the security protocol which it has selected to use for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This field MUST be set to
//   0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +--------------------------------+------------------------------------------+
// | EXTENDED_CLIENT_DATA_SUPPORTED | The server supports extended client data |
// | 0x00000001                     | blocks in the GCC Conference Create      |
// |                                | Request user data (section 2.2.1.3).     |
// +--------------------------------+------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the
//   selected security protocol.

// +---------------------------------------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Legacy RDP encryption                        |
// +---------------------------------------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +---------------------------------------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +---------------------------------------------------------------------------+

// 2.2.1.2.2   RDP Negotiation Failure (RDP_NEG_FAILURE)
// =====================================================

//  The RDP Negotiation Failure structure is used by a server to inform the
//  client of a failure that has occurred while preparing security for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the
//   packet is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
//   failure code.

// +---------------------------+-----------------------------------------------+
// | SSL_REQUIRED_BY_SERVER    | The server requires that the client support   |
// | 0x00000001                | Enhanced RDP Security (section 5.4) with      |
// |                           | either TLS 1.0 (section 5.4.5.1) or CredSSP   |
// |                           | (section 5.4.5.2). If only CredSSP was        |
// |                           | requested then the server only supports TLS.  |
// +---------------------------+-----------------------------------------------+
// | SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only use Standard |
// | 0x00000002                | RDP Security mechanisms (section 5.3) and     |
// |                           | does not support any External                 |
// |                           | Security Protocols (section 5.4.5).           |
// +---------------------------+-----------------------------------------------+
// | SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid server    |
// | 0x00000003                | authentication certificate and cannot         |
// |                           | initialize the External Security Protocol     |
// |                           | Provider (section 5.4.5).                     |
// +---------------------------+-----------------------------------------------+
// | INCONSISTENT_FLAGS        | The list of requested security protocols is   |
// | 0x00000004                | not consistent with the current security      |
// |                           | protocol in effect. This error is only        |
// |                           | possible when the Direct Approach (see        |
// |                           | sections 5.4.2.2 and 1.3.1.2) is used and an  |
// |                           | External Security Protocol (section 5.4.5) is |
// |                           | already being used.                           |
// +---------------------------+-----------------------------------------------+
// | HYBRID_REQUIRED_BY_SERVER | The server requires that the client support   |
// | 0x00000005                | Enhanced RDP Security (section 5.4) with      |
// |                           | CredSSP (section 5.4.5.2).                    |
// +---------------------------+-----------------------------------------------+


class SecOut
{
    Stream & stream;
    uint16_t offhdr;
    uint8_t crypt_level;
    CryptContext & crypt;
    public:
    SecOut(Stream & stream, uint8_t crypt_level, uint32_t flags, CryptContext & crypt)
        : stream(stream), offhdr(stream.p - stream.data), crypt_level(crypt_level), crypt(crypt)
    {
        if (crypt_level > 1){
            this->stream.out_uint32_le(flags);
            this->stream.skip_uint8(8);
        }
        else {
            this->stream.out_uint32_le(0);
        }
    }

    void end(){
        if (crypt_level > 1){
            uint8_t * data = this->stream.data + this->offhdr + 12;
            int datalen = this->stream.p - data;
            this->crypt.sign(this->stream.data + this->offhdr + 4, 8, data, datalen);
            this->crypt.encrypt(data, datalen);
        }
    }
};


class SecIn
{
    public:
    uint32_t flags;
    SecIn(Stream & stream, CryptContext & crypt)
    {
        this->flags = stream.in_uint32_le();
        if ((this->flags & SEC_ENCRYPT)  || (this->flags & 0x0400)){
            #warning shouldn't we check signature ?
            stream.skip_uint8(8); /* signature */
            // decrypting to the end of tpdu
            crypt.decrypt(stream.p, stream.end - stream.p);
        }
    }

    void end(){
        #warning put some assertion here to ensure all data has been consumed
    }

};

static inline int rdp_sec_parse_public_sig(Stream & stream, int len, uint8_t* modulus, uint8_t* exponent, int server_public_key_len)
{
    /* Parse a public key structure */
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
    return ssl_sig_ok(exponent, SEC_EXPONENT_SIZE, modulus, server_public_key_len, signature, sig_len);
}

struct Sec
{

// only in server_sec : need cleanup

    uint8_t server_random[32];
    uint8_t client_random[64];

    uint8_t pub_exp[24];
    uint8_t pub_mod[512];
    uint8_t pub_sig[512];
    uint8_t pri_exp[512];

// only in rdp_sec : need cleanup
    int server_public_key_len;

// shared
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

//    ChannelList channel_list;

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];


    CryptContext encrypt, decrypt;


    uint8_t crypt_level;
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */

    Sec(uint8_t crypt_level) :
      licence_data(0),
      licence_size(0),
      crypt_level(crypt_level)
    {
        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);
        this->server_public_key_len = 0;

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        switch (crypt_level) {
        case 1:
        case 2:
            this->rc4_key_size = 1; /* 40 bits */
            this->decrypt.rc4_key_len = 8; /* 8 = 40 bit */
            this->encrypt.rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->rc4_key_size = 2; /* 128 bits */
            this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
            this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }

    }

    ~Sec() {}


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


    void server_sec_send_lic_initial(Transport * trans, int userid) throw (Error)
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

    void server_sec_send_lic_response(Transport * trans, int userid) throw (Error)
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

    void server_sec_send_media_lic_response(Transport * trans, int userid) throw (Error)
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

    void server_sec_process_logon_info(Stream & stream, ClientInfo * client_info) throw (Error)
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
            client_info->sound_code = 1;
        }
        if ((flags & RDP_LOGON_AUTO) && (!client_info->is_mce))
            /* todo, for now not allowing autologon and mce both */
        {
            client_info->rdp_autologin = 1;
        }
        if (flags & RDP_COMPRESSION) {
            client_info->rdp_compression = 1;
        }
        unsigned len_domain = stream.in_uint16_le();
        unsigned len_user = stream.in_uint16_le();
        unsigned len_password = stream.in_uint16_le();
        unsigned len_program = stream.in_uint16_le();
        unsigned len_directory = stream.in_uint16_le();
        /* todo, we should error out in any of the above lengths are > 512 */
        /* to avoid buffer overruns */
        unicode_in(stream, len_domain, (uint8_t*)client_info->domain, 255);
        unicode_in(stream, len_user, (uint8_t*)client_info->username, 255);
        // LOG(LOG_DEBUG, "setting username to %s\n", client_info->username);

        if (flags & RDP_LOGON_AUTO) {
            unicode_in(stream, len_password, (uint8_t*)client_info->password, 255);
        } else {
            stream.skip_uint8(len_password + 2);
        }
        unicode_in(stream, len_program, (uint8_t*)client_info->program, 255);
        unicode_in(stream, len_directory, (uint8_t*)client_info->directory, 255);
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
            client_info->rdp5_performanceflags = stream.in_uint32_le();
        }
    }


    /*****************************************************************************/
    void server_sec_disconnect(Transport * trans)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(trans);
    }



    void send_mcs_connect_response_pdu_with_gcc_conference_create_response(Transport * trans, ClientInfo * client_info, const ChannelList & channel_list) throw(Error)
    {
        Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);
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

        memcpy(this->pub_exp, rsa_keys.pub_exp, 4);
        memcpy(this->pub_mod, rsa_keys.pub_mod, 64);
        memcpy(this->pub_sig, rsa_keys.pub_sig, 64);
        memcpy(this->pri_exp, rsa_keys.pri_exp, 64);

        /* Same code above using list_test */
        int num_channels = (int) channel_list.size();
        int num_channels_even = num_channels + (num_channels & 1);

        Stream data(8192);

        data.out_uint16_be(5);
        data.out_uint16_be(0x14);
        data.out_uint8(0x7c);
        data.out_uint16_be(1);
        data.out_uint8(0x2a);
        data.out_uint8(0x14);
        data.out_uint8(0x76);
        data.out_uint8(0x0a);
        data.out_uint8(1);
        data.out_uint8(1);
        data.out_uint8(0);
        data.out_uint16_le(0xc001);
        data.out_uint8(0);
        data.out_uint8(0x4d); /* M */
        data.out_uint8(0x63); /* c */
        data.out_uint8(0x44); /* D */
        data.out_uint8(0x6e); /* n */
        data.out_uint16_be(0x80fc + (num_channels_even * 2));
        data.out_uint16_le(SEC_TAG_SRV_INFO);
        data.out_uint16_le(8); /* len */
        data.out_uint8(4); /* 4 = rdp5 1 = rdp4 */
        data.out_uint8(0);
        data.out_uint8(8);
        data.out_uint8(0);
        data.out_uint16_le(SEC_TAG_SRV_CHANNELS);
        data.out_uint16_le(8 + (num_channels_even * 2)); /* len */
        data.out_uint16_le(MCS_GLOBAL_CHANNEL); /* 1003, 0x03eb main channel */
        data.out_uint16_le(num_channels); /* number of other channels */

        for (int index = 0; index < num_channels_even; index++) {
            if (index < num_channels) {
                data.out_uint16_le(MCS_GLOBAL_CHANNEL + (index + 1));
            } else {
                data.out_uint16_le( 0);
            }
        }
        data.out_uint16_le(SEC_TAG_SRV_CRYPT);
        data.out_uint16_le(0x00ec); /* len is 236 */
        data.out_uint32_le(this->rc4_key_size); /* key len 1 = 40 bit 2 = 128 bit */
        data.out_uint32_le(client_info->crypt_level); /* crypt level 1 = low 2 = medium */
        /* 3 = high */
        data.out_uint32_le(32);     /* 32 bytes random len */
        data.out_uint32_le(0xb8);   /* 184 bytes rsa info(certificate) len */
        data.out_copy_bytes(this->server_random, 32);
        /* here to end is certificate */
        /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
        /* TermService\Parameters\Certificate */
        data.out_uint32_le(1);
        data.out_uint32_le(1);
        data.out_uint32_le(1);
        data.out_uint16_le(SEC_TAG_PUBKEY);
        data.out_uint16_le(0x005c); /* 92 bytes length of SEC_TAG_PUBKEY */
        data.out_uint32_le(SEC_RSA_MAGIC);
        data.out_uint32_le(0x48); /* 72 bytes modulus len */
        data.out_uint32_be(0x00020000);
        data.out_uint32_be(0x3f000000);
        data.out_copy_bytes(this->pub_exp, 4); /* pub exp */
        data.out_copy_bytes(this->pub_mod, 64); /* pub mod */
        data.out_clear_bytes(8); /* pad */
        data.out_uint16_le(SEC_TAG_KEYSIG);
        data.out_uint16_le(72); /* len */
        data.out_copy_bytes(this->pub_sig, 64); /* pub sig */
        data.out_clear_bytes(8); /* pad */
        /* end certificate */
        data.mark_end();

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
        tpdu.send(trans);
    }


    void rdp_lic_generate_hwid(uint8_t* hwid, const char * hostname)
    {
        buf_out_uint32(hwid, 2);
        memcpy(hwid + 4, hostname, LICENCE_HWID_SIZE - 4);
    }

    void rdp_lic_process_authreq(Transport * trans, Stream & stream, const char * hostname, int userid)
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
        ssl.rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        memcpy(decrypt_token, in_token, LICENCE_TOKEN_SIZE);
        ssl.rc4_crypt(crypt_key, decrypt_token, decrypt_token, LICENCE_TOKEN_SIZE);
        /* Generate a signature for a buffer of token and HWID */
        this->rdp_lic_generate_hwid(hwid, hostname);
        memcpy(sealed_buffer, decrypt_token, LICENCE_TOKEN_SIZE);
        memcpy(sealed_buffer + LICENCE_TOKEN_SIZE, hwid, LICENCE_HWID_SIZE);
        sec_sign(out_sig, 16, this->lic_layer.licence_sign_key, 16, sealed_buffer, sizeof(sealed_buffer));
        /* Now encrypt the HWID */
        ssl.rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        memcpy(crypt_hwid, hwid, LICENCE_HWID_SIZE);
        ssl.rc4_crypt(crypt_key, crypt_hwid, crypt_hwid, LICENCE_HWID_SIZE);

        this->rdp_lic_send_authresp(trans, out_token, crypt_hwid, out_sig, userid);
    }

    void rdp_lic_send_authresp(Transport * trans, uint8_t* token, uint8_t* crypt_hwid, uint8_t* signature, int userid)
    {
        int length = 58;

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        stream.out_uint8(this->lic_layer.licence_issued
                         ? LICENCE_TAG_AUTHRESP
                         : SEC_LICENCE_NEG);

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

    void rdp_lic_process_demand(Transport * trans, Stream & stream, const char * hostname, const char * username, int userid)
    {
        uint8_t null_data[SEC_MODULUS_SIZE];
        uint8_t signature[LICENCE_SIGNATURE_SIZE];
        uint8_t hwid[LICENCE_HWID_SIZE];
        uint8_t* licence_data;

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
            this->sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 65);
            this->sec_hash_48(key_block, master_secret, server_random, client_random, 65);
            /* Store first 16 bytes of session key as MAC secret */
            memcpy(this->lic_layer.licence_sign_key, key_block, 16);
            /* Generate RC4 key from next 16 bytes */
            this->sec_hash_16(this->lic_layer.licence_key, key_block + 16, client_random, server_random);
        }

        if (this->licence_size > 0) {
            /* Generate a signature for the HWID buffer */
            this->rdp_lic_generate_hwid(hwid, hostname);
            sec_sign(signature, 16, this->lic_layer.licence_sign_key, 16, hwid, sizeof(hwid));
            /* Now encrypt the HWID */
            ssllib ssl;

            SSL_RC4 crypt_key;
            ssl.rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
            ssl.rc4_crypt(crypt_key, hwid, hwid, sizeof(hwid));

            this->rdp_lic_present(trans, null_data, null_data,
                                  this->licence_data,
                                  this->licence_size,
                                  hwid, signature, userid);
        }
        else {
            this->rdp_lic_send_request(trans, null_data, null_data, hostname, username, userid);
        }
    }

    void rdp_lic_send_request(Transport * trans, uint8_t* client_random, uint8_t* rsa_data, const char * hostname, const char * username, int userid)
    {
        int userlen = strlen(username) + 1;
        int hostlen = strlen(hostname) + 1;
        int length = 128 + userlen + hostlen;

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        #warning if we are performing licence request doesn't it mean that licence has not been issued ?
        stream.out_uint8(this->lic_layer.licence_issued?LICENCE_TAG_REQUEST:SEC_LICENCE_NEG);
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
                uint8_t* signature, int userid)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        McsOut sdrq_out(stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        int length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                 licence_size + LICENCE_HWID_SIZE + LICENCE_SIGNATURE_SIZE;

        stream.out_uint8(this->lic_layer.licence_issued ?LICENCE_TAG_PRESENT:SEC_LICENCE_NEG);
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

    int rdp_lic_process_issue(Stream & stream, const char * hostname)
    {
        stream.skip_uint8(2); /* 3d 45 - unknown */
        int length = stream.in_uint16_le();
        if (!stream.check_rem(length)) {
            #warning use exception
            return 0;
        }
        ssllib ssl;
        SSL_RC4 crypt_key;
        ssl.rc4_set_key(crypt_key, this->lic_layer.licence_key, 16);
        ssl.rc4_crypt(crypt_key, stream.p, stream.p, length);
        int check = stream.in_uint16_le();
        if (check != 0) {
            #warning use exception
            return 0;
        }
        this->lic_layer.licence_issued = 1;
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


// 2.2.1.12 Server License Error PDU - Valid Client
// ================================================

// The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
// from server to client during the Licensing phase of the RDP Connection
// Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
// phases). This licensing PDU indicates that the server will not issue the
// client a license to store and that the Licensing Phase has ended
// successfully. This is one possible licensing PDU that may be sent during the
// Licensing Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible
// licensing PDUs).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU)
// which encapsulates an MCS Send Data Indication structure (SDin, choice 26
// from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure
// definitions are given in [T125] section 7, parts 7 and 10). The userData
// field of the MCS Send Data Indication contains a Security Header and a Valid
// Client License Data (section 2.2.1.12.1) structure.

// securityHeader (variable): Security header. The format of the security header
// depends on the Encryption Level and Encryption Method selected by the server
// (sections 5.3.2 and 2.2.1.4.3).

// This field MUST contain one of the following headers:
//  - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
// selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW
// (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008)
// flag.
//  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method
// selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
// ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
// (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008)
// flag.
//  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
// selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the
// embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

// If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2),
// ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field
// of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the
// licensing PDU is not encrypted), then the field MUST contain a Basic Security
// Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not
// set on the Security Exchange PDU (section 2.2.1.10).

// The flags field of the security header MUST contain the SEC_LICENSE_PKT
// (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

// validClientLicenseData (variable): The actual contents of the License Error
// (Valid Client) PDU, as specified in section 2.2.1.12.1.

    int rdp_lic_process(Transport * trans, const char * hostname, const char * username, int userid)
    {
        LOG(LOG_INFO, "rdp lic process");
        int res = 0;
        Stream stream(65535);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In in_tpdu(trans, stream);
        McsIn mcs_in(stream);
        if ((mcs_in.opcode >> 2) != MCS_SDIN) {
            throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
        }
        int len = mcs_in.len;
        SecIn sec(stream, this->decrypt);

        if (sec.flags & SEC_LICENCE_NEG) { /* 0x80 */
            uint8_t tag = stream.in_uint8();
            stream.skip_uint8(3); /* version, length */
            switch (tag) {
            case LICENCE_TAG_DEMAND:
                LOG(LOG_INFO, "LICENCE_TAG_DEMAND");
                this->rdp_lic_process_demand(trans, stream, hostname, username, userid);
                break;
            case LICENCE_TAG_AUTHREQ:
                LOG(LOG_INFO, "LICENCE_TAG_AUTHREQ");
                this->rdp_lic_process_authreq(trans, stream, hostname, userid);
                break;
            case LICENCE_TAG_ISSUE:
                LOG(LOG_INFO, "LICENCE_TAG_ISSUE");
                res = this->rdp_lic_process_issue(stream, hostname);
                break;
            case LICENCE_TAG_REISSUE:
                LOG(LOG_INFO, "LICENCE_TAG_REISSUE");
                break;
            case LICENCE_TAG_RESULT:
                LOG(LOG_INFO, "LICENCE_TAG_RESULT");
                res = 1;
                break;
            default:
                break;
                /* todo unimpl("licence tag 0x%x\n", tag); */
            }
        }
        else {
            LOG(LOG_INFO, "ERR_SEC_EXPECTED_LICENCE_NEGOTIATION_PDU");
            throw Error(ERR_SEC_EXPECTED_LICENCE_NEGOTIATION_PDU);
        }
        #warning we haven't actually read all the actual data available, hence we can't check end. Implement full decoding and activate it.
//        in_tpdu.end();
        return res;
    }


    /*****************************************************************************/
    void rdp_sec_generate_keys(uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
    {
        uint8_t pre_master_secret[48];
        uint8_t master_secret[48];
        uint8_t key_block[48];

        /* Construct pre-master secret (session key) */
        memcpy(pre_master_secret, client_random, 24);
        memcpy(pre_master_secret + 24, server_random, 24);

        /* Generate master secret and then key material */
        this->sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
        this->sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

        /* First 16 bytes of key material is MAC secret */
        memcpy(this->encrypt.sign_key, key_block, 16);

        /* Generate export keys from next two blocks of 16 bytes */
        this->sec_hash_16(this->decrypt.key, &key_block[16], client_random, server_random);
        this->sec_hash_16(this->encrypt.key, &key_block[32], client_random, server_random);

        if (rc4_key_size == 1) {
            // LOG(LOG_DEBUG, "40-bit encryption enabled\n");
            sec_make_40bit(this->encrypt.sign_key);
            sec_make_40bit(this->decrypt.key);
            sec_make_40bit(this->encrypt.key);
            this->decrypt.rc4_key_len = 8;
            this->encrypt.rc4_key_len = 8;
        }
        else {
            //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled\n", rc4_key_size);
            this->decrypt.rc4_key_len = 16;
            this->encrypt.rc4_key_len = 16;
        }

        /* Save initial RC4 keys as update keys */
        memcpy(this->decrypt.update_key, this->decrypt.key, 16);
        memcpy(this->encrypt.update_key, this->encrypt.key, 16);

        ssllib ssl;

        ssl.rc4_set_key(this->decrypt.rc4_info, this->decrypt.key, this->decrypt.rc4_key_len);
        ssl.rc4_set_key(this->encrypt.rc4_info, this->encrypt.key, this->encrypt.rc4_key_len);
    }

    /* Parse a crypto information structure */
    int rdp_sec_parse_crypt_info(Stream & stream, uint32_t *rc4_key_size,
                                  uint8_t * server_random,
                                  uint8_t* modulus, uint8_t* exponent,
                                  int & server_public_key_len,
                                  uint8_t & crypt_level)
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
        crypt_level = stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
        if (crypt_level == 0) { /* no encryption */
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
                        /* Parse a public key structure */
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
                        server_public_key_len = modulus_len;

                        if (!stream.check()){
                            throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
                        }
                    }
                    catch (...) {
                        return 0;
                    }
                    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
                    break;
                case SEC_TAG_KEYSIG:
                    LOG(LOG_DEBUG, "SEC_TAG_KEYSIG RDP4-style\n");
//                    if (!rdp_sec_parse_public_sig(stream, length, modulus, exponent, server_public_key_len)){
//                        return 0;
//                    }
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
                server_public_key = ssl_cert_to_rkey(server_cert, server_public_key_len);
                if (NULL == server_public_key){
                    LOG(LOG_DEBUG, "Didn't parse X509 correctly\n");
                    ssl_cert_free(server_cert);
                    throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

                }
                ssl_cert_free(server_cert);
                LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d\n", server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);
                if ((server_public_key_len < SEC_MODULUS_SIZE) ||
                    (server_public_key_len > SEC_MAX_MODULUS_SIZE)){
                    LOG(LOG_DEBUG, "Bad server public key size (%u bits)\n",
                        server_public_key_len * 8);
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
    void rdp_sec_process_crypt_info(Stream & stream, int & server_public_key_len, uint8_t & crypt_level)
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
        if (!this->rdp_sec_parse_crypt_info(stream, &rc4_key_size, server_random, modulus, exponent, server_public_key_len, crypt_level)){
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
        ssllib ssl;
        ssl.rsa_encrypt(this->client_crypt_random, client_random, SEC_RANDOM_SIZE, server_public_key_len, modulus, exponent);

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

    void send_security_exchange_PDU(Transport * trans, int userid)
    {
        LOG(LOG_INFO, "Iso Layer : setting encryption\n");
        /* Send the client random to the server */
        //      if (this->encryption)
        Stream sdrq_stream(8192);
        X224Out sdrq_tpdu(X224Packet::DT_TPDU, sdrq_stream);
        McsOut sdrq_out(sdrq_stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        sdrq_stream.out_uint32_le(SEC_CLIENT_RANDOM);
        sdrq_stream.out_uint32_le(this->server_public_key_len + SEC_PADDING_SIZE);
        LOG(LOG_INFO, "Server public key is %d bytes long", this->server_public_key_len);
        sdrq_stream.out_copy_bytes(this->client_crypt_random, this->server_public_key_len);
        sdrq_stream.out_clear_bytes(SEC_PADDING_SIZE);

        sdrq_out.end();
        sdrq_tpdu.end();
        sdrq_tpdu.send(trans);
    }

};



#endif
