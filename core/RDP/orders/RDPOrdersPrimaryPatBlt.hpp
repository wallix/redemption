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

   New RDP Orders Coder / Decoder : Primary Order Pat Blt

*/

#if !defined(__RDPORDERSPRIMARYPATBLT_HPP__)
#define __RDPORDERSPRIMARYPATBLT_HPP__


class RDPPatBlt {
    public:
    Rect rect;
    uint8_t rop;
    uint32_t back_color;
    uint32_t fore_color;
    RDPBrush brush;

    RDPPatBlt(const Rect rect, uint8_t rop,
              uint32_t back_color, uint32_t fore_color,
              const RDPBrush & brush) :
        rect(rect),
        rop(rop),
        back_color(back_color),
        fore_color(fore_color),
        brush(brush)
        {
        }

    bool operator==(const RDPPatBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->back_color == other.back_color)
             && (this->fore_color == other.fore_color)
             && (this->brush.org_x == other.brush.org_x)
             && (this->brush.org_y == other.brush.org_y)
             && (this->brush.style == other.brush.style)
             && (this->brush.hatch == other.brush.hatch)
             && ((this->brush.style != 0x03)
                || (0 == memcmp(this->brush.extra, other.brush.extra, 7)))
             ;
    }

    void emit(Stream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPPatBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // PATBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: Back color (3 bytes)
        // 0x40: Fore color (3 bytes)
        // 0x80: Brush Org X (1 byte)

        // 0x0100: Brush Org Y (1 byte)
        // 0x0200: Brush style (1 byte)
        // 0x0400: Brush Hatch (1 byte)

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft   != 0) * 0x01
                       | (dr.dtop     != 0) * 0x02
                       | (dr.dwidth   != 0) * 0x04
                       | (dr.dheight  != 0) * 0x08
                       | (this->rop         != oldcmd.rop        ) *  0x10
                       | (this->back_color  != oldcmd.back_color ) *  0x20
                       | (this->fore_color  != oldcmd.fore_color ) *  0x40

                       | (this->brush.org_x != oldcmd.brush.org_x) *  0x80
                       | (this->brush.org_y != oldcmd.brush.org_y) * 0x100
                       | (this->brush.style != oldcmd.brush.style) * 0x200
                       | (this->brush.hatch != oldcmd.brush.hatch) * 0x400
                       | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0) * 0x800
                       ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10) {
            stream.out_uint8(this->rop);
        }
        if (header.fields & 0x20) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x40) {
            stream.out_uint8(this->fore_color);
            stream.out_uint8(this->fore_color >> 8);
            stream.out_uint8(this->fore_color >> 16);
        }

        header.emit_brush(stream, 0x80, this->brush, oldcmd.brush);
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }
        if (header.fields & 0x20) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }
        if (header.fields & 0x40) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->fore_color = r + (g << 8) + (b << 16);
        }

        header.receive_brush(stream, 0x080, this->brush);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);

        if (this->brush.style == 0x3){
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d brush.extra=[%.2x %.2x %.2x %.2x %.2x %.2x %.2x])\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                this->rop,
                this->back_color,
                this->fore_color,
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch,
                this->brush.extra[0],
                this->brush.extra[1],
                this->brush.extra[2],
                this->brush.extra[3],
                this->brush.extra[4],
                this->brush.extra[5],
                this->brush.extra[6]
                );
        }
        else {
            lg += snprintf(
                buffer+lg,
                sz-lg,
                "patblt(rect(%d,%d,%d,%d) rop=%x "
                    "back_color=%x fore_color=%x "
                    "brush.org_x=%d brush.org_y=%d "
                    "brush.style=%d brush.hatch=%d)\n",
                this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
                this->rop,
                this->back_color,
                this->fore_color,
                this->brush.org_x,
                this->brush.org_y,
                this->brush.style,
                this->brush.hatch
                );
        }
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};


#endif
