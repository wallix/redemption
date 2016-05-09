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

//#include <endian.h>

class SslSha1
{
    SHA_CTX sha1;

    public:
    SslSha1()
    {
        int res = 0;
        res = SHA1_Init(&this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_INIT_FAILED);
        }
    }

    void update(const uint8_t * const data,  size_t data_size)
    {
        int res = 0;
        res = SHA1_Update(&this->sha1, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(SHA_DIGEST_LENGTH == out_data_size);
        int res = 0;
        res = SHA1_Final(out_data, &this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_FINAL_FAILED);
        }
    }
};






class SslSha1_direct
{
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
    enum: int {
        LITTLE_ENDIAN = 0,
        BIG_ENDIAN    = 1
    };

    struct sha1 {
        u_int32_t state[5];
        u_int32_t count[2];
        unsigned char buffer[64];
    } sha1;

    typedef union {
        unsigned char c[64];
        u_int32_t l[16];
    } CHAR64LONG16;


    int endian = LITTLE_ENDIAN;



    u_int32_t rol(u_int32_t value, int bits) {
        return ( (value << bits) | (value >> (32 - bits)) );
    }

    u_int32_t blk0(int i, CHAR64LONG16 * block) {
        if (this->endian == LITTLE_ENDIAN) {
            return (block->l[i] = ( ((block->l[i] << 24) | (block->l[i] >> 8)) & 0xFF00FF00 ) |( ((block->l[i] << 8) | (block->l[i] >> 24)) & 0x00FF00FF ) );
        } else {
            return block->l[i];
        }
    }

    /* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
    void R0(u_int32_t v, u_int32_t & w, u_int32_t x, u_int32_t y, u_int32_t & z, int i, CHAR64LONG16 * block) {
        z += ((w&(x^y))^y) + blk0(i, block) + 0x5A827999 + ( (v << 5) | (v >> 27) );
        w = (w << 30) | (w >> 2);
    }

    void R1(u_int32_t v, u_int32_t & w, u_int32_t x, u_int32_t y, u_int32_t & z, int i, CHAR64LONG16 * block) {
        z+=((w&(x^y))^y)+(block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15],1))+0x5A827999 + ( (v << 5) | (v >> 27) );
        w = (w << 30) | (w >> 2);
    }

    void R2(u_int32_t v, u_int32_t & w, u_int32_t x, u_int32_t y, u_int32_t & z, int i, CHAR64LONG16 * block) {
        z+=(w^x^y)+(block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15],1))+0x6ED9EBA1 + ( (v << 5) | (v >> 27) );
        w = (w << 30) | (w >> 2);
    }

    void R3(u_int32_t v, u_int32_t & w, u_int32_t x, u_int32_t y, u_int32_t & z, int i, CHAR64LONG16 * block) {
        z+=(((w|x)&y)|(w&x))+(block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15],1))+0x8F1BBCDC + ( (v << 5) | (v >> 27) );
        w = (w << 30) | (w >> 2);
    }

    void R4(u_int32_t v, u_int32_t & w, u_int32_t x, u_int32_t y, u_int32_t & z, int i, CHAR64LONG16 * block) {
        z+=(w^x^y)+(block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15],1))+0xCA62C1D6 + ( (v << 5) | (v >> 27) );
        w = (w << 30) | (w >> 2);
    }


    /* Hash a single 512-bit block. This is the core of the algorithm. */

    void SHA1Transform(u_int32_t state[5], const unsigned char buffer[64])
    {

        u_int32_t a, b, c, d, e;
        CHAR64LONG16 block[1];  /* use array to appear as a pointer */

        memcpy(block, buffer, 64);


        /* Copy context->state[] to working vars */
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        /* 4 rounds of 20 operations each. Loop unrolled. */
        R0(a,b,c,d,e, 0, block); R0(e,a,b,c,d, 1, block); R0(d,e,a,b,c, 2, block); R0(c,d,e,a,b, 3, block);
        R0(b,c,d,e,a, 4, block); R0(a,b,c,d,e, 5, block); R0(e,a,b,c,d, 6, block); R0(d,e,a,b,c, 7, block);
        R0(c,d,e,a,b, 8, block); R0(b,c,d,e,a, 9, block); R0(a,b,c,d,e,10, block); R0(e,a,b,c,d,11, block);
        R0(d,e,a,b,c,12, block); R0(c,d,e,a,b,13, block); R0(b,c,d,e,a,14, block); R0(a,b,c,d,e,15, block);
        R1(e,a,b,c,d,16, block); R1(d,e,a,b,c,17, block); R1(c,d,e,a,b,18, block); R1(b,c,d,e,a,19, block);
        R2(a,b,c,d,e,20, block); R2(e,a,b,c,d,21, block); R2(d,e,a,b,c,22, block); R2(c,d,e,a,b,23, block);
        R2(b,c,d,e,a,24, block); R2(a,b,c,d,e,25, block); R2(e,a,b,c,d,26, block); R2(d,e,a,b,c,27, block);
        R2(c,d,e,a,b,28, block); R2(b,c,d,e,a,29, block); R2(a,b,c,d,e,30, block); R2(e,a,b,c,d,31, block);
        R2(d,e,a,b,c,32, block); R2(c,d,e,a,b,33, block); R2(b,c,d,e,a,34, block); R2(a,b,c,d,e,35, block);
        R2(e,a,b,c,d,36, block); R2(d,e,a,b,c,37, block); R2(c,d,e,a,b,38, block); R2(b,c,d,e,a,39, block);
        R3(a,b,c,d,e,40, block); R3(e,a,b,c,d,41, block); R3(d,e,a,b,c,42, block); R3(c,d,e,a,b,43, block);
        R3(b,c,d,e,a,44, block); R3(a,b,c,d,e,45, block); R3(e,a,b,c,d,46, block); R3(d,e,a,b,c,47, block);
        R3(c,d,e,a,b,48, block); R3(b,c,d,e,a,49, block); R3(a,b,c,d,e,50, block); R3(e,a,b,c,d,51, block);
        R3(d,e,a,b,c,52, block); R3(c,d,e,a,b,53, block); R3(b,c,d,e,a,54, block); R3(a,b,c,d,e,55, block);
        R3(e,a,b,c,d,56, block); R3(d,e,a,b,c,57, block); R3(c,d,e,a,b,58, block); R3(b,c,d,e,a,59, block);
        R4(a,b,c,d,e,60, block); R4(e,a,b,c,d,61, block); R4(d,e,a,b,c,62, block); R4(c,d,e,a,b,63, block);
        R4(b,c,d,e,a,64, block); R4(a,b,c,d,e,65, block); R4(e,a,b,c,d,66, block); R4(d,e,a,b,c,67, block);
        R4(c,d,e,a,b,68, block); R4(b,c,d,e,a,69, block); R4(a,b,c,d,e,70, block); R4(e,a,b,c,d,71, block);
        R4(d,e,a,b,c,72, block); R4(c,d,e,a,b,73, block); R4(b,c,d,e,a,74, block); R4(a,b,c,d,e,75, block);
        R4(e,a,b,c,d,76, block); R4(d,e,a,b,c,77, block); R4(c,d,e,a,b,78, block); R4(b,c,d,e,a,79, block);
        /* Add the working vars back into context.state[] */
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        /* Wipe variables */
        a = b = c = d = e = 0;
    #ifdef SHA1HANDSOFF
        memset(block, '\0', sizeof(block));
    #endif
    }


    /* SHA1Init - Initialize new context */

    void SHA1Init(struct sha1* context)
    {
        /* SHA1 initialization constants */
        context->state[0] = 0x67452301;
        context->state[1] = 0xEFCDAB89;
        context->state[2] = 0x98BADCFE;
        context->state[3] = 0x10325476;
        context->state[4] = 0xC3D2E1F0;
        context->count[0] = context->count[1] = 0;
    }


    /* Run your data through this. */

    void SHA1Update(struct sha1* context, const unsigned char* data, u_int32_t len)
    {
    u_int32_t i;
    u_int32_t j;

        j = context->count[0];
        if ((context->count[0] += len << 3) < j)
        context->count[1]++;
        context->count[1] += (len>>29);
        j = (j >> 3) & 63;
        if ((j + len) > 63) {
            memcpy(&context->buffer[j], data, (i = 64-j));
            SHA1Transform(context->state, context->buffer);
            for ( ; i + 63 < len; i += 64) {
                SHA1Transform(context->state, &data[i]);
            }
            j = 0;
        }
        else i = 0;
        memcpy(&context->buffer[j], &data[i], len - i);
    }


    /* Add padding and return the message digest. */

    void SHA1Final(unsigned char digest[20], struct sha1* context)
    {
    unsigned i;
    unsigned char finalcount[8];
    unsigned char c;


        for (i = 0; i < 8; i++) {
            finalcount[i] = static_cast<unsigned char>((context->count[(i >= 4 ? 0 : 1)]
             >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
        }

        c = 0200;
        SHA1Update(context, &c, 1);
        while ((context->count[0] & 504) != 448) {
        c = 0000;
            SHA1Update(context, &c, 1);
        }
        SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
        for (i = 0; i < 20; i++) {
            digest[i] = static_cast<unsigned char>
             ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
        }
        /* Wipe variables */
        memset(context, '\0', sizeof(*context));
        memset(&finalcount, '\0', sizeof(finalcount));
    }


    public:
    SslSha1_direct()
    {
        SslSha1_direct::SHA1Init(&this->sha1);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        SslSha1_direct::SHA1Update(&this->sha1, data, data_size);
    }

    void final(unsigned char digest[20], size_t data_size)
    {
        SslSha1_direct::SHA1Final(digest, &this->sha1);
    }
};
