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

   New RDP Orders Coder / Decoder : Primary Order Line To

*/


#pragma once

#include "RDPOrdersCommon.hpp"

class RDPLineTo {
    // LINE fields bytes
    // ------------------
    // 0x01: Back mode
    // 0x02: x_start coordinate
    // 0x04: y_start coordinate
    // 0x08: x_end coordinate
    // 0x10: y_end byte
    // 0x20: BackColor (3 bytes)
    // 0x40: rop2
    // 0x80: pen style

    // 0x100: pen width
    // 0x200: pen color (3 bytes)

    public:
    uint8_t back_mode;
    int16_t startx;
    int16_t starty;
    int16_t endx;
    int16_t endy;
    RDPColor back_color;
    uint8_t rop2;
    RDPPen pen;

    static uint8_t id(void)
    {
        return RDP::LINE;
    }

    RDPLineTo(uint8_t back_mode,
              int16_t startx, int16_t starty, int16_t endx, int16_t endy,
              RDPColor back_color,
              uint8_t rop2,
              const RDPPen & pen) :
        back_mode(back_mode),
        startx(startx),
        starty(starty),
        endx(endx),
        endy(endy),
        back_color(back_color),
        rop2(rop2),
        pen(pen)
        {
        }

    bool operator==(const RDPLineTo &other) const {
        return  (this->startx == other.startx)
             && (this->starty == other.starty)
             && (this->endx == other.endx)
             && (this->endy == other.endy)
             && (this->rop2 == other.rop2)
             && (this->back_color == other.back_color)
             && (this->pen.style == other.pen.style)
             && (this->pen.width == other.pen.width)
             && (this->pen.color == other.pen.color)
             ;
    }

    void emit(OutStream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPLineTo & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        // TODO check that
        if (!(common.clip.contains_pt(this->startx, this->starty)
           && common.clip.contains_pt(this->endx, this->endy))){
           header.control |= BOUNDS;
        }

        header.control |= (is_1_byte(this->startx - oldcmd.startx)
                 && is_1_byte(this->starty - oldcmd.starty)
                 && is_1_byte(this->endx - oldcmd.endx)
                 && is_1_byte(this->endy - oldcmd.endy)) * DELTA;

        header.fields =
                (this->back_mode  != oldcmd.back_mode  ) * 0x001
              | (this->startx     != oldcmd.startx     ) * 0x002
              | (this->starty     != oldcmd.starty     ) * 0x004
              | (this->endx       != oldcmd.endx       ) * 0x008
              | (this->endy       != oldcmd.endy       ) * 0x010
              | (this->back_color != oldcmd.back_color ) * 0x020
              | (this->rop2       != oldcmd.rop2       ) * 0x040
              | (this->pen.style  != oldcmd.pen.style  ) * 0x080
              | (this->pen.width  != oldcmd.pen.width  ) * 0x100
              | (this->pen.color  != oldcmd.pen.color  ) * 0x200
              ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x001) { stream.out_uint16_le(this->back_mode); }

        header.emit_coord(stream, 0x02, this->startx, oldcmd.startx);
        header.emit_coord(stream, 0x04, this->starty, oldcmd.starty);
        header.emit_coord(stream, 0x08, this->endx,   oldcmd.endx);
        header.emit_coord(stream, 0x10, this->endy,   oldcmd.endy);

        if (header.fields & 0x20) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x40) { stream.out_uint8(this->rop2); }
        header.emit_pen(stream, 0x80, this->pen, oldcmd.pen);
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x001) {
            this->back_mode = stream.in_uint16_le();
        }

        header.receive_coord(stream, 0x002, this->startx);
        header.receive_coord(stream, 0x004, this->starty);
        header.receive_coord(stream, 0x008, this->endx);
        header.receive_coord(stream, 0x010, this->endy);

        if (header.fields & 0x020) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }

        if (header.fields & 0x040) {
            this->rop2 = stream.in_uint8();
        }

        header.receive_pen(stream, 0x080, this->pen);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz, true);

        lg += snprintf(
            buffer+lg,
            sz-lg,
            "lineto(back_mode=%.2x "
                "startx=%d starty=%d endx=%d endy=%d "
                "rop2=%d back_color=%.6x"
                "pen.style=%d pen.width=%d pen.color=%.6x "
                "\n",
            unsigned(this->back_mode),
            this->startx, this->starty, this->endx, this->endy,
            this->rop2,
            this->back_color.to_u32(),
            this->pen.style,
            this->pen.width,
            this->pen.color.to_u32()
            );
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect clip) const {
        char buffer[1024];
        this->str(buffer, 1024, RDPOrderCommon(this->id(), clip));
        LOG(level, "%s", buffer);
    }

    void print(const Rect clip) const {
        char buffer[1024];
        this->str(buffer, 1024, RDPOrderCommon(this->id(), clip));
        printf("%s", buffer);
    }

    void move(int offset_x, int offset_y) {
        this->startx += offset_x;
        this->starty += offset_y;
        this->endx   += offset_x;
        this->endy   += offset_y;
    }
};
