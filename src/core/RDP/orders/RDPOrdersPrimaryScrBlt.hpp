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


#pragma once

#include "core/RDP/orders/RDPOrdersCommon.hpp"

// 2.2.2.2.1.1.2.7 ScrBlt (SCRBLT_ORDER)
// =====================================

// The ScrBlt Primary Drawing Order is used to perform a bit-block transfer from
// a source region to a destination region. The source surface is always the
// primary drawing surface, while the target surface is the current target
// surface, specified by the Switch Surface Alternate Secondary Drawing Order
// (section 2.2.2.2.1.3.3).

// Encoding order number: 2 (0x02)
// Negotiation order number: 2 (0x02)
// Number of fields: 7
// Number of field encoding bytes: 1
// Maximum encoded field length: 13 bytes

// nLeftRect (variable): The left coordinate of the destination rectangle
// specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nTopRect (variable): The top coordinate of the destination rectangle
// specified by using a Coord Field (section 2.2.2.2.1.1.1.1).

// nWidth (variable): The width of the destination rectangle specified by using
// a Coord Field (section 2.2.2.2.1.1.1.1).

// nHeight (variable): The height of the destination rectangle specified by
// using a Coord Field (section 2.2.2.2.1.1.1.1).

// bRop (1 byte): The index of the ternary raster operation to perform (see
// section 2.2.2.2.1.1.1.7). The resultant ROP3 operation MUST only depend on
// the destination and source bits. (There MUST NOT be any dependence on pattern
// bits.)

// As scrblt does not involve pattern, only the following subset of rop3 values
// should be provided in rop field fort ScrBlt order.

// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
// |      | RPN: DSon                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x22 | ROP: 0x00220326               |
// |      | RPN: DSna                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
// |      | RPN: Sn                       |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x44 | ROP: 0x00440328 (SRCERASE)    |
// |      | RPN: SDna                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
// |      | RPN: DSx                      |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x77 | ROP: 0x007700E6               |
// |      | RPN: DSan                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x88 | ROP: 0x008800C6 (SRCAND)      |
// |      | RPN: DSa                      |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0x99 | ROP: 0x00990066               |
// |      | RPN: DSxn                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xAA | ROP: 0x00AA0029               |
// |      | RPN: D                        |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
// |      | RPN: DSno                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
// |      | RPN: S                        |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xDD | ROP: 0x00DD0228               |
// |      | RPN: SDno                     |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
// |      | RPN: DSo                      |
// +------+-------------------------------+
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+

// nXSrc (variable): The x-coordinate of the source rectangle specified by using
// a Coord Field (section 2.2.2.2.1.1.1.1).

// nYSrc (variable): The y-coordinate of the source rectangle specified by using
// a Coord Field (section 2.2.2.2.1.1.1.1).


class RDPScrBlt {
    public:
    Rect rect;
    uint8_t rop;
    uint16_t srcx;
    uint16_t srcy;

    static uint8_t id(void)
    {
        return RDP::SCREENBLT;
    }

    RDPScrBlt(const Rect rect, uint8_t rop, uint16_t srcx, uint16_t srcy) :
        rect(rect), rop(rop), srcx(srcx), srcy(srcy)
        {}

    bool operator==(const RDPScrBlt &other) const {
        return  (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->srcx == other.srcx)
             && (this->srcy == other.srcy)
             ;
    }

    void emit(OutStream & stream,
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

    void receive(InStream & stream, const RDPPrimaryOrderHeader & header)
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
        size_t lg = common.str(buffer, sz, !common.clip.contains(this->rect));
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "scrblt(rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d)\n",
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            unsigned(this->rop), this->srcx, this->srcy);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level, const Rect clip) const {
        char buffer[1024];
        this->str(buffer, 1024, RDPOrderCommon(RDP::SCREENBLT, clip));
        LOG(level, "%s", buffer);
    }

    void move(int offset_x, int offset_y) {
        this->rect = this->rect.offset(offset_x, offset_y);

        this->srcx += offset_x;
        this->srcy += offset_y;
    }
};
