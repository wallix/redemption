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

   New RDP Orders Coder / Decoder : Primary Order Scr Blt

*/

#if !defined(__RDPORDERSPRIMARYSCRBLT_HPP__)
#define __RDPORDERSPRIMARYSCRBLT_HPP__

class RDPScrBlt {
    public:
    Rect rect;
    uint8_t rop;
    uint16_t srcx;
    uint16_t srcy;

    RDPScrBlt(Rect rect, uint8_t rop, uint16_t srcx, uint16_t srcy) :
        rect(rect), rop(rop), srcx(srcx), srcy(srcy)
        {}

    bool operator==(const RDPScrBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->srcx == other.srcx)
             && (this->srcy == other.srcy)
             ;
    }

    void emit(Stream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPScrBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // SCREENBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte
        // 0x20: srcx coordinate
        // 0x40: srcy coordinate

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= ((dr.fully_relative()
                   && is_1_byte(this->srcx - oldcmd.srcx)
                   && is_1_byte(this->srcy - oldcmd.srcy))
                                                          * DELTA);

        header.fields = (dr.dleft     != 0               ) * 0x01
                      | (dr.dtop      != 0               ) * 0x02
                      | (dr.dwidth    != 0               ) * 0x04
                      | (dr.dheight   != 0               ) * 0x08
                      | (this->rop    != oldcmd.rop      ) * 0x10
                      | ((this->srcx - oldcmd.srcx) != 0 ) * 0x20
                      | ((this->srcy - oldcmd.srcy) != 0 ) * 0x40
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->rop);
        }

        header.emit_src(stream, 0x20,
                        this->srcx, this->srcy,
                        oldcmd.srcx, oldcmd.srcy);

    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }

        header.receive_src(stream, 0x20, this->srcx, this->srcy);
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "scrblt(rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            this->rop, this->srcx, this->srcy);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};


#endif
