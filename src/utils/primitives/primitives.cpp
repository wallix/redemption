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

static inline uint8_t* writePixelBGRX(uint8_t* dst, uint16_t formatSize, uint32_t format,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = B;
    *dst++ = G;
    *dst++ = R;
    *dst++ = A;
    return dst;
}

static inline uint8_t* writePixelRGBX(uint8_t* dst, uint16_t formatSize, uint32_t format,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = R;
    *dst++ = G;
    *dst++ = B;
    *dst++ = A;
    return dst;
}

static inline uint8_t* writePixelXBGR(uint8_t* dst, uint16_t formatSize, uint32_t format,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = A;
    *dst++ = B;
    *dst++ = G;
    *dst++ = R;
    return dst;
}

static inline uint8_t* writePixelXRGB(uint8_t* dst, uint16_t formatSize, uint32_t format,
                                   uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    *dst++ = A;
    *dst++ = R;
    *dst++ = G;
    *dst++ = B;
    return dst;
}

static inline uint8_t* writePixelGeneric(uint8_t* dst, uint16_t formatSize, uint32_t format,
                                      uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
#if 0
    uint32_t color = FreeRDPGetColor(format, R, G, B, A);
    WriteColor(dst, format, color);
#endif
    return dst + formatSize;
}

typedef uint8_t* (*fkt_writePixel)(uint8_t*, uint16_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t);

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

static inline uint8_t CLIP(int32_t X) {
    if (X > 255L)
        return 255L;

    if (X < 0L)
        return 0L;

    return X;
}

static Primitives::pstatus_t general_lShiftC_16s(const int16_t * pSrc, uint32_t val, int16_t * pDst, uint32_t len)
{
    if (val == 0)
        return Primitives::SUCCESS;

    while (len--)
        *pDst++ = *pSrc++ << val;

    return Primitives::SUCCESS;
}

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
    int dstPad = (dstStep - (roi->width * 4)) / 4;
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
    int dstPad = (dstStep - (roi->width * 4)) / 4;
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

Primitives Primitives::s_instance;
Primitives Primitives::s_genericInstance;





Primitives::Primitives()
    : accel(Primitives::ACCEL_GENERIC)
    , lShiftC_16s(general_lShiftC_16s)
    , yCbCrToRGB_16s8u_P3AC4R(general_yCbCrToRGB_16s8u_P3AC4R)
{
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
