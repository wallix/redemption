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
    Copyright (C) Wallix 2013
    Author(s): Clément Moroldo
*/


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"

// [MS-RDPEGDI] - 2.2.2.2.1.1.2.12 SaveBitmap (SAVEBITMAP_ORDER)

// The SaveBitmap Primary Drawing Order encodes a rectangle of the screen image for saving or restoring by the client.
//
//      Encoding order number: 11 (0x0B)
//      Negotiation order number: 11 (0x0B)
//      Number of fields: 6
//      Number of field encoding bytes: 1
//      Maximum encoded field length: 13 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                SavedBitmapPosition (optional)                 |
// +-------------------------------+-------------------------------+
// |      nLeftRect (variable)     |      nTopRect (variable)      |
// +-------------------------------+-------------------------------+
// |     nRightRect (variable)     |    nBottomRect (variable)     |
// +---------------+---------------+---------------+---------------+
// |                   CodedDeltaList (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// SavedBitmapPosition (4 bytes): A 32-bit, unsigned integer. Encoded start position of the rectangle in the Saved Bitmap that will be read from (in the case of a bitmap restore operation) or written to (in the case of a bitmap save operation), depending on the value of the Operation field.
//
//     The SavedBitmapPosition field is constructed by using the desktopSaveXGranularity and desktopSaveYGranularity values specified in the Order Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3). The size of the Save Bitmap MUST be 480 x 480 pixels.
//
//     Specification [T128] section 8.16.17 shows how to compute the value to insert into the SavedBitmapPosition field. When performing a save operation, the SavedBitmapPosition field contains the cumulative area of the virtual desktop rectangles already in the Save Bitmap (the area of the rectangle being written to the Save Bitmap is excluded). When performing a restore operation, the SavedBitmapPosition field contains the cumulative area of all the rectangles that were written to the Save Bitmap before the rectangle being restored was saved.
//
//     The functions used to compute the area by using the specified X and Y granularity are as follows.
//
//          AreaWidthInPixels = [(width + XGranularity - 1) / XGranularity] * XGranularity
//          AreaHeightInPixels = [(height + YGranularity - 1) / YGranularity] * YGranularity
//          Area = AreaWidthInPixels * AreaHeightInPixels
//
//     To determine the X and Y position in the Save Bitmap using the YGranularity and the Save Bitmap width of 480, the following functions are used.
//
//          Y = [SaveBitmapPosition / (480 * YGranularity)] * YGranularity
//          X = [SaveBitmapPosition - (Y * 480)] / YGranularity
//
//     An example of calculations to obtain the X and Y positions from the SavedBitmapPosition field are defined visually in section 4.5.
//
// nLeftRect (variable): The left coordinate of the virtual desktop rectangle to save specified by using a Coord Field (section 2.2.2.2.1.1.1.1).
//
// nTopRect (variable): The top coordinate of the virtual desktop rectangle to save specified by using a Coord Field (section 2.2.2.2.1.1.1.1).
//
// nRightRect (variable): The right inclusive coordinate of the virtual desktop rectangle to save specified by using a Coord Field (section 2.2.2.2.1.1.1.1).
//
// nBottomRect (variable): The bottom inclusive coordinate of the virtual desktop rectangle to save specified by using a Coord Field (section 2.2.2.2.1.1.1.1).
//
// Operation (1 byte): An 8-bit, unsigned integer. The operation to perform that MUST be one of the following values.

//  +---------------------+---------------------------------------------------------+
//  | Value               | Meaning                                                 |
//  +---------------------+---------------------------------------------------------+
//  | SV_SAVEBITS         | Save bitmap operation.                                  |
//  | 0x00                |                                                         |
//  +---------------------+---------------------------------------------------------+
//  | SV_RESTOREBITS      | Restore bitmap operation.                               |
//  | 0x01                |                                                         |
//  +---------------------+---------------------------------------------------------+


class RDPSaveBitmap {
public:
    uint32_t SavedBitmapPosition{0};
    uint16_t nLeftRect{0};
    uint16_t nTopRect{0};
    uint16_t nRightRect{0};
    uint16_t nBottomRect{0};
    uint8_t Operation{0};

    RDPSaveBitmap()
     
    = default;

    RDPSaveBitmap(uint32_t SavedBitmapPosition, int16_t nLeftRect, int16_t nTopRect, int16_t nRightRect, int16_t nBottomRect, uint8_t Operation)
    : SavedBitmapPosition(SavedBitmapPosition)
    , nLeftRect(nLeftRect)
    , nTopRect(nTopRect)
    , nRightRect(nRightRect)
    , nBottomRect(nBottomRect)
    , Operation(Operation)
    {}

    void emit( OutStream & stream) const {

    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {

    }   

    void log(int level, const Rect clip) const {
//         char buffer[2048];
//         this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
//         buffer[sizeof(buffer) - 1] = 0;
//         LOG(level, "%s", buffer);
    }


};  // class RDPMultiOpaqueRect
