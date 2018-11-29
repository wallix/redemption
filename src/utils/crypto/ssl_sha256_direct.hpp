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
   Author(s): Christophe Grosjean, Raphaël Zhou, Meng Tan
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

#include "utils/crypto/basic_hmac_direct.hpp"


class SslSha256_direct
{
public:
    enum : int { DIGEST_LENGTH = 32 };

    SslSha256_direct() noexcept
    {
        this->sha256_init();
    }

    void update(const_bytes_view data) noexcept
    {
        this->sha256_update(data.to_u8p(), data.size());
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH]) noexcept
    {
        this->sha256_final(out_data);
    }

    void unchecked_final(uint8_t * out_data)
    {
        this->sha256_final(out_data);
    }

private:
    /* public domain sha256 implementation based on rfc1321 and libtomcrypt */

    struct sha256 {
        uint32_t datalen;          /*!< number of bytes processed  */
        uint32_t state[8];          /*!< intermediate digest state  */
        unsigned char data[64];   /*!< data block being processed */
        uint64_t bitlen;
    } ctx;

    static unsigned ROTRIGHT(unsigned a, int b) noexcept
    {
        return ((a) >> (b)) | ((a) << (32-(b)));
    }

    static unsigned CH(unsigned x,unsigned y,unsigned z) noexcept
    {
        return ((x) & (y)) ^ (~(x) & (z));
    }

    static unsigned MAJ(unsigned x,unsigned y,unsigned z) noexcept
    {
        return ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z));
    }

    static unsigned EP0(unsigned x) noexcept
    {
        return ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22);
    }

    static unsigned EP1(unsigned x) noexcept
    {
        return ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25);
    }

    static unsigned SIG0(unsigned x) noexcept
    {
        return ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ (x >> 3);
    }

    static unsigned SIG1(unsigned x) noexcept
    {
        return ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ (x >> 10);
    }

    //static uint32_t ror(uint32_t n, int k) { return (n >> k) | (n << (32-k)); }

    void processblock(const uint8_t *buf) noexcept
    {

        static const unsigned k[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

        unsigned a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

        for (i = 0, j = 0; i < 16; ++i, j += 4)
            m[i] = (buf[j] << 24) | (buf[j + 1] << 16) | (buf[j + 2] << 8) | (buf[j + 3]);
        for ( ; i < 64; ++i)
            m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

        a = this->ctx.state[0];
        b = this->ctx.state[1];
        c = this->ctx.state[2];
        d = this->ctx.state[3];
        e = this->ctx.state[4];
        f = this->ctx.state[5];
        g = this->ctx.state[6];
        h = this->ctx.state[7];

        for (i = 0; i < 64; ++i) {
            t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
            t2 = EP0(a) + MAJ(a,b,c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        this->ctx.state[0] += a;
        this->ctx.state[1] += b;
        this->ctx.state[2] += c;
        this->ctx.state[3] += d;
        this->ctx.state[4] += e;
        this->ctx.state[5] += f;
        this->ctx.state[6] += g;
        this->ctx.state[7] += h;
    }

    void sha256_init() noexcept
    {
        this->ctx.datalen = 0;
        this->ctx.bitlen = 0;
        this->ctx.state[0] = 0x6a09e667;
        this->ctx.state[1] = 0xbb67ae85;
        this->ctx.state[2] = 0x3c6ef372;
        this->ctx.state[3] = 0xa54ff53a;
        this->ctx.state[4] = 0x510e527f;
        this->ctx.state[5] = 0x9b05688c;
        this->ctx.state[6] = 0x1f83d9ab;
        this->ctx.state[7] = 0x5be0cd19;
    }

    void sha256_final(uint8_t *md) noexcept
    {
        unsigned i;

        i = this->ctx.datalen;

        // Pad whatever data is left in the buffer.
        if (this->ctx.datalen < 56) {
            this->ctx.data[i++] = 0x80;
            while (i < 56)
                this->ctx.data[i++] = 0x00;
        }
        else {
            this->ctx.data[i++] = 0x80;
            while (i < 64)
                this->ctx.data[i++] = 0x00;
            this->processblock(this->ctx.data);
            memset(this->ctx.data, 0, 56);
        }

        // Append to the padding the total message's length in bits and transform.
        this->ctx.bitlen += this->ctx.datalen * 8;
        this->ctx.data[63] = this->ctx.bitlen;
        this->ctx.data[62] = this->ctx.bitlen >> 8;
        this->ctx.data[61] = this->ctx.bitlen >> 16;
        this->ctx.data[60] = this->ctx.bitlen >> 24;
        this->ctx.data[59] = this->ctx.bitlen >> 32;
        this->ctx.data[58] = this->ctx.bitlen >> 40;
        this->ctx.data[57] = this->ctx.bitlen >> 48;
        this->ctx.data[56] = this->ctx.bitlen >> 56;
        processblock(this->ctx.data);

        // Since this implementation uses little endian byte ordering and SHA uses big endian,
        // reverse all the bytes when copying the final state to the output hash.
        for (i = 0; i < 4; ++i) {
            md[i]      = (this->ctx.state[0] >> (24 - i * 8)) & 0x000000ff;
            md[i + 4]  = (this->ctx.state[1] >> (24 - i * 8)) & 0x000000ff;
            md[i + 8]  = (this->ctx.state[2] >> (24 - i * 8)) & 0x000000ff;
            md[i + 12] = (this->ctx.state[3] >> (24 - i * 8)) & 0x000000ff;
            md[i + 16] = (this->ctx.state[4] >> (24 - i * 8)) & 0x000000ff;
            md[i + 20] = (this->ctx.state[5] >> (24 - i * 8)) & 0x000000ff;
            md[i + 24] = (this->ctx.state[6] >> (24 - i * 8)) & 0x000000ff;
            md[i + 28] = (this->ctx.state[7] >> (24 - i * 8)) & 0x000000ff;
        }
    }


    void sha256_update(const uint8_t *m, unsigned long len) noexcept
    {
        unsigned i;

        for (i = 0; i < len; ++i) {
            this->ctx.data[this->ctx.datalen] = m[i];
            this->ctx.datalen++;
            if (this->ctx.datalen == 64) {
                processblock(this->ctx.data);
                this->ctx.bitlen += 512;
                this->ctx.datalen = 0;
            }
        }
    }
};


using SslHMAC_Sha256_direct = detail_::basic_HMAC_direct<SslSha256_direct, 64>;
using SslHMAC_Sha256_Delayed_direct = detail_::DelayedHMAC_direct<SslSha256_direct, 64>;
