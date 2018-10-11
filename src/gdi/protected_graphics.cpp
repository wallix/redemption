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
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "utils/bitmap.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "gdi/graphic_api.hpp"


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
                Bitmap sub_bmp(bmp, Rect(subrect.x - rectBmp.x, subrect.y - rectBmp.y, subrect.cx, subrect.cy));

                RDPBitmapData sub_bmp_data = bitmap_data;

                sub_bmp_data.dest_left = subrect.x;
                sub_bmp_data.dest_top = subrect.y;
                sub_bmp_data.dest_right = std::min<uint16_t>(sub_bmp_data.dest_left + subrect.cx - 1, bitmap_data.dest_right);
                sub_bmp_data.dest_bottom = sub_bmp_data.dest_top + subrect.cy - 1;

                sub_bmp_data.width = sub_bmp.cx();
                sub_bmp_data.height = sub_bmp.cy();
                sub_bmp_data.bits_per_pixel = safe_int(sub_bmp.bpp());
                sub_bmp_data.flags = 0;

                sub_bmp_data.bitmap_length = sub_bmp.bmp_size();

                this->drawable.draw(sub_bmp_data, sub_bmp);
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

} // namespace gdi
