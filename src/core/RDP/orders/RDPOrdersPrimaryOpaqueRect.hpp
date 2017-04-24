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

class RDPOpaqueRect {
    public:
    Rect rect;
    RDPColor color;

    static uint8_t id(void)
    {
        return RDP::RECT;
    }

    RDPOpaqueRect(const Rect r, RDPColor c) :
        rect(r), color(c)
        {}

    bool operator==(const RDPOpaqueRect &other) const {
        return  (this->rect == other.rect)
             && (this->color == other.color);
    }

    // order to stream returns true if state clip must be changed
    // it does not change state by itself
    void emit(OutStream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPOpaqueRect & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // OPAQUERECT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: red color byte
        // 0x20: green color byte
        // 0x40: blue color byte

        // Note by CGR:
        // ------------
        // As far as we can see the OPAQUERECT fields called "red" "green" and
        // "blue" don't care much for actual color components. Really they
        // should be called "first color byte", "second color byte" and "third
        // color byte". They are red green and blue only in 24 bits. In 15 or 16
        // one byte is always empty and changing green component will
        // change both used bytes.

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        uint32_t const diff_color = this->color.as_bgr().to_u32() ^ oldcmd.color.as_bgr().to_u32();

//        LOG(LOG_INFO, "emit opaque rect old_color = %.6x new_color = %.6x\n", oldcmd.color, this->color);

        header.fields =   (dr.dleft                != 0) * 0x01
                        | (dr.dtop                 != 0) * 0x02
                        | (dr.dwidth               != 0) * 0x04
                        | (dr.dheight              != 0) * 0x08
                        | ((diff_color & 0x0000FF) != 0) * 0x10
                        | ((diff_color & 0x00FF00) != 0) * 0x20
                        | ((diff_color & 0xFF0000) != 0) * 0x40
                        ;

        common.emit(stream, header, oldcommon);
        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->color.as_bgr().red());
        }
        if (header.fields & 0x20){
            stream.out_uint8(this->color.as_bgr().green());
        }
        if (header.fields & 0x40){
            stream.out_uint8(this->color.as_bgr().blue());
        }
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        uint8_t r = this->color.as_bgr().red();
        uint8_t g = this->color.as_bgr().green();
        uint8_t b = this->color.as_bgr().blue();

        if (header.fields & 0x10) {
            r = stream.in_uint8();
        }
        if (header.fields & 0x20) {
            g = stream.in_uint8();
        }
        if (header.fields & 0x40) {
            b = stream.in_uint8();
        }
        this->color = RDPColor::from(BGRColor(b, g, r).to_u32());
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz, !common.clip.contains(this->rect));
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "opaquerect(rect(%d,%d,%d,%d) color=0x%.6x)",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy, this->color.as_bgr().to_u32());
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

    void move(int offset_x, int offset_y) {
        this->rect = this->rect.offset(offset_x, offset_y);
    }
};
