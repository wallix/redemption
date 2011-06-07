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

   New RDP Orders Coder / Decoder : Primary Order Mem Blt

*/

#if !defined(__RDPORDERSPRIMARYMEMBLT_HPP__)
#define __RDPORDERSPRIMARYMEMBLT_HPP__

class RDPMemBlt {
    public:
    uint16_t cache_id;
    Rect rect;
    uint8_t rop;
    uint16_t srcx;
    uint16_t srcy;
    uint16_t cache_idx;

    RDPMemBlt(uint16_t cache_id, Rect rect, uint8_t rop, uint16_t srcx, uint16_t srcy, uint16_t cache_idx) :
        cache_id(cache_id),
        rect(rect),
        rop(rop),
        srcx(srcx),
        srcy(srcy),
        cache_idx(cache_idx)
        {
        }

    bool operator==(const RDPMemBlt &other) const {
        return  (this->cache_id == other.cache_id)
             && (this->rect == other.rect)
             && (this->rop == other.rop)
             && (this->srcx == other.srcx)
             && (this->srcy == other.srcy)
             && (this->cache_idx == other.cache_idx)
             ;
    }

    void emit(Stream & stream,
                RDPOrderCommon & common,
                const RDPOrderCommon & oldcommon,
                const RDPMemBlt & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->rect)){
            header.control |= BOUNDS;
        }

        // MEMBLT fields bytes
        // ------------------------------
        // 0x01: cacheId
        // 0x02: x coordinate
        // 0x04: y coordinate
        // 0x08: cx coordinate
        // 0x10: cy coordinate
        // 0x20: rop byte
        // 0x40: srcx
        // 0x80: srcy

        // 0x100: cacheidx

        DeltaRect dr(this->rect, oldcmd.rect);

        // RDP specs says that we can have DELTA only if we
        // have bounds. Can't see the rationale and rdesktop don't do it
        // by the book. Behavior should be checked with server and clients
        // from Microsoft. Looks like an error in RDP specs.
        header.control |= ((dr.fully_relative()
                   && is_1_byte(this->srcx - oldcmd.srcx)
                   && is_1_byte(this->srcy - oldcmd.srcy))
                                                           * DELTA);

        header.fields = (this->cache_id != oldcmd.cache_id  ) * 0x001
                      | (dr.dleft   != 0                    ) * 0x002
                      | (dr.dtop    != 0                    ) * 0x004
                      | (dr.dwidth  != 0                    ) * 0x008
                      | (dr.dheight != 0                    ) * 0x010
                      | (this->rop != oldcmd.rop            ) * 0x020
                      | ((this->srcx - oldcmd.srcx) != 0    ) * 0x040
                      | ((this->srcy - oldcmd.srcy) != 0    ) * 0x080
                      | (this->cache_idx != oldcmd.cache_idx) * 0x100
                      ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x01){
            stream.out_uint16_le(this->cache_id);
        }

        header.emit_rect(stream, 0x02, this->rect, oldcmd.rect);

        if (header.fields & 0x20){
            stream.out_uint8(this->rop);
        }

        header.emit_src(stream, 0x40, this->srcx, this->srcy, oldcmd.srcx, oldcmd.srcy);

        if (header.fields & 0x100){
            stream.out_uint16_le(this->cache_idx);
        }
    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x01) {
            this->cache_id = stream.in_uint16_le();
        }

        header.receive_rect(stream, 0x02, this->rect);

        if (header.fields & 0x20) {
            this->rop = stream.in_uint8();
        }

        header.receive_src(stream, 0x40, this->srcx, this->srcy);

        if (header.fields & 0x100) {
            this->cache_idx = stream.in_uint16_le();
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg  += snprintf(
            buffer+lg,
            sz-lg,
            "memblt(cache_id=%d rect(%d,%d,%d,%d) rop=%x srcx=%d srcy=%d cache_idx=%d)\n",
            this->cache_id,
            this->rect.x, this->rect.y, this->rect.cx, this->rect.cy,
            this->rop,
            this->srcx, this->srcy,
            this->cache_idx);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }


};


#endif
