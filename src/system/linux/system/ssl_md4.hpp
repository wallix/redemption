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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan

   openssl headers

   Based on xrdp and rdesktop
   Copyright (C) Jay Sorg 2004-2010
   Copyright (C) Matthew Chapman 1999-2007
*/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "core/error.hpp"
#include "openssl_crypto.hpp"
#include "utils/log.hpp"
#include "utils/bitfu.hpp"

class SslMd4
{
    MD4_CTX md4;

    public:
    SslMd4()
    {
        MD4_Init(&this->md4);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        MD4_Update(&this->md4, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        if (MD4_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD4_DIGEST_LENGTH];
            MD4_Final(tmp, &this->md4);
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        MD4_Final(out_data, &this->md4);
    }
};


class SslMd4_direct
{
    struct md4 {
        uint32_t total[2];
        uint32_t state[4];
        unsigned char buffer[64];
        md4()
            : total{}
            , state{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476}
            , buffer{}
        {}
    } md4;

    static uint32_t UINT32_LE(const uint8_t * b)
    {
        return ( static_cast<uint32_t>( b[0])     )
             | ( static_cast<uint32_t>( b[1] <<  8) )
             | ( static_cast<uint32_t>( b[2] << 16) )
             | ( static_cast<uint32_t>( b[3] << 24) );
    }

    static void P_1(uint32_t & a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s) {
        a += ( (b & c) | ((~b) & d) ) + x;
        a = (a << s) | ((a & 0xFFFFFFFF) >> (32 - s));
    }

    static void P_2(uint32_t & a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s) {
        a += ( (b & c) | (b & d) | (c & d) ) + x + 0x5A827999;
        a = (a << s) | ((a & 0xFFFFFFFF) >> (32 - s));
    }

    static void P_3(uint32_t & a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s) {
        a += ( (b ^ c ^ d) ) + x + 0x6ED9EBA1;
        a = (a << s) | ((a & 0xFFFFFFFF) >> (32 - s));
    }

    static void processblock(struct md4 *s, const uint8_t *buf)
    {
        uint32_t X[16], A, B, C, D;

        X[ 0] = UINT32_LE(buf +  0 );
        X[ 1] = UINT32_LE(buf +  4 );
        X[ 2] = UINT32_LE(buf +  8 );
        X[ 3] = UINT32_LE(buf + 12 );
        X[ 4] = UINT32_LE(buf + 16 );
        X[ 5] = UINT32_LE(buf + 20 );
        X[ 6] = UINT32_LE(buf + 24 );
        X[ 7] = UINT32_LE(buf + 28 );
        X[ 8] = UINT32_LE(buf + 32 );
        X[ 9] = UINT32_LE(buf + 36 );
        X[10] = UINT32_LE(buf + 40 );
        X[11] = UINT32_LE(buf + 44 );
        X[12] = UINT32_LE(buf + 48 );
        X[13] = UINT32_LE(buf + 52 );
        X[14] = UINT32_LE(buf + 56 );
        X[15] = UINT32_LE(buf + 60 );

        A = s->state[0];
        B = s->state[1];
        C = s->state[2];
        D = s->state[3];


        P_1( A, B, C, D, X[ 0],  3 );
        P_1( D, A, B, C, X[ 1],  7 );
        P_1( C, D, A, B, X[ 2], 11 );
        P_1( B, C, D, A, X[ 3], 19 );
        P_1( A, B, C, D, X[ 4],  3 );
        P_1( D, A, B, C, X[ 5],  7 );
        P_1( C, D, A, B, X[ 6], 11 );
        P_1( B, C, D, A, X[ 7], 19 );
        P_1( A, B, C, D, X[ 8],  3 );
        P_1( D, A, B, C, X[ 9],  7 );
        P_1( C, D, A, B, X[10], 11 );
        P_1( B, C, D, A, X[11], 19 );
        P_1( A, B, C, D, X[12],  3 );
        P_1( D, A, B, C, X[13],  7 );
        P_1( C, D, A, B, X[14], 11 );
        P_1( B, C, D, A, X[15], 19 );


        P_2( A, B, C, D, X[ 0],  3 );
        P_2( D, A, B, C, X[ 4],  5 );
        P_2( C, D, A, B, X[ 8],  9 );
        P_2( B, C, D, A, X[12], 13 );
        P_2( A, B, C, D, X[ 1],  3 );
        P_2( D, A, B, C, X[ 5],  5 );
        P_2( C, D, A, B, X[ 9],  9 );
        P_2( B, C, D, A, X[13], 13 );
        P_2( A, B, C, D, X[ 2],  3 );
        P_2( D, A, B, C, X[ 6],  5 );
        P_2( C, D, A, B, X[10],  9 );
        P_2( B, C, D, A, X[14], 13 );
        P_2( A, B, C, D, X[ 3],  3 );
        P_2( D, A, B, C, X[ 7],  5 );
        P_2( C, D, A, B, X[11],  9 );
        P_2( B, C, D, A, X[15], 13 );


        P_3( A, B, C, D, X[ 0],  3 );
        P_3( D, A, B, C, X[ 8],  9 );
        P_3( C, D, A, B, X[ 4], 11 );
        P_3( B, C, D, A, X[12], 15 );
        P_3( A, B, C, D, X[ 2],  3 );
        P_3( D, A, B, C, X[10],  9 );
        P_3( C, D, A, B, X[ 6], 11 );
        P_3( B, C, D, A, X[14], 15 );
        P_3( A, B, C, D, X[ 1],  3 );
        P_3( D, A, B, C, X[ 9],  9 );
        P_3( C, D, A, B, X[ 5], 11 );
        P_3( B, C, D, A, X[13], 15 );
        P_3( A, B, C, D, X[ 3],  3 );
        P_3( D, A, B, C, X[11],  9 );
        P_3( C, D, A, B, X[ 7], 11 );
        P_3( B, C, D, A, X[15], 15 );

        s->state[0] += A;
        s->state[1] += B;
        s->state[2] += C;
        s->state[3] += D;
    }

    void MD4_init()
    {
    }

    static void PUT_UINT32_LE(uint32_t n, unsigned char * b, int i) {
        b[i    ] = static_cast<unsigned char> ( ( n       ) & 0xFF );
        b[i + 1] = static_cast<unsigned char> ( ( n >>  8 ) & 0xFF );
        b[i + 2] = static_cast<unsigned char> ( ( n >> 16 ) & 0xFF );
        b[i + 3] = static_cast<unsigned char> ( ( n >> 24 ) & 0xFF );
    }

    void MD4_final(uint8_t *md)
    {
        uint32_t last, padn;
        uint32_t high, low;
        unsigned char msglen[8];

        high = ( this->md4.total[0] >> 29 )
                | ( this->md4.total[1] <<  3 );
        low  = ( this->md4.total[0] <<  3 );

        PUT_UINT32_LE( low,  msglen, 0 );
        PUT_UINT32_LE( high, msglen, 4 );

        last = this->md4.total[0] & 0x3F;
        padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

        const unsigned char md4_padding[64] =
        {
            0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };

        this->MD4_update(md4_padding, padn );
        this->MD4_update(msglen, 8 );

        PUT_UINT32_LE(this->md4.state[0], md,  0 );
        PUT_UINT32_LE(this->md4.state[1], md,  4 );
        PUT_UINT32_LE(this->md4.state[2], md,  8 );
        PUT_UINT32_LE(this->md4.state[3], md, 12 );
    }


    void MD4_update(const uint8_t *m, unsigned long len)
    {
        size_t fill;
        uint32_t left;

        if( len == 0 )
            return;

        left = this->md4.total[0] & 0x3F;
        fill = 64 - left;

        this->md4.total[0] += static_cast<uint32_t> (len);
        this->md4.total[0] &= 0xFFFFFFFF;

        if( this->md4.total[0] < static_cast<uint32_t> (len) )
            this->md4.total[1]++;

        if( left && len >= fill )
        {
            memcpy( static_cast<void *>(this->md4.buffer + left),
                    (m), fill );
            processblock(&this->md4, this->md4.buffer );
            m += fill;
            len  -= fill;
            left = 0;
        }

        while( len >= 64 )
        {
            processblock(&this->md4, m );
            m += 64;
            len  -= 64;
        }

        if( len > 0 )
        {
            memcpy(static_cast<void *> (this->md4.buffer + left), m, len);
        }
    }

    public:
    SslMd4_direct()
    {
        this->MD4_init();
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        this->MD4_update(data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(MD4_DIGEST_LENGTH == out_data_size);
        this->MD4_final(out_data);
    }
};


// the HMAC_MD4 transform looks like:
// MD4(K XOR opad, MD4(K XOR ipad, text))
// where K is an n byte key
// ipad is the byte 0x36 repeated 64 times
// opad is the byte 0x5c repeated 64 times
// and text is the data being protected

class SslHMAC_Md4_direct
{
    uint8_t k_ipad[64]; 
    uint8_t k_opad[64]; 
    SslMd4_direct context;

    public:
    SslHMAC_Md4_direct(const uint8_t * const key, size_t key_len)
        : k_ipad{
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
         },
         k_opad{
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
            0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
         }
    {
         const uint8_t * k = key;
         if (key_len > 64) {
             unsigned char digest[MD4_DIGEST_LENGTH];
             SslMd4_direct md4;
             md4.update(digest, MD4_DIGEST_LENGTH);
             md4.final(digest, MD4_DIGEST_LENGTH);
             key_len = MD4_DIGEST_LENGTH;
             k = key;
         }
         size_t i;
         for (i = 0; i < key_len; i++){
            k_ipad[i] ^= k[i];
            k_opad[i] ^= k[i];
         }
         context.update(k_ipad, 64);
    }

    ~SslHMAC_Md4_direct()
    {
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        context.update(data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(MD4_DIGEST_LENGTH == out_data_size);
        context.final(out_data, MD4_DIGEST_LENGTH);

        SslMd4_direct md4;
        md4.update(this->k_opad, 64);
        md4.update(out_data, MD4_DIGEST_LENGTH);
        md4.final(out_data, MD4_DIGEST_LENGTH);
    }
};


class SslHMAC_Md4
{
    HMAC_CTX hmac;

    public:
    SslHMAC_Md4(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_md4(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC_Md4()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (MD4_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD4_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};

