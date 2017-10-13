/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Primary Order Opaque Rect

*/


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"



// 2.2.2.2.1.1.2.21 DrawNineGrid (DRAWNINEGRID_ORDER)
//
// The DrawNineGrid Primary Drawing Order encodes a single NineGrid drawing command with a single
// bounding rectangle.
//
//      Encoding order number: 7 (0x07)
//      Negotiation order number: 7 (0x07)
//      Number of fields: 5
//      Number of field encoding bytes: 1
//      Maximum encoded field length: 10 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          ulLeftWidth          |         ulRightWidth          |
// +-------------------------------+-------------------------------+
// |          ulTopHeight          |         ulBottomHeight        |
// +-------------------------------+-------------------------------+
// |                            bitmapId                           |
// +---------------------------------------------------------------+

// srcLeft (variable): The left coordinate of the clipping rectangle to be applied to the bitmap stored
// at the entry given by the bitmapId field. The coordinate is specified by using a Coord Field
// (section 2.2.2.2.1.1.1.1).
//
// srcTop (variable): The top coordinate of the clipping rectangle to be applied to the bitmap stored at
// the entry given by the bitmapId field. The coordinate is specified by using a Coord Field
// (section 2.2.2.2.1.1.1.1).
//
// srcRight (variable): The right coordinate of the clipping rectangle to be applied to the bitmap stored
// at the entry given by the bitmapId field. The coordinate is specified by using a Coord Field
// (section 2.2.2.2.1.1.1.1).
//
// srcBottom (variable): The bottom coordinate of the clipping rectangle to be applied to the bitmap stored
// at the entry given by the bitmapId field. The coordinate is specified by using a Coord Field
// (section 2.2.2.2.1.1.1.1).
//
// bitmapId (2 bytes): A 16-bit, unsigned integer. The index of the NineGrid Bitmap Cache entry wherein
// the bitmap and NineGrid transformation information are stored. This value MUST be greater than or equal
// to 0 and less than the maximum number of entries allowed in the NineGrid Bitmap Cache as specified by
// the drawNineGridCacheEntries field of the DrawNineGrid Cache Capability Set (section 2.2.1.2). The bitmap
// and transformation information stored in the cache MUST have already been cached in response to a Create
// NineGrid Bitmap (section 2.2.2.2.1.3.4) Alternate Secondary Drawing Order.



class RDPNineGrid {
public:
    uint16_t ulLeftWidth = 0;
    uint16_t ulRightWidth = 0;
    uint16_t ulTopHeight = 0;
    uint16_t ulBottomHeight = 0;
    uint16_t bitmapId = 0;

    RDPNineGrid() = default;

    RDPNineGrid(uint16_t ulLeftWidth,
                uint16_t ulRightWidth,
                uint16_t ulTopHeight,
                uint16_t ulBottomHeight,
                uint16_t bitmapId)
        : ulLeftWidth(ulLeftWidth)
        , ulRightWidth(ulRightWidth)
        , ulTopHeight(ulTopHeight)
        , ulBottomHeight(ulBottomHeight)
        , bitmapId(bitmapId)
        {}


    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->ulLeftWidth);
        stream.out_uint16_le(this->ulRightWidth);
        stream.out_uint16_le(this->ulTopHeight);
        stream.out_uint16_le(this->ulBottomHeight);
        stream.out_uint16_le(this->bitmapId);
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
    {
        this->ulLeftWidth = stream.in_uint16_le();
        this->ulRightWidth = stream.in_uint16_le();
        this->ulTopHeight = stream.in_uint16_le();
        this->ulBottomHeight = stream.in_uint16_le();
        this->bitmapId = stream.in_uint16_le();
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
//         size_t lg = common.str(buffer, sz, !common.clip.contains(this->rect));
//         lg += snprintf(
//             buffer+lg,
//             sz-lg,
//             "opaquerect(rect(%d,%d,%d,%d) color=0x%.6x)",
//             this->rect.x, this->rect.y, this->rect.cx, this->rect.cy, this->color.as_bgr().to_u32());
//         if (lg >= sz){
//             return sz;
//         }
        return 0;
    }

    void log(int level, const Rect clip) const {
//         char buffer[1024];
//         this->str(buffer, 1024, RDPOrderCommon(this->id(), clip));
//         LOG(level, "%s", buffer);
    }
};
