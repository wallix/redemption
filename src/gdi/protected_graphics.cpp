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
 *   Foundation, Inc.; 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#include "gdi/protected_graphics.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/bitmap.hpp"
#include "gdi/graphic_api.hpp"

#include "gdi/clip_from_cmd.hpp"

namespace gdi
{

void ProtectedGraphics::draw_impl(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
{
    Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                , bitmap_data.dest_right - bitmap_data.dest_left + 1
                , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

    if (rectBmp.has_intersection(this->protected_rect)) {
        this->drawable.begin_update();
        for (const Rect & subrect : subrect4(rectBmp, this->protected_rect)) {
            if (!subrect.isempty()) {
                this->drawable.draw(
                    RDPMemBlt(0, subrect, 0xCC, subrect.x - rectBmp.x, subrect.y - rectBmp.y, 0),
                    subrect, bmp
                );
            }
        }
        this->drawable.end_update();
    }
    else {
        this->drawable.draw(bitmap_data, bmp);
    }
}

void ProtectedGraphics::draw_impl(const RDPScrBlt & cmd, const Rect clip)
{
    const Rect drect = cmd.rect.intersect(clip);
    const int deltax = cmd.srcx - cmd.rect.x;
    const int deltay = cmd.srcy - cmd.rect.y;
    const int srcx = drect.x + deltax;
    const int srcy = drect.y + deltay;
    const Rect srect(srcx, srcy, drect.cx, drect.cy);

    const bool has_dest_intersec_fg = drect.has_intersection(this->protected_rect);
    const bool has_src_intersec_fg = srect.has_intersection(this->protected_rect);

    if (!has_dest_intersec_fg && !has_src_intersec_fg) {
        this->drawable.draw(cmd, clip);
    }
    else {
        this->drawable.begin_update();
        subrect4_t rects = subrect4(drect, this->protected_rect);
        auto e = std::remove_if(rects.begin(), rects.end(), [](const Rect & rect) { return rect.isempty(); });
        auto av = make_array_view(rects.begin(), e);
        this->refresh_rects(av);
        this->drawable.end_update();
    }
}

}
