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
#if 0

    struct sha1 {
        SHA_LONG h0, h1, h2, h3, h4;
        SHA_LONG Nl, Nh;
        SHA_LONG data[SHA_LBLOCK];
        unsigned int num;
    } sha1;


static void sha1_block_data_order(SHA_CTX *c, const void *p, size_t num)
{
    const unsigned char *data = p;
    register unsigned MD32_REG_T A, B, C, D, E, T, l;
#  ifndef MD32_XARRAY
    unsigned MD32_REG_T XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
        XX8, XX9, XX10, XX11, XX12, XX13, XX14, XX15;
#  else
    SHA_LONG XX[16];
#  endif

    A = c->h0;
    B = c->h1;
    C = c->h2;
    D = c->h3;
    E = c->h4;

    for (;;) {
        const union {
            long one;
            char little;
        } is_endian = {
            1
        };

        if (!is_endian.little && sizeof(SHA_LONG) == 4
            && ((size_t)p % 4) == 0) {
            const SHA_LONG *W = (const SHA_LONG *)data;

            X(0) = W[0];
            X(1) = W[1];
            BODY_00_15(0, A, B, C, D, E, T, X(0));
            X(2) = W[2];
            BODY_00_15(1, T, A, B, C, D, E, X(1));
            X(3) = W[3];
            BODY_00_15(2, E, T, A, B, C, D, X(2));
            X(4) = W[4];
            BODY_00_15(3, D, E, T, A, B, C, X(3));
            X(5) = W[5];
            BODY_00_15(4, C, D, E, T, A, B, X(4));
            X(6) = W[6];
            BODY_00_15(5, B, C, D, E, T, A, X(5));
            X(7) = W[7];
            BODY_00_15(6, A, B, C, D, E, T, X(6));
            X(8) = W[8];
            BODY_00_15(7, T, A, B, C, D, E, X(7));
            X(9) = W[9];
            BODY_00_15(8, E, T, A, B, C, D, X(8));
            X(10) = W[10];
            BODY_00_15(9, D, E, T, A, B, C, X(9));
            X(11) = W[11];
            BODY_00_15(10, C, D, E, T, A, B, X(10));
            X(12) = W[12];
            BODY_00_15(11, B, C, D, E, T, A, X(11));
            X(13) = W[13];
            BODY_00_15(12, A, B, C, D, E, T, X(12));
            X(14) = W[14];
            BODY_00_15(13, T, A, B, C, D, E, X(13));
            X(15) = W[15];
            BODY_00_15(14, E, T, A, B, C, D, X(14));
            BODY_00_15(15, D, E, T, A, B, C, X(15));

            data += SHA_CBLOCK;
        } else {
            (void)HOST_c2l(data, l);
            X(0) = l;
            (void)HOST_c2l(data, l);
            X(1) = l;
            BODY_00_15(0, A, B, C, D, E, T, X(0));
            (void)HOST_c2l(data, l);
            X(2) = l;
            BODY_00_15(1, T, A, B, C, D, E, X(1));
            (void)HOST_c2l(data, l);
            X(3) = l;
            BODY_00_15(2, E, T, A, B, C, D, X(2));
            (void)HOST_c2l(data, l);
            X(4) = l;
            BODY_00_15(3, D, E, T, A, B, C, X(3));
            (void)HOST_c2l(data, l);
            X(5) = l;
            BODY_00_15(4, C, D, E, T, A, B, X(4));
            (void)HOST_c2l(data, l);
            X(6) = l;
            BODY_00_15(5, B, C, D, E, T, A, X(5));
            (void)HOST_c2l(data, l);
            X(7) = l;
            BODY_00_15(6, A, B, C, D, E, T, X(6));
            (void)HOST_c2l(data, l);
            X(8) = l;
            BODY_00_15(7, T, A, B, C, D, E, X(7));
            (void)HOST_c2l(data, l);
            X(9) = l;
            BODY_00_15(8, E, T, A, B, C, D, X(8));
            (void)HOST_c2l(data, l);
            X(10) = l;
            BODY_00_15(9, D, E, T, A, B, C, X(9));
            (void)HOST_c2l(data, l);
            X(11) = l;
            BODY_00_15(10, C, D, E, T, A, B, X(10));
            (void)HOST_c2l(data, l);
            X(12) = l;
            BODY_00_15(11, B, C, D, E, T, A, X(11));
            (void)HOST_c2l(data, l);
            X(13) = l;
            BODY_00_15(12, A, B, C, D, E, T, X(12));
            (void)HOST_c2l(data, l);
            X(14) = l;
            BODY_00_15(13, T, A, B, C, D, E, X(13));
            (void)HOST_c2l(data, l);
            X(15) = l;
            BODY_00_15(14, E, T, A, B, C, D, X(14));
            BODY_00_15(15, D, E, T, A, B, C, X(15));
        }

        BODY_16_19(16, C, D, E, T, A, B, X(0), X(0), X(2), X(8), X(13));
        BODY_16_19(17, B, C, D, E, T, A, X(1), X(1), X(3), X(9), X(14));
        BODY_16_19(18, A, B, C, D, E, T, X(2), X(2), X(4), X(10), X(15));
        BODY_16_19(19, T, A, B, C, D, E, X(3), X(3), X(5), X(11), X(0));

        BODY_20_31(20, E, T, A, B, C, D, X(4), X(4), X(6), X(12), X(1));
        BODY_20_31(21, D, E, T, A, B, C, X(5), X(5), X(7), X(13), X(2));
        BODY_20_31(22, C, D, E, T, A, B, X(6), X(6), X(8), X(14), X(3));
        BODY_20_31(23, B, C, D, E, T, A, X(7), X(7), X(9), X(15), X(4));
        BODY_20_31(24, A, B, C, D, E, T, X(8), X(8), X(10), X(0), X(5));
        BODY_20_31(25, T, A, B, C, D, E, X(9), X(9), X(11), X(1), X(6));
        BODY_20_31(26, E, T, A, B, C, D, X(10), X(10), X(12), X(2), X(7));
        BODY_20_31(27, D, E, T, A, B, C, X(11), X(11), X(13), X(3), X(8));
        BODY_20_31(28, C, D, E, T, A, B, X(12), X(12), X(14), X(4), X(9));
        BODY_20_31(29, B, C, D, E, T, A, X(13), X(13), X(15), X(5), X(10));
        BODY_20_31(30, A, B, C, D, E, T, X(14), X(14), X(0), X(6), X(11));
        BODY_20_31(31, T, A, B, C, D, E, X(15), X(15), X(1), X(7), X(12));

        BODY_32_39(32, E, T, A, B, C, D, X(0), X(2), X(8), X(13));
        BODY_32_39(33, D, E, T, A, B, C, X(1), X(3), X(9), X(14));
        BODY_32_39(34, C, D, E, T, A, B, X(2), X(4), X(10), X(15));
        BODY_32_39(35, B, C, D, E, T, A, X(3), X(5), X(11), X(0));
        BODY_32_39(36, A, B, C, D, E, T, X(4), X(6), X(12), X(1));
        BODY_32_39(37, T, A, B, C, D, E, X(5), X(7), X(13), X(2));
        BODY_32_39(38, E, T, A, B, C, D, X(6), X(8), X(14), X(3));
        BODY_32_39(39, D, E, T, A, B, C, X(7), X(9), X(15), X(4));

        BODY_40_59(40, C, D, E, T, A, B, X(8), X(10), X(0), X(5));
        BODY_40_59(41, B, C, D, E, T, A, X(9), X(11), X(1), X(6));
        BODY_40_59(42, A, B, C, D, E, T, X(10), X(12), X(2), X(7));
        BODY_40_59(43, T, A, B, C, D, E, X(11), X(13), X(3), X(8));
        BODY_40_59(44, E, T, A, B, C, D, X(12), X(14), X(4), X(9));
        BODY_40_59(45, D, E, T, A, B, C, X(13), X(15), X(5), X(10));
        BODY_40_59(46, C, D, E, T, A, B, X(14), X(0), X(6), X(11));
        BODY_40_59(47, B, C, D, E, T, A, X(15), X(1), X(7), X(12));
        BODY_40_59(48, A, B, C, D, E, T, X(0), X(2), X(8), X(13));
        BODY_40_59(49, T, A, B, C, D, E, X(1), X(3), X(9), X(14));
        BODY_40_59(50, E, T, A, B, C, D, X(2), X(4), X(10), X(15));
        BODY_40_59(51, D, E, T, A, B, C, X(3), X(5), X(11), X(0));
        BODY_40_59(52, C, D, E, T, A, B, X(4), X(6), X(12), X(1));
        BODY_40_59(53, B, C, D, E, T, A, X(5), X(7), X(13), X(2));
        BODY_40_59(54, A, B, C, D, E, T, X(6), X(8), X(14), X(3));
        BODY_40_59(55, T, A, B, C, D, E, X(7), X(9), X(15), X(4));
        BODY_40_59(56, E, T, A, B, C, D, X(8), X(10), X(0), X(5));
        BODY_40_59(57, D, E, T, A, B, C, X(9), X(11), X(1), X(6));
        BODY_40_59(58, C, D, E, T, A, B, X(10), X(12), X(2), X(7));
        BODY_40_59(59, B, C, D, E, T, A, X(11), X(13), X(3), X(8));

        BODY_60_79(60, A, B, C, D, E, T, X(12), X(14), X(4), X(9));
        BODY_60_79(61, T, A, B, C, D, E, X(13), X(15), X(5), X(10));
        BODY_60_79(62, E, T, A, B, C, D, X(14), X(0), X(6), X(11));
        BODY_60_79(63, D, E, T, A, B, C, X(15), X(1), X(7), X(12));
        BODY_60_79(64, C, D, E, T, A, B, X(0), X(2), X(8), X(13));
        BODY_60_79(65, B, C, D, E, T, A, X(1), X(3), X(9), X(14));
        BODY_60_79(66, A, B, C, D, E, T, X(2), X(4), X(10), X(15));
        BODY_60_79(67, T, A, B, C, D, E, X(3), X(5), X(11), X(0));
        BODY_60_79(68, E, T, A, B, C, D, X(4), X(6), X(12), X(1));
        BODY_60_79(69, D, E, T, A, B, C, X(5), X(7), X(13), X(2));
        BODY_60_79(70, C, D, E, T, A, B, X(6), X(8), X(14), X(3));
        BODY_60_79(71, B, C, D, E, T, A, X(7), X(9), X(15), X(4));
        BODY_60_79(72, A, B, C, D, E, T, X(8), X(10), X(0), X(5));
        BODY_60_79(73, T, A, B, C, D, E, X(9), X(11), X(1), X(6));
        BODY_60_79(74, E, T, A, B, C, D, X(10), X(12), X(2), X(7));
        BODY_60_79(75, D, E, T, A, B, C, X(11), X(13), X(3), X(8));
        BODY_60_79(76, C, D, E, T, A, B, X(12), X(14), X(4), X(9));
        BODY_60_79(77, B, C, D, E, T, A, X(13), X(15), X(5), X(10));
        BODY_60_79(78, A, B, C, D, E, T, X(14), X(0), X(6), X(11));
        BODY_60_79(79, T, A, B, C, D, E, X(15), X(1), X(7), X(12));

        c->h0 = (c->h0 + E) & 0xffffffffL;
        c->h1 = (c->h1 + T) & 0xffffffffL;
        c->h2 = (c->h2 + A) & 0xffffffffL;
        c->h3 = (c->h3 + B) & 0xffffffffL;
        c->h4 = (c->h4 + C) & 0xffffffffL;

        if (--num == 0)
            break;

        A = c->h0;
        B = c->h1;
        C = c->h2;
        D = c->h3;
        E = c->h4;

    }
}



    static int SHA1Init(struct sha1 *c)
    {
        memset(c, 0, sizeof(*c));
        c->h0 = INIT_DATA_h0;
        c->h1 = INIT_DATA_h1;
        c->h2 = INIT_DATA_h2;
        c->h3 = INIT_DATA_h3;
        c->h4 = INIT_DATA_h4;
        return 1;
    }


    static int SHA1_Update(struct sha1 *c, const void *data_, size_t len)
    {
        const unsigned char *data = unsigned data_;
        unsigned char *p;
        SHA_LONG l;
        size_t n;

        if (len == 0)
            return 1;

        l = (c->Nl + (((SHA_LONG) len) << 3)) & 0xffffffffUL;
        /*
         * 95-05-24 eay Fixed a bug with the overflow handling, thanks to Wei Dai
         * <weidai@eskimo.com> for pointing it out.
         */
        if (l < c->Nl)              /* overflow */
            c->Nh++;
        c->Nh += (SHA_LONG) (len >> 29); /* might cause compiler warning on
                                           * 16-bit */
        c->Nl = l;

        n = c->num;
        if (n != 0) {
            p = (unsigned char *)c->data;

            if (len >= SHA_CBLOCK || len + n >= SHA_CBLOCK) {
                memcpy(p + n, data, SHA_CBLOCK - n);
                sha1_block_data_order(c, p, 1);
                n = SHA_CBLOCK - n;
                data += n;
                len -= n;
                c->num = 0;
                memset(p, 0, SHA_CBLOCK); /* keep it zeroed */
            } else {
                memcpy(p + n, data, len);
                c->num += (unsigned int)len;
                return 1;
            }
    }

    n = len / SHA_CBLOCK;
    if (n > 0) {
        sha1_block_data_order(c, data, n);
        n *= SHA_CBLOCK;
        data += n;
        len -= n;
    }

    if (len != 0) {
        p = (unsigned char *)c->data;
        c->num = (unsigned int)len;
        memcpy(p, data, len);
    }
    return 1;

    }




    static int SHA1_Final(unsigned char *md, struct sha1 *c)
    {

        unsigned char *p = (unsigned char *)c->data;
        size_t n = c->num;

        p[n] = 0x80;                /* there is always room for one */
        n++;

        if (n > (HASH_CBLOCK - 8)) {
            memset(p + n, 0, HASH_CBLOCK - n);
            n = 0;
            sha1_block_data_order(c, p, 1);
        }
        memset(p + n, 0, HASH_CBLOCK - 8 - n);

        p += HASH_CBLOCK - 8;
#if   defined(DATA_ORDER_IS_BIG_ENDIAN)
        (void)HOST_l2c(c->Nh, p);
        (void)HOST_l2c(c->Nl, p);
    #elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
        (void)HOST_l2c(c->Nl, p);
        (void)HOST_l2c(c->Nh, p);
    #endif
        p -= HASH_CBLOCK;
        sha1_block_data_order(c, p, 1);
        c->num = 0;
        memset(p, 0, HASH_CBLOCK);

#ifndef HASH_MAKE_STRING
        # error "HASH_MAKE_STRING must be defined!"
#else
        HASH_MAKE_STRING(c, md);
#endif

        return 1;
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

#endif

// #if 0

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
    #ifdef SHA1HANDSOFF
    CHAR64LONG16 block[1];  /* use array to appear as a pointer */
        memcpy(block, buffer, 64);
    #else
        /* The following had better never be used because it causes the
         * pointer-to-const buffer to be cast into a pointer to non-const.
         * And the result is written through.  I threw a "const" in, hoping
         * this will cause a diagnostic.
         */
    CHAR64LONG16* block = reinterpret_cast<const CHAR64LONG16*>(buffer);
    #endif
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
            finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
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
            digest[i] = (unsigned char)
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
    
// #endif

};

