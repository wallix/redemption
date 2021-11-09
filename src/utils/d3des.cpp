/*
 * This is D3DES (V5.09) by Richard Outerbridge with the double and
 * triple-length support removed for use in VNC.  Also the bytebit[] array
 * has been reversed so that the most significant bit in each byte of the
 * key is ignored, not the least significant.
 *
 * These changes are:
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/* D3DES (V5.09) -
 *
 * A portable, public domain, version of the Data Encryption Standard.
 *
 * Written with Symantec's THINK (Lightspeed) C by Richard Outerbridge.
 * Thanks to: Dan Hoey for his excellent Initial and Inverse permutation
 * code;  Jim Gillogly & Phil Karn for the DES key schedule code; Dennis
 * Ferguson, Eric Young and Dana How for comparing notes; and Ray Lau,
 * for humouring me on.
 *
 * Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * (GEnie : OUTER; CIS : [71755,204]) Graven Imagery, 1992.
 */

#include "utils/d3des.hpp"

static void scrunch(uint8_t const * outof, uint32_t * into) noexcept;
static void unscrun(uint32_t const * outof, uint8_t * into) noexcept;
static void desfunc(uint32_t * block, uint32_t const * keys) noexcept;

constexpr unsigned short bytebit[8]    = {
    01, 02, 04, 010, 020, 040, 0100, 0200
};

constexpr uint32_t bigbyte[24] = {
    0x800000u,  0x400000u,  0x200000u,  0x100000u,
    0x80000u,   0x40000u,   0x20000u,   0x10000u,
    0x8000u,    0x4000u,    0x2000u,    0x1000u,
    0x800u,     0x400u,     0x200u,     0x100u,
    0x80u,      0x40u,      0x20u,      0x10u,
    0x8u,       0x4u,       0x2u,       0x1u,
};

/* Use the key schedule specified in the Standard (ANSI X3.92-1981). */

constexpr uint8_t pc1[56] = {
    56, 48, 40, 32, 24, 16,  8,     0, 57, 49, 41, 33, 25, 17,
    9,   1, 58, 50, 42, 34, 26,    18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,     6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,    20, 12,  4, 27, 19, 11,  3,
};

constexpr uint8_t totrot[16] = {
    1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

constexpr uint8_t pc2[48] = {
    13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31,
};

/* Thanks to James Gillogly & Phil Karn! */
RfbD3DesEncrypter::RfbD3DesEncrypter(sized_u8_array_view<8> key) noexcept
{
    uint8_t pc1m[56];

    for (unsigned j = 0; j < 56; ++j) {
        auto l = pc1[j];
        auto m = l & 07;
        pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
    }

    uint32_t kn[32]{};

    for (int i = 0; i < 16; ++i) {
        auto m = /*(edf == DE1) ? ((15 - i) << 1) : */(i << 1);
        auto n = m + 1;

        uint8_t pcr[56];

        for (int j = 0; j < 28; ++j) {
            auto l = j + totrot[i];
            pcr[j] = (l < 28) ? pc1m[l] : pc1m[l - 28];
        }

        for (int j = 28; j < 56; ++j) {
            auto l = j + totrot[i];
            pcr[j] = (l < 56) ? pc1m[l] : pc1m[l - 28];
        }

        for (int j = 0; j < 24; ++j) {
            if ( pcr[pc2[j]] ) { kn[m] |= bigbyte[j]; }
            if ( pcr[pc2[j+24]] ) { kn[n] |= bigbyte[j]; }
        }
    }

    // cookey
    uint32_t * raw1 = kn;
    uint32_t * cook = KnL;
    for (int i = 0; i < 16; ++i, ++raw1) {
        uint32_t * raw0 = raw1++;
        *cook    = (*raw0 & 0x00fc0000u) << 6;
        *cook   |= (*raw0 & 0x00000fc0u) << 10;
        *cook   |= (*raw1 & 0x00fc0000u) >> 10;
        *cook++ |= (*raw1 & 0x00000fc0u) >> 6;
        *cook    = (*raw0 & 0x0003f000u) << 12;
        *cook   |= (*raw0 & 0x0000003fu) << 16;
        *cook   |= (*raw1 & 0x0003f000u) >> 4;
        *cook++ |= (*raw1 & 0x0000003fu);
    }
}

static void d3des_encrypt_decrypt_block8(uint8_t const* from, uint8_t* to, uint32_t const (&kn)[32]) noexcept
{
    uint32_t work[2];

    scrunch(from, work);
    desfunc(work, kn);
    unscrun(work, to);
}

void RfbD3DesEncrypter::encrypt_block(sized_u8_array_view<8> from, sized_writable_u8_array_view<8> to) const noexcept
{
    d3des_encrypt_decrypt_block8(from.data(), to.data(), KnL);
}

void RfbD3DesEncrypter::encrypt_text_impl(uint8_t const* from, uint8_t* to, std::size_t length, sized_u8_array_view<8> iv) const noexcept
{
    uint8_t inblock8[8];

    auto init_block8 = [&](uint8_t const* in, uint8_t const* k){
        for (auto& x : inblock8) {
            x = *in++ ^ *k++;
        }
    };

    init_block8(from, iv.data());
    d3des_encrypt_decrypt_block8(inblock8, to, KnL);

    for (unsigned int i = 8; i < length; i += 8) {
        init_block8(from + i, to + i - 8);
        d3des_encrypt_decrypt_block8(inblock8, to + i, KnL);
    }
}

static void scrunch(uint8_t const *outof, uint32_t *into) noexcept
{
    *into    = (*outof++ & 0xffu) << 24;
    *into   |= (*outof++ & 0xffu) << 16;
    *into   |= (*outof++ & 0xffu) << 8;
    *into++ |= (*outof++ & 0xffu);
    *into    = (*outof++ & 0xffu) << 24;
    *into   |= (*outof++ & 0xffu) << 16;
    *into   |= (*outof++ & 0xffu) << 8;
    *into   |= (*outof   & 0xffu);
}

static void unscrun(uint32_t const *outof, uint8_t *into) noexcept
{
    *into++ = ((*outof >> 24) & 0xffu);
    *into++ = ((*outof >> 16) & 0xffu);
    *into++ = ((*outof >>  8) & 0xffu);
    *into++ = ( *outof++      & 0xffu);
    *into++ = ((*outof >> 24) & 0xffu);
    *into++ = ((*outof >> 16) & 0xffu);
    *into++ = ((*outof >>  8) & 0xffu);
    *into   = ( *outof        & 0xffu);
}

constexpr uint32_t SP1[64] = {
    0x01010400u, 0x00000000u, 0x00010000u, 0x01010404u,
    0x01010004u, 0x00010404u, 0x00000004u, 0x00010000u,
    0x00000400u, 0x01010400u, 0x01010404u, 0x00000400u,
    0x01000404u, 0x01010004u, 0x01000000u, 0x00000004u,
    0x00000404u, 0x01000400u, 0x01000400u, 0x00010400u,
    0x00010400u, 0x01010000u, 0x01010000u, 0x01000404u,
    0x00010004u, 0x01000004u, 0x01000004u, 0x00010004u,
    0x00000000u, 0x00000404u, 0x00010404u, 0x01000000u,
    0x00010000u, 0x01010404u, 0x00000004u, 0x01010000u,
    0x01010400u, 0x01000000u, 0x01000000u, 0x00000400u,
    0x01010004u, 0x00010000u, 0x00010400u, 0x01000004u,
    0x00000400u, 0x00000004u, 0x01000404u, 0x00010404u,
    0x01010404u, 0x00010004u, 0x01010000u, 0x01000404u,
    0x01000004u, 0x00000404u, 0x00010404u, 0x01010400u,
    0x00000404u, 0x01000400u, 0x01000400u, 0x00000000u,
    0x00010004u, 0x00010400u, 0x00000000u, 0x01010004u,
};

constexpr uint32_t SP2[64] = {
    0x80108020u, 0x80008000u, 0x00008000u, 0x00108020u,
    0x00100000u, 0x00000020u, 0x80100020u, 0x80008020u,
    0x80000020u, 0x80108020u, 0x80108000u, 0x80000000u,
    0x80008000u, 0x00100000u, 0x00000020u, 0x80100020u,
    0x00108000u, 0x00100020u, 0x80008020u, 0x00000000u,
    0x80000000u, 0x00008000u, 0x00108020u, 0x80100000u,
    0x00100020u, 0x80000020u, 0x00000000u, 0x00108000u,
    0x00008020u, 0x80108000u, 0x80100000u, 0x00008020u,
    0x00000000u, 0x00108020u, 0x80100020u, 0x00100000u,
    0x80008020u, 0x80100000u, 0x80108000u, 0x00008000u,
    0x80100000u, 0x80008000u, 0x00000020u, 0x80108020u,
    0x00108020u, 0x00000020u, 0x00008000u, 0x80000000u,
    0x00008020u, 0x80108000u, 0x00100000u, 0x80000020u,
    0x00100020u, 0x80008020u, 0x80000020u, 0x00100020u,
    0x00108000u, 0x00000000u, 0x80008000u, 0x00008020u,
    0x80000000u, 0x80100020u, 0x80108020u, 0x00108000u,
};

constexpr uint32_t SP3[64] = {
    0x00000208u, 0x08020200u, 0x00000000u, 0x08020008u,
    0x08000200u, 0x00000000u, 0x00020208u, 0x08000200u,
    0x00020008u, 0x08000008u, 0x08000008u, 0x00020000u,
    0x08020208u, 0x00020008u, 0x08020000u, 0x00000208u,
    0x08000000u, 0x00000008u, 0x08020200u, 0x00000200u,
    0x00020200u, 0x08020000u, 0x08020008u, 0x00020208u,
    0x08000208u, 0x00020200u, 0x00020000u, 0x08000208u,
    0x00000008u, 0x08020208u, 0x00000200u, 0x08000000u,
    0x08020200u, 0x08000000u, 0x00020008u, 0x00000208u,
    0x00020000u, 0x08020200u, 0x08000200u, 0x00000000u,
    0x00000200u, 0x00020008u, 0x08020208u, 0x08000200u,
    0x08000008u, 0x00000200u, 0x00000000u, 0x08020008u,
    0x08000208u, 0x00020000u, 0x08000000u, 0x08020208u,
    0x00000008u, 0x00020208u, 0x00020200u, 0x08000008u,
    0x08020000u, 0x08000208u, 0x00000208u, 0x08020000u,
    0x00020208u, 0x00000008u, 0x08020008u, 0x00020200u,
};

constexpr uint32_t SP4[64] = {
    0x00802001u, 0x00002081u, 0x00002081u, 0x00000080u,
    0x00802080u, 0x00800081u, 0x00800001u, 0x00002001u,
    0x00000000u, 0x00802000u, 0x00802000u, 0x00802081u,
    0x00000081u, 0x00000000u, 0x00800080u, 0x00800001u,
    0x00000001u, 0x00002000u, 0x00800000u, 0x00802001u,
    0x00000080u, 0x00800000u, 0x00002001u, 0x00002080u,
    0x00800081u, 0x00000001u, 0x00002080u, 0x00800080u,
    0x00002000u, 0x00802080u, 0x00802081u, 0x00000081u,
    0x00800080u, 0x00800001u, 0x00802000u, 0x00802081u,
    0x00000081u, 0x00000000u, 0x00000000u, 0x00802000u,
    0x00002080u, 0x00800080u, 0x00800081u, 0x00000001u,
    0x00802001u, 0x00002081u, 0x00002081u, 0x00000080u,
    0x00802081u, 0x00000081u, 0x00000001u, 0x00002000u,
    0x00800001u, 0x00002001u, 0x00802080u, 0x00800081u,
    0x00002001u, 0x00002080u, 0x00800000u, 0x00802001u,
    0x00000080u, 0x00800000u, 0x00002000u, 0x00802080u,
};

constexpr uint32_t SP5[64] = {
    0x00000100u, 0x02080100u, 0x02080000u, 0x42000100u,
    0x00080000u, 0x00000100u, 0x40000000u, 0x02080000u,
    0x40080100u, 0x00080000u, 0x02000100u, 0x40080100u,
    0x42000100u, 0x42080000u, 0x00080100u, 0x40000000u,
    0x02000000u, 0x40080000u, 0x40080000u, 0x00000000u,
    0x40000100u, 0x42080100u, 0x42080100u, 0x02000100u,
    0x42080000u, 0x40000100u, 0x00000000u, 0x42000000u,
    0x02080100u, 0x02000000u, 0x42000000u, 0x00080100u,
    0x00080000u, 0x42000100u, 0x00000100u, 0x02000000u,
    0x40000000u, 0x02080000u, 0x42000100u, 0x40080100u,
    0x02000100u, 0x40000000u, 0x42080000u, 0x02080100u,
    0x40080100u, 0x00000100u, 0x02000000u, 0x42080000u,
    0x42080100u, 0x00080100u, 0x42000000u, 0x42080100u,
    0x02080000u, 0x00000000u, 0x40080000u, 0x42000000u,
    0x00080100u, 0x02000100u, 0x40000100u, 0x00080000u,
    0x00000000u, 0x40080000u, 0x02080100u, 0x40000100u,
};

constexpr uint32_t SP6[64] = {
    0x20000010u, 0x20400000u, 0x00004000u, 0x20404010u,
    0x20400000u, 0x00000010u, 0x20404010u, 0x00400000u,
    0x20004000u, 0x00404010u, 0x00400000u, 0x20000010u,
    0x00400010u, 0x20004000u, 0x20000000u, 0x00004010u,
    0x00000000u, 0x00400010u, 0x20004010u, 0x00004000u,
    0x00404000u, 0x20004010u, 0x00000010u, 0x20400010u,
    0x20400010u, 0x00000000u, 0x00404010u, 0x20404000u,
    0x00004010u, 0x00404000u, 0x20404000u, 0x20000000u,
    0x20004000u, 0x00000010u, 0x20400010u, 0x00404000u,
    0x20404010u, 0x00400000u, 0x00004010u, 0x20000010u,
    0x00400000u, 0x20004000u, 0x20000000u, 0x00004010u,
    0x20000010u, 0x20404010u, 0x00404000u, 0x20400000u,
    0x00404010u, 0x20404000u, 0x00000000u, 0x20400010u,
    0x00000010u, 0x00004000u, 0x20400000u, 0x00404010u,
    0x00004000u, 0x00400010u, 0x20004010u, 0x00000000u,
    0x20404000u, 0x20000000u, 0x00400010u, 0x20004010u,
};

constexpr uint32_t SP7[64] = {
    0x00200000u, 0x04200002u, 0x04000802u, 0x00000000u,
    0x00000800u, 0x04000802u, 0x00200802u, 0x04200800u,
    0x04200802u, 0x00200000u, 0x00000000u, 0x04000002u,
    0x00000002u, 0x04000000u, 0x04200002u, 0x00000802u,
    0x04000800u, 0x00200802u, 0x00200002u, 0x04000800u,
    0x04000002u, 0x04200000u, 0x04200800u, 0x00200002u,
    0x04200000u, 0x00000800u, 0x00000802u, 0x04200802u,
    0x00200800u, 0x00000002u, 0x04000000u, 0x00200800u,
    0x04000000u, 0x00200800u, 0x00200000u, 0x04000802u,
    0x04000802u, 0x04200002u, 0x04200002u, 0x00000002u,
    0x00200002u, 0x04000000u, 0x04000800u, 0x00200000u,
    0x04200800u, 0x00000802u, 0x00200802u, 0x04200800u,
    0x00000802u, 0x04000002u, 0x04200802u, 0x04200000u,
    0x00200800u, 0x00000000u, 0x00000002u, 0x04200802u,
    0x00000000u, 0x00200802u, 0x04200000u, 0x00000800u,
    0x04000002u, 0x04000800u, 0x00000800u, 0x00200002u,
};

constexpr uint32_t SP8[64] = {
    0x10001040u, 0x00001000u, 0x00040000u, 0x10041040u,
    0x10000000u, 0x10001040u, 0x00000040u, 0x10000000u,
    0x00040040u, 0x10040000u, 0x10041040u, 0x00041000u,
    0x10041000u, 0x00041040u, 0x00001000u, 0x00000040u,
    0x10040000u, 0x10000040u, 0x10001000u, 0x00001040u,
    0x00041000u, 0x00040040u, 0x10040040u, 0x10041000u,
    0x00001040u, 0x00000000u, 0x00000000u, 0x10040040u,
    0x10000040u, 0x10001000u, 0x00041040u, 0x00040000u,
    0x00041040u, 0x00040000u, 0x10041000u, 0x00001000u,
    0x00000040u, 0x10040040u, 0x00001000u, 0x00041040u,
    0x10001000u, 0x00000040u, 0x10000040u, 0x10040000u,
    0x10040040u, 0x10000000u, 0x00040000u, 0x10001040u,
    0x00000000u, 0x10041040u, 0x00040040u, 0x10000040u,
    0x10040000u, 0x10001000u, 0x10001040u, 0x00000000u,
    0x10041040u, 0x00041000u, 0x00041000u, 0x00001040u,
    0x00001040u, 0x00040040u, 0x10000000u, 0x10041000u,
};

static void desfunc(uint32_t* block, uint32_t const *keys) noexcept
{
    uint32_t work;
    uint32_t right;
    uint32_t leftt;

    leftt = block[0];
    right = block[1];
    work = ((leftt >> 4) ^ right) & 0x0f0f0f0fu;
    right ^= work;
    leftt ^= (work << 4);
    work = ((leftt >> 16) ^ right) & 0x0000ffffu;
    right ^= work;
    leftt ^= (work << 16);
    work = ((right >> 2) ^ leftt) & 0x33333333u;
    leftt ^= work;
    right ^= (work << 2);
    work = ((right >> 8) ^ leftt) & 0x00ff00ffu;
    leftt ^= work;
    right ^= (work << 8);
    right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffu;
    work = (leftt ^ right) & 0xaaaaaaaau;
    leftt ^= work;
    right ^= work;
    leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffu;

    for (int round = 0; round < 8; round++ ) {
        work  = (right << 28) | (right >> 4);
        work ^= *keys++;
        uint32_t fval;
        fval  = SP7[ work         & 0x3fu];
        fval |= SP5[(work >>  8) & 0x3fu];
        fval |= SP3[(work >> 16) & 0x3fu];
        fval |= SP1[(work >> 24) & 0x3fu];
        work  = right ^ *keys++;
        fval |= SP8[ work         & 0x3fu];
        fval |= SP6[(work >>  8) & 0x3fu];
        fval |= SP4[(work >> 16) & 0x3fu];
        fval |= SP2[(work >> 24) & 0x3fu];
        leftt ^= fval;
        work  = (leftt << 28) | (leftt >> 4);
        work ^= *keys++;
        fval  = SP7[ work         & 0x3fu];
        fval |= SP5[(work >>  8) & 0x3fu];
        fval |= SP3[(work >> 16) & 0x3fu];
        fval |= SP1[(work >> 24) & 0x3fu];
        work  = leftt ^ *keys++;
        fval |= SP8[ work         & 0x3fu];
        fval |= SP6[(work >>  8) & 0x3fu];
        fval |= SP4[(work >> 16) & 0x3fu];
        fval |= SP2[(work >> 24) & 0x3fu];
        right ^= fval;
    }

    right = (right << 31) | (right >> 1);
    work = (leftt ^ right) & 0xaaaaaaaau;
    leftt ^= work;
    right ^= work;
    leftt = (leftt << 31) | (leftt >> 1);
    work = ((leftt >> 8) ^ right) & 0x00ff00ffu;
    right ^= work;
    leftt ^= (work << 8);
    work = ((leftt >> 2) ^ right) & 0x33333333u;
    right ^= work;
    leftt ^= (work << 2);
    work = ((right >> 16) ^ leftt) & 0x0000ffffu;
    leftt ^= work;
    right ^= (work << 16);
    work = ((right >> 4) ^ leftt) & 0x0f0f0f0fu;
    leftt ^= work;
    right ^= (work << 4);
    *block++ = right;
    *block = leftt;
}

/* Validation sets:
 *
 * Single-length key, single-length plaintext -
 * Key      : 0123 4567 89ab cdef
 * Plain  : 0123 4567 89ab cde7
 * Cipher : c957 4425 6a5e d31d
 *
 * Double-length key, single-length plaintext -
 * Key      : 0123 4567 89ab cdef fedc ba98 7654 3210
 * Plain  : 0123 4567 89ab cde7
 * Cipher : 7f1d 0a77 826b 8aff
 *
 * Double-length key, double-length plaintext -
 * Key      : 0123 4567 89ab cdef fedc ba98 7654 3210
 * Plain  : 0123 4567 89ab cdef 0123 4567 89ab cdff
 * Cipher : 27a0 8440 406a df60 278f 47cf 42d6 15d7
 *
 * Triple-length key, single-length plaintext -
 * Key      : 0123 4567 89ab cdef fedc ba98 7654 3210 89ab cdef 0123 4567
 * Plain  : 0123 4567 89ab cde7
 * Cipher : de0b 7c06 ae5e 0ed5
 *
 * Triple-length key, double-length plaintext -
 * Key      : 0123 4567 89ab cdef fedc ba98 7654 3210 89ab cdef 0123 4567
 * Plain  : 0123 4567 89ab cdef 0123 4567 89ab cdff
 * Cipher : ad0d 1b30 ac17 cf07 0ed1 1c63 81e4 4de5
 *
 * d3des V5.0a rwo 9208.07 18:44 Graven Imagery
 **********************************************************************/
