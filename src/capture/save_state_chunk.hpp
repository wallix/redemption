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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "core/RDP/state_chunk.hpp"
#include "utils/stream.hpp"


class SaveStateChunk
{
public:
    SaveStateChunk() = default;

    void recv(InStream & stream, StateChunk & sc, uint8_t info_version) {
        this->send_recv(stream, sc, info_version);
    }

    void send(OutStream & stream, StateChunk & sc) {
        this->send_recv(stream, sc, ~0);
    }

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

    static void io_uint32_le(InStream & stream, RDPColor & value) { value = RDPColor::from(BGRColor(stream.in_uint32_le()).to_u32()); }
    static void io_uint32_le(OutStream & stream, RDPColor value) { stream.out_uint32_le(value.as_bgr().to_u32()); }

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

    static void io_color(InStream & stream, RDPColor & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = RDPColor::from(BGRColor(blue, green, red).to_u32());
    }
    static void io_color(OutStream & stream, RDPColor color) {
        stream.out_uint8(color.as_bgr().red());
        stream.out_uint8(color.as_bgr().green());
        stream.out_uint8(color.as_bgr().blue());
    }

    static void io_copy_bytes(InStream & stream, uint8_t * buf, unsigned n) { stream.in_copy_bytes(buf, n); }
    static void io_copy_bytes(OutStream & stream, uint8_t * buf, unsigned n) { stream.out_copy_bytes(buf, n); }

    template<class Stream>
    static void io_delta_encoded_rects(Stream & stream, array_view<RDP::DeltaEncodedRectangle> delta_rectangles) {
        // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
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
    void send_recv(Stream & stream, StateChunk & sc, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        io_uint8(stream, sc.common.order);
        io_rect(stream, sc.common.clip);

        // RDPDestBlt destblt;
        io_rect(stream, sc.destblt.rect);
        io_uint8(stream, sc.destblt.rop);

        // RDPPatBlt patblt;
        io_rect(stream, sc.patblt.rect);
        io_uint8(stream, sc.patblt.rop);
        io_uint32_le(stream, sc.patblt.back_color);
        io_uint32_le(stream, sc.patblt.fore_color);
        io_brush(stream, sc.patblt.brush);

        // RDPScrBlt scrblt;
        io_rect(stream, sc.scrblt.rect);
        io_uint8(stream, sc.scrblt.rop);
        io_uint16_le(stream, sc.scrblt.srcx);
        io_uint16_le(stream, sc.scrblt.srcy);

        // RDPOpaqueRect opaquerect;
        io_rect(stream, sc.opaquerect.rect);
        io_color(stream, sc.opaquerect.color);

        // RDPMemBlt memblt;
        io_uint16_le(stream, sc.memblt.cache_id);
        io_rect(stream, sc.memblt.rect);
        io_uint8(stream, sc.memblt.rop);
        // TODO bad length
        io_uint8_unsafe(stream, sc.memblt.srcx);
        io_uint8_unsafe(stream, sc.memblt.srcy);
        io_uint16_le(stream, sc.memblt.cache_idx);

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            io_uint16_le(stream, sc.mem3blt.cache_id);
            io_rect(stream, sc.mem3blt.rect);
            io_uint8(stream, sc.mem3blt.rop);
            io_uint8_unsafe(stream, sc.mem3blt.srcx);
            io_uint8_unsafe(stream, sc.mem3blt.srcy);
            io_uint32_le(stream, sc.mem3blt.back_color);
            io_uint32_le(stream, sc.mem3blt.fore_color);
            io_brush(stream, sc.mem3blt.brush);
            io_uint16_le(stream, sc.mem3blt.cache_idx);
        }

        // RDPLineTo lineto;
        io_uint8(stream, sc.lineto.back_mode);
        io_sint16_le(stream, sc.lineto.startx);
        io_sint16_le(stream, sc.lineto.starty);
        io_sint16_le(stream, sc.lineto.endx);
        io_sint16_le(stream, sc.lineto.endy);
        io_uint32_le(stream, sc.lineto.back_color);
        io_uint8(stream, sc.lineto.rop2);
        io_uint8(stream, sc.lineto.pen.style);
        io_uint8(stream, sc.lineto.pen.width);
        io_uint32_le(stream, sc.lineto.pen.color);

        // RDPGlyphIndex glyphindex;
        io_uint8(stream, sc.glyphindex.cache_id);
        io_sint16_le(stream, sc.glyphindex.fl_accel);
        io_sint16_le(stream, sc.glyphindex.ui_charinc);
        io_sint16_le(stream, sc.glyphindex.f_op_redundant);
        io_uint32_le(stream, sc.glyphindex.back_color);
        io_uint32_le(stream, sc.glyphindex.fore_color);
        io_rect(stream, sc.glyphindex.bk);
        io_rect(stream, sc.glyphindex.op);
        io_brush(stream, sc.glyphindex.brush);
        io_sint16_le(stream, sc.glyphindex.glyph_x);
        io_sint16_le(stream, sc.glyphindex.glyph_y);
        io_uint8(stream, sc.glyphindex.data_len);
        io_copy_bytes(stream, sc.glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            io_sint16_le(stream, sc.polyline.xStart);
            io_sint16_le(stream, sc.polyline.yStart);
            io_uint8(stream, sc.polyline.bRop2);
            io_uint16_le(stream, sc.polyline.BrushCacheEntry);
            io_uint32_le(stream, sc.polyline.PenColor);
            io_uint8(stream, sc.polyline.NumDeltaEntries);
            // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
            for (uint8_t i = 0; i < sc.polyline.NumDeltaEntries; i++) {
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].xDelta);
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].yDelta);
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            io_sint16_le(stream, sc.multidstblt.nLeftRect);
            io_sint16_le(stream, sc.multidstblt.nTopRect);
            io_sint16_le(stream, sc.multidstblt.nWidth);
            io_sint16_le(stream, sc.multidstblt.nHeight);
            io_uint8(stream, sc.multidstblt.bRop);
            io_uint8(stream, sc.multidstblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multidstblt.deltaEncodedRectangles,
                sc.multidstblt.nDeltaEntries
            });
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            io_sint16_le(stream, sc.multiopaquerect.nLeftRect);
            io_sint16_le(stream, sc.multiopaquerect.nTopRect);
            io_sint16_le(stream, sc.multiopaquerect.nWidth);
            io_sint16_le(stream, sc.multiopaquerect.nHeight);
            io_color(stream, sc.multiopaquerect._Color);
            io_uint8(stream, sc.multiopaquerect.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiopaquerect.deltaEncodedRectangles,
                sc.multiopaquerect.nDeltaEntries
            });
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            io_rect(stream, sc.multipatblt.rect);
            io_uint8(stream, sc.multipatblt.bRop);
            io_uint32_le(stream, sc.multipatblt.BackColor);
            io_uint32_le(stream, sc.multipatblt.ForeColor);
            io_brush(stream, sc.multipatblt.brush);
            io_uint8(stream, sc.multipatblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multipatblt.deltaEncodedRectangles,
                sc.multipatblt.nDeltaEntries
            });
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            io_rect(stream, sc.multiscrblt.rect);
            io_uint8(stream, sc.multiscrblt.bRop);
            io_sint16_le(stream, sc.multiscrblt.nXSrc);
            io_sint16_le(stream, sc.multiscrblt.nYSrc);
            io_uint8(stream, sc.multiscrblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiscrblt.deltaEncodedRectangles,
                sc.multiscrblt.nDeltaEntries
            });
        }
    }
};
