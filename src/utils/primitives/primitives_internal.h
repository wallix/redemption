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
#pragma once

#include <stdint.h>

static inline uint8_t CLIP(int32_t X)
{
	if (X > 255L)
		return 255L;

	if (X < 0L)
		return 0L;

	return X;
}

/* Use lddqu for unaligned; load for 16-byte aligned. */
#define LOAD_SI128(_ptr_) \
	((reinterpret_cast<uint64_t>(_ptr_) & 0x0f) \
	 ? _mm_lddqu_si128(reinterpret_cast<const __m128i *>(_ptr_)) \
	 : _mm_load_si128(reinterpret_cast<const __m128i *>(_ptr_)))



/* These are prototypes for SSE (potentially NEON) routines that do a
 * simple SSE operation over an array of data.  Since so much of this
 * code is shared except for the operation itself, these prototypes are
 * used rather than duplicating code.  The naming convention depends on
 * the parameters:  S=Source param; C=Constant; D=Destination.
 * All the macros have parameters for a fallback procedure if the data
 * is too small and an operation "the slow way" for use at 16-byte edges.
 */

/* SSE3 note:  If someone needs to support an SSE2 version of these without
 * SSE3 support, an alternative version could be added that merely checks
 * that 16-byte alignment on both destination and source(s) can be
 * achieved, rather than use LDDQU for unaligned reads.
 */

/* Note: the compiler is good at turning (16/sizeof(_type_)) into a constant.
 * It easily can't do that if the value is stored in a variable.
 * So don't save it as an intermediate value.
 */

/* ----------------------------------------------------------------------------
 * SCD = Source, Constant, Destination
 */
#define SSE3_SCD_ROUTINE(_name_, _type_, _fallback_, _op_, _slowWay_) \
	static Primitives::pstatus_t _name_(const _type_ *pSrc, uint32_t val, _type_ *pDst, uint32_t len) \
    { \
	    int32_t shifts = 0; \
	    uint32_t offBeatMask; \
	    const _type_ *sptr = pSrc; \
	    _type_ *dptr = pDst; \
	    size_t count; \
	    if (len < 16)   /* pointless if too small */ \
        { \
	        return _fallback_(pSrc, val, pDst, len); \
	    } \
	    if      (sizeof(_type_) == 1) shifts = 1; \
	    else if (sizeof(_type_) == 2) shifts = 2; \
	    else if (sizeof(_type_) == 4) shifts = 3; \
	    else if (sizeof(_type_) == 8) shifts = 4; \
	    offBeatMask = (1 << (shifts - 1)) - 1; \
	    if (reinterpret_cast<uint64_t>(pDst) & offBeatMask) \
        { \
	        /* Incrementing the pointer skips over 16-byte boundary. */ \
	        return _fallback_(pSrc, val, pDst, len); \
	    } \
	    /* Get to the 16-byte boundary now. */ \
	    while (reinterpret_cast<uint64_t>(pDst) & 0x0f) \
        { \
	        _slowWay_; \
	        if (--len == 0) \
	        	return Primitives::SUCCESS; \
	    } \
	    /* Use 8 128-bit SSE registers. */ \
	    count = len >> (8-shifts); \
	    len -= count << (8-shifts); \
	    if (reinterpret_cast<uint64_t>(sptr) & 0x0f) \
        { \
	        while (count--) \
            { \
	            __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7; \
	            xmm0 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm1 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm2 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm3 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm4 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm5 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm6 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm7 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm0 = _op_(xmm0, val); \
	            xmm1 = _op_(xmm1, val); \
	            xmm2 = _op_(xmm2, val); \
	            xmm3 = _op_(xmm3, val); \
	            xmm4 = _op_(xmm4, val); \
	            xmm5 = _op_(xmm5, val); \
	            xmm6 = _op_(xmm6, val); \
	            xmm7 = _op_(xmm7, val); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm0); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm1); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm2); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm3); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm4); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm5); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm6); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm7); \
	            dptr += (16 / sizeof(_type_)); \
	        } \
	    } \
	    else \
        { \
	        while (count--) \
            { \
	            __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7; \
	            xmm0 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm1 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm2 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm3 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm4 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm5 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm6 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm7 = _mm_load_si128(reinterpret_cast<const __m128i *>(sptr)); \
	            sptr += (16 / sizeof(_type_)); \
	            xmm0 = _op_(xmm0, val); \
	            xmm1 = _op_(xmm1, val); \
	            xmm2 = _op_(xmm2, val); \
	            xmm3 = _op_(xmm3, val); \
	            xmm4 = _op_(xmm4, val); \
	            xmm5 = _op_(xmm5, val); \
	            xmm6 = _op_(xmm6, val); \
	            xmm7 = _op_(xmm7, val); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm0); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm1); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm2); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm3); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm4); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm5); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm6); \
	            dptr += (16 / sizeof(_type_)); \
	            _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm7); \
	            dptr += (16 / sizeof(_type_)); \
	        } \
	    } \
	    /* Use a single 128-bit SSE register. */ \
	    count = len >> (5-shifts); \
	    len -= count << (5-shifts); \
	    while (count--) \
        { \
	        __m128i xmm0 = LOAD_SI128(sptr); sptr += (16 / sizeof(_type_)); \
	        xmm0 = _op_(xmm0, val); \
	        _mm_store_si128(reinterpret_cast<__m128i *>(dptr), xmm0); \
	        dptr += (16 / sizeof(_type_)); \
	    } \
	    /* Finish off the remainder. */ \
	    while (len--) { \
	    	_slowWay_; \
	    } \
	    return Primitives::SUCCESS; \
	}
