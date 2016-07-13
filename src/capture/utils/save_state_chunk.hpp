/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2012-2015
*   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
*              Jonathan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "utils/stream.hpp"

class SaveStateChunk {
public:
    RDPOrderCommon          common;
    RDPDestBlt              destblt;
    RDPMultiDstBlt          multidstblt;
    RDPMultiOpaqueRect      multiopaquerect;
    RDP::RDPMultiPatBlt     multipatblt;
    RDP::RDPMultiScrBlt     multiscrblt;
    RDPPatBlt               patblt;
    RDPScrBlt               scrblt;
    RDPOpaqueRect           opaquerect;
    RDPMemBlt               memblt;
    RDPMem3Blt              mem3blt;
    RDPLineTo               lineto;
    RDPGlyphIndex           glyphindex;
    RDPPolyline             polyline;
    RDPEllipseSC            ellipseSC;

public:
    SaveStateChunk()
        : common(RDP::PATBLT, Rect(0, 0, 1, 1))
        , destblt(Rect(), 0)
        , multidstblt()
        , multiopaquerect()
        , multipatblt()
        , multiscrblt()
        , patblt(Rect(), 0, 0, 0, RDPBrush())
        , scrblt(Rect(), 0, 0, 0)
        , opaquerect(Rect(), 0)
        , memblt(0, Rect(), 0, 0, 0, 0)
        , mem3blt(0, Rect(), 0, 0, 0, 0, 0, RDPBrush(), 0)
        , lineto(0, 0, 0, 0, 0, 0, 0, RDPPen(0, 0, 0))
        , glyphindex(0, 0, 0, 0, 0, 0, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                    , reinterpret_cast<const uint8_t *>(""))
        , polyline()
        , ellipseSC()
    {}

private:
    static void io_uint8(InStream & stream, uint8_t & value) { value = stream.in_uint8(); }
    static void io_uint8(OutStream & stream, uint8_t value) { stream.out_uint8(value); }

    static void io_sint8(InStream & stream, int8_t & value) { value = stream.in_sint8(); }
    static void io_sint8(OutStream & stream, int8_t value) { stream.out_sint8(value); }

    // TODO BUG this is an error
    static void io_uint8_unsafe(InStream & stream, uint16_t & value) { value = stream.in_uint8(); }
    static void io_uint8_unsafe(OutStream & stream, uint16_t value) { stream.out_uint8(value); }

    static void io_uint16_le(InStream & stream, uint16_t & value) { value = stream.in_uint16_le(); }
    static void io_uint16_le(OutStream & stream, uint16_t value) { stream.out_uint16_le(value); }

    static void io_sint16_le(InStream & stream, int16_t & value) { value = stream.in_sint16_le(); }
    static void io_sint16_le(OutStream & stream, int16_t value) { stream.out_sint16_le(value); }

    static void io_uint32_le(InStream & stream, uint32_t & value) { value = stream.in_uint32_le(); }
    static void io_uint32_le(OutStream & stream, uint32_t value) { stream.out_uint32_le(value); }

    static void io_color(InStream & stream, uint32_t & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = red | green << 8 | blue << 16;
    }
    static void io_color(OutStream & stream, uint32_t color) {
        stream.out_uint8(color);
        stream.out_uint8(color >> 8);
        stream.out_uint8(color >> 16);
    }

    static void io_copy_bytes(InStream & stream, uint8_t * buf, unsigned n) { stream.in_copy_bytes(buf, n); }
    static void io_copy_bytes(OutStream & stream, uint8_t * buf, unsigned n) { stream.out_copy_bytes(buf, n); }

    template<class Stream>
    static void io_delta_encoded_rects(Stream & stream, array_view<RDP::DeltaEncodedRectangle> delta_rectangles) {
        for (RDP::DeltaEncodedRectangle & delta_rectangle : delta_rectangles) {
            io_sint16_le(stream, delta_rectangle.leftDelta);
            io_sint16_le(stream, delta_rectangle.topDelta);
            io_sint16_le(stream, delta_rectangle.width);
            io_sint16_le(stream, delta_rectangle.height);
        }
    }

    template<class Stream>
    static void io_brush(Stream & stream, RDPBrush & brush) {
        io_sint8(stream, brush.org_x);
        io_sint8(stream, brush.org_y);
        io_uint8(stream, brush.style);
        io_uint8(stream, brush.hatch);
        io_copy_bytes(stream, brush.extra, 7);
    }

    template<class Stream>
    static void io_rect(Stream & stream, Rect & rect) {
        io_sint16_le(stream, rect.x);
        io_sint16_le(stream, rect.y);
        io_uint16_le(stream, rect.cx);
        io_uint16_le(stream, rect.cy);
    }

    template<class Stream>
    void send_recv(Stream & stream, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        io_uint8(stream, this->common.order);
        io_rect(stream, this->common.clip);

        // RDPDestBlt destblt;
        io_rect(stream, this->destblt.rect);
        io_uint8(stream, this->destblt.rop);

        // RDPPatBlt patblt;
        io_rect(stream, this->patblt.rect);
        io_uint8(stream, this->patblt.rop);
        io_uint32_le(stream, this->patblt.back_color);
        io_uint32_le(stream, this->patblt.fore_color);
        io_brush(stream, this->patblt.brush);

        // RDPScrBlt scrblt;
        io_rect(stream, this->scrblt.rect);
        io_uint8(stream, this->scrblt.rop);
        io_uint16_le(stream, this->scrblt.srcx);
        io_uint16_le(stream, this->scrblt.srcy);

        // RDPOpaqueRect opaquerect;
        io_rect(stream, this->opaquerect.rect);
        io_color(stream, this->opaquerect.color);

        // RDPMemBlt memblt;
        io_uint16_le(stream, this->memblt.cache_id);
        io_rect(stream, this->memblt.rect);
        io_uint8(stream, this->memblt.rop);
        // TODO bad length
        io_uint8_unsafe(stream, this->memblt.srcx);
        io_uint8_unsafe(stream, this->memblt.srcy);
        io_uint16_le(stream, this->memblt.cache_idx);

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            io_uint16_le(stream, this->mem3blt.cache_id);
            io_rect(stream, this->mem3blt.rect);
            io_uint8(stream, this->mem3blt.rop);
            io_uint8_unsafe(stream, this->mem3blt.srcx);
            io_uint8_unsafe(stream, this->mem3blt.srcy);
            io_uint32_le(stream, this->mem3blt.back_color);
            io_uint32_le(stream, this->mem3blt.fore_color);
            io_brush(stream, this->mem3blt.brush);
            io_uint16_le(stream, this->mem3blt.cache_idx);
        }

        // RDPLineTo lineto;
        io_uint8(stream, this->lineto.back_mode);
        io_sint16_le(stream, this->lineto.startx);
        io_sint16_le(stream, this->lineto.starty);
        io_sint16_le(stream, this->lineto.endx);
        io_sint16_le(stream, this->lineto.endy);
        io_uint32_le(stream, this->lineto.back_color);
        io_uint8(stream, this->lineto.rop2);
        io_uint8(stream, this->lineto.pen.style);
        io_uint8(stream, this->lineto.pen.width);
        io_uint32_le(stream, this->lineto.pen.color);

        // RDPGlyphIndex glyphindex;
        io_uint8(stream, this->glyphindex.cache_id);
        io_sint16_le(stream, this->glyphindex.fl_accel);
        io_sint16_le(stream, this->glyphindex.ui_charinc);
        io_sint16_le(stream, this->glyphindex.f_op_redundant);
        io_uint32_le(stream, this->glyphindex.back_color);
        io_uint32_le(stream, this->glyphindex.fore_color);
        io_rect(stream, this->glyphindex.bk);
        io_rect(stream, this->glyphindex.op);
        io_brush(stream, this->glyphindex.brush);
        io_sint16_le(stream, this->glyphindex.glyph_x);
        io_sint16_le(stream, this->glyphindex.glyph_y);
        io_uint8(stream, this->glyphindex.data_len);
        io_copy_bytes(stream, this->glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            io_sint16_le(stream, this->polyline.xStart);
            io_sint16_le(stream, this->polyline.yStart);
            io_uint8(stream, this->polyline.bRop2);
            io_uint16_le(stream, this->polyline.BrushCacheEntry);
            io_uint32_le(stream, this->polyline.PenColor);
            io_uint8(stream, this->polyline.NumDeltaEntries);
            for (uint8_t i = 0; i < this->polyline.NumDeltaEntries; i++) {
                io_sint16_le(stream, this->polyline.deltaEncodedPoints[i].xDelta);
                io_sint16_le(stream, this->polyline.deltaEncodedPoints[i].yDelta);
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            io_sint16_le(stream, this->multidstblt.nLeftRect);
            io_sint16_le(stream, this->multidstblt.nTopRect);
            io_sint16_le(stream, this->multidstblt.nWidth);
            io_sint16_le(stream, this->multidstblt.nHeight);
            io_uint8(stream, this->multidstblt.bRop);
            io_uint8(stream, this->multidstblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                this->multidstblt.deltaEncodedRectangles,
                this->multidstblt.nDeltaEntries
            });
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            io_sint16_le(stream, this->multiopaquerect.nLeftRect);
            io_sint16_le(stream, this->multiopaquerect.nTopRect);
            io_sint16_le(stream, this->multiopaquerect.nWidth);
            io_sint16_le(stream, this->multiopaquerect.nHeight);
            io_color(stream, this->multiopaquerect._Color);
            io_uint8(stream, this->multiopaquerect.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                this->multiopaquerect.deltaEncodedRectangles,
                this->multiopaquerect.nDeltaEntries
            });
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            io_rect(stream, this->multipatblt.rect);
            io_uint8(stream, this->multipatblt.bRop);
            io_uint32_le(stream, this->multipatblt.BackColor);
            io_uint32_le(stream, this->multipatblt.ForeColor);
            io_brush(stream, this->multipatblt.brush);
            io_uint8(stream, this->multipatblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                this->multipatblt.deltaEncodedRectangles,
                this->multipatblt.nDeltaEntries
            });
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            io_rect(stream, this->multiscrblt.rect);
            io_uint8(stream, this->multiscrblt.bRop);
            io_sint16_le(stream, this->multiscrblt.nXSrc);
            io_sint16_le(stream, this->multiscrblt.nYSrc);
            io_uint8(stream, this->multiscrblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                this->multiscrblt.deltaEncodedRectangles,
                this->multiscrblt.nDeltaEntries
            });
        }
    }

public:
    void recv(InStream & stream, uint8_t info_version) {
        this->send_recv(stream, info_version);
    }

    void send(OutStream & stream) {
        this->send_recv(stream, ~0);
    }
};
