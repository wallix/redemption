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

#include "utils/primitives/primitives.hpp"

#include "utils/primitives/primitives_internal.hpp"

struct writePixelBGRX
{
    static inline uint8_t* write(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                 uint8_t R, uint8_t G, uint8_t B, uint8_t A) noexcept
    {
        *dst++ = B;
        *dst++ = G;
        *dst++ = R;
        *dst++ = A;
        return dst;
    }
};

struct writePixelRGBX
{
    static inline uint8_t* write(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                 uint8_t R, uint8_t G, uint8_t B, uint8_t A) noexcept
    {
        *dst++ = R;
        *dst++ = G;
        *dst++ = B;
        *dst++ = A;
        return dst;
    }
};

struct writePixelXBGR
{
    static inline uint8_t* write(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                 uint8_t R, uint8_t G, uint8_t B, uint8_t A) noexcept
    {
        *dst++ = A;
        *dst++ = B;
        *dst++ = G;
        *dst++ = R;
        return dst;
    }
};

struct writePixelXRGB
{
    static inline uint8_t* write(uint8_t* dst, uint16_t /*formatSize*/, uint32_t /*format*/,
                                 uint8_t R, uint8_t G, uint8_t B, uint8_t A) noexcept
    {
        *dst++ = A;
        *dst++ = R;
        *dst++ = G;
        *dst++ = B;
        return dst;
    }
};

struct writePixelGeneric
{
    static inline uint8_t* write(uint8_t* dst, uint16_t formatSize, uint32_t /*format*/,
                                 uint8_t /*R*/, uint8_t /*G*/, uint8_t /*B*/, uint8_t /*A*/)
    {
    #if 0
        uint32_t color = FreeRDPGetColor(format, R, G, B, A);
        WriteColor(dst, format, color);
    #endif
        return dst + formatSize;
    }
};


Primitives::pstatus_t general_lShiftC_16s(const int16_t * pSrc, uint32_t val, int16_t * pDst, uint32_t len) noexcept
{
    if (val == 0) {
        return Primitives::SUCCESS;
    }

    while (len--) {
        *pDst++ = *pSrc++ << val;
    }

    return Primitives::SUCCESS;
}


/**
 *
 */
template<class WritePixel>
static Primitives::pstatus_t yCbCrToRGB_16s8u_P3AC4R_XXX(
    const int16_t *pSrc[3], uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat dstFormat,
    const Primitives::prim_size_t *roi) noexcept
{
    uint8_t* pRGB = pDst;
    const int16_t *pY  = pSrc[0];
    const int16_t *pCb = pSrc[1];
    const int16_t *pCr = pSrc[2];
    int srcPad = (srcStep - (roi->width * 2)) / 2;
    int dstPad = static_cast<int>(dstStep - (roi->width * 4));
    const uint16_t formatSize = Primitives::pixelFormatSize(dstFormat);

    for (uint32_t y = 0; y < roi->height; y++) {
        for (uint32_t x = 0; x < roi->width; x++) {
            const int32_t divisor = 16;
            const int32_t Y = ((*pY++) + 4096) << divisor;
            const int32_t Cb = (*pCb++);
            const int32_t Cr = (*pCr++);
            const int32_t CrR = Cr * static_cast<int32_t>(1.402525f * (1 << divisor));
            const int32_t CrG = Cr * static_cast<int32_t>(0.714401f * (1 << divisor));
            const int32_t CbG = Cb * static_cast<int32_t>(0.343730f * (1 << divisor));
            const int32_t CbB = Cb * static_cast<int32_t>(1.769905f * (1 << divisor));
            const int16_t R = static_cast<int16_t>((CrR + Y) >> divisor) >> 5;
            const int16_t G = static_cast<int16_t>((Y - CbG - CrG) >> divisor) >> 5;
            const int16_t B = static_cast<int16_t>((CbB + Y) >> divisor) >> 5;
            pRGB = WritePixel::write(pRGB, formatSize, dstFormat, CLIP(R), CLIP(G),
                                     CLIP(B), 0xFF);
        }

        pY += srcPad;
        pCb += srcPad;
        pCr += srcPad;
        pRGB += dstPad;
    }

    return Primitives::SUCCESS;
}

Primitives::pstatus_t general_yCbCrToRGB_16s8u_P3AC4R(
    const int16_t *pSrc[3], uint32_t srcStep,
    uint8_t *pDst, uint32_t dstStep, Primitives::PixelFormat dstFormat,
    const Primitives::prim_size_t *roi) noexcept
{
    switch (dstFormat)
    {
        case Primitives::PIXEL_FORMAT_ARGB32:
        case Primitives::PIXEL_FORMAT_XRGB32:
            return yCbCrToRGB_16s8u_P3AC4R_XXX<writePixelXRGB>(pSrc, srcStep, pDst, dstStep, dstFormat, roi);

        case Primitives::PIXEL_FORMAT_ABGR32:
        case Primitives::PIXEL_FORMAT_XBGR32:
            return yCbCrToRGB_16s8u_P3AC4R_XXX<writePixelXBGR>(pSrc, srcStep, pDst, dstStep, dstFormat, roi);

        case Primitives::PIXEL_FORMAT_RGBA32:
        case Primitives::PIXEL_FORMAT_RGBX32:
            return yCbCrToRGB_16s8u_P3AC4R_XXX<writePixelRGBX>(pSrc, srcStep, pDst, dstStep, dstFormat, roi);

        case Primitives::PIXEL_FORMAT_BGRA32:
        case Primitives::PIXEL_FORMAT_BGRX32:
            return yCbCrToRGB_16s8u_P3AC4R_XXX<writePixelBGRX>(pSrc, srcStep, pDst, dstStep, dstFormat, roi);
    }

    return yCbCrToRGB_16s8u_P3AC4R_XXX<writePixelGeneric>(pSrc, srcStep, pDst, dstStep, dstFormat, roi);
}


#ifndef __EMSCRIPTEN__

#include <cpuid.h>

static bool haveSSSE3()
{
    int a, b, c, d;
    int nIds;

    __cpuid_count(0, 0, nIds, b, c, d);

    if (nIds >= 1) {
        __cpuid_count(0x1, 0, a, b, c, d);
        return c & (1 << 9);
    }
    return false;
}
#endif


Primitives Primitives::s_instance;

Primitives::Primitives() noexcept
    : lShiftC_16s(general_lShiftC_16s)
    , yCbCrToRGB_16s8u_P3AC4R(general_yCbCrToRGB_16s8u_P3AC4R)
{
#ifndef __EMSCRIPTEN__
    if (haveSSSE3()) {
        init_sse(this);
    }
#endif
}

Primitives *Primitives::instance()
{
    return &s_instance;
}

uint16_t Primitives::pixelFormatSize(PixelFormat /*format*/) noexcept
{
    return 4;
}
