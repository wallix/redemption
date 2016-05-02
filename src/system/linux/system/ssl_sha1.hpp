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

#include <endian.h>

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

# define SHA_LONG unsigned int
# define SHA_LBLOCK      16
# define SHA_CBLOCK      (SHA_LBLOCK*4)/* SHA treats input data as a
                                        * contiguous array of 32 bit wide
                                        * big-endian values. */

#define INIT_DATA_h0 0x67452301UL
#define INIT_DATA_h1 0xefcdab89UL
#define INIT_DATA_h2 0x98badcfeUL
#define INIT_DATA_h3 0x10325476UL
#define INIT_DATA_h4 0xc3d2e1f0UL



# define BODY_00_15(i,a,b,c,d,e,f,xi) \
        (f)=xi+(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_16_19(i,a,b,c,d,e,f,xi,xa,xb,xc,xd) \
        Xupdate(f,xi,xa,xb,xc,xd); \
        (f)+=(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_20_31(i,a,b,c,d,e,f,xi,xa,xb,xc,xd) \
        Xupdate(f,xi,xa,xb,xc,xd); \
        (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_32_39(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_40_59(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)+=(e)+K_40_59+ROTATE((a),5)+F_40_59((b),(c),(d)); \
        (b)=ROTATE((b),30);

# define BODY_60_79(i,a,b,c,d,e,f,xa,xb,xc,xd) \
        Xupdate(f,xa,xa,xb,xc,xd); \
        (f)=xa+(e)+K_60_79+ROTATE((a),5)+F_60_79((b),(c),(d)); \
        (b)=ROTATE((b),30);


# ifndef HOST_l2c
#  define HOST_l2c(l,c)   (*((c)++)=(unsigned char)(((l)>>24)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
                         *((c)++)=(unsigned char)(((l)    )&0xff),      \
                         l)
# endif       

#define HASH_MAKE_STRING(c,s)   do {    \
        unsigned long ll;               \
        ll=(c)->h0; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h1; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h2; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h3; (void)HOST_l2c(ll,(s));     \
        ll=(c)->h4; (void)HOST_l2c(ll,(s));     \
        } while (0)                        


#ifndef MD32_REG_T
# if defined(__alpha) || defined(__sparcv9) || defined(__mips)
#  define MD32_REG_T long
/*
 * This comment was originaly written for MD5, which is why it
 * discusses A-D. But it basically applies to all 32-bit digests,
 * which is why it was moved to common header file.
 *
 * In case you wonder why A-D are declared as long and not
 * as MD5_LONG. Doing so results in slight performance
 * boost on LP64 architectures. The catch is we don't
 * really care if 32 MSBs of a 64-bit register get polluted
 * with eventual overflows as we *save* only 32 LSBs in
 * *either* case. Now declaring 'em long excuses the compiler
 * from keeping 32 MSBs zeroed resulting in 13% performance
 * improvement under SPARC Solaris7/64 and 5% under AlphaLinux.
 * Well, to be honest it should say that this *prevents*
 * performance degradation.
 *                              <appro@fy.chalmers.se>
 */
# else
/*
 * Above is not absolute and there are LP64 compilers that
 * generate better code if MD32_REG_T is defined int. The above
 * pre-processor condition reflects the circumstances under which
 * the conclusion was made and is subject to further extension.
 *                              <appro@fy.chalmers.se>
 */
#  define MD32_REG_T int
# endif
#endif



class SslSha1_direct
{
    struct sha1 {
        u_int32_t state[5];
        u_int32_t count[2];
        unsigned char buffer[64];
    } sha1;

    #define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

    /* blk0() and blk() perform the initial expand. */
    /* I got the idea of expanding during the round function from SSLeay */
    #if BYTE_ORDER == LITTLE_ENDIAN
    #define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
        |(rol(block->l[i],8)&0x00FF00FF))
    #elif BYTE_ORDER == BIG_ENDIAN
    #define blk0(i) block->l[i]
    #else
    #error "Endianness not defined!"
    #endif
    #define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
        ^block->l[(i+2)&15]^block->l[i&15],1))

    /* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
    #define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
    #define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
    #define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
    #define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
    #define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


    /* Hash a single 512-bit block. This is the core of the algorithm. */

    void SHA1Transform(u_int32_t state[5], const unsigned char buffer[64])
    {
    u_int32_t a, b, c, d, e;
    typedef union {
        unsigned char c[64];
        u_int32_t l[16];
    } CHAR64LONG16;
    CHAR64LONG16 block[1];  /* use array to appear as a pointer */
        memcpy(block, buffer, 64);


        /* Copy context->state[] to working vars */
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        /* 4 rounds of 20 operations each. Loop unrolled. */
        R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
        R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
        R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
        R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
        R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
        R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
        R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
        R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
        R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
        R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
        R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
        R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
        R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
        R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
        R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
        R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
        R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
        R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
        R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
        R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
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
