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
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/


#pragma once

#include "utils/ellipse.hpp"
#include "RDPOrdersCommon.hpp"
// 2.2.2.2.1.1.1.1 Coord Field (COORD_FIELD)
// =========================================
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

// 2.2.2.2.1.1.2.19 EllipseSC (ELLIPSE_SC_ORDER)
// =============================================
// The EllipseSC Primary Drawing Order encodes a single, solid-color ellipse.

// Encoding order number: 25 (0x19)
// Negotiation order number: 25 (0x19)
// Number of fields: 7
// Number of field encoding bytes: 1
// Maximum encoded field length: 13 bytes

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      LeftRect (variable)      |      TopRect (variable)       |
// +---------------+---------------+---------------+---------------+
// |      RightRect (variable)     |     BottomRect (variable)     |
// +---------------+---------------+---------------+---------------+
// |     bRop2     |    FillMode   |           Color               |
// |   (optional)  |   (optional)  |         (optional)            |
// +---------------+---------------+---------------+---------------+
// |      ...      |
// +---------------+

// LeftRect (variable): The left coordinate of the inclusive rectangle for
//  the ellipse specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// TopRect (variable): The top coordinate of the inclusive rectangle for
//  the ellipse specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// RightRect (variable): The right coordinate of the inclusive rectangle for
//  the ellipse specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// BottomRect (variable): The bottom coordinate of the inclusive rectangle for
//  the ellipse specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// bRop2 (1 byte): The binary raster operation to perform
//  (see section 2.2.2.2.1.1.1.6).

// FillMode (1 byte): An 8-bit, unsigned integer that specifies the fill mode.
//  FillMode MUST be one of the following values.

// +-----------+----------------------------------------------------+
// | Value     | Meaning                                            |
// +-----------+----------------------------------------------------+
// | NOFILLL   | A polyline ellipse (that is, a non-filled ellipse) |
// | 0x00      | MUST be drawn.                                     |
// +-----------+----------------------------------------------------+
// | ALTERNATE | See section 2.2.2.2.1.1.1.9 for an explanation of  |
// | 0x01      | this value.                                        |
// +-----------+----------------------------------------------------+
// | WINDING   | See section 2.2.2.2.1.1.1.9 for an explanation of  |
// | 0x02      | this value.                                        |
// +-----------+----------------------------------------------------+

// Color (3 bytes): The foreground color described by using a Generic Color
//  (section 2.2.2.2.1.1.1.8) structure.


class RDPEllipseSC {
public:
    Ellipse el;
    uint8_t  bRop2;
    uint8_t  fillMode;
    RDPColor color;

    RDPEllipseSC()
    : bRop2(0x0)
    , fillMode(0x0)
    , color(0x000000)
    {}

    RDPEllipseSC(const Rect r, int c)
    : RDPEllipseSC(r, c, 0x0D, 0x01)
    {}

    RDPEllipseSC(const Rect r, int c, uint8_t rop, uint8_t fill)
    : el(r)
    , bRop2(rop)
    , fillMode(fill)
    , color(c)
    {}

    bool operator==(const RDPEllipseSC & other) const {
        return this->el == other.el
            && this->bRop2 == other.bRop2
            && this->fillMode == other.fillMode
            && this->color == other.color;
    }

    static uint8_t id(void) {
        return RDP::ELLIPSESC;
    }

    void emit(OutStream & stream, RDPOrderCommon & common, const RDPOrderCommon & oldcommon,
              const RDPEllipseSC & oldcmd) const {
        RDPPrimaryOrderHeader header(RDP::STANDARD, 0);

        if (!common.clip.contains(this->el.get_rect())){
            header.control |= RDP::BOUNDS;
        }

        const int16_t oldleft   = oldcmd.el.left();
        const int16_t oldtop    = oldcmd.el.top();
        const int16_t oldright  = oldcmd.el.right();
        const int16_t oldbottom = oldcmd.el.bottom();

        header.control |= (is_1_byte(this->el.left() - oldleft) &&
                           is_1_byte(this->el.top() - oldtop) &&
                           is_1_byte(this->el.right() - oldright) &&
                           is_1_byte(this->el.bottom() - oldbottom)) * RDP::DELTA;

        header.fields =
            ( this->el.left()   != oldleft        ) * 0x0001
            |(this->el.top()    != oldtop         ) * 0x0002
            |(this->el.right()  != oldright       ) * 0x0004
            |(this->el.bottom() != oldbottom      ) * 0x0008
            |(this->bRop2    != oldcmd.bRop2   ) * 0x0010
            |(this->fillMode != oldcmd.fillMode) * 0x0020
            |(this->color    != oldcmd.color   ) * 0x0040;

        common.emit(stream, header, oldcommon);
        header.emit_coord(stream, 0x0001, this->el.left(),   oldleft);
        header.emit_coord(stream, 0x0002, this->el.top(),    oldtop);
        header.emit_coord(stream, 0x0004, this->el.right(),  oldright);
        header.emit_coord(stream, 0x0008, this->el.bottom(), oldbottom);

        if (header.fields & 0x0010) { stream.out_uint8(this->bRop2); }

        if (header.fields & 0x0020) { stream.out_uint8(this->fillMode); }

        if (header.fields & 0x0040) {
            stream.out_uint8(this->color);
            stream.out_uint8(this->color >> 8);
            stream.out_uint8(this->color >> 16);
        }

        // LOG(LOG_INFO, "RDPEllipseSC::emit: header fields=0x%02X", header.fields);
        // LOG(LOG_INFO, "RDPEllipseSC::emit: header color=0x%02X", this->color);
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header) {
        // LOG(LOG_INFO, "RDPEllipseSC::receive: header fields=0x%02X", header.fields);
        int16_t leftRect   = this->el.left();
        int16_t topRect    = this->el.top();
        int16_t rightRect  = this->el.right();
        int16_t bottomRect = this->el.bottom();
        header.receive_coord(stream, 0x0001, leftRect);
        header.receive_coord(stream, 0x0002, topRect);
        header.receive_coord(stream, 0x0004, rightRect);
        header.receive_coord(stream, 0x0008, bottomRect);

        this->el = Ellipse(leftRect, topRect, rightRect, bottomRect);

        if (header.fields & 0x0010) {
            this->bRop2  = stream.in_uint8();
        }
        if (header.fields & 0x0020) {
            this->fillMode  = stream.in_uint8();
        }
        if (header.fields & 0x0040) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->color = RDPColor(r + (g << 8) + (b << 16));
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const {
        size_t lg = 0;
        lg += common.str(buffer + lg, sz - lg, true);
        lg += snprintf(buffer + lg, sz - lg,
            "ellipseSC(leftRect=%d topRect=%d rightRect=%d bottomRect=%d bRop2=0x%02X "
            "fillMode=%d Color=%.6x)",
                       this->el.left(), this->el.top(), this->el.right(), this->el.bottom(),
                       unsigned(this->bRop2), this->fillMode, this->color.to_u32());
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
        printf("%s", buffer);
    }
};
