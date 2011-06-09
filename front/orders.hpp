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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__ORDERS_HPP__)
#define __ORDERS_HPP__

#include "font.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "rect.hpp"
#include "NewRDPOrders.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include <algorithm>
#include "altoco.hpp"
#include "bitmap.hpp"

namespace RDP {

enum compression_type_t {
    NOT_COMPRESSED,
    COMPRESSED,
    COMPRESSED_SMALL_HEADERS,
    NEW_NOT_COMPRESSED,
    NEW_COMPRESSED
};

struct Orders
{
    // State
    RDPOrderCommon common;
    RDPDestBlt destblt;
    RDPPatBlt patblt;
    RDPScrBlt scrblt;
    RDPOpaqueRect opaquerect;
    RDPMemBlt memblt;
    RDPLineTo lineto;
    RDPGlyphIndex glyph_index;

    Stream out_stream;

    uint8_t* order_count_ptr;
    int order_count;
    int order_level;

    Orders() :
        common(0, Rect(0, 0, 1, 1)),
        destblt(Rect(), 0),
        patblt(Rect(), 0, 0, 0, RDPBrush()),
        scrblt(Rect(), 0, 0, 0),
        opaquerect(Rect(), 0),
        memblt(0, Rect(), 0, 0, 0, 0),
        lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0)),
        glyph_index(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)""),
        out_stream(16384)
    {
        this->order_count = 0;
        this->order_level = 0;
    }

    ~Orders()
    {
    }

    void reset_xx() throw (Error)
    {
        common = RDPOrderCommon(0,  Rect(0, 0, 1, 1));
        memblt = RDPMemBlt(0, Rect(), 0, 0, 0, 0);
        opaquerect = RDPOpaqueRect(Rect(), 0);
        scrblt = RDPScrBlt(Rect(), 0, 0, 0);
        destblt = RDPDestBlt(Rect(), 0);
        patblt = RDPPatBlt(Rect(), 0, 0, 0, RDPBrush());
        lineto = RDPLineTo(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0));
        this->glyph_index = RDPGlyphIndex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0, (uint8_t*)"");
        common.order = PATBLT;

        this->order_count = 0;
        this->order_level = 0;
    }

    int init()
    {
        this->order_level++;
        if (this->order_level == 1) {
            this->order_count = 0;
        }
        return 0;
    }


    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(RECT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->opaquerect);
        this->common = newcommon;
        this->opaquerect = cmd;
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(SCREENBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->scrblt);
        this->common = newcommon;
        this->scrblt = cmd;
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect &clip)
    {
        RDPOrderCommon newcommon(DESTBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->destblt);
        this->common = newcommon;
        this->destblt = cmd;
    }

    void pat_blt(const RDPPatBlt & cmd, const Rect &clip)
    {
        RDPOrderCommon newcommon(PATBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->patblt);
        this->common = newcommon;
        this->patblt = cmd;
    }

    void mem_blt(const RDPMemBlt & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(MEMBLT, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->memblt);
        this->common = newcommon;
        this->memblt = cmd;
    }

    void line_to(const RDPLineTo & cmd, const Rect & clip)
    {
        RDPOrderCommon newcommon(LINE, clip);
        cmd.emit(this->out_stream, newcommon, this->common, this->lineto);
        this->common = newcommon;
        this->lineto = cmd;
    }

};
} /* namespaces */

#endif
