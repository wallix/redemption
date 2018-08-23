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
   Copyright (C) Wallix 2010-2016
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
*/

#pragma once

#include <cstdint>
#include <cstring>

#include "utils/crypto/basic_hmac_direct.hpp"


class SslMd4_direct
{
public:
    enum {  DIGEST_LENGTH = 16 };

    SslMd4_direct()
    {
        this->MD4_init();
    }

    void update(const_bytes_view data)
    {
        this->MD4_update(data.data(), data.size());
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH])
    {
        this->MD4_final(out_data);
    }

private:
    struct {
        uint32_t total[2] {};
        uint32_t state[4] {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
        unsigned char buffer[64] {};
    } ctx;


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

    void processblock(const uint8_t *buf)
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

        A = this->ctx.state[0];
        B = this->ctx.state[1];
        C = this->ctx.state[2];
        D = this->ctx.state[3];

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

        this->ctx.state[0] += A;
        this->ctx.state[1] += B;
        this->ctx.state[2] += C;
        this->ctx.state[3] += D;
    }

    void MD4_init()
    {
    }

    static void PUT_UINT32_LE(uint32_t n, unsigned char * b) {
        b[0] = static_cast<unsigned char> ( ( n       ) & 0xFF );
        b[1] = static_cast<unsigned char> ( ( n >>  8 ) & 0xFF );
        b[2] = static_cast<unsigned char> ( ( n >> 16 ) & 0xFF );
        b[3] = static_cast<unsigned char> ( ( n >> 24 ) & 0xFF );
    }

    void MD4_final(uint8_t *md)
    {
        uint32_t last, padn;
        uint32_t high, low;
        unsigned char msglen[8];

        high = ( this->ctx.total[0] >> 29 )
                | ( this->ctx.total[1] <<  3 );
        low  = ( this->ctx.total[0] <<  3 );

        PUT_UINT32_LE( low,  msglen + 0 );
        PUT_UINT32_LE( high, msglen + 4 );

        last = this->ctx.total[0] & 0x3F;
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

        PUT_UINT32_LE(this->ctx.state[0], md +  0 );
        PUT_UINT32_LE(this->ctx.state[1], md +  4 );
        PUT_UINT32_LE(this->ctx.state[2], md +  8 );
        PUT_UINT32_LE(this->ctx.state[3], md + 12 );
    }


    void MD4_update(const uint8_t *m, unsigned long len)
    {
        size_t fill;
        uint32_t left;

        if( len == 0 )
            return;

        left = this->ctx.total[0] & 0x3F;
        fill = 64 - left;

        this->ctx.total[0] += static_cast<uint32_t> (len);
        this->ctx.total[0] &= 0xFFFFFFFF;

        if( this->ctx.total[0] < static_cast<uint32_t> (len) )
            this->ctx.total[1]++;

        if( left && len >= fill )
        {
            memcpy( static_cast<void *>(this->ctx.buffer + left),
                    (m), fill );
            processblock(this->ctx.buffer );
            m += fill;
            len  -= fill;
            left = 0;
        }

        while( len >= 64 )
        {
            processblock(m );
            m += 64;
            len  -= 64;
        }

        if( len > 0 )
        {
            memcpy(static_cast<void *> (this->ctx.buffer + left), m, len);
        }
    }
};


using SslHMAC_Md4_direct = detail_::basic_HMAC_direct<SslMd4_direct, 64>;
