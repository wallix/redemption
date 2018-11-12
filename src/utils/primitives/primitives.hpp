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
#include <stddef.h>

/**
 * @brief primitives
 */
class Primitives {
public:
	/** @brief primitive size */
	typedef struct {
		uint32_t width;
		uint32_t height;
	} prim_size_t;

	/** @brief result of a primitive execution */
	typedef enum {
		SUCCESS,
	} pstatus_t;

	/** @brief the format of a pixel */
	typedef enum {
		PIXEL_FORMAT_ARGB32,
		PIXEL_FORMAT_XRGB32,
		PIXEL_FORMAT_ABGR32,
		PIXEL_FORMAT_XBGR32,
		PIXEL_FORMAT_RGBA32,
		PIXEL_FORMAT_RGBX32,
		PIXEL_FORMAT_BGRA32,
		PIXEL_FORMAT_BGRX32
	} PixelFormat;

	typedef pstatus_t (*__lShiftC_16s_t)(const int16_t * pSrc, uint32_t val, int16_t * pSrcDst, uint32_t len);

	typedef pstatus_t (*__yCbCrToRGB_16s8u_P3AC4R_t)(const int16_t * pSrc[3], uint32_t srcStep,
	    uint8_t * pDst, uint32_t dstStep, PixelFormat DstFormat,
	    const prim_size_t* roi);

public:
	/**
	 *
	 */
	static Primitives *instance();

	/**
	 *
	 */
	static size_t pixelFormatSize(PixelFormat format);

protected:
	Primitives();

	/** @brief kind of acceleration */
	enum {
		ACCEL_GENERIC,
		ACCEL_SSE,
	} accel;

public:
	__lShiftC_16s_t lShiftC_16s;
	__yCbCrToRGB_16s8u_P3AC4R_t yCbCrToRGB_16s8u_P3AC4R;

protected:
	static Primitives s_instance;
	static Primitives s_genericInstance;
};
