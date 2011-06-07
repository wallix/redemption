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

#if !defined(__RDPORDERSPRIMARYOPAQUERECT_HPP__)
#define __RDPORDERSPRIMARYOPAQUERECT_HPP__


class RDPOpaqueRect {
    public:
    Rect rect;
    uint32_t color;

    RDPOpaqueRect(Rect rect, int color) :
        rect(rect), color(color)
        {}

    bool operator==(const RDPOpaqueRect &other) const {
        return  (this->rect == other.rect)
             && (this->color == other.color);
    }

    // order to stream returns true if state clip must be changed
    // it does not change state by itself
    void emit(Stream & stream,
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

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        uint32_t diff_color = this->color ^ oldcmd.color;

//        LOG(LOG_INFO, "emit opaque rect old_color = %.6x new_color = %.6x\n", oldcmd.color, this->color);

        header.fields =   (dr.dleft                != 0) * 0x01
                        | (dr.dtop                 != 0) * 0x02
                        | (dr.dwidth               != 0) * 0x04
                        | (dr.dheight              != 0) * 0x08
                        | ((diff_color & 0xFF)     != 0) * 0x10
                        | ((diff_color & 0xFF00)   != 0) * 0x20
                        | ((diff_color & 0xFF0000) != 0) * 0x40
                        ;

        #warning dirty hack to fix color problems with opaque_rect. It seems I will have to pass in bpp if I want to fix it. (And how it should be fixed is not obvious as - as far I can see - support for 16bpp and 15bpp is undocumented in official documentation provided by Microsoft). It seems I will have to guess.
        if (diff_color) { header.fields |= 0x70; }
        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->color);
        }
        if (header.fields & 0x20){
                stream.out_uint8(this->color >> 8);
        }
        if (header.fields & 0x40){
                stream.out_uint8(this->color >> 16);
        }
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        uint32_t old_color = this->color;

        if (header.fields & 0x10) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0xffff00) | i;
        }
        if (header.fields & 0x20) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0xff00ff) | (i << 8);
        }
        if (header.fields & 0x40) {
            unsigned i = stream.in_uint8();
            this->color = (this->color & 0x00ffff) | (i << 16);
        }

//        LOG(LOG_INFO, "receive opaque rect old_color = %.6x new_color = %.6x\n", old_color, this->color);

    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "opaquerect(rect(%d,%d,%d,%d) color=%x)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy, this->color);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }


};


#endif
