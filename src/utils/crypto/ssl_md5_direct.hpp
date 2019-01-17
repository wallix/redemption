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
#include <cstring>

#include "utils/crypto/basic_hmac_direct.hpp"


class SslMd5_direct
{
public:
    enum {  DIGEST_LENGTH = 16 };

    SslMd5_direct()
    {
        this->md5_init();
    }

    void update(const_bytes_view data)
    {
        this->md5_update(data.to_u8p(), data.size());
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH])
    {
        this->md5_sum(out_data);
    }

    void unchecked_final(uint8_t * out_data)
    {
        this->md5_sum(out_data);
    }

private:
   /* public domain md5 implementation based on rfc1321 and libtomcrypt */

    struct {
        uint64_t len;    /* processed message length */
        uint32_t h[4];   /* hash state */
        uint8_t buf[64]; /* message block buffer */
    } md5;

    static uint32_t rol(uint32_t n, int k) { return (n << k) | (n >> (32-k)); }
    static uint32_t F(uint32_t x, uint32_t  y, uint32_t z) { return (z ^ (x & (y ^ z))); }
    static uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (y ^ (z & (y ^ x))); }
    static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return (x ^ y ^ z); }
    static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return (y ^ (x | ~z)); }
    static uint32_t FF(uint32_t a, uint32_t b, uint32_t c, uint32_t  d, uint32_t w, uint32_t s, uint32_t t)
    {
        return rol(a + F(b,c,d) + w + t, s) + b;
    }
    static uint32_t GG(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t w, uint32_t s, uint32_t t)
    {
        return rol(a + G(b,c,d) + w + t, s) + b;
    }
    static uint32_t HH(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t w, uint32_t s, uint32_t t)
    {
        return rol(a + H(b,c,d) + w + t, s) + b;
    }
    static uint32_t II(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t w, uint32_t s, uint32_t t)
    {
        return rol(a + I(b,c,d) + w + t, s) + b;
    }

    static uint32_t UINT32_LE(const uint8_t * b)
    {
        return ( static_cast<uint32_t>( b[0])     )
             | ( static_cast<uint32_t>( b[1] <<  8) )
             | ( static_cast<uint32_t>( b[2] << 16) )
             | ( static_cast<uint32_t>( b[3] << 24) );
    }

    void processblock(const uint8_t *buf)
    {
        uint32_t W[16];

        W[ 0] = UINT32_LE(buf + 0);
        W[ 1] = UINT32_LE(buf + 4);
        W[ 2] = UINT32_LE(buf + 8);
        W[ 3] = UINT32_LE(buf + 12);
        W[ 4] = UINT32_LE(buf + 16);
        W[ 5] = UINT32_LE(buf + 20);
        W[ 6] = UINT32_LE(buf + 24);
        W[ 7] = UINT32_LE(buf + 28);
        W[ 8] = UINT32_LE(buf + 32);
        W[ 9] = UINT32_LE(buf + 36);
        W[10] = UINT32_LE(buf + 40);
        W[11] = UINT32_LE(buf + 44);
        W[12] = UINT32_LE(buf + 48);
        W[13] = UINT32_LE(buf + 52);
        W[14] = UINT32_LE(buf + 56);
        W[15] = UINT32_LE(buf + 60);

        uint32_t a = this->md5.h[0];
        uint32_t b = this->md5.h[1];
        uint32_t c = this->md5.h[2];
        uint32_t d = this->md5.h[3];

        uint32_t i = 0;
        a = FF(a,b,c,d, W[i],  7, 0xd76aa478); i++;
        d = FF(d,a,b,c, W[i], 12, 0xe8c7b756); i++;
        c = FF(c,d,a,b, W[i], 17, 0x242070db); i++;
        b = FF(b,c,d,a, W[i], 22, 0xc1bdceee); i++;
        a = FF(a,b,c,d, W[i],  7, 0xf57c0faf); i++;
        d = FF(d,a,b,c, W[i], 12, 0x4787c62a); i++;
        c = FF(c,d,a,b, W[i], 17, 0xa8304613); i++;
        b = FF(b,c,d,a, W[i], 22, 0xfd469501); i++;
        a = FF(a,b,c,d, W[i],  7, 0x698098d8); i++;
        d = FF(d,a,b,c, W[i], 12, 0x8b44f7af); i++;
        c = FF(c,d,a,b, W[i], 17, 0xffff5bb1); i++;
        b = FF(b,c,d,a, W[i], 22, 0x895cd7be); i++;
        a = FF(a,b,c,d, W[i],  7, 0x6b901122); i++;
        d = FF(d,a,b,c, W[i], 12, 0xfd987193); i++;
        c = FF(c,d,a,b, W[i], 17, 0xa679438e); i++;
        b = FF(b,c,d,a, W[i], 22, 0x49b40821); i++;

        a = GG(a,b,c,d, W[(5*i+1)%16],  5, 0xf61e2562); i++;
        d = GG(d,a,b,c, W[(5*i+1)%16],  9, 0xc040b340); i++;
        c = GG(c,d,a,b, W[(5*i+1)%16], 14, 0x265e5a51); i++;
        b = GG(b,c,d,a, W[(5*i+1)%16], 20, 0xe9b6c7aa); i++;

        a = GG(a,b,c,d, W[(5*i+1)%16],  5, 0xd62f105d); i++;
        d = GG(d,a,b,c, W[(5*i+1)%16],  9, 0x02441453); i++;
        c = GG(c,d,a,b, W[(5*i+1)%16], 14, 0xd8a1e681); i++;
        b = GG(b,c,d,a, W[(5*i+1)%16], 20, 0xe7d3fbc8); i++;

        a = GG(a,b,c,d, W[(5*i+1)%16],  5, 0x21e1cde6); i++;
        d = GG(d,a,b,c, W[(5*i+1)%16],  9, 0xc33707d6); i++;
        c = GG(c,d,a,b, W[(5*i+1)%16], 14, 0xf4d50d87); i++;
        b = GG(b,c,d,a, W[(5*i+1)%16], 20, 0x455a14ed); i++;

        a = GG(a,b,c,d, W[(5*i+1)%16],  5, 0xa9e3e905); i++;
        d = GG(d,a,b,c, W[(5*i+1)%16],  9, 0xfcefa3f8); i++;
        c = GG(c,d,a,b, W[(5*i+1)%16], 14, 0x676f02d9); i++;
        b = GG(b,c,d,a, W[(5*i+1)%16], 20, 0x8d2a4c8a); i++;

        a = HH(a,b,c,d, W[(3*i+5)%16],  4, 0xfffa3942); i++;
        d = HH(d,a,b,c, W[(3*i+5)%16], 11, 0x8771f681); i++;
        c = HH(c,d,a,b, W[(3*i+5)%16], 16, 0x6d9d6122); i++;
        b = HH(b,c,d,a, W[(3*i+5)%16], 23, 0xfde5380c); i++;

        a = HH(a,b,c,d, W[(3*i+5)%16],  4, 0xa4beea44); i++;
        d = HH(d,a,b,c, W[(3*i+5)%16], 11, 0x4bdecfa9); i++;
        c = HH(c,d,a,b, W[(3*i+5)%16], 16, 0xf6bb4b60); i++;
        b = HH(b,c,d,a, W[(3*i+5)%16], 23, 0xbebfbc70); i++;

        a = HH(a,b,c,d, W[(3*i+5)%16],  4, 0x289b7ec6); i++;
        d = HH(d,a,b,c, W[(3*i+5)%16], 11, 0xeaa127fa); i++;
        c = HH(c,d,a,b, W[(3*i+5)%16], 16, 0xd4ef3085); i++;
        b = HH(b,c,d,a, W[(3*i+5)%16], 23, 0x04881d05); i++;

        a = HH(a,b,c,d, W[(3*i+5)%16],  4, 0xd9d4d039); i++;
        d = HH(d,a,b,c, W[(3*i+5)%16], 11, 0xe6db99e5); i++;
        c = HH(c,d,a,b, W[(3*i+5)%16], 16, 0x1fa27cf8); i++;
        b = HH(b,c,d,a, W[(3*i+5)%16], 23, 0xc4ac5665); i++;

        a = II(a,b,c,d, W[7*i%16],  6, 0xf4292244); i++;
        d = II(d,a,b,c, W[7*i%16], 10, 0x432aff97); i++;
        c = II(c,d,a,b, W[7*i%16], 15, 0xab9423a7); i++;
        b = II(b,c,d,a, W[7*i%16], 21, 0xfc93a039); i++;

        a = II(a,b,c,d, W[7*i%16],  6, 0x655b59c3); i++;
        d = II(d,a,b,c, W[7*i%16], 10, 0x8f0ccc92); i++;
        c = II(c,d,a,b, W[7*i%16], 15, 0xffeff47d); i++;
        b = II(b,c,d,a, W[7*i%16], 21, 0x85845dd1); i++;

        a = II(a,b,c,d, W[7*i%16],  6, 0x6fa87e4f); i++;
        d = II(d,a,b,c, W[7*i%16], 10, 0xfe2ce6e0); i++;
        c = II(c,d,a,b, W[7*i%16], 15, 0xa3014314); i++;
        b = II(b,c,d,a, W[7*i%16], 21,  0x4e0811a1); i++;

        a = II(a,b,c,d, W[7*i%16],  6, 0xf7537e82); i++;
        d = II(d,a,b,c, W[7*i%16], 10, 0xbd3af235); i++;
        c = II(c,d,a,b, W[7*i%16], 15, 0x2ad7d2bb); i++;
        b = II(b,c,d,a, W[7*i%16], 21, 0xeb86d391); i++;

        this->md5.h[0] += a;
        this->md5.h[1] += b;
        this->md5.h[2] += c;
        this->md5.h[3] += d;
    }

    void pad()
    {
        unsigned r = this->md5.len % 64;

        this->md5.buf[r++] = 0x80;
        if (r > 56) {
            memset(this->md5.buf + r, 0, 64 - r);
            r = 0;
            this->processblock(this->md5.buf);
        }
        memset(this->md5.buf + r, 0, 56 - r);
        this->md5.len *= 8;
        this->md5.buf[56] = this->md5.len;
        this->md5.buf[57] = this->md5.len >> 8;
        this->md5.buf[58] = this->md5.len >> 16;
        this->md5.buf[59] = this->md5.len >> 24;
        this->md5.buf[60] = this->md5.len >> 32;
        this->md5.buf[61] = this->md5.len >> 40;
        this->md5.buf[62] = this->md5.len >> 48;
        this->md5.buf[63] = this->md5.len >> 56;
        this->processblock(this->md5.buf);
    }

    void md5_init()
    {
        this->md5.len = 0;
        this->md5.h[0] = 0x67452301;
        this->md5.h[1] = 0xefcdab89;
        this->md5.h[2] = 0x98badcfe;
        this->md5.h[3] = 0x10325476;
    }

    void md5_sum(uint8_t *md)
    {
        int i;

        this->pad();
        for (i = 0; i < 4; i++) {
            md[4*i] = this->md5.h[i];
            md[4*i+1] = this->md5.h[i] >> 8;
            md[4*i+2] = this->md5.h[i] >> 16;
            md[4*i+3] = this->md5.h[i] >> 24;
        }
    }

    void md5_update(const uint8_t *m, unsigned long len)
    {
        const uint8_t *p = m;
        unsigned r = this->md5.len % 64;

        this->md5.len += len;
        if (r) {
            if (len < 64 - r) {
                memcpy(this->md5.buf + r, p, len);
                return;
            }
            memcpy(this->md5.buf + r, p, 64 - r);
            len -= 64 - r;
            p += 64 - r;
            this->processblock(this->md5.buf);
        }
        for (; len >= 64; len -= 64, p += 64){
            this->processblock(p);
        }
        memcpy(this->md5.buf, p, len);
    }
};


using SslHMAC_Md5_direct = detail_::basic_HMAC_direct<SslMd5_direct, 64>;
