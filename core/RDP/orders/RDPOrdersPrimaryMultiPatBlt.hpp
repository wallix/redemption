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
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIPATBLT_HPP_
#define _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIPATBLT_HPP_

namespace RDP {

// // [MS-RDPEGDI] - 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// ===========================================================
// The COORD_FIELD structure is used to describe a value in the range -32768
//  to 32767.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     signedValue (variable)    |
// +-------------------------------+

// signedValue (variable): A signed, 1-byte or 2-byte value that describes a
//  coordinate in the range -32768 to 32767.

//  When the controlFlags field (see section 2.2.2.2.1.1.2) of the primary
//   drawing order that contains the COORD_FIELD structure has the
//   TS_DELTA_COORDINATES flag (0x10) set, the signedValue field MUST contain
//   a signed 1-byte value. If the TS_DELTA_COORDINATES flag is not set, the
//   signedValue field MUST contain a 2-byte signed value.

//  The 1-byte format contains a signed delta from the previous value of the
//   Coord field. To obtain the new value of the field, the decoder MUST
//   increment the previous value of the field by the signed delta to produce
//   the current value. The 2-byte format is simply the full value of the
//   field that MUST replace the previous value.

// [MS-RDPEGDI] - 2.2.2.2.1.1.2.4 MultiPatBlt (MULTI_PATBLT_ORDER)
// ===============================================================

// The MultiPatBlt Primary Drawing Order is used to paint multiple rectangles
//  by using a specified brush and three-way raster operation.

//  Encoding order number: 16 (0x10)
//  Negotiation order number: 16 (0x10)
//  Number of fields: 14
//  Number of field encoding bytes: 2
//  Maximum encoded field length: 412 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      nLeftRect (variable)     |      nTopRect (variable)      |
// +-------------------------------+-------------------------------+
// |       nWidth (variable)       |       nHeight (variable)      |
// +-------------------------------+-------------------------------+
// |bRop (optional)|              BackColor (optional)             |
// +---------------+-------------------------------+---------------+
// |              ForeColor (optional)             |   BrushOrgX   |
// |                                               |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |   BrushOrgY   |   BrushStyle  |   BrushHatch  |   BrushExtra  |
// |   (optional)  |   (optional)  |   (optional)  |   (optional)  |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +-------------------------------+---------------+---------------+
// |              ...              | nDeltaEntries | CodedDeltaList|
// |                               |   (optional)  |   (variable)  |
// +-------------------------------+---------------+---------------+
// |                              ...                              |
// +-------------------------------+---------------+---------------+

// nLeftRect (variable): The left coordinate of the destination rectangle
//  specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): The top coordinate of the destination rectangle
//  specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nWidth (variable): The width of the destination rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): The height of the destination rectangle specified by
//  using a Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): The index of the ternary raster operation to perform (see
//  section 2.2.2.2.1.1.1.7).

// BackColor (3 bytes): The background color described by using a Generic
//  Color (section 2.2.2.2.1.1.1.8) structure.

// ForeColor (3 bytes): The foreground color described by using a Generic
//  Color (section 2.2.2.2.1.1.1.8) structure.

// BrushOrgX (1 byte): An 8-bit, signed integer. The x-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushOrgY (1 byte): An 8-bit, signed integer. The y-coordinate of the
//  point where the top leftmost pixel of a brush pattern MUST be anchored.

// BrushStyle (1 byte): An 8-bit, unsigned integer. The contents and format
//  of this field are the same as the BrushStyle field of the PatBlt (section
//  2.2.2.2.1.1.2.3) Primary Drawing Order.

// BrushHatch (1 byte): An 8-bit, unsigned integer. The contents and format
//  of this field are the same as the BrushHatch field of the PatBlt (section
//  2.2.2.2.1.1.2.3) Primary Drawing Order.

// BrushExtra (7 bytes): The contents and format of this field are the same
//  as the BrushExtra field of the PatBlt (section 2.2.2.2.1.1.2.3) Primary
//  Drawing Order.

// nDeltaEntries (1 byte): An 8-bit, unsigned integer. The number of bounding
//  rectangles described by the CodedDeltaList field.

// CodedDeltaList (variable): A Two-Byte Header Variable Field (section
//  2.2.2.2.1.1.1.3) structure that encapsulates a Delta-Encoded Rectangles
//  (section 2.2.2.2.1.1.1.5) structure that contains bounding rectangles to
//  use when rendering the order. The number of rectangles described by the
//  Delta-Encoded Rectangles structure is specified by the nDeltaEntries
//  field.

#include "RDPOrdersCommon.hpp"
#include "rect.hpp"

class RDPMultiPatBlt {
public:
    Rect       rect;
    int16_t  & nLeftRect;
    int16_t  & nTopRect;
    uint16_t & nWidth;
    uint16_t & nHeight;
    uint8_t    bRop;
    uint32_t   BackColor;
    uint32_t   ForeColor;
    RDPBrush   brush;
    int8_t   & BrushOrgX;
    int8_t   & BrushOrgY;
    uint8_t  & BrushStyle;
    uint8_t  & BrushHatch;
    uint8_t  (&BrushExtra)[7];
    uint8_t    nDeltaEntries;

    struct DeltaEncodedRectangle deltaEncodedRectangles[45];

    static const uint8_t id(void)
    {
        return RDP::MULTIPATBLT;
    }

    RDPMultiPatBlt()
    : rect(0, 0, 0, 0)
    , nLeftRect(rect.x)
    , nTopRect(rect.y)
    , nWidth(rect.cx)
    , nHeight(rect.cy)
    , bRop(0)
    , BackColor(0)
    , ForeColor(0)
    , brush(0, 0, 0, 0)
    , BrushOrgX(brush.org_x)
    , BrushOrgY(brush.org_y)
    , BrushStyle(brush.style)
    , BrushHatch(brush.hatch)
    , BrushExtra(brush.extra)
    , nDeltaEntries(0) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
    }

    RDPMultiPatBlt( const Rect & _rect, uint8_t bRop, uint32_t BackColor, uint32_t ForeColor, const RDPBrush & _brush
                  , uint8_t nDeltaEntries, Stream & deltaEncodedRectangles)
    : rect(_rect)
    , nLeftRect(rect.x)
    , nTopRect(rect.y)
    , nWidth(rect.cx)
    , nHeight(rect.cy)
    , bRop(bRop)
    , BackColor(BackColor)
    , ForeColor(ForeColor)
    , brush(_brush)
    , BrushOrgX(brush.org_x)
    , BrushOrgY(brush.org_y)
    , BrushStyle(brush.style)
    , BrushHatch(brush.hatch)
    , BrushExtra(brush.extra)
    , nDeltaEntries(0) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
        for (int i = 0; i < this->nDeltaEntries; i++) {
            this->deltaEncodedRectangles[i].leftDelta = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].topDelta  = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].width     = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].height    = deltaEncodedRectangles.in_sint16_le();
        }
    }

    bool operator==(const RDPMultiPatBlt & other) const {
        return (this->rect          == other.rect)
            && (this->bRop          == other.bRop)
            && (this->BackColor     == other.BackColor)
            && (this->ForeColor     == other.ForeColor)
            && (this->brush.org_x   == other.brush.org_x)
            && (this->brush.org_y   == other.brush.org_y)
            && (this->brush.style   == other.brush.style)
            && (this->brush.hatch   == other.brush.hatch)
            && (   (this->brush.style != 0x03)
                || (0 == memcmp(this->brush.extra, other.brush.extra, 7)))
            && (this->nDeltaEntries == other.nDeltaEntries)
            && !memcmp(this->deltaEncodedRectangles, other.deltaEncodedRectangles, sizeof(DeltaEncodedRectangle) * this->nDeltaEntries)
            ;
    }

    void emit( Stream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon
             , const RDPMultiPatBlt & oldcmd) const {
        RDPPrimaryOrderHeader header(RDP::STANDARD, 0);
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header) {
        //LOG(LOG_INFO, "RDPMultiPatBlt::receive: header fields=0x%02X", header.fields);

        header.receive_rect(stream, 0x01, this->rect);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = 0;
        lg += common.str(buffer + lg, sz - lg, true);
        lg += snprintf( buffer + lg, sz - lg
                      , "MultiPatBlt(nLeftRect=%d nTopRect=%d nWidth=%d nHeight=%d bRop=0x%02X "
                        "BackColor=%x ForeColor=%x "
                        "BrushOrgX=%d BrushOrgY=%d BrushStyle=%u BrushHatch=%u "
                        "BrushExtra=[%.2x %.2x %.2x %.2x %.2x %.2x %.2x] "
                        "nDeltaEntries=%d "
                        "CodedDeltaList=("
                      , this->nLeftRect, this->nTopRect, this->nWidth, this->nHeight, this->bRop
                      , this->BackColor, this->ForeColor
                      , this->BrushOrgX, this->BrushOrgY, this->BrushStyle, this->BrushHatch
                      , this->BrushExtra[0], this->BrushExtra[1], this->BrushExtra[2], this->BrushExtra[3]
                      , this->BrushExtra[4], this->BrushExtra[5], this->BrushExtra[6]
                      , this->nDeltaEntries);
        for (uint8_t i = 0; i < this->nDeltaEntries; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }
            lg += snprintf(buffer + lg, sz - lg, "(%d, %d, %d, %d)", this->deltaEncodedRectangles[i].leftDelta,
                this->deltaEncodedRectangles[i].topDelta, this->deltaEncodedRectangles[i].width,
                this->deltaEncodedRectangles[i].height);
        }
        lg += snprintf(buffer + lg, sz - lg, "))");
        if (lg >= sz) {
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }

    void print(const Rect & clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        printf("%s", buffer);
    }
};  // class RDPMultiPatBlt

}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSPRIMARYMULTIPATBLT_HPP_
