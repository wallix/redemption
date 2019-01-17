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

   New RDP Orders Coder / Decoder : Primary Order Dest Blt

*/


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"

//2.2.2.2.1.1.2.1 DstBlt (DSTBLT_ORDER)
// ------------------------------------

// The DstBlt Primary Drawing Order is used to paint a rectangle by using a
// destination-only raster operation.

// Encoding order number: 0 (0x00)
// Negotiation order number: 0 (0x00)
// Number of fields: 5
// Number of field encoding bytes: 1
// Maximum encoded field length: 9 bytes

// nLeftRect (variable): Left coordinate of the destination rectangle specified
//                       using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): Top coordinate of the destination rectangle specified
//                      using a Coord Field (section 2.2.2.2.1.1.1.1)

// nWidth (variable): Width of the destination rectangle specified by using a
//                    Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): Height of the destination rectangle specified by using a
//                     Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): Index of the ternary raster operation to perform (see section
//                2.2.2.2.1.1.1.7 below). The resultant ROP3 operation MUST only
//                depend on the destination bits (there MUST NOT be any
//                dependence on source or pattern bits).

//                ie: Only 4 possible values:
//                      0x00 = BLACKNESS,
//                      0x55 = Dn (DSTINVERT),
//                      0xAA = D (do nothing)
//                      0xFF = WHITENESS

class RDPDestBlt {
    public:
    Rect rect;
    uint8_t rop;

    static uint8_t id()
    {
        return RDP::DESTBLT;
    }

    RDPDestBlt(const Rect r, uint8_t rop) :
        rect(r), rop(rop)
        {}

    bool operator==(const RDPDestBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             ;
    }

    void emit(OutStream & stream,
              RDPOrderCommon & common,
              const RDPOrderCommon & oldcommon,
              const RDPDestBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // DESTBLT fields bytes (1 byte)
        // ------------------------------
        // 0x01: x coordinate
        // 0x02: y coordinate
        // 0x04: cx coordinate
        // 0x08: cy coordinate
        // 0x10: rop byte

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= dr.fully_relative() * DELTA;

        header.fields = (dr.dleft   != 0) * 0x01
                      | (dr.dtop    != 0) * 0x02
                      | (dr.dwidth  != 0) * 0x04
                      | (dr.dheight != 0) * 0x08
                      | (this->rop  != oldcmd.rop) * 0x10
                      ;

        common.emit(stream, header, oldcommon);

        header.emit_rect(stream, 0x01, this->rect, oldcmd.rect);

        if (header.fields & 0x10){
            stream.out_uint8(this->rop);
        }
    }

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        header.receive_rect(stream, 0x01, this->rect);

        if (header.fields & 0x10) {
            this->rop = stream.in_uint8();
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz, !common.clip.contains(this->rect));
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "destblt(rect(%d,%d,%d,%d) rop=%x)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            unsigned(this->rop));
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
