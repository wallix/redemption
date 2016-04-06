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
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
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

    void recv(InStream & stream, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        this->common.order = stream.in_uint8();
        this->common.clip.x = stream.in_uint16_le();
        this->common.clip.y = stream.in_uint16_le();
        this->common.clip.cx = stream.in_uint16_le();
        this->common.clip.cy = stream.in_uint16_le();

        // RDPDestBlt destblt;
        this->destblt.rect.x = stream.in_uint16_le();
        this->destblt.rect.y = stream.in_uint16_le();
        this->destblt.rect.cx = stream.in_uint16_le();
        this->destblt.rect.cy = stream.in_uint16_le();
        this->destblt.rop = stream.in_uint8();

        // RDPPatBlt patblt;
        this->patblt.rect.x = stream.in_uint16_le();
        this->patblt.rect.y = stream.in_uint16_le();
        this->patblt.rect.cx = stream.in_uint16_le();
        this->patblt.rect.cy = stream.in_uint16_le();
        this->patblt.rop = stream.in_uint8();
        this->patblt.back_color = stream.in_uint32_le();
        this->patblt.fore_color = stream.in_uint32_le();
        this->patblt.brush.org_x = stream.in_uint8();
        this->patblt.brush.org_y = stream.in_uint8();
        this->patblt.brush.style = stream.in_uint8();
        this->patblt.brush.hatch = stream.in_uint8();
        stream.in_copy_bytes(this->patblt.brush.extra, 7);

        // RDPScrBlt scrblt;
        this->scrblt.rect.x = stream.in_uint16_le();
        this->scrblt.rect.y = stream.in_uint16_le();
        this->scrblt.rect.cx = stream.in_uint16_le();
        this->scrblt.rect.cy = stream.in_uint16_le();
        this->scrblt.rop = stream.in_uint8();
        this->scrblt.srcx = stream.in_uint16_le();
        this->scrblt.srcy = stream.in_uint16_le();

        // RDPOpaqueRect opaquerect;
        this->opaquerect.rect.x  = stream.in_uint16_le();
        this->opaquerect.rect.y  = stream.in_uint16_le();
        this->opaquerect.rect.cx = stream.in_uint16_le();
        this->opaquerect.rect.cy = stream.in_uint16_le();
        {
            uint8_t red              = stream.in_uint8();
            uint8_t green            = stream.in_uint8();
            uint8_t blue             = stream.in_uint8();
            this->opaquerect.color = red | green << 8 | blue << 16;
        }

        // RDPMemBlt memblt;
        this->memblt.cache_id = stream.in_uint16_le();
        this->memblt.rect.x  = stream.in_uint16_le();
        this->memblt.rect.y  = stream.in_uint16_le();
        this->memblt.rect.cx = stream.in_uint16_le();
        this->memblt.rect.cy = stream.in_uint16_le();
        this->memblt.rop = stream.in_uint8();
        this->memblt.srcx    = stream.in_uint8();
        this->memblt.srcy    = stream.in_uint8();
        this->memblt.cache_idx = stream.in_uint16_le();

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            this->mem3blt.cache_id    = stream.in_uint16_le();
            this->mem3blt.rect.x      = stream.in_uint16_le();
            this->mem3blt.rect.y      = stream.in_uint16_le();
            this->mem3blt.rect.cx     = stream.in_uint16_le();
            this->mem3blt.rect.cy     = stream.in_uint16_le();
            this->mem3blt.rop         = stream.in_uint8();
            this->mem3blt.srcx        = stream.in_uint8();
            this->mem3blt.srcy        = stream.in_uint8();
            this->mem3blt.back_color  = stream.in_uint32_le();
            this->mem3blt.fore_color  = stream.in_uint32_le();
            this->mem3blt.brush.org_x = stream.in_uint8();
            this->mem3blt.brush.org_y = stream.in_uint8();
            this->mem3blt.brush.style = stream.in_uint8();
            this->mem3blt.brush.hatch = stream.in_uint8();
            stream.in_copy_bytes(this->mem3blt.brush.extra, 7);
            this->mem3blt.cache_idx   = stream.in_uint16_le();
        }

        // RDPLineTo lineto;
        this->lineto.back_mode = stream.in_uint8();
        this->lineto.startx = stream.in_uint16_le();
        this->lineto.starty = stream.in_uint16_le();
        this->lineto.endx = stream.in_uint16_le();
        this->lineto.endy = stream.in_uint16_le();
        this->lineto.back_color = stream.in_uint32_le();
        this->lineto.rop2 = stream.in_uint8();
        this->lineto.pen.style = stream.in_uint8();
        this->lineto.pen.width = stream.in_sint8();
        this->lineto.pen.color = stream.in_uint32_le();

        // RDPGlyphIndex glyphindex;
        this->glyphindex.cache_id  = stream.in_uint8();
        this->glyphindex.fl_accel  = stream.in_sint16_le();
        this->glyphindex.ui_charinc  = stream.in_sint16_le();
        this->glyphindex.f_op_redundant = stream.in_sint16_le();
        this->glyphindex.back_color = stream.in_uint32_le();
        this->glyphindex.fore_color = stream.in_uint32_le();
        this->glyphindex.bk.x  = stream.in_uint16_le();
        this->glyphindex.bk.y  = stream.in_uint16_le();
        this->glyphindex.bk.cx = stream.in_uint16_le();
        this->glyphindex.bk.cy = stream.in_uint16_le();
        this->glyphindex.op.x  = stream.in_uint16_le();
        this->glyphindex.op.y  = stream.in_uint16_le();
        this->glyphindex.op.cx = stream.in_uint16_le();
        this->glyphindex.op.cy = stream.in_uint16_le();
        this->glyphindex.brush.org_x = stream.in_uint8();
        this->glyphindex.brush.org_y = stream.in_uint8();
        this->glyphindex.brush.style = stream.in_uint8();
        this->glyphindex.brush.hatch = stream.in_uint8();
        stream.in_copy_bytes(this->glyphindex.brush.extra, 7);
        this->glyphindex.glyph_x = stream.in_sint16_le();
        this->glyphindex.glyph_y = stream.in_sint16_le();
        this->glyphindex.data_len = stream.in_uint8();
        stream.in_copy_bytes(this->glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            this->polyline.xStart          = stream.in_sint16_le();
            this->polyline.yStart          = stream.in_sint16_le();
            this->polyline.bRop2           = stream.in_uint8();
            this->polyline.BrushCacheEntry = stream.in_uint16_le();
            this->polyline.PenColor        = stream.in_uint32_le();
            this->polyline.NumDeltaEntries = stream.in_uint8();
            for (uint8_t i = 0; i < this->polyline.NumDeltaEntries; i++) {
                this->polyline.deltaEncodedPoints[i].xDelta = stream.in_sint16_le();
                this->polyline.deltaEncodedPoints[i].yDelta = stream.in_sint16_le();
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            this->multidstblt.nLeftRect     = stream.in_sint16_le();
            this->multidstblt.nTopRect      = stream.in_sint16_le();
            this->multidstblt.nWidth        = stream.in_sint16_le();
            this->multidstblt.nHeight       = stream.in_sint16_le();
            this->multidstblt.bRop          = stream.in_uint8();
            this->multidstblt.nDeltaEntries = stream.in_uint8();
            for (uint8_t i = 0; i < this->multidstblt.nDeltaEntries; i++) {
                this->multidstblt.deltaEncodedRectangles[i].leftDelta = stream.in_sint16_le();
                this->multidstblt.deltaEncodedRectangles[i].topDelta  = stream.in_sint16_le();
                this->multidstblt.deltaEncodedRectangles[i].width     = stream.in_sint16_le();
                this->multidstblt.deltaEncodedRectangles[i].height    = stream.in_sint16_le();
            }
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            this->multiopaquerect.nLeftRect         = stream.in_sint16_le();
            this->multiopaquerect.nTopRect          = stream.in_sint16_le();
            this->multiopaquerect.nWidth            = stream.in_sint16_le();
            this->multiopaquerect.nHeight           = stream.in_sint16_le();
            {
                uint8_t red                         = stream.in_uint8();
                uint8_t green                       = stream.in_uint8();
                uint8_t blue                        = stream.in_uint8();
                this->multiopaquerect._Color        = red | green << 8 | blue << 16;
            }
            this->multiopaquerect.nDeltaEntries     = stream.in_uint8();
            for (uint8_t i = 0; i < this->multiopaquerect.nDeltaEntries; i++) {
                this->multiopaquerect.deltaEncodedRectangles[i].leftDelta = stream.in_sint16_le();
                this->multiopaquerect.deltaEncodedRectangles[i].topDelta  = stream.in_sint16_le();
                this->multiopaquerect.deltaEncodedRectangles[i].width     = stream.in_sint16_le();
                this->multiopaquerect.deltaEncodedRectangles[i].height    = stream.in_sint16_le();
            }
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            this->multipatblt.nLeftRect  = stream.in_sint16_le();
            this->multipatblt.nTopRect   = stream.in_sint16_le();
            this->multipatblt.nWidth     = stream.in_uint16_le();
            this->multipatblt.nHeight    = stream.in_uint16_le();
            this->multipatblt.bRop       = stream.in_uint8();
            this->multipatblt.BackColor  = stream.in_uint32_le();
            this->multipatblt.ForeColor  = stream.in_uint32_le();
            this->multipatblt.BrushOrgX  = stream.in_uint8();
            this->multipatblt.BrushOrgY  = stream.in_uint8();
            this->multipatblt.BrushStyle = stream.in_uint8();
            this->multipatblt.BrushHatch = stream.in_uint8();
            stream.in_copy_bytes(this->multipatblt.BrushExtra, 7);
            this->multipatblt.nDeltaEntries = stream.in_uint8();
            for (uint8_t i = 0; i < this->multipatblt.nDeltaEntries; i++) {
                this->multipatblt.deltaEncodedRectangles[i].leftDelta = stream.in_sint16_le();
                this->multipatblt.deltaEncodedRectangles[i].topDelta  = stream.in_sint16_le();
                this->multipatblt.deltaEncodedRectangles[i].width     = stream.in_sint16_le();
                this->multipatblt.deltaEncodedRectangles[i].height    = stream.in_sint16_le();
            }
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            this->multiscrblt.nLeftRect  = stream.in_sint16_le();
            this->multiscrblt.nTopRect   = stream.in_sint16_le();
            this->multiscrblt.nWidth     = stream.in_uint16_le();
            this->multiscrblt.nHeight    = stream.in_uint16_le();
            this->multiscrblt.bRop       = stream.in_uint8();
            this->multiscrblt.nXSrc      = stream.in_sint16_le();
            this->multiscrblt.nYSrc      = stream.in_sint16_le();
            this->multiscrblt.nDeltaEntries = stream.in_uint8();
            for (uint8_t i = 0; i < this->multiscrblt.nDeltaEntries; i++) {
                this->multiscrblt.deltaEncodedRectangles[i].leftDelta = stream.in_sint16_le();
                this->multiscrblt.deltaEncodedRectangles[i].topDelta  = stream.in_sint16_le();
                this->multiscrblt.deltaEncodedRectangles[i].width     = stream.in_sint16_le();
                this->multiscrblt.deltaEncodedRectangles[i].height    = stream.in_sint16_le();
            }
        }
    }

    void send(OutStream & stream) {
        // RDPOrderCommon common;
        stream.out_uint8(this->common.order);
        stream.out_uint16_le(this->common.clip.x);
        stream.out_uint16_le(this->common.clip.y);
        stream.out_uint16_le(this->common.clip.cx);
        stream.out_uint16_le(this->common.clip.cy);
        // RDPDestBlt destblt;
        stream.out_uint16_le(this->destblt.rect.x);
        stream.out_uint16_le(this->destblt.rect.y);
        stream.out_uint16_le(this->destblt.rect.cx);
        stream.out_uint16_le(this->destblt.rect.cy);
        stream.out_uint8(this->destblt.rop);
        // RDPPatBlt patblt;
        stream.out_uint16_le(this->patblt.rect.x);
        stream.out_uint16_le(this->patblt.rect.y);
        stream.out_uint16_le(this->patblt.rect.cx);
        stream.out_uint16_le(this->patblt.rect.cy);
        stream.out_uint8(this->patblt.rop);
        stream.out_uint32_le(this->patblt.back_color);
        stream.out_uint32_le(this->patblt.fore_color);
        stream.out_uint8(this->patblt.brush.org_x);
        stream.out_uint8(this->patblt.brush.org_y);
        stream.out_uint8(this->patblt.brush.style);
        stream.out_uint8(this->patblt.brush.hatch);
        stream.out_copy_bytes(this->patblt.brush.extra, 7);
        // RDPScrBlt scrblt;
        stream.out_uint16_le(this->scrblt.rect.x);
        stream.out_uint16_le(this->scrblt.rect.y);
        stream.out_uint16_le(this->scrblt.rect.cx);
        stream.out_uint16_le(this->scrblt.rect.cy);
        stream.out_uint8(this->scrblt.rop);
        stream.out_uint16_le(this->scrblt.srcx);
        stream.out_uint16_le(this->scrblt.srcy);
        // RDPOpaqueRect opaquerect;
        stream.out_uint16_le(this->opaquerect.rect.x);
        stream.out_uint16_le(this->opaquerect.rect.y);
        stream.out_uint16_le(this->opaquerect.rect.cx);
        stream.out_uint16_le(this->opaquerect.rect.cy);
        stream.out_uint8(this->opaquerect.color);
        stream.out_uint8(this->opaquerect.color >> 8);
        stream.out_uint8(this->opaquerect.color >> 16);
        // RDPMemBlt memblt;
        stream.out_uint16_le(this->memblt.cache_id);
        stream.out_uint16_le(this->memblt.rect.x);
        stream.out_uint16_le(this->memblt.rect.y);
        stream.out_uint16_le(this->memblt.rect.cx);
        stream.out_uint16_le(this->memblt.rect.cy);
        stream.out_uint8(this->memblt.rop);
        stream.out_uint8(this->memblt.srcx);
        stream.out_uint8(this->memblt.srcy);
        stream.out_uint16_le(this->memblt.cache_idx);
        // RDPMem3Blt memblt;
        stream.out_uint16_le (this->mem3blt.cache_id);
        stream.out_uint16_le (this->mem3blt.rect.x);
        stream.out_uint16_le (this->mem3blt.rect.y);
        stream.out_uint16_le (this->mem3blt.rect.cx);
        stream.out_uint16_le (this->mem3blt.rect.cy);
        stream.out_uint8     (this->mem3blt.rop);
        stream.out_uint8     (this->mem3blt.srcx);
        stream.out_uint8     (this->mem3blt.srcy);
        stream.out_uint32_le (this->mem3blt.back_color);
        stream.out_uint32_le (this->mem3blt.fore_color);
        stream.out_uint8     (this->mem3blt.brush.org_x);
        stream.out_uint8     (this->mem3blt.brush.org_y);
        stream.out_uint8     (this->mem3blt.brush.style);
        stream.out_uint8     (this->mem3blt.brush.hatch);
        stream.out_copy_bytes(this->mem3blt.brush.extra, 7);
        stream.out_uint16_le (this->mem3blt.cache_idx);
        // RDPLineTo lineto;
        stream.out_uint8(this->lineto.back_mode);
        stream.out_uint16_le(this->lineto.startx);
        stream.out_uint16_le(this->lineto.starty);
        stream.out_uint16_le(this->lineto.endx);
        stream.out_uint16_le(this->lineto.endy);
        stream.out_uint32_le(this->lineto.back_color);
        stream.out_uint8(this->lineto.rop2);
        stream.out_uint8(this->lineto.pen.style);
        stream.out_sint8(this->lineto.pen.width);
        stream.out_uint32_le(this->lineto.pen.color);
        // RDPGlyphIndex glyphindex;
        stream.out_uint8(this->glyphindex.cache_id);
        stream.out_sint16_le(this->glyphindex.fl_accel);
        stream.out_sint16_le(this->glyphindex.ui_charinc);
        stream.out_sint16_le(this->glyphindex.f_op_redundant);
        stream.out_uint32_le(this->glyphindex.back_color);
        stream.out_uint32_le(this->glyphindex.fore_color);
        stream.out_uint16_le(this->glyphindex.bk.x);
        stream.out_uint16_le(this->glyphindex.bk.y);
        stream.out_uint16_le(this->glyphindex.bk.cx);
        stream.out_uint16_le(this->glyphindex.bk.cy);
        stream.out_uint16_le(this->glyphindex.op.x);
        stream.out_uint16_le(this->glyphindex.op.y);
        stream.out_uint16_le(this->glyphindex.op.cx);
        stream.out_uint16_le(this->glyphindex.op.cy);
        stream.out_uint8(this->glyphindex.brush.org_x);
        stream.out_uint8(this->glyphindex.brush.org_y);
        stream.out_uint8(this->glyphindex.brush.style);
        stream.out_uint8(this->glyphindex.brush.hatch);
        stream.out_copy_bytes(this->glyphindex.brush.extra, 7);
        stream.out_sint16_le(this->glyphindex.glyph_x);
        stream.out_sint16_le(this->glyphindex.glyph_y);
        stream.out_uint8(this->glyphindex.data_len);
        memset(this->glyphindex.data
                + this->glyphindex.data_len, 0,
            sizeof(this->glyphindex.data)
                - this->glyphindex.data_len);
        stream.out_copy_bytes(this->glyphindex.data, 256);
        // RDPPolyline polyline;
        stream.out_sint16_le(this->polyline.xStart);
        stream.out_sint16_le(this->polyline.yStart);
        stream.out_uint8(this->polyline.bRop2);
        stream.out_uint16_le(this->polyline.BrushCacheEntry);
        stream.out_uint32_le(this->polyline.PenColor);
        stream.out_uint8(this->polyline.NumDeltaEntries);
        for (uint8_t i = 0; i < this->polyline.NumDeltaEntries; i++) {
            stream.out_sint16_le(this->polyline.deltaEncodedPoints[i].xDelta);
            stream.out_sint16_le(this->polyline.deltaEncodedPoints[i].yDelta);
        }
        // RDPMultiDstBlt multidstblt;
        stream.out_sint16_le(this->multidstblt.nLeftRect);
        stream.out_sint16_le(this->multidstblt.nTopRect);
        stream.out_sint16_le(this->multidstblt.nWidth);
        stream.out_sint16_le(this->multidstblt.nHeight);
        stream.out_uint8(this->multidstblt.bRop);
        stream.out_uint8(this->multidstblt.nDeltaEntries);
        for (uint8_t i = 0; i < this->multidstblt.nDeltaEntries; i++) {
            stream.out_sint16_le(this->multidstblt.deltaEncodedRectangles[i].leftDelta);
            stream.out_sint16_le(this->multidstblt.deltaEncodedRectangles[i].topDelta);
            stream.out_sint16_le(this->multidstblt.deltaEncodedRectangles[i].width);
            stream.out_sint16_le(this->multidstblt.deltaEncodedRectangles[i].height);
        }
        // RDPMultiOpaqueRect multiopaquerect;
        stream.out_sint16_le(this->multiopaquerect.nLeftRect);
        stream.out_sint16_le(this->multiopaquerect.nTopRect);
        stream.out_sint16_le(this->multiopaquerect.nWidth);
        stream.out_sint16_le(this->multiopaquerect.nHeight);
        stream.out_uint8(this->multiopaquerect._Color);
        stream.out_uint8(this->multiopaquerect._Color >> 8);
        stream.out_uint8(this->multiopaquerect._Color >> 16);
        stream.out_uint8(this->multiopaquerect.nDeltaEntries);
        for (uint8_t i = 0; i < this->multiopaquerect.nDeltaEntries; i++) {
            stream.out_sint16_le(this->multiopaquerect.deltaEncodedRectangles[i].leftDelta);
            stream.out_sint16_le(this->multiopaquerect.deltaEncodedRectangles[i].topDelta);
            stream.out_sint16_le(this->multiopaquerect.deltaEncodedRectangles[i].width);
            stream.out_sint16_le(this->multiopaquerect.deltaEncodedRectangles[i].height);
        }
        // RDPMultiPatBlt multipatblt;
        stream.out_sint16_le(this->multipatblt.nLeftRect);
        stream.out_sint16_le(this->multipatblt.nTopRect);
        stream.out_uint16_le(this->multipatblt.nWidth);
        stream.out_uint16_le(this->multipatblt.nHeight);
        stream.out_uint8(this->multipatblt.bRop);
        stream.out_uint32_le(this->multipatblt.BackColor);
        stream.out_uint32_le(this->multipatblt.ForeColor);
        stream.out_uint8(this->multipatblt.BrushOrgX);
        stream.out_uint8(this->multipatblt.BrushOrgY);
        stream.out_uint8(this->multipatblt.BrushStyle);
        stream.out_uint8(this->multipatblt.BrushHatch);
        stream.out_copy_bytes(this->multipatblt.BrushExtra, 7);
        stream.out_uint8(this->multipatblt.nDeltaEntries);
        for (uint8_t i = 0; i < this->multipatblt.nDeltaEntries; i++) {
            stream.out_sint16_le(this->multipatblt.deltaEncodedRectangles[i].leftDelta);
            stream.out_sint16_le(this->multipatblt.deltaEncodedRectangles[i].topDelta);
            stream.out_sint16_le(this->multipatblt.deltaEncodedRectangles[i].width);
            stream.out_sint16_le(this->multipatblt.deltaEncodedRectangles[i].height);
        }
        // RDPMultiScrBlt multiscrblt;
        stream.out_sint16_le(this->multiscrblt.nLeftRect);
        stream.out_sint16_le(this->multiscrblt.nTopRect);
        stream.out_uint16_le(this->multiscrblt.nWidth);
        stream.out_uint16_le(this->multiscrblt.nHeight);
        stream.out_uint8(this->multiscrblt.bRop);
        stream.out_sint16_le(this->multiscrblt.nXSrc);
        stream.out_sint16_le(this->multiscrblt.nYSrc);
        stream.out_uint8(this->multiscrblt.nDeltaEntries);
        for (uint8_t i = 0; i < this->multiscrblt.nDeltaEntries; i++) {
            stream.out_sint16_le(this->multiscrblt.deltaEncodedRectangles[i].leftDelta);
            stream.out_sint16_le(this->multiscrblt.deltaEncodedRectangles[i].topDelta);
            stream.out_sint16_le(this->multiscrblt.deltaEncodedRectangles[i].width);
            stream.out_sint16_le(this->multiscrblt.deltaEncodedRectangles[i].height);
        }
    }
};