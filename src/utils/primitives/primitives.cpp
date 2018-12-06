/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2018
    Author(s): David Fort
*/
#include <emmintrin.h>
#include <pmmintrin.h>

#include "utils/primitives/primitives.hpp"

#include "primitives_internal.h"


static inline uint8_t* writePixelBGRX(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = B;
    *dst++ = G;
    *dst++ = R;
    *dst++ = A;
    return dst;
}

static inline uint8_t* writePixelRGBX(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = R;
    *dst++ = G;
    *dst++ = B;
    *dst++ = A;
    return dst;
}

static inline uint8_t* writePixelXBGR(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = A;
    *dst++ = B;
    *dst++ = G;
    *dst++ = R;
    return dst;
}

static inline uint8_t* writePixelXRGB(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = A;
    *dst++ = R;
    *dst++ = G;
    *dst++ = B;
    return dst;
}

static inline uint8_t* writePixelGeneric(uint8_t* dst, uint16_t formatSize, uint32_t /*format*/,
                                      uint8_t /*R*/, uint8_t /*G*/, uint8_t /*B*/, uint8_t /*A*/)
{
#if 0
    uint32_t color = FreeRDPGetColor(format, R, G, B, A);
    WriteColor(dst, format, color);
#endif
    return dst + formatSize;
}

using fkt_writePixel = uint8_t* (*)(uint8_t*, uint16_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t);

static inline fkt_writePixel getPixelWriteFunction(uint16_t format)
{
    switch (format)
    {
        case Primitives::PIXEL_FORMAT_ARGB32:
        case Primitives::PIXEL_FORMAT_XRGB32:
            return writePixelXRGB;

        case Primitives::PIXEL_FORMAT_ABGR32:
        case Primitives::PIXEL_FORMAT_XBGR32:
            return writePixelXBGR;

        case Primitives::PIXEL_FORMAT_RGBA32:
        case Primitives::PIXEL_FORMAT_RGBX32:
            return writePixelRGBX;

        case Primitives::PIXEL_FORMAT_BGRA32:
        case Primitives::PIXEL_FORMAT_BGRX32:
            return writePixelBGRX;

        default:
            return writePixelGeneric;
    }
}


static Primitives::pstatus_t general_lShiftC_16s(const int16_t * pSrc, uint32_t val, int16_t * pDst, uint32_t len)
{
    if (val == 0) {
        return Primitives::SUCCESS;
    }

    while (len--) {
        *pDst++ = *pSrc++ << val;
    }

    return Primitives::SUCCESS;
}

/* ------------------------------------------------------------------------- */
SSE3_SCD_ROUTINE(sse2_lShiftC_16s, int16_t, general_lShiftC_16s, _mm_slli_epi16, *dptr++ = *sptr++ << val)

/**
 *
 */

static Primitives::pstatus_t general_yCbCrToRGB_16s8u_P3AC4R_BGRX(
    const int16_t *pSrc[3], uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat dstFormat,
    const Primitives::prim_size_t *roi)
{
    uint32_t x, y;
    uint8_t *pRGB = pDst;
    const int16_t *pY  = pSrc[0];
    const int16_t *pCb = pSrc[1];
    const int16_t *pCr = pSrc[2];
    int srcPad = (srcStep - (roi->width * 2)) / 2;
    int dstPad = (dstStep - (roi->width * 4));
    const uint16_t formatSize = Primitives::pixelFormatSize(dstFormat);

    for (y = 0; y < roi->height; y++) {
        for (x = 0; x < roi->width; x++) {
            int16_t R, G, B;
            const int32_t divisor = 16;
            const int32_t Y = ((*pY++) + 4096) << divisor;
            const int32_t Cb = (*pCb++);
            const int32_t Cr = (*pCr++);
            const int32_t CrR = Cr * static_cast<int32_t>(1.402525f * (1 << divisor));
            const int32_t CrG = Cr * static_cast<int32_t>(0.714401f * (1 << divisor));
            const int32_t CbG = Cb * static_cast<int32_t>(0.343730f * (1 << divisor));
            const int32_t CbB = Cb * static_cast<int32_t>(1.769905f * (1 << divisor));
            R = (static_cast<int16_t>((CrR + Y) >> divisor)) >> 5;
            G = (static_cast<int16_t>((Y - CbG - CrG) >> divisor)) >> 5;
            B = (static_cast<int16_t>((CbB + Y) >> divisor)) >> 5;
            pRGB = writePixelBGRX(pRGB, formatSize, dstFormat, CLIP(R), CLIP(G),
                                  CLIP(B), 0xFF);
        }

        pY += srcPad;
        pCb += srcPad;
        pCr += srcPad;
        pRGB += dstPad;
    }

    return Primitives::SUCCESS;
}

static Primitives::pstatus_t general_yCbCrToRGB_16s8u_P3AC4R_general(
    const int16_t *pSrc[3], uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat dstFormat,
    const Primitives::prim_size_t *roi)
{
    uint32_t x, y;
    uint8_t* pRGB = pDst;
    const int16_t *pY  = pSrc[0];
    const int16_t *pCb = pSrc[1];
    const int16_t *pCr = pSrc[2];
    int srcPad = (srcStep - (roi->width * 2)) / 2;
    int dstPad = (dstStep - (roi->width * 4));
    // TODO: use templates instead of function pointer
    fkt_writePixel writePixel = getPixelWriteFunction(dstFormat);
    const uint16_t formatSize = Primitives::pixelFormatSize(dstFormat);

    for (y = 0; y < roi->height; y++) {
        for (x = 0; x < roi->width; x++) {
            int16_t R, G, B;
            const int32_t divisor = 16;
            const int32_t Y = ((*pY++) + 4096) << divisor;
            const int32_t Cb = (*pCb++);
            const int32_t Cr = (*pCr++);
            const int32_t CrR = Cr * static_cast<int32_t>(1.402525f * (1 << divisor));
            const int32_t CrG = Cr * static_cast<int32_t>(0.714401f * (1 << divisor));
            const int32_t CbG = Cb * static_cast<int32_t>(0.343730f * (1 << divisor));
            const int32_t CbB = Cb * static_cast<int32_t>(1.769905f * (1 << divisor));
            R = static_cast<int16_t>((CrR + Y) >> divisor) >> 5;
            G = static_cast<int16_t>((Y - CbG - CrG) >> divisor) >> 5;
            B = static_cast<int16_t>((CbB + Y) >> divisor) >> 5;
            pRGB = (*writePixel)(pRGB, formatSize, dstFormat, CLIP(R), CLIP(G),
                                 CLIP(B), 0xFF);
        }

        pY += srcPad;
        pCb += srcPad;
        pCr += srcPad;
        pRGB += dstPad;
    }

    return Primitives::SUCCESS;
}

static Primitives::pstatus_t general_yCbCrToRGB_16s8u_P3AC4R(
    const int16_t *pSrc[3], uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat dstFormat,
    const Primitives::prim_size_t *roi)
{
    switch (dstFormat)
    {
        case Primitives::PIXEL_FORMAT_BGRA32:
        case Primitives::PIXEL_FORMAT_BGRX32:
            return general_yCbCrToRGB_16s8u_P3AC4R_BGRX(pSrc, srcStep, pDst, dstStep, dstFormat, roi);

        default:
            return general_yCbCrToRGB_16s8u_P3AC4R_general(pSrc, srcStep, pDst, dstStep, dstFormat, roi);
    }
}

#define _mm_between_epi16(_val, _min, _max) \
	do { _val = _mm_min_epi16(_max, _mm_max_epi16(_val, _min)); } while (0)


static Primitives::pstatus_t sse2_yCbCrToRGB_16s8u_P3AC4R_BGRX(const int16_t* pSrc[3], uint32_t /*srcStep*/,
    uint8_t *pDst, uint32_t dstStep,
    const Primitives::prim_size_t* roi)	/* region of interest */
{
	const __m128i zero = _mm_setzero_si128();
	const __m128i max = _mm_set1_epi16(255);
	const __m128i r_cr = _mm_set1_epi16(22986);	/*  1.403 << 14 */
	const __m128i g_cb = _mm_set1_epi16(-5636);	/* -0.344 << 14 */
	const __m128i g_cr = _mm_set1_epi16(-11698);	/* -0.714 << 14 */
	const __m128i b_cb = _mm_set1_epi16(28999);	/*  1.770 << 14 */
	const __m128i c4096 = _mm_set1_epi16(4096);
	const int16_t* y_buf = pSrc[0];
	const int16_t* cb_buf = pSrc[1];
	const int16_t* cr_buf = pSrc[2];
	const uint32_t pad = roi->width % 16;
	const uint32_t step = sizeof(__m128i) / sizeof(int16_t);
	const uint32_t imax = (roi->width - pad) * sizeof(int16_t) / sizeof(__m128i);
	uint8_t* d_buf = pDst;
	uint32_t yp;
	const size_t dstPad = (dstStep - roi->width * 4);
#ifdef DO_PREFETCH

	/* Prefetch Y's, Cb's, and Cr's. */
	for (yp = 0; yp < roi->height; yp++)
	{
		int i;

		for (i = 0; i < imax;
		     i += (CACHE_LINE_BYTES / sizeof(__m128i)))
		{
			_mm_prefetch((char*)(&((__m128i*)y_buf)[i]),  _MM_HINT_NTA);
			_mm_prefetch((char*)(&((__m128i*)cb_buf)[i]), _MM_HINT_NTA);
			_mm_prefetch((char*)(&((__m128i*)cr_buf)[i]), _MM_HINT_NTA);
		}

		y_buf  += srcStep / sizeof(INT16);
		cb_buf += srcStep / sizeof(INT16);
		cr_buf += srcStep / sizeof(INT16);
	}

	y_buf  = (INT16*)pSrc[0];
	cb_buf = (INT16*)pSrc[1];
	cr_buf = (INT16*)pSrc[2];
#endif /* DO_PREFETCH */

	for (yp = 0; yp < roi->height; ++yp)
	{
		uint32_t i;

		for (i = 0; i < imax; i += 2)
		{
			/* In order to use SSE2 signed 16-bit integer multiplication
			 * we need to convert the floating point factors to signed int
			 * without losing information.
			 * The result of this multiplication is 32 bit and we have two
			 * SSE instructions that return either the hi or lo word.
			 * Thus we will multiply the factors by the highest possible 2^n,
			 * take the upper 16 bits of the signed 32-bit result
			 * (_mm_mulhi_epi16) and correct this result by multiplying
			 * it by 2^(16-n).
			 *
			 * For the given factors in the conversion matrix the best
			 * possible n is 14.
			 *
			 * Example for calculating r:
			 * r = (y>>5) + 128 + (cr*1.403)>>5             // our base formula
			 * r = (y>>5) + 128 + (HIWORD(cr*(1.403<<14)<<2))>>5   // see above
			 * r = (y+4096)>>5 + (HIWORD(cr*22986)<<2)>>5     // simplification
			 * r = ((y+4096)>>2 + HIWORD(cr*22986)) >> 3
			 */
			/* y = (y_r_buf[i] + 4096) >> 2 */
			__m128i y1, y2, cb1, cb2, cr1, cr2, r1, r2, g1, g2, b1, b2;
			y1 = _mm_load_si128(reinterpret_cast<const __m128i*>(y_buf));
			y_buf += step;
			y1 = _mm_add_epi16(y1, c4096);
			y1 = _mm_srai_epi16(y1, 2);
			/* cb = cb_g_buf[i]; */
			cb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(cb_buf));
			cb_buf += step;
			/* cr = cr_b_buf[i]; */
			cr1 = _mm_load_si128(reinterpret_cast<const __m128i*>(cr_buf));
			cr_buf += step;
			/* (y + HIWORD(cr*22986)) >> 3 */
			r1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cr1, r_cr));
			r1 = _mm_srai_epi16(r1, 3);
			/* r_buf[i] = CLIP(r); */
			_mm_between_epi16(r1, zero, max);
			/* (y + HIWORD(cb*-5636) + HIWORD(cr*-11698)) >> 3 */
			g1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cb1, g_cb));
			g1 = _mm_add_epi16(g1, _mm_mulhi_epi16(cr1, g_cr));
			g1 = _mm_srai_epi16(g1, 3);
			/* g_buf[i] = CLIP(g); */
			_mm_between_epi16(g1, zero, max);
			/* (y + HIWORD(cb*28999)) >> 3 */
			b1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cb1, b_cb));
			b1 = _mm_srai_epi16(b1, 3);
			/* b_buf[i] = CLIP(b); */
			_mm_between_epi16(b1, zero, max);
			y2 = _mm_load_si128(reinterpret_cast<const __m128i*>(y_buf));
			y_buf += step;
			y2 = _mm_add_epi16(y2, c4096);
			y2 = _mm_srai_epi16(y2, 2);
			/* cb = cb_g_buf[i]; */
			cb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(cb_buf));
			cb_buf += step;
			/* cr = cr_b_buf[i]; */
			cr2 = _mm_load_si128(reinterpret_cast<const __m128i*>(cr_buf));
			cr_buf += step;
			/* (y + HIWORD(cr*22986)) >> 3 */
			r2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cr2, r_cr));
			r2 = _mm_srai_epi16(r2, 3);
			/* r_buf[i] = CLIP(r); */
			_mm_between_epi16(r2, zero, max);
			/* (y + HIWORD(cb*-5636) + HIWORD(cr*-11698)) >> 3 */
			g2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cb2, g_cb));
			g2 = _mm_add_epi16(g2, _mm_mulhi_epi16(cr2, g_cr));
			g2 = _mm_srai_epi16(g2, 3);
			/* g_buf[i] = CLIP(g); */
			_mm_between_epi16(g2, zero, max);
			/* (y + HIWORD(cb*28999)) >> 3 */
			b2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cb2, b_cb));
			b2 = _mm_srai_epi16(b2, 3);
			/* b_buf[i] = CLIP(b); */
			_mm_between_epi16(b2, zero, max);
			{
				__m128i R0, R1, R2, R3, R4;
				/* The comments below pretend these are 8-byte registers
				 * rather than 16-byte, for readability.
				 */
				R0 = b1; /* R0 = 00B300B200B100B0 */
				R1 = b2; /* R1 = 00B700B600B500B4 */
				R0 = _mm_packus_epi16(R0, R1);	/* R0 = B7B6B5B4B3B2B1B0 */
				R1 = g1;		/* R1 = 00G300G200G100G0 */
				R2 = g2;		/* R2 = 00G700G600G500G4 */
				R1 = _mm_packus_epi16(R1, R2);				/* R1 = G7G6G5G4G3G2G1G0 */
				R2 = R1;						/* R2 = G7G6G5G4G3G2G1G0 */
				R2 = _mm_unpacklo_epi8(R0, R2);				/* R2 = B3G3B2G2B1G1B0G0 */
				R1 = _mm_unpackhi_epi8(R0, R1);				/* R1 = B7G7B6G6B5G5B4G4 */
				R0 = r1;		/* R0 = 00R300R200R100R0 */
				R3 = r2;		/* R3 = 00R700R600R500R4 */
				R0 = _mm_packus_epi16(R0, R3);				/* R0 = R7R6R5R4R3R2R1R0 */
				R3 = _mm_set1_epi32(0xFFFFFFFFU);				/* R3 = FFFFFFFFFFFFFFFF */
				R4 = R3;						/* R4 = FFFFFFFFFFFFFFFF */
				R4 = _mm_unpacklo_epi8(R0, R4);				/* R4 = R3FFR2FFR1FFR0FF */
				R3 = _mm_unpackhi_epi8(R0, R3);				/* R3 = R7FFR6FFR5FFR4FF */
				R0 = R4;						/* R0 = R4               */
				R0 = _mm_unpacklo_epi16(R2, R0);				/* R0 = B1G1R1FFB0G0R0FF */
				R4 = _mm_unpackhi_epi16(R2, R4);				/* R4 = B3G3R3FFB2G2R2FF */
				R2 = R3;						/* R2 = R3               */
				R2 = _mm_unpacklo_epi16(R1, R2);				/* R2 = B5G5R5FFB4G4R4FF */
				R3 = _mm_unpackhi_epi16(R1, R3);				/* R3 = B7G7R7FFB6G6R6FF */
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R0); /* B1G1R1FFB0G0R0FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R4); /* B3G3R3FFB2G2R2FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R2); /* B5G5R5FFB4G4R4FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R3); /* B7G7R7FFB6G6R6FF      */
				d_buf += sizeof(__m128i);
			}
		}

		for (i = 0; i < pad; i++)
		{
			const int32_t divisor = 16;
			const int32_t Y = ((*y_buf++) + 4096) << divisor;
			const int32_t Cb = (*cb_buf++);
			const int32_t Cr = (*cr_buf++);
			const int32_t CrR = Cr * static_cast<int32_t>(1.402525f * (1 << divisor));
			const int32_t CrG = Cr * static_cast<int32_t>(0.714401f * (1 << divisor));
			const int32_t CbG = Cb * static_cast<int32_t>(0.343730f * (1 << divisor));
			const int32_t CbB = Cb * static_cast<int32_t>(1.769905f * (1 << divisor));
			const int16_t R = (static_cast<int16_t>((CrR + Y) >> divisor) >> 5);
			const int16_t G = (static_cast<int16_t>((Y - CbG - CrG) >> divisor) >> 5);
			const int16_t B = (static_cast<int16_t>((CbB + Y) >> divisor) >> 5);
			*d_buf++ = CLIP(B);
			*d_buf++ = CLIP(G);
			*d_buf++ = CLIP(R);
			*d_buf++ = 0xFF;
		}

		d_buf += dstPad;
	}

	return Primitives::SUCCESS;
}

static Primitives::pstatus_t sse2_yCbCrToRGB_16s8u_P3AC4R_RGBX(const int16_t* pSrc[3], uint32_t /*srcStep*/,
    uint8_t* pDst, uint32_t dstStep,
    const Primitives::prim_size_t* roi)	/* region of interest */
{
	const __m128i zero = _mm_setzero_si128();
	const __m128i max = _mm_set1_epi16(255);
	const __m128i r_cr = _mm_set1_epi16(22986);	/*  1.403 << 14 */
	const __m128i g_cb = _mm_set1_epi16(-5636);	/* -0.344 << 14 */
	const __m128i g_cr = _mm_set1_epi16(-11698);	/* -0.714 << 14 */
	const __m128i b_cb = _mm_set1_epi16(28999);	/*  1.770 << 14 */
	const __m128i c4096 = _mm_set1_epi16(4096);
	const int16_t* y_buf = pSrc[0];
	const int16_t* cb_buf = pSrc[1];
	const int16_t* cr_buf = pSrc[2];
	const uint32_t pad = roi->width % 16;
	const uint32_t step = sizeof(__m128i) / sizeof(int16_t);
	const uint32_t imax = (roi->width - pad) * sizeof(int16_t) / sizeof(__m128i);
	uint8_t* d_buf = pDst;
	uint32_t yp;
	const size_t dstPad = (dstStep - roi->width * 4);
#ifdef DO_PREFETCH

	/* Prefetch Y's, Cb's, and Cr's. */
	for (yp = 0; yp < roi->height; yp++)
	{
		int i;

		for (i = 0; i < imax;
		     i += (CACHE_LINE_BYTES / sizeof(__m128i)))
		{
			_mm_prefetch((char*)(&((__m128i*)y_buf)[i]),  _MM_HINT_NTA);
			_mm_prefetch((char*)(&((__m128i*)cb_buf)[i]), _MM_HINT_NTA);
			_mm_prefetch((char*)(&((__m128i*)cr_buf)[i]), _MM_HINT_NTA);
		}

		y_buf  += srcStep / sizeof(INT16);
		cb_buf += srcStep / sizeof(INT16);
		cr_buf += srcStep / sizeof(INT16);
	}

	y_buf  = (INT16*)(pSrc[0]);
	cb_buf = (INT16*)(pSrc[1]);
	cr_buf = (INT16*)(pSrc[2]);
#endif /* DO_PREFETCH */

	for (yp = 0; yp < roi->height; ++yp)
	{
		uint32_t i;

		for (i = 0; i < imax; i += 2)
		{
			/* In order to use SSE2 signed 16-bit integer multiplication
			 * we need to convert the floating point factors to signed int
			 * without losing information.
			 * The result of this multiplication is 32 bit and we have two
			 * SSE instructions that return either the hi or lo word.
			 * Thus we will multiply the factors by the highest possible 2^n,
			 * take the upper 16 bits of the signed 32-bit result
			 * (_mm_mulhi_epi16) and correct this result by multiplying
			 * it by 2^(16-n).
			 *
			 * For the given factors in the conversion matrix the best
			 * possible n is 14.
			 *
			 * Example for calculating r:
			 * r = (y>>5) + 128 + (cr*1.403)>>5             // our base formula
			 * r = (y>>5) + 128 + (HIWORD(cr*(1.403<<14)<<2))>>5   // see above
			 * r = (y+4096)>>5 + (HIWORD(cr*22986)<<2)>>5     // simplification
			 * r = ((y+4096)>>2 + HIWORD(cr*22986)) >> 3
			 */
			/* y = (y_r_buf[i] + 4096) >> 2 */
			__m128i y1, y2, cb1, cb2, cr1, cr2, r1, r2, g1, g2, b1, b2;
			y1 = _mm_load_si128(reinterpret_cast<const __m128i*>(y_buf));
			y_buf += step;
			y1 = _mm_add_epi16(y1, c4096);
			y1 = _mm_srai_epi16(y1, 2);
			/* cb = cb_g_buf[i]; */
			cb1 = _mm_load_si128(reinterpret_cast<const __m128i*>(cb_buf));
			cb_buf += step;
			/* cr = cr_b_buf[i]; */
			cr1 = _mm_load_si128(reinterpret_cast<const __m128i*>(cr_buf));
			cr_buf += step;
			/* (y + HIWORD(cr*22986)) >> 3 */
			r1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cr1, r_cr));
			r1 = _mm_srai_epi16(r1, 3);
			/* r_buf[i] = CLIP(r); */
			_mm_between_epi16(r1, zero, max);
			/* (y + HIWORD(cb*-5636) + HIWORD(cr*-11698)) >> 3 */
			g1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cb1, g_cb));
			g1 = _mm_add_epi16(g1, _mm_mulhi_epi16(cr1, g_cr));
			g1 = _mm_srai_epi16(g1, 3);
			/* g_buf[i] = CLIP(g); */
			_mm_between_epi16(g1, zero, max);
			/* (y + HIWORD(cb*28999)) >> 3 */
			b1 = _mm_add_epi16(y1, _mm_mulhi_epi16(cb1, b_cb));
			b1 = _mm_srai_epi16(b1, 3);
			/* b_buf[i] = CLIP(b); */
			_mm_between_epi16(b1, zero, max);
			y2 = _mm_load_si128(reinterpret_cast<const __m128i*>(y_buf));
			y_buf += step;
			y2 = _mm_add_epi16(y2, c4096);
			y2 = _mm_srai_epi16(y2, 2);
			/* cb = cb_g_buf[i]; */
			cb2 = _mm_load_si128(reinterpret_cast<const __m128i*>(cb_buf));
			cb_buf += step;
			/* cr = cr_b_buf[i]; */
			cr2 = _mm_load_si128(reinterpret_cast<const __m128i*>(cr_buf));
			cr_buf += step;
			/* (y + HIWORD(cr*22986)) >> 3 */
			r2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cr2, r_cr));
			r2 = _mm_srai_epi16(r2, 3);
			/* r_buf[i] = CLIP(r); */
			_mm_between_epi16(r2, zero, max);
			/* (y + HIWORD(cb*-5636) + HIWORD(cr*-11698)) >> 3 */
			g2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cb2, g_cb));
			g2 = _mm_add_epi16(g2, _mm_mulhi_epi16(cr2, g_cr));
			g2 = _mm_srai_epi16(g2, 3);
			/* g_buf[i] = CLIP(g); */
			_mm_between_epi16(g2, zero, max);
			/* (y + HIWORD(cb*28999)) >> 3 */
			b2 = _mm_add_epi16(y2, _mm_mulhi_epi16(cb2, b_cb));
			b2 = _mm_srai_epi16(b2, 3);
			/* b_buf[i] = CLIP(b); */
			_mm_between_epi16(b2, zero, max);
			{
				__m128i R0, R1, R2, R3, R4;
				/* The comments below pretend these are 8-byte registers
				 * rather than 16-byte, for readability.
				 */
				R0 = r1; /* R0 = 00R300R200R100R0 */
				R1 = r2; /* R1 = 00R700R600R500R4 */
				R0 = _mm_packus_epi16(R0, R1);	/* R0 = R7R6R5R4R3R2R1R0 */
				R1 = g1;		/* R1 = 00G300G200G100G0 */
				R2 = g2;		/* R2 = 00G700G600G500G4 */
				R1 = _mm_packus_epi16(R1, R2);				/* R1 = G7G6G5G4G3G2G1G0 */
				R2 = R1;						/* R2 = G7G6G5G4G3G2G1G0 */
				R2 = _mm_unpacklo_epi8(R0, R2);				/* R2 = R3G3R2G2R1G1R0G0 */
				R1 = _mm_unpackhi_epi8(R0, R1);				/* R1 = R7G7R6G6R5G5R4G4 */
				R0 = b1;		/* R0 = 00B300B200B100B0 */
				R3 = b2;		/* R3 = 00B700B600B500B4 */
				R0 = _mm_packus_epi16(R0, R3);				/* R0 = B7B6B5B4B3B2B1B0 */
				R3 = _mm_set1_epi32(0xFFFFFFFFU);				/* R3 = FFFFFFFFFFFFFFFF */
				R4 = R3;						/* R4 = FFFFFFFFFFFFFFFF */
				R4 = _mm_unpacklo_epi8(R0, R4);				/* R4 = B3FFB2FFB1FFB0FF */
				R3 = _mm_unpackhi_epi8(R0, R3);				/* R3 = B7FFB6FFB5FFB4FF */
				R0 = R4;						/* R0 = R4               */
				R0 = _mm_unpacklo_epi16(R2, R0);				/* R0 = R1G1B1FFR0G0B0FF */
				R4 = _mm_unpackhi_epi16(R2, R4);				/* R4 = R3G3B3FFR2G2B2FF */
				R2 = R3;						/* R2 = R3               */
				R2 = _mm_unpacklo_epi16(R1, R2);				/* R2 = R5G5B5FFR4G4B4FF */
				R3 = _mm_unpackhi_epi16(R1, R3);				/* R3 = R7G7B7FFR6G6B6FF */
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R0); /* R1G1B1FFR0G0B0FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R4); /* R3G3B3FFR2G2B2FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R2); /* R5G5B5FFR4G4B4FF      */
				d_buf += sizeof(__m128i);
				_mm_store_si128(reinterpret_cast<__m128i*>(d_buf), R3); /* R7G7B7FFR6G6B6FF      */
				d_buf += sizeof(__m128i);
			}
		}

		for (i = 0; i < pad; i++)
		{
			const int32_t divisor = 16;
			const int32_t Y = ((*y_buf++) + 4096) << divisor;
			const int32_t Cb = (*cb_buf++);
			const int32_t Cr = (*cr_buf++);
			const int32_t CrR = Cr * static_cast<int32_t>(1.402525f * (1 << divisor));
			const int32_t CrG = Cr * static_cast<int32_t>(0.714401f * (1 << divisor));
			const int32_t CbG = Cb * static_cast<int32_t>(0.343730f * (1 << divisor));
			const int32_t CbB = Cb * static_cast<int32_t>(1.769905f * (1 << divisor));
			const int16_t R = (static_cast<int16_t>((CrR + Y) >> divisor) >> 5);
			const int16_t G = (static_cast<int16_t>((Y - CbG - CrG) >> divisor) >> 5);
			const int16_t B = (static_cast<int16_t>((CbB + Y) >> divisor) >> 5);
			*d_buf++ = CLIP(R);
			*d_buf++ = CLIP(G);
			*d_buf++ = CLIP(B);
			*d_buf++ = 0xFF;
		}

		d_buf += dstPad;
	}

	return Primitives::SUCCESS;
}


static Primitives::pstatus_t sse2_yCbCrToRGB_16s8u_P3AC4R(
    const int16_t *pSrc[3],
	uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat DstFormat,
    const Primitives::prim_size_t* roi)	/* region of interest */
{
	if ((reinterpret_cast<uint64_t>(pSrc[0]) & 0x0f) ||
		(reinterpret_cast<uint64_t>(pSrc[1]) & 0x0f) ||
		(reinterpret_cast<uint64_t>(pSrc[2]) & 0x0f) ||
		(reinterpret_cast<uint64_t>(pDst) & 0x0f) ||
		(srcStep & 0x0f) ||
		(dstStep & 0x0f))
	{
		/* We can't maintain 16-byte alignment. */
		return general_yCbCrToRGB_16s8u_P3AC4R(pSrc, srcStep, pDst, dstStep, DstFormat, roi);
	}

	switch (DstFormat)
	{
		case Primitives::PIXEL_FORMAT_BGRA32:
		case Primitives::PIXEL_FORMAT_BGRX32:
			return sse2_yCbCrToRGB_16s8u_P3AC4R_BGRX(pSrc, srcStep, pDst, dstStep, roi);

		case Primitives::PIXEL_FORMAT_RGBA32:
		case Primitives::PIXEL_FORMAT_RGBX32:
			return sse2_yCbCrToRGB_16s8u_P3AC4R_RGBX(pSrc, srcStep, pDst, dstStep, roi);

		default:
			return general_yCbCrToRGB_16s8u_P3AC4R(pSrc, srcStep, pDst, dstStep, DstFormat, roi);
	}
}


Primitives Primitives::s_instance;
Primitives Primitives::s_genericInstance;

#include <cpuid.h>

static bool haveSSSE3() {
	int a, b, c, d;
	int nIds;

    __cpuid_count(0, 0, nIds, b, c, d);

    if (nIds >= 1) {
    	__cpuid_count(0x1, 0, a, b, c, d);
    	return c & (1 << 9);
    }
    return false;
}


Primitives::Primitives() noexcept
    : accel(Primitives::ACCEL_GENERIC)
    , lShiftC_16s(general_lShiftC_16s)
    , yCbCrToRGB_16s8u_P3AC4R(general_yCbCrToRGB_16s8u_P3AC4R)
{
	if (haveSSSE3()) {
		lShiftC_16s = sse2_lShiftC_16s;
		yCbCrToRGB_16s8u_P3AC4R = sse2_yCbCrToRGB_16s8u_P3AC4R;
	}
}

Primitives *Primitives::instance() {
    return &s_instance;
}

size_t Primitives::pixelFormatSize(PixelFormat format) {
    switch(format) {
    case Primitives::PIXEL_FORMAT_BGRA32:
    case Primitives::PIXEL_FORMAT_BGRX32:
        return 4;
    default:
        return 4;
    }
}
