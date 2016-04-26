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

class SslMd5
{
    MD5_CTX md5;

    public:
    SslMd5()
    {
        MD5_Init(&this->md5);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        MD5_Update(&this->md5, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        if (MD5_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD5_DIGEST_LENGTH];
            MD5_Final(tmp, &this->md5);
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        MD5_Final(out_data, &this->md5);
    }
};

static const uint32_t SslMd5_direct_tab[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};


class SslMd5_direct
{
    /* public domain md5 implementation based on rfc1321 and libtomcrypt */

    struct md5 {
	    uint64_t len;    /* processed message length */
	    uint32_t h[4];   /* hash state */
	    uint8_t buf[64]; /* message block buffer */
    } md5;

    static uint32_t rol(uint32_t n, int k) { return (n << k) | (n >> (32-k)); }
    #define F(x,y,z) (z ^ (x & (y ^ z)))
    #define G(x,y,z) (y ^ (z & (y ^ x)))
    #define H(x,y,z) (x ^ y ^ z)
    #define I(x,y,z) (y ^ (x | ~z))
    #define FF(a,b,c,d,w,s,t) a += F(b,c,d) + w + t; a = rol(a,s) + b
    #define GG(a,b,c,d,w,s,t) a += G(b,c,d) + w + t; a = rol(a,s) + b
    #define HH(a,b,c,d,w,s,t) a += H(b,c,d) + w + t; a = rol(a,s) + b
    #define II(a,b,c,d,w,s,t) a += I(b,c,d) + w + t; a = rol(a,s) + b

    static void processblock(struct md5 *s, const uint8_t *buf)
    {
	    uint32_t i, W[16], a, b, c, d;

	    for (i = 0; i < 16; i++) {
		    W[i] = buf[4*i];
		    W[i] |= (uint32_t)buf[4*i+1]<<8;
		    W[i] |= (uint32_t)buf[4*i+2]<<16;
		    W[i] |= (uint32_t)buf[4*i+3]<<24;
	    }

	    a = s->h[0];
	    b = s->h[1];
	    c = s->h[2];
	    d = s->h[3];

	    i = 0;
	    FF(a,b,c,d, W[i],  7, 0xd76aa478); i++;
	    FF(d,a,b,c, W[i], 12, 0xe8c7b756); i++;
	    FF(c,d,a,b, W[i], 17, 0x242070db); i++;
	    FF(b,c,d,a, W[i], 22, 0xc1bdceee); i++;
	    FF(a,b,c,d, W[i],  7, 0xf57c0faf); i++;
	    FF(d,a,b,c, W[i], 12, 0x4787c62a); i++;
	    FF(c,d,a,b, W[i], 17, 0xa8304613); i++;
	    FF(b,c,d,a, W[i], 22, 0xfd469501); i++;
	    FF(a,b,c,d, W[i],  7, 0x698098d8); i++;
	    FF(d,a,b,c, W[i], 12, 0x8b44f7af); i++;
	    FF(c,d,a,b, W[i], 17, 0xffff5bb1); i++;
	    FF(b,c,d,a, W[i], 22, 0x895cd7be); i++;
	    FF(a,b,c,d, W[i],  7, 0x6b901122); i++;
	    FF(d,a,b,c, W[i], 12, 0xfd987193); i++;
	    FF(c,d,a,b, W[i], 17, 0xa679438e); i++;
	    FF(b,c,d,a, W[i], 22, 0x49b40821); i++;

	    while (i < 32) {
		    GG(a,b,c,d, W[(5*i+1)%16],  5, SslMd5_direct_tab[i]); i++;
		    GG(d,a,b,c, W[(5*i+1)%16],  9, SslMd5_direct_tab[i]); i++;
		    GG(c,d,a,b, W[(5*i+1)%16], 14, SslMd5_direct_tab[i]); i++;
		    GG(b,c,d,a, W[(5*i+1)%16], 20, SslMd5_direct_tab[i]); i++;
	    }
	    while (i < 48) {
		    HH(a,b,c,d, W[(3*i+5)%16],  4, SslMd5_direct_tab[i]); i++;
		    HH(d,a,b,c, W[(3*i+5)%16], 11, SslMd5_direct_tab[i]); i++;
		    HH(c,d,a,b, W[(3*i+5)%16], 16, SslMd5_direct_tab[i]); i++;
		    HH(b,c,d,a, W[(3*i+5)%16], 23, SslMd5_direct_tab[i]); i++;
	    }
	    while (i < 64) {
		    II(a,b,c,d, W[7*i%16],  6, SslMd5_direct_tab[i]); i++;
		    II(d,a,b,c, W[7*i%16], 10, SslMd5_direct_tab[i]); i++;
		    II(c,d,a,b, W[7*i%16], 15, SslMd5_direct_tab[i]); i++;
		    II(b,c,d,a, W[7*i%16], 21, SslMd5_direct_tab[i]); i++;
	    }

	    s->h[0] += a;
	    s->h[1] += b;
	    s->h[2] += c;
	    s->h[3] += d;
    }

    static void pad(struct md5 *s)
    {
	    unsigned r = s->len % 64;

	    s->buf[r++] = 0x80;
	    if (r > 56) {
		    memset(s->buf + r, 0, 64 - r);
		    r = 0;
		    processblock(s, s->buf);
	    }
	    memset(s->buf + r, 0, 56 - r);
	    s->len *= 8;
	    s->buf[56] = s->len;
	    s->buf[57] = s->len >> 8;
	    s->buf[58] = s->len >> 16;
	    s->buf[59] = s->len >> 24;
	    s->buf[60] = s->len >> 32;
	    s->buf[61] = s->len >> 40;
	    s->buf[62] = s->len >> 48;
	    s->buf[63] = s->len >> 56;
	    processblock(s, s->buf);
    }

    static void md5_init(struct md5 *s)
    {
	    s->len = 0;
	    s->h[0] = 0x67452301;
	    s->h[1] = 0xefcdab89;
	    s->h[2] = 0x98badcfe;
	    s->h[3] = 0x10325476;
    }

    static void md5_sum(struct md5 *s, uint8_t *md)
    {
	    int i;

	    pad(s);
	    for (i = 0; i < 4; i++) {
		    md[4*i] = s->h[i];
		    md[4*i+1] = s->h[i] >> 8;
		    md[4*i+2] = s->h[i] >> 16;
		    md[4*i+3] = s->h[i] >> 24;
	    }
    }

    static void md5_update(struct md5 *s, const void *m, unsigned long len)
    {
	    const uint8_t *p = reinterpret_cast<const uint8_t*>(m);
	    unsigned r = s->len % 64;

	    s->len += len;
	    if (r) {
		    if (len < 64 - r) {
			    memcpy(s->buf + r, p, len);
			    return;
		    }
		    memcpy(s->buf + r, p, 64 - r);
		    len -= 64 - r;
		    p += 64 - r;
		    processblock(s, s->buf);
	    }
	    for (; len >= 64; len -= 64, p += 64)
		    processblock(s, p);
	    memcpy(s->buf, p, len);
    }

    public:
    SslMd5_direct()
    {
        SslMd5_direct::md5_init(&this->md5);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        SslMd5_direct::md5_update(&this->md5, data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(MD5_DIGEST_LENGTH == out_data_size);
        SslMd5_direct::md5_sum(&this->md5, out_data);
    }
};

