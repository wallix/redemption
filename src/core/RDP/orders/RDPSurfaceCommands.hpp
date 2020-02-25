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
#include "utils/hexdump.hpp"
#include "utils/region.hpp"
#include "core/error.hpp"
#include "core/stream_throw_helpers.hpp"


/** @brief a surface content update */
class RDPSurfaceContent {
public:
    RDPSurfaceContent(uint16_t width, uint16_t height, uint16_t stride, const Rect &rect,
            const SubRegion &region, array_view_const_u8 content)
    : stride(stride)
    , width(width)
    , data(new uint8_t[stride * height * 4]())
    , rect(rect)
    , region(region)
    , encodedContent(content)
    {
    }

    ~RDPSurfaceContent() {
        delete[] data;
    }

public:
    uint16_t stride;
    uint16_t width;
    uint8_t *data;
    Rect rect;
    const SubRegion &region;
    array_view_const_u8 encodedContent;
};

/** @brief a SetSurface command */
class RDPSetSurfaceCommand {
public:
    /** @brief flags */
    enum {
        EX_COMPRESSED_BITMAP_HEADER_PRESENT = 0x1
    };

    enum SetSurfaceCodec {
        SETSURFACE_CODEC_UNKNOWN,
        SETSURFACE_CODEC_REMOTEFX
    };

    void recv(InStream & stream) {
        // 2.2.9.2.1 Set Surface Bits Command (TS_SURFCMD_SET_SURF_BITS)
        
        // The Set Surface Bits Command is used to transport encoded bitmap data
        //  destined for a rectangular region of the primary drawing surface
        //  from an RDP server to an RDP client.
        //
        // cmdType (2 bytes): A 16-bit, unsigned integer. Surface Command type.
        //   This field MUST be set to CMDTYPE_SET_SURFACE_BITS (0x0001).
        
        // destLeft (2 bytes): A 16-bit, unsigned integer. Left bound of the
        //   destination rectangle that will contain the decoded bitmap data.
        
        // destTop (2 bytes): A 16-bit, unsigned integer. Top bound of the
        //   destination rectangle that will contain the decoded bitmap data.
        
        // destRight (2 bytes): A 16-bit, unsigned integer. Exclusive right
        //   bound of the destination rectangle that will contain the decoded
        //   bitmap data. This field SHOULD be ignored, as the width of the
        //   encoded bitmap image is specified in the Extended Bitmap Data
        //   (section 2.2.9.2.1.1) present in the variable-length bitmapData
        //   field.
        
        // destBottom (2 bytes): A 16-bit, unsigned integer. Exclusive bottom
        //   bound of the destination rectangle that will contain the decoded
        //   bitmap data. This field SHOULD be ignored, as the height of the
        //   encoded bitmap image is specified in the Extended Bitmap Data
        //   present in the variable-length bitmapData field.

        // bitmapData (variable): An Extended Bitmap Data structure that
        //   contains an encoded bitmap image.

        // 2.2.9.2.1.1 Extended Bitmap Data (TS_ BITMAP_DATA_EX)
        // The TS_BITMAP_DATA_EX structure is used to encapsulate encoded bitmap data.
        //
        // bpp (1 byte): An 8-bit, unsigned integer. The color depth of the
        //   bitmap data in bits-per-pixel.
        
        // flags (1 byte): An 8-bit, unsigned integer that contains flags.
        // +-------------------------------------+-----------------------------+
        // |               Flag                  |    Meaning                  |
        // +-------------------------------------+-----------------------------+
        // | EX_COMPRESSED_BITMAP_HEADER_PRESENT | Indicates that the optional |
        // |               0x01                  | exBitmapDataHeader field    |
        // |                                     | is present.                 |
        // +-------------------------------------+-----------------------------+
        //
        // reserved (1 byte): An 8-bit, unsigned integer. This field is reserved
        //   for future use. It MUST be set to zero.
        // codecID (1 byte): An 8-bit, unsigned integer. The client-assigned ID
        //   that identifies the bitmap codec that was used to encode the bitmap
        //   data. Bitmap codec parameters are exchanged in the Bitmap Codecs
        //   Capability Set (section 2.2.7.2.10). If this field is 0, then the
        //   bitmap data is not encoded and can be used without performing any
        //   decoding transformation.
        // width (2 bytes): A 16-bit, unsigned integer. The width of the decoded
        //   bitmap image in pixels.
        // height (2 bytes): A 16-bit, unsigned integer. The height of the 
        //   decoded bitmap image in pixels.
        // bitmapDataLength (4 bytes): A 32-bit, unsigned integer. The size in
        //   bytes of the bitmapData field.
        // exBitmapDataHeader (variable): An optional Extended Compressed Bitmap
        //   Header (section 2.2.9.2.1.1.1) structure that contains non 
        //   essential information associated with bitmap data in the bitmapData
        //   field. This field MUST be present if the 
        //   EX_COMPRESSED_BITMAP_HEADER_PRESENT (0x01) flag is present.
        // bitmapData (variable): A variable-length array of bytes containing
        //   bitmap data encoded using the codec identified by the ID in the
        //   codecID field.

        ::check_throw(stream, 10 + 12, "RDPSetSurfaceCommand::recv SetSurfaceBitsCommand", ERR_RDP_DATA_TRUNCATED);

        uint16_t destLeft = stream.in_uint16_le();
        uint16_t destTop = stream.in_uint16_le();
        uint16_t destRight = stream.in_uint16_le();
        uint16_t destBottom = stream.in_uint16_le();
        this->destRect = Rect(destLeft, destTop, destRight, destBottom);

        this->bpp = stream.in_uint8();
        this->flags = stream.in_uint8();
        stream.in_skip_bytes(1);
        this->codecId = stream.in_uint8();
        this->width = stream.in_uint16_le();
        this->height = stream.in_uint16_le();
        this->bitmapDataLength = stream.in_uint32_le();

        if (this->flags & EX_COMPRESSED_BITMAP_HEADER_PRESENT) {

            ::check_throw(stream, 24, "RDPSetSurfaceCommand::recv SetSurfaceBitsCommand EX_COMPRESSED_BITMAP_HEADER_PRESENT", ERR_RDP_DATA_TRUNCATED);

            this->highUniqueId = stream.in_uint32_le();
            this->lowUniqueId = stream.in_uint32_le();
            this->tmMilliseconds = stream.in_uint64_le();
            this->tmSeconds = stream.in_uint64_le();
        }

        ::check_throw(stream, bitmapDataLength, "RDPSetSurfaceCommand::recv SetSurfaceBitsCommand bitmapDataLength", ERR_RDP_DATA_TRUNCATED);

        // TODO: Check stream lifespan, we are getting a pointer on the stream inside the object, looks dangerous
        this->bitmapData = stream.get_current();
    }

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->destRect.ileft());
        stream.out_uint16_le(this->destRect.itop());
        stream.out_uint16_le(this->destRect.eright());
        stream.out_uint16_le(this->destRect.ebottom());

        stream.out_uint8(this->bpp);
        stream.out_uint8(this->flags);
        stream.out_uint8(0); /* reserved */
        stream.out_uint8(this->codecId);
        stream.out_uint16_le(this->width);
        stream.out_uint16_le(this->height);
        stream.out_uint32_le(this->bitmapDataLength);

        if (flags & EX_COMPRESSED_BITMAP_HEADER_PRESENT) {
            stream.out_uint32_le(this->highUniqueId);
            stream.out_uint32_le(this->lowUniqueId);
            stream.out_uint64_le(this->tmMilliseconds);
            stream.out_uint64_le(this->tmSeconds);
        }

        stream.out_copy_bytes(this->bitmapData, this->bitmapDataLength);
    }

    void log(int level, bool dump) const {
        LOG(level, "RDPSurfaceCommand: CMDTYPE_SET_SURFACE_BITS [%s] width=%u heigh=%u bitmapDataLength=%u bpp=%u flags=%.2x codecId=%.2x",
            this->destRect, this->width, this->height, this->bitmapDataLength,
            this->bpp, this->flags, this->codecId);
        if (this->flags & EX_COMPRESSED_BITMAP_HEADER_PRESENT) {
            LOG(level, "RDPSurfaceCommand:  hUI=%.8x lUI=%.8x tm.s=%lu tm.ms=%lu",
                this->highUniqueId, this->lowUniqueId, this->tmSeconds, this->tmMilliseconds);
        }
        if (dump){
            hexdump_d(this->bitmapData, this->bitmapDataLength);
        }
    }

public:
    Rect destRect;
    uint8_t bpp;
    uint8_t flags;
    uint8_t codecId;
    uint16_t width;
    uint16_t height;

    uint32_t bitmapDataLength;
    const uint8_t *bitmapData;

    uint32_t highUniqueId;
    uint32_t lowUniqueId;
    uint64_t tmMilliseconds;
    uint64_t tmSeconds;

    SetSurfaceCodec codec{SETSURFACE_CODEC_UNKNOWN};
};


