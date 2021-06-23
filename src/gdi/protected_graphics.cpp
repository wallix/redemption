/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "gdi/protected_graphics.hpp"
#include "core/callback.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "utils/bitmap.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "gdi/subrect4.hpp"
#include "utils/sugar/array_view.hpp"


namespace
{

inline void draw_impl(
    gdi::GraphicApi& drawable, const Rect protected_rect,
    RDPBitmapData const& bitmap_data, Bitmap const& bmp)
{
    Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                , bitmap_data.dest_right - bitmap_data.dest_left + 1
                , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

    if (REDEMPTION_UNLIKELY(protected_rect.contains(rectBmp) || rectBmp.isempty())) {
        // nada: leave the protected_rect untouched
    }
    else if (rectBmp.has_intersection(protected_rect)) {
        for (const Rect & subrect : gdi::subrect4(rectBmp, protected_rect)) {
            if (!subrect.isempty()) {
                // draw the parts of the screen outside protected_rect
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

                drawable.draw(sub_bmp_data, sub_bmp);
            }
        }
    }
    else {
        // draw the parts of the screen outside protected_rect
        drawable.draw(bitmap_data, bmp);
    }
}

inline void draw_impl(
    gdi::GraphicApi& drawable, RdpInput & rdp_input, const Rect protected_rect,
    RDPScrBlt const& cmd, const Rect clip)
{
    const Rect drect = cmd.rect.intersect(clip);
    const int deltax = cmd.srcx - cmd.rect.x;
    const int deltay = cmd.srcy - cmd.rect.y;
    const int srcx = drect.x + deltax;
    const int srcy = drect.y + deltay;
    const Rect srect(srcx, srcy, drect.cx, drect.cy);

    const bool has_dest_intersec_fg = drect.has_intersection(protected_rect);
    const bool has_src_intersec_fg = srect.has_intersection(protected_rect);

    if (!has_dest_intersec_fg && !has_src_intersec_fg) {
        // neither scr or dest rect intersect with protected_rect
        drawable.draw(cmd, clip);
    }
    else {
        rdp_input.rdp_input_invalidate2(gdi::subrect4(drect, protected_rect));
    }
}

template<class Command, class... Args>
inline void draw_impl(
    gdi::GraphicApi& drawable, const Rect protected_rect,
    Command const& cmd, Rect clip, Args const &... args)
{
    auto const& rect = clip_from_cmd(cmd).intersect(clip);
    if (protected_rect.contains(rect) || rect.isempty()) {
        // nada: leave the protected_rect untouched
    }
    else if (rect.has_intersection(protected_rect)) {
        // draw the parts of the screen outside protected_rect
        // TODO used multi orders
        for (const Rect & subrect : gdi::subrect4(rect, protected_rect)) {
            if (!subrect.isempty()) {
                drawable.draw(cmd, subrect, args...);
            }
        }
    }
    else {
        // The drawing order is fully ouside protected_rect
        drawable.draw(cmd, clip, args...);
    }
}

} // namespace anonymous


void gdi::ProtectedGraphics::draw(RDPDstBlt const& cmd, Rect clip)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip);
}

void gdi::ProtectedGraphics::draw(RDPMultiDstBlt const& cmd, Rect clip)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip);
}

void gdi::ProtectedGraphics::draw(RDPPatBlt const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDP::RDPMultiPatBlt const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPOpaqueRect const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPMultiOpaqueRect const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPScrBlt const& cmd, Rect clip)
{
    ::draw_impl(this->drawable, this->rdp_input, this->protected_rect, cmd, clip);
}

void gdi::ProtectedGraphics::draw(RDP::RDPMultiScrBlt const& cmd, Rect clip)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip);
}

void gdi::ProtectedGraphics::draw(RDPLineTo const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPPolygonSC const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPPolygonCB const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPPolyline const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPEllipseSC const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPEllipseCB const& cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx);
}

void gdi::ProtectedGraphics::draw(RDPBitmapData const& cmd, Bitmap const& bmp)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, bmp);
}

void gdi::ProtectedGraphics::draw(RDPMemBlt const& cmd, Rect clip, Bitmap const& bmp)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, bmp);
}

void gdi::ProtectedGraphics::draw(RDPMem3Blt const& cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx, bmp);
}

void gdi::ProtectedGraphics::draw(RDPGlyphIndex const& cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const& gly_cache)
{
    ::draw_impl(this->drawable, this->protected_rect, cmd, clip, color_ctx, gly_cache);
}
