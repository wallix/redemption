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
   Author(s): Christophe Grosjean

   sha512 headers
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

#include "utils/crypto/basic_hmac_direct.hpp"


class SslSha512_direct
{
public:
    enum : int { DIGEST_LENGTH = 64 };

    SslSha512_direct() noexcept
    {
        this->sha512_init();
    }

    void update(const uint8_t * const data, size_t data_size) noexcept
    {
        this->sha512_update(data, data_size);
    }

    void final(uint8_t * out_data) noexcept
    {
        this->sha512_sum(out_data);
    }

private:
     struct sha512 {
        uint64_t len;     /* processed message length */
        uint64_t h[8];    /* hash state */
        uint8_t buf[128]; /* message block buffer */
    } ctx;

    static uint64_t ror(uint64_t n, int k) noexcept
    {
        return (n >> k) | (n << (64-k));
    }

    static uint64_t Ch(uint64_t x,uint64_t y,uint64_t z) noexcept
    {
        return z ^ (x & (y ^ z));
    }

    static uint64_t Maj(uint64_t x,uint64_t y,uint64_t z) noexcept
    {
        return (x & y) | (z & (x | y));
    }

    static uint64_t S0(uint64_t x) noexcept
    {
        return ror(x,28) ^ ror(x,34) ^ ror(x,39);
    }

    static uint64_t S1(uint64_t x) noexcept
    {
        return ror(x,14) ^ ror(x,18) ^ ror(x,41);
    }

    static uint64_t R0(uint64_t x) noexcept
    {
        return ror(x,1) ^ ror(x,8) ^ (x>>7);
    }

    static uint64_t R1(uint64_t x) noexcept
    {
        return ror(x,19) ^ ror(x,61) ^ (x>>6);
    }


    void processblock(const uint8_t *buf) noexcept
    {
        uint64_t W[80], t1, t2, a, b, c, d, e, f, g, h;
        int i;

        static const uint64_t K[80] = {
        0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
        0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
        0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
        0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
        0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
        0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
        0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
        0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
        0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
        0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
        0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
        0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
        0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
        0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
        0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
        0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
        0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
        0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
        0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
        0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
        };

        for (i = 0; i < 16; i++) {
            W[i] = static_cast<uint64_t>(buf[8*i])<<56;
            W[i] |= static_cast<uint64_t>(buf[8*i+1])<<48;
            W[i] |= static_cast<uint64_t>(buf[8*i+2])<<40;
            W[i] |= static_cast<uint64_t>(buf[8*i+3])<<32;
            W[i] |= static_cast<uint64_t>(buf[8*i+4])<<24;
            W[i] |= static_cast<uint64_t>(buf[8*i+5])<<16;
            W[i] |= static_cast<uint64_t>(buf[8*i+6])<<8;
            W[i] |= buf[8*i+7];
        }
        for (; i < 80; i++)
            W[i] = R1(W[i-2]) + W[i-7] + R0(W[i-15]) + W[i-16];
        a = this->ctx.h[0];
        b = this->ctx.h[1];
        c = this->ctx.h[2];
        d = this->ctx.h[3];
        e = this->ctx.h[4];
        f = this->ctx.h[5];
        g = this->ctx.h[6];
        h = this->ctx.h[7];
        for (i = 0; i < 80; i++) {
            t1 = h + S1(e) + Ch(e,f,g) + K[i] + W[i];
            t2 = S0(a) + Maj(a,b,c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        this->ctx.h[0] += a;
        this->ctx.h[1] += b;
        this->ctx.h[2] += c;
        this->ctx.h[3] += d;
        this->ctx.h[4] += e;
        this->ctx.h[5] += f;
        this->ctx.h[6] += g;
        this->ctx.h[7] += h;
    }

    void pad() noexcept
    {
        unsigned r = this->ctx.len % 128;

        this->ctx.buf[r++] = 0x80;
        if (r > 112) {
            memset(this->ctx.buf + r, 0, 128 - r);
            r = 0;
            processblock(this->ctx.buf);
        }
        memset(this->ctx.buf + r, 0, 120 - r);
        this->ctx.len *= 8;
        this->ctx.buf[120] = this->ctx.len >> 56;
        this->ctx.buf[121] = this->ctx.len >> 48;
        this->ctx.buf[122] = this->ctx.len >> 40;
        this->ctx.buf[123] = this->ctx.len >> 32;
        this->ctx.buf[124] = this->ctx.len >> 24;
        this->ctx.buf[125] = this->ctx.len >> 16;
        this->ctx.buf[126] = this->ctx.len >> 8;
        this->ctx.buf[127] = this->ctx.len;
        processblock(this->ctx.buf);
    }

    void sha512_init() noexcept
    {
        this->ctx.len = 0;
        this->ctx.h[0] = 0x6a09e667f3bcc908ULL;
        this->ctx.h[1] = 0xbb67ae8584caa73bULL;
        this->ctx.h[2] = 0x3c6ef372fe94f82bULL;
        this->ctx.h[3] = 0xa54ff53a5f1d36f1ULL;
        this->ctx.h[4] = 0x510e527fade682d1ULL;
        this->ctx.h[5] = 0x9b05688c2b3e6c1fULL;
        this->ctx.h[6] = 0x1f83d9abfb41bd6bULL;
        this->ctx.h[7] = 0x5be0cd19137e2179ULL;
    }

    void sha512_sum(uint8_t *md) noexcept
    {
        int i;

        pad();
        for (i = 0; i < 8; i++) {
            md[8*i] = this->ctx.h[i] >> 56;
            md[8*i+1] = this->ctx.h[i] >> 48;
            md[8*i+2] = this->ctx.h[i] >> 40;
            md[8*i+3] = this->ctx.h[i] >> 32;
            md[8*i+4] = this->ctx.h[i] >> 24;
            md[8*i+5] = this->ctx.h[i] >> 16;
            md[8*i+6] = this->ctx.h[i] >> 8;
            md[8*i+7] = this->ctx.h[i];
        }
    }

    void sha512_update(const uint8_t *m, unsigned long len) noexcept
    {
        const uint8_t *p = m;
        unsigned r = this->ctx.len % 128;

        this->ctx.len += len;
        if (r) {
            if (len < 128 - r) {
                memcpy(this->ctx.buf + r, p, len);
                return;
            }
            memcpy(this->ctx.buf + r, p, 128 - r);
            len -= 128 - r;
            p += 128 - r;
            processblock(this->ctx.buf);
        }
        for (; len >= 128; len -= 128, p += 128)
            processblock(p);
        memcpy(this->ctx.buf, p, len);
    }
};


using SslHMAC_Sha512_direct = detail_::basic_HMAC_direct<SslSha512_direct, 128>;
