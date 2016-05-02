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

/*
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
};*/



class SslMd4_direct
{
    /* public domain md4 implementation based on rfc1321 and libtomcrypt */

    struct md4 {
        uint32_t total[2];   /* number of bytes processed */
        uint32_t state[4];   /* intermediate digest state */
        unsigned char buffer[64]; /* data block being processed  */
    } md4;


    //static uint32_t rol(uint32_t n, int k) { return (n << k) | (n >> (32-k)); }

    static void GET_UINT32_LE(uint32_t & n,const uint8_t * b, int i)
    {
        n = (   static_cast<uint32_t>( b[i   ])       )
            | ( static_cast<uint32_t>( b[i + 1] <<  8) )
            | ( static_cast<uint32_t>( b[i + 2] << 16) )
            | ( static_cast<uint32_t>( b[i + 3] << 24) );
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

        GET_UINT32_LE( X[ 0], buf,  0 );
        GET_UINT32_LE( X[ 1], buf,  4 );
        GET_UINT32_LE( X[ 2], buf,  8 );
        GET_UINT32_LE( X[ 3], buf, 12 );
        GET_UINT32_LE( X[ 4], buf, 16 );
        GET_UINT32_LE( X[ 5], buf, 20 );
        GET_UINT32_LE( X[ 6], buf, 24 );
        GET_UINT32_LE( X[ 7], buf, 28 );
        GET_UINT32_LE( X[ 8], buf, 32 );
        GET_UINT32_LE( X[ 9], buf, 36 );
        GET_UINT32_LE( X[10], buf, 40 );
        GET_UINT32_LE( X[11], buf, 44 );
        GET_UINT32_LE( X[12], buf, 48 );
        GET_UINT32_LE( X[13], buf, 52 );
        GET_UINT32_LE( X[14], buf, 56 );
        GET_UINT32_LE( X[15], buf, 60 );


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

    static void MD4_init(struct md4 *s)
    {
        memset( s, 0, sizeof(md4) );

        s->total[0] = 0;
        s->total[1] = 0;

        s->state[0] = 0x67452301;
        s->state[1] = 0xEFCDAB89;
        s->state[2] = 0x98BADCFE;
        s->state[3] = 0x10325476;
    }

    static void PUT_UINT32_LE(uint32_t n, unsigned char * b, int i) {
        b[i    ] = static_cast<unsigned char> ( ( n       ) & 0xFF );
        b[i + 1] = static_cast<unsigned char> ( ( n >>  8 ) & 0xFF );
        b[i + 2] = static_cast<unsigned char> ( ( n >> 16 ) & 0xFF );
        b[i + 3] = static_cast<unsigned char> ( ( n >> 24 ) & 0xFF );
    }

    static void MD4_final(struct md4 *s, uint8_t *md)
    {
        uint32_t last, padn;
        uint32_t high, low;
        unsigned char msglen[8];

        high = ( s->total[0] >> 29 )
                | ( s->total[1] <<  3 );
        low  = ( s->total[0] <<  3 );

        PUT_UINT32_LE( low,  msglen, 0 );
        PUT_UINT32_LE( high, msglen, 4 );

        last = s->total[0] & 0x3F;
        padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

        const unsigned char md4_padding[64] =
        {
            0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };

        MD4_update( s, md4_padding, padn );
        MD4_update( s, msglen, 8 );

        PUT_UINT32_LE( s->state[0], md,  0 );
        PUT_UINT32_LE( s->state[1], md,  4 );
        PUT_UINT32_LE( s->state[2], md,  8 );
        PUT_UINT32_LE( s->state[3], md, 12 );
    }


    static void MD4_update(struct md4 *s, const uint8_t *m, unsigned long len)
    {
        size_t fill;
        uint32_t left;

        if( len == 0 )
            return;

        left = s->total[0] & 0x3F;
        fill = 64 - left;

        s->total[0] += static_cast<uint32_t> (len);
        s->total[0] &= 0xFFFFFFFF;

        if( s->total[0] < static_cast<uint32_t> (len) )
            s->total[1]++;

        if( left && len >= fill )
        {
            memcpy( static_cast<void *>(s->buffer + left),
                    (m), fill );
            processblock( s, s->buffer );
            m += fill;
            len  -= fill;
            left = 0;
        }

        while( len >= 64 )
        {
            processblock( s, m );
            m += 64;
            len  -= 64;
        }

        if( len > 0 )
        {
            memcpy( static_cast<void *> (s->buffer + left),
                    (m), len );
        }
    }

    public:
    SslMd4_direct()
    {
        SslMd4_direct::MD4_init(&this->md4);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        SslMd4_direct::MD4_update(&this->md4, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(MD4_DIGEST_LENGTH == out_data_size);
        SslMd4_direct::MD4_final(&this->md4, out_data);
    }
};

