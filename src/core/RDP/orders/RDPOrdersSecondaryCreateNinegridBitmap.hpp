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
    Copyright (C) Wallix 2014
    Author(s): Clément Moroldo
*/


#pragma once

#include "utils/log.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"



// 2.2.2.2.1.3.4 Create NineGrid Bitmap (CREATE_NINEGRID_BITMAP_ORDER)
//
// The Create NineGrid Bitmap Alternate Secondary Drawing Order is used by the server
// to instruct the client to create a NineGrid bitmap of a particular width and height
// in the NineGrid Bitmap Cache (the color depth MUST be 32 bpp). Support for NineGrid
// drawing is specified in the DrawNineGrid Cache Capability Set (section 2.2.1.2).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     header    |   BitmapBpp   |           BitmapId            |
// +---------------+---------------+-------------------------------+
// |              cx               |               cy              |
// +-------------------------------+-------------------------------+
// |                     nineGridInfo (16 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// header (1 byte): An Alternate Secondary Order Header, as defined in section 2.2.2.2.1.3.1.1.
// The embedded orderType field MUST be set to TS_ALTSEC_CREATE_NINEGRID_BITMAP (0x04).
//
// BitmapBpp (1 byte): An 8-bit, unsigned integer. The color depth in bits per pixel of the
// NineGrid bitmap to create. Currently, all NineGrid bitmaps are sent in 32 bpp, so this field
// MUST be set to 0x20.
//
// BitmapId (2 bytes): A 16-bit, unsigned integer. The index of the NineGrid Bitmap Cache entry
// wherein the bitmap and NineGrid transformation information MUST be stored. This value MUST be
// greater than or equal to 0 and less than the maximum number of entries allowed in the NineGrid
//  Bitmap Cache as specified by the drawNineGridCacheEntries field of the DrawNineGrid Cache
// Capability Set (section 2.2.1.2).
//
// cx (2 bytes): A 16-bit, unsigned integer. The width in pixels of the NineGrid bitmap to create.
//
// cy (2 bytes): A 16-bit, unsigned integer. The height in pixels of the NineGrid bitmap to create.
//
// nineGridInfo (16 bytes): A NineGrid Bitmap Information (section 2.2.2.2.1.3.4.1) structure that
// describes properties of the NineGrid bitmap to be created.




// 2.2.2.2.1.3.4.1 NineGrid Bitmap Information (NINEGRID_BITMAP_INFO)
//
// The NINEGRID_BITMAP_INFO structure is used to describe a NineGrid source bitmap (see section 4.4).
// For more information about NineGrid bitmaps, see [NINEGRID].

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                            flFlags                            |
// +-------------------------------+-------------------------------+
// |          ulLeftWidth          |         ulRightWidth          |
// +-------------------------------+-------------------------------+
// |          ulTopHeight          |         ulBottomHeight        |
// +-------------------------------+-------------------------------+
// |                         crTransparent                         |
// +---------------------------------------------------------------+

// flFlags (4 bytes): A 32-bit, unsigned integer. Option flags for the NineGrid bitmap represented by
// this structure.

//  +---------------------+---------------------------------------------------------+
//  | Value               | Meaning                                                 |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_STRETCH       | Indicates that the center portion of the source bitmap  |
//  | 0x00000001          | MUST be stretched to fill the center of the destination |
//  |                     | NineGrid.                                               |
//  |                     |                                                         |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_TILE          | Indicates that the center portion source bitmap MUST be |
//  | 0x00000002          | tiled to fill the center of the destination NineGrid.   |                                                   //  |                     |                                                         |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_PERPIXELALPHA | Indicates that an AlphaBlend operation MUST be used to  |
//  | 0x00000004          | compose the destination NineGrid. The source bitmap is  |
//  |                     | expected to have per-pixel alpha values. For a          |
//  |                     | description of the AlphaBlend operation, see            |
//  |                     | [MSDN-ABLEND].                                          |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_TILE          | Indicates that a TransparentBlt operation MUST be used  |
//  | 0x00000008          | to compose the destination NineGrid. The crTransparent  |
//  |                     | field MUST contain the transparent color. For a         |
//  |                     | description of the TransparentBlt operation, see        |
//  |                     | [MSDN-TransparentBlt].                                  |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_TILE          | Indicates that the source NineGrid MUST be flipped on a |
//  | 0x00000010          | vertical axis.                                          |
//  +---------------------+---------------------------------------------------------+
//  | DSDNG_TILE          | Indicates that the source bitmap MUST be transferred    |
//  | 0x00000020          | without stretching or tiling.                           |
//  +---------------------+---------------------------------------------------------+

//     If the DSDNG_TILE (0x00000002) flag is not set, the DSDNG_STRETCH (0x00000001) flag is implied.
//     If neither the DSDNG_PERPIXELALPHA (0x00000004) nor DSDNG_TRANSPARENT (0x00000008) is indicated, a BitBlt
//     operation MUST be applied. For a description of the BitBlt operation, see [MSDN-BitBlt].
//
// ulLeftWidth (2 bytes): A 16-bit, unsigned integer. The width of the left-side NineGrid border.
// For a visual illustration of this field, see section 4.4.
//
// ulRightWidth (2 bytes): A 16-bit, unsigned integer. The width of the right-side NineGrid border.
// For a visual illustration of this field, see section 4.4.
//
// ulTopHeight (2 bytes): A 16-bit, unsigned integer. The height of the top NineGrid border. For a
// visual illustration of this field, see section 4.4.
//
// ulBottomHeight (2 bytes): A 16-bit, unsigned integer. The height of the bottom NineGrid border. For
// a visual illustration of this field, see section 4.4.
//
// crTransparent (4 bytes): The RGB color in the source bitmap to treat as transparent represented using
// a Color Reference (section 2.2.2.2.1.3.4.1.1) structure. This field is used if the DSDNG_TRANSPARENT
// (0x00000008) flag is set in the flFlags field.


class CreateNineGridBitmap {
public:

    uint8_t controlFlags;

    uint8_t BitmapBpp;
    uint16_t BitmapId;
    uint16_t cx;
    uint16_t cy;

    uint32_t flFlags;
    uint16_t ulLeftWidth;
    uint16_t ulRightWidth;
    uint16_t ulTopHeight;
    uint16_t ulBottomHeight;
    uint32_t crTransparent;

    CreateNineGridBitmap() : controlFlags(RDP::AltsecDrawingOrderHeader::CreateNinegridBitmap) {}

    explicit CreateNineGridBitmap(                          //uint8_t controlFlags,
                                  uint16_t BitmapId,
                                  uint16_t cx,
                                  uint16_t cy,
                                  uint32_t flFlags,
                                  uint16_t ulLeftWidth,
                                  uint16_t ulRightWidth,
                                  uint16_t ulTopHeight,
                                  uint16_t ulBottomHeight,
                                  uint32_t crTransparent)
                            : controlFlags(RDP::AltsecDrawingOrderHeader::CreateNinegridBitmap)
                            , BitmapBpp(0x20)
                            , BitmapId(BitmapId)
                            , cx(cx)
                            , cy(cy)
                            , flFlags(flFlags)
                            , ulLeftWidth(ulLeftWidth)
                            , ulRightWidth(ulRightWidth)
                            , ulTopHeight(ulTopHeight)
                            , ulBottomHeight(ulBottomHeight)
                            , crTransparent(crTransparent) {}

    void emit(OutStream & stream) const {
        stream.out_uint8(this->controlFlags);

        stream.out_uint8(this->BitmapBpp);
        stream.out_uint16_le(this->BitmapId);
        stream.out_uint16_le(this->cx);
        stream.out_uint16_le(this->cy);

        stream.out_uint32_le(this->flFlags);
        stream.out_uint16_le(this->ulLeftWidth);
        stream.out_uint16_le(this->ulRightWidth);
        stream.out_uint16_le(this->ulTopHeight);
        stream.out_uint16_le(this->ulBottomHeight);
        stream.out_uint32_le(this->crTransparent);
    }

    void receive(InStream & stream) {
        this->controlFlags = stream.in_uint8();

        this->BitmapBpp = stream.in_uint8();
        this->BitmapId = stream.in_uint16_le();
        this->cx = stream.in_uint16_le();
        this->cy = stream.in_uint16_le();

        this->flFlags = stream.in_uint32_le();
        this->ulLeftWidth = stream.in_uint16_le();
        this->ulRightWidth = stream.in_uint16_le();
        this->ulTopHeight = stream.in_uint16_le();
        this->ulBottomHeight = stream.in_uint16_le();
        this->crTransparent = stream.in_uint32_le();
    }

    size_t str(char * buffer, size_t sz) const {
//         size_t lg  = snprintf( buffer
//                              , sz
//                              , "RDPFrameMarker(action=%s(%u))\n"
//                              , (  (this->action == FrameStart)
//                                 ? "FrameStart"
//                                 : (  (this->action == FrameEnd)
//                                    ? "FrameEnd"
//                                    : "<Unexpected>"
//                                   )
//                                )
//                              , this->action);
//         if (lg >= sz) {
//             return sz;
//         }
        return 0;
    }

    void log(int level) const {
//         char buffer[1024];
//         this->str(buffer, 1024);
//         LOG(level, "%s", buffer);
    }
};  // class CreateNineGridBitmap


