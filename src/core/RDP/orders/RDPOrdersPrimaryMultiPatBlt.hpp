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


#pragma once

#include "RDPOrdersCommon.hpp"
#include "utils/rect.hpp"

namespace RDP {

// [MS-RDPEGDI] - 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// ========================================================
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
// +---------------+---------------+-------------------------------+
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
// +---------------------------------------------------------------+

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

class RDPMultiPatBlt {
public:
    Rect       rect {}; //nLeftRect, nTopRect, nWidth, nHeight
    uint8_t    bRop {};
    RDPColor   BackColor {};
    RDPColor   ForeColor {};
    RDPBrush   brush {}; // BrushOrgX , BrushOrgY , BrushStyle , BrushHatch , BrushExtra
    uint8_t    nDeltaEntries {};

    DeltaEncodedRectangle deltaEncodedRectangles[45] {};

    static uint8_t id(void) {
        return MULTIPATBLT;
    }

    RDPMultiPatBlt() = default;

    RDPMultiPatBlt(RDPMultiPatBlt const &) = default;

    RDPMultiPatBlt( const Rect _rect, uint8_t bRop, RDPColor BackColor, RDPColor ForeColor, const RDPBrush & _brush
                  , uint8_t nDeltaEntries, InStream & deltaEncodedRectangles)
    : rect(_rect)
    , bRop(bRop)
    , BackColor(BackColor)
    , ForeColor(ForeColor)
    , brush(_brush)
    , nDeltaEntries(nDeltaEntries) {
        ::memset(this->deltaEncodedRectangles, 0, sizeof(this->deltaEncodedRectangles));
        for (int i = 0; i < this->nDeltaEntries; i++) {
            this->deltaEncodedRectangles[i].leftDelta = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].topDelta  = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].width     = deltaEncodedRectangles.in_sint16_le();
            this->deltaEncodedRectangles[i].height    = deltaEncodedRectangles.in_sint16_le();
        }
    }

    RDPMultiPatBlt & operator=(const RDPMultiPatBlt & other) = default;

    bool operator==(const RDPMultiPatBlt & other) const {
        return (this->rect          == other.rect)
            && (this->bRop          == other.bRop)
            && (this->BackColor     == other.BackColor)
            && (this->ForeColor     == other.ForeColor)
            && (this->brush         == other.brush)
            && (this->nDeltaEntries == other.nDeltaEntries)
            && !memcmp(this->deltaEncodedRectangles, other.deltaEncodedRectangles, sizeof(DeltaEncodedRectangle) * this->nDeltaEntries)
            ;
    }

    void emit( OutStream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon
             , const RDPMultiPatBlt & oldcmd) const {
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // MultiPatBlt field encoding bytes (2)
        // ====================================

        // 0x0001: nLeftRect
        // 0x0002: nTopRect
        // 0x0004: nWidth
        // 0x0008: nHeight
        // 0x0010: bRop
        // 0x0020: BackColor
        // 0x0040: ForeColor
        // 0x0080: BrushOrgX
        // 0x0100: BrushOrgY
        // 0x0200: BrushStyle
        // 0x0400: BrushHatch
        // 0x0800: BrushExtra
        // 0x1000: nDeltaEntries
        // 0x2000: CodedDeltaList

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we have bounds.
        //  Can't see the rationale and rdesktop don't do it by the book.
        //  Behavior should be checked with server and clients from
        //  Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft            != 0                   ) * 0x0001
                      | (dr.dtop             != 0                   ) * 0x0002
                      | (dr.dwidth           != 0                   ) * 0x0004
                      | (dr.dheight          != 0                   ) * 0x0008

                      | (this->bRop          != oldcmd.bRop         ) * 0x0010
                      | (this->BackColor     != oldcmd.BackColor    ) * 0x0020
                      | (this->ForeColor     != oldcmd.ForeColor    ) * 0x0040

                      | (this->brush.org_x   != oldcmd.brush.org_x  ) * 0x0080
                      | (this->brush.org_y   != oldcmd.brush.org_y  ) * 0x0100
                      | (this->brush.style   != oldcmd.brush.style  ) * 0x0200
                      | (this->brush.hatch   != oldcmd.brush.hatch  ) * 0x0400
                      | (::memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0) * 0x0800
                      | (this->nDeltaEntries != oldcmd.nDeltaEntries) * 0x1000
                      | (
                         (this->nDeltaEntries != oldcmd.nDeltaEntries) ||
                         memcmp(this->deltaEncodedRectangles, oldcmd.deltaEncodedRectangles,
                                this->nDeltaEntries * sizeof(DeltaEncodedRectangle))
                                                                    ) * 0x2000
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x0001, this->rect, oldcmd.rect);

        if (header.fields & 0x0010) {
            stream.out_uint8(this->bRop);
        }
        if (header.fields & 0x0020) {
            emit_rdp_color(stream, this->BackColor);
        }
        if (header.fields & 0x0040) {
            emit_rdp_color(stream, this->ForeColor);
        }

        header.emit_brush(stream, 0x0080, this->brush, oldcmd.brush);

        if (header.fields & 0x1000) { stream.out_uint8(this->nDeltaEntries); }

        if (header.fields & 0x2000) {
            uint32_t offset_cbData = stream.get_offset();
            stream.out_clear_bytes(2);

            uint8_t * zeroBit = stream.get_current();
            stream.out_clear_bytes((this->nDeltaEntries + 1) / 2);
            *zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (i && !m2) {
                    *(++zeroBit) = 0;
                }

                if (!this->deltaEncodedRectangles[i].leftDelta) {
                    *zeroBit |= (1 << (7 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].leftDelta);
                }

                if (!this->deltaEncodedRectangles[i].topDelta) {
                    *zeroBit |= (1 << (6 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].topDelta);
                }

                if (!this->deltaEncodedRectangles[i].width) {
                    *zeroBit |= (1 << (5 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].width);
                }

                if (!this->deltaEncodedRectangles[i].height) {
                    *zeroBit |= (1 << (4 - m2 * 4));
                }
                else {
                    stream.out_DEP(this->deltaEncodedRectangles[i].height);
                }
            }

            stream.set_out_uint16_le(stream.get_offset() - offset_cbData - 2, offset_cbData);
        }
    }   // void emit( OutStream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon, const RDPMultiPatBlt & oldcmd) const

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {
        //LOG(LOG_INFO, "RDPMultiPatBlt::receive: header fields=0x%02X", header.fields);

        header.receive_rect(stream, 0x0001, this->rect);

        if (header.fields & 0x0010) {
            this->bRop = stream.in_uint8();
        }
        if (header.fields & 0x0020) {
            receive_rdp_color(stream, this->BackColor);
        }
        if (header.fields & 0x0040) {
            receive_rdp_color(stream, this->ForeColor);
        }

        header.receive_brush(stream, 0x0080, this->brush);

        if (header.fields & 0x1000) {
            this->nDeltaEntries = stream.in_uint8();
        }

        if (header.fields & 0x2000) {
            uint16_t cbData = stream.in_uint16_le();
            //LOG(LOG_INFO, "cbData=%d", cbData);

            InStream rgbData(stream.get_current(), cbData);
            stream.in_skip_bytes(cbData);
            //hexdump_d(rgbData.get_current(), rgbData.get_capacity());

            uint8_t zeroBitsSize = ((this->nDeltaEntries + 1) / 2);
            //LOG(LOG_INFO, "zeroBitsSize=%d", zeroBitsSize);

            InStream zeroBits(rgbData.get_current(), zeroBitsSize);
            rgbData.in_skip_bytes(zeroBitsSize);

            uint8_t zeroBit = 0;

            for (uint8_t i = 0, m2 = 0; i < this->nDeltaEntries; i++, m2++) {
                if (m2 == 2) {
                    m2 = 0;
                }

                if (!m2) {
                    zeroBit = zeroBits.in_uint8();
                    //LOG(LOG_INFO, "0x%02X", zeroBit);
                }

                this->deltaEncodedRectangles[i].leftDelta = (!(zeroBit & 0x80) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].topDelta  = (!(zeroBit & 0x40) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].width     = (!(zeroBit & 0x20) ? rgbData.in_DEP() : 0);
                this->deltaEncodedRectangles[i].height    = (!(zeroBit & 0x10) ? rgbData.in_DEP() : 0);

                //LOG(LOG_INFO, "RDPMultiPatBlt::receive: delta rectangle=(%d, %d, %d, %d)",
                //    this->deltaEncodedRectangles[i].leftDelta, this->deltaEncodedRectangles[i].topDelta,
                //    this->deltaEncodedRectangles[i].width, this->deltaEncodedRectangles[i].height);

                zeroBit <<= 4;
            }
        }
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
                      , this->rect.x, this->rect.y, this->rect.cx, this->rect.cy, unsigned(this->bRop)
                      , this->BackColor.to_u32(), this->ForeColor.to_u32()
                      , this->brush.org_x, this->brush.org_y
                      , unsigned(this->brush.style), unsigned(this->brush.hatch)
                      , unsigned(this->brush.extra[0]), unsigned(this->brush.extra[1])
                      , unsigned(this->brush.extra[2]), unsigned(this->brush.extra[3])
                      , unsigned(this->brush.extra[4]), unsigned(this->brush.extra[5])
                      , unsigned(this->brush.extra[6]), this->nDeltaEntries);
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

    void log(int level, const Rect clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }

    void print(const Rect clip) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer), RDPOrderCommon(this->id(), clip));
        buffer[sizeof(buffer) - 1] = 0;
        printf("%s\n", buffer);
    }

    void move(int offset_x, int offset_y) {
        this->rect = this->rect.offset(offset_x, offset_y);
    }
};  // class RDPMultiPatBlt

}   // namespace RDP

