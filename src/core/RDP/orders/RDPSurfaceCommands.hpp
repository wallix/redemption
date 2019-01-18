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

#include "utils/stream.hpp"
#include "utils/log.hpp"
#include "utils/rect.hpp"
#include "core/error.hpp"

/** @brief a surface content update */
class RDPSurfaceContent {
public:
	RDPSurfaceContent(uint16_t stride, uint16_t height)
	: stride(stride)
	, data(new uint8_t[stride * height * 4]())
	{
	}

	~RDPSurfaceContent() {
		delete[] data;
	}

public:
	uint16_t stride;
	uint8_t *data;
};

/** @brief a SetSurface command */
class RDPSetSurfaceCommand {
public:
    enum {
    	EX_COMPRESSED_BITMAP_HEADER_PRESENT = 0x1
    };

	void recv(InStream & stream) {
		unsigned expected = 2;
		// 2.2.9.2.1 Set Surface Bits Command (TS_SURFCMD_SET_SURF_BITS)
		// The Set Surface Bits Command is used to transport encoded bitmap data destined for a rectangular
		// region of the primary drawing surface from an RDP server to an RDP client.
		//
		// cmdType (2 bytes): A 16-bit, unsigned integer. Surface Command type. This field MUST be set to
		// 			CMDTYPE_SET_SURFACE_BITS (0x0001).
		// destLeft (2 bytes): A 16-bit, unsigned integer. Left bound of the destination rectangle that will
		//			contain the decoded bitmap data.
		// destTop (2 bytes): A 16-bit, unsigned integer. Top bound of the destination rectangle that will
		//			contain the decoded bitmap data.
		// destRight (2 bytes): A 16-bit, unsigned integer. Exclusive right bound of the destination rectangle
		//			that will contain the decoded bitmap data. This field SHOULD be ignored, as the width of the
		//			encoded bitmap image is specified in the Extended Bitmap Data (section 2.2.9.2.1.1) present in
		//			the variable-length bitmapData field.
		// destBottom (2 bytes): A 16-bit, unsigned integer. Exclusive bottom bound of the destination
		//			rectangle that will contain the decoded bitmap data. This field SHOULD be ignored, as the height
		//			of the encoded bitmap image is specified in the Extended Bitmap Data present in the variable-
		//			length bitmapData field.
		// bitmapData (variable): An Extended Bitmap Data structure that contains an encoded bitmap image.
		//

		// 2.2.9.2.1.1 Extended Bitmap Data (TS_ BITMAP_DATA_EX)
		// The TS_BITMAP_DATA_EX structure is used to encapsulate encoded bitmap data.
		//
		// bpp (1 byte): An 8-bit, unsigned integer. The color depth of the bitmap data in bits-per-pixel.
		// flags (1 byte): An 8-bit, unsigned integer that contains flags.
		//      +-------------------------------------+------------------------------------------------+
		//      |               Flag                  |    Meaning                                     |
		//      +-------------------------------------+------------------------------------------------+
		//    	| EX_COMPRESSED_BITMAP_HEADER_PRESENT | Indicates that the optional exBitmapDataHeader |
		//      |               0x01                  | field is present.                              |
		//      +-------------------------------------+------------------------------------------------+
		//
		// reserved (1 byte): An 8-bit, unsigned integer. This field is reserved for future use. It MUST be set to
		//  	zero.
		// codecID (1 byte): An 8-bit, unsigned integer. The client-assigned ID that identifies the bitmap codec
		// 		that was used to encode the bitmap data. Bitmap codec parameters are exchanged in the Bitmap
		//		Codecs Capability Set (section 2.2.7.2.10). If this field is 0, then the bitmap data is not encoded
		//		and can be used without performing any decoding transformation.
		// width (2 bytes): A 16-bit, unsigned integer. The width of the decoded bitmap image in pixels.
		// height (2 bytes): A 16-bit, unsigned integer. The height of the decoded bitmap image in pixels.
		// bitmapDataLength (4 bytes): A 32-bit, unsigned integer. The size in bytes of the bitmapData field.
		// exBitmapDataHeader (variable): An optional Extended Compressed Bitmap Header (section 2.2.9.2.1.1.1) structure
		// 		that contains non essential information associated with bitmap data in the bitmapData field. This field MUST
		//		be present if the EX_COMPRESSED_BITMAP_HEADER_PRESENT (0x01) flag is present.
		// bitmapData (variable): A variable-length array of bytes containing bitmap data encoded using the
		//		codec identified by the ID in the codecID field.

		expected = 10 + 12;
		if (!stream.in_check_rem(expected)) {
			LOG(LOG_ERR, "Truncated SetSurfaceBitsCommand, need=%u remains=%zu", expected, stream.in_remain());
			throw Error(ERR_RDP_DATA_TRUNCATED);
		}

		uint16_t destLeft = stream.in_uint16_le();
		uint16_t destTop = stream.in_uint16_le();
		uint16_t destRight = stream.in_uint16_le();
		uint16_t destBottom = stream.in_uint16_le();
		destRect = Rect(destLeft, destTop, destRight, destBottom);

		bpp = stream.in_uint8();
		flags = stream.in_uint8();
		stream.in_skip_bytes(1);
		codecId = stream.in_uint8();
		width = stream.in_uint16_le();
		height = stream.in_uint16_le();
		bitmapDataLength = stream.in_uint32_le();

		Rect rect(destLeft, destTop, width, height);

		if (flags & EX_COMPRESSED_BITMAP_HEADER_PRESENT) {
			expected = 24;
			if (!stream.in_check_rem(expected)) {
				LOG(LOG_ERR, "Truncated SetSurfaceBitsCommand, need=%u remains=%zu", expected, stream.in_remain());
				throw Error(ERR_RDP_DATA_TRUNCATED);
			}

			/*uint32_t highUniqueId = */stream.in_uint32_le();
			/*uint32_t lowUniqueId = */ stream.in_uint32_le();
			/*uint64_t tmMilliseconds = */stream.in_uint64_le();
			/*uint64_t tmSeconds = */stream.in_uint64_le();
		}

		if (!stream.in_check_rem(bitmapDataLength)) {
			LOG(LOG_ERR, "Truncated SetSurfaceBitsCommand, need=%u remains=%zu", bitmapDataLength, stream.in_remain());
			throw Error(ERR_RDP_DATA_TRUNCATED);
		}
	}

	void log(int level, const RDPSurfaceContent &/*content*/) const {
        LOG(level, "RDPSetSurfaceCommand");
	}

public:
	Rect destRect;
	uint8_t bpp;
	uint8_t flags;
	uint8_t codecId;
	uint16_t width, height;

	uint32_t bitmapDataLength;

	uint32_t highUniqueId;
	uint32_t lowUniqueId;
	uint64_t tmMilliseconds;
	uint64_t tmSeconds;
};


