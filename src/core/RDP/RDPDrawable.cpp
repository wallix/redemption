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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Poelen Jonathan, Raphael Zhou, Meng Tan
*/

#include <utility>

#include "core/RDP/RDPDrawable.hpp"

#include "utils/bitmap.hpp"

#include "core/font.hpp"

#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/rdp_draw_glyphs.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"
#include "core/RDP/orders/for_each_delta_rect.hpp"
#include "gdi/clip_from_cmd.hpp"


namespace
{
    using Color = Drawable::Color;

    Color u32_to_color(Drawable const& drawable, BGRColor color)
    {
        return drawable.u32bgr_to_color(color.as_u32());
    }

    Color u32rgb_to_color(Drawable const& drawable, gdi::ColorCtx color_ctx, RDPColor color)
    {
        return u32_to_color(drawable, color_decode(color, color_ctx));
    }

    std::pair<Color, Color> u32rgb_to_color(
        Drawable const& drawable, gdi::ColorCtx color_ctx, RDPColor color1, RDPColor color2,
        BGRPalette const& mod_palette_rgb)
    {
        using gdi::Depth;

        switch (color_ctx.depth()){
            case Depth::depth8():
                // TODO color_ctx.palette()
                return {
                    u32_to_color(drawable, decode_color8()(color1, mod_palette_rgb)),
                    u32_to_color(drawable, decode_color8()(color2, mod_palette_rgb))
                };
            case Depth::depth15():
                return {
                    u32_to_color(drawable, decode_color15()(color1)),
                    u32_to_color(drawable, decode_color15()(color2))
                };
            case Depth::depth16():
                return {
                    u32_to_color(drawable, decode_color16()(color1)),
                    u32_to_color(drawable, decode_color16()(color2))
                };
            case Depth::depth24():
                return {
                    u32_to_color(drawable, decode_color24()(color1)),
                    u32_to_color(drawable, decode_color24()(color2))
                };
            case Depth::unspecified():
                break;
        }

        assert(false);
        return {Color{0, 0, 0}, Color{0, 0, 0}};
    }

    Rect intersect(Drawable const& drawable, Rect const& a, Rect const& b)
    {
        return a.intersect(drawable.width(), drawable.height()).intersect(b);
    }

    template<class RDPMulti, class FRect>
    void draw_multi(Drawable const& drawable, const RDPMulti & cmd, Rect clip, FRect f)
    {
        const Rect clip_drawable_cmd_intersect = intersect(drawable, clip, clip_from_cmd(cmd));
        for_each_delta_rect(cmd, [&](Rect const& cmd_rect){
            f(clip_drawable_cmd_intersect.intersect(cmd_rect));
        });
    }
} // namespace


RDPDrawable::RDPDrawable(const uint16_t width, const uint16_t height)
: RDPDrawable(width, height, drawable_default_pointer())
{}

RDPDrawable::RDPDrawable(const uint16_t width, const uint16_t height, Pointer const& cursor)
: drawable(width, height)
, save_mouse_x(0)
, save_mouse_y(0)
, mouse_cursor_pos_x(width / 2)
, mouse_cursor_pos_y(height / 2)
, current_pointer(cursor)
, frame_start_count(0)
, mod_palette_rgb(BGRPalette::classic_332())
{
    const auto hotspot = cursor.get_hotspot();
    this->mouse_cursor_hotspot_x = hotspot.x;
    this->mouse_cursor_hotspot_y = hotspot.y;
}

void RDPDrawable::resize(uint16_t width, uint16_t height)
{
    this->drawable.resize(width, height);
}

void RDPDrawable::draw(RDPColCache   const & /*cmd*/)
{}

void RDPDrawable::draw(RDPBrushCache const & /*cmd*/)
{}

void RDPDrawable::draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    this->drawable.opaquerect(trect, u32rgb_to_color(this->drawable, color_ctx, cmd.color));
    this->last_update_index++;
}

void RDPDrawable::draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO clip is not used
    (void)clip;
    this->drawable.ellipse(cmd.el, cmd.bRop2, cmd.fillMode, u32rgb_to_color(this->drawable, color_ctx, cmd.color));
    this->last_update_index++;
}

// TODO This will draw a standard ellipse without brush style
void RDPDrawable::draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO clip is not used
    (void)clip;
    this->drawable.ellipse(cmd.el, cmd.brop2, cmd.fill_mode, u32rgb_to_color(this->drawable, color_ctx, cmd.back_color));
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPScrBlt & cmd, Rect clip)
{
    const int deltax = cmd.srcx - cmd.rect.x;
    const int deltay = cmd.srcy - cmd.rect.y;
    const Rect drect = intersect(this->drawable, clip, cmd.rect);
    const Rect src = drect.offset(deltax, deltay)
        .intersect(this->drawable.width(), this->drawable.height());
    const Rect trect(drect.x, drect.y, std::min(drect.cx, src.cx), std::min(drect.cy, src.cy));
    this->drawable.scrblt(src.x, src.y, trect, cmd.rop);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPDstBlt & cmd, Rect clip)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    this->drawable.destblt(trect, cmd.rop);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
    draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
        this->drawable.destblt(trect, cmd.bRop);
    });
    this->last_update_index++;
}

void RDPDrawable::draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd._Color);
    draw_multi(this->drawable, cmd, clip, [color, this](const Rect & trect) {
        this->drawable.opaquerect(trect, color);
    });
    this->last_update_index++;
}

void RDPDrawable::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // TODO PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.
    if (cmd.brush.style == 0x03 && (cmd.bRop == 0xF0 || cmd.bRop == 0x5A)) {
        enum { BackColor, ForeColor };
        auto colors = u32rgb_to_color(this->drawable, color_ctx, cmd.BackColor, cmd.ForeColor, this->mod_palette_rgb);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;
        draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
            this->drawable.patblt_ex(
                trect, cmd.bRop,
                std::get<BackColor>(colors), std::get<ForeColor>(colors),
                brush_data, cmd.brush.org_x, cmd.brush.org_y
            );
        });
    }
    else {
        const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.BackColor);
        draw_multi(this->drawable, cmd, clip, [&](const Rect & trect) {
            this->drawable.patblt(trect, cmd.bRop, color);
        });
    }
    this->last_update_index++;
}

void RDPDrawable::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    const int deltax = cmd.nXSrc - cmd.rect.x;
    const int deltay = cmd.nYSrc - cmd.rect.y;

    clip = intersect(this->drawable, clip, cmd.rect);

    Rect cmd_rect;

    for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
        cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
        cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
        cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
        cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
        Rect drect = clip.intersect(cmd_rect);
        Rect src = cmd_rect.offset(deltax, deltay)
            .intersect(this->drawable.width(), this->drawable.height());
        Rect trect(drect.x, drect.y, std::min(drect.cx, src.cx), std::min(drect.cy, src.cy));
        this->drawable.scrblt(src.x, src.y, trect, cmd.bRop);
    }

    this->last_update_index++;
}

void RDPDrawable::draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Rect trect = intersect(this->drawable, clip, cmd.rect);
    // TODO PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.

    if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {
        enum { BackColor, ForeColor };
        auto colors = u32rgb_to_color(this->drawable, color_ctx, cmd.back_color, cmd.fore_color, this->mod_palette_rgb);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;

        this->drawable.patblt_ex(
            trect, cmd.rop,
            std::get<BackColor>(colors), std::get<ForeColor>(colors),
            brush_data, cmd.brush.org_x, cmd.brush.org_y
        );
    }
    else {
        this->drawable.patblt(trect, cmd.rop, u32rgb_to_color(this->drawable, color_ctx, cmd.back_color));
    }
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPMemBlt & cmd_, Rect clip, const Bitmap & bmp)
{
    RDPMemBlt cmd(cmd_);

    cmd.rect = cmd_.rect.intersect(this->drawable.width(), this->drawable.height());
    cmd.srcx += (cmd.rect.x - cmd_.rect.x);
    cmd.srcy += (cmd.rect.y - cmd_.rect.y);

    const Rect rect = clip.intersect(cmd.rect);
    if (rect.isempty()){
        return ;
    }

    switch (cmd.rop) {
    case 0x00:
        this->drawable.black_color(rect);
    break;
    case 0xFF:
        this->drawable.white_color(rect);
    break;
    case 0x55:
        this->drawable.mem_blt_invert(rect, bmp
            , cmd.srcx + (rect.x - cmd.rect.x)
            , cmd.srcy + (rect.y - cmd.rect.y));
    break;
    case 0xCC:
        this->drawable.mem_blt(rect, bmp
            , cmd.srcx + (rect.x - cmd.rect.x)
            , cmd.srcy + (rect.y - cmd.rect.y));
    break;
    case 0x22:  // dest = dest AND (NOT source)
    case 0x66:  // dest = source XOR dest (SRCINVERT)
    case 0x88:  // dest = source AND dest (SRCAND)
    case 0xBB:  // dest = (NOT source) OR dest (MERGEPAINT)
    case 0xEE:  // dest = source OR dest (SRCPAINT)
        this->drawable.mem_blt_ex(rect,
                                  bmp,
                                  cmd.srcx + (rect.x - cmd.rect.x),
                                  cmd.srcy + (rect.y - cmd.rect.y),
                                  cmd.rop);
        break;
    default:
        // should not happen
        //LOG(LOG_INFO, "Unsupported Rop=0x%02X", cmd.rop);
    break;
    }
    this->last_update_index++;
}

void RDPDrawable::draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bmp)
{
    const Rect rect = clip.intersect(cmd.rect);
    if (rect.isempty()){
        return ;
    }

    this->drawable.mem_3_blt(rect, bmp
        , cmd.srcx + (rect.x  - cmd.rect.x)
        , cmd.srcy + (rect.y  - cmd.rect.y)
        , cmd.rop
        , u32rgb_to_color(this->drawable, color_ctx, cmd.fore_color)
    );
    this->last_update_index++;
}

void RDPDrawable::draw(RDPSetSurfaceCommand const & /*cmd*/) {}

void RDPDrawable::draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content)
{
    /* no remoteFx support in recording, transcode to bitmapUpdates */
    for (const Rect & rect : content.region.rects) {
        // LOG(LOG_INFO, "RDPDrawable::draw(RDPSetSurfaceCommand cmd, RDPSurfaceContent const &content) stride=%u, rect=%s",
        //     content.stride, rect);
        Bitmap bitmap(content.data, content.stride, rect);
        RDPBitmapData bitmap_data;
        bitmap_data.dest_left = cmd.destRect.x + rect.ileft();
        bitmap_data.dest_right = cmd.destRect.x + rect.eright()-1;
        bitmap_data.dest_top = cmd.destRect.y + rect.itop();
        bitmap_data.dest_bottom = cmd.destRect.y + rect.ebottom()-1;

        bitmap_data.width = bitmap.cx();
        bitmap_data.height = bitmap.cy();
        bitmap_data.bits_per_pixel = 32;
        bitmap_data.flags = /*NO_BITMAP_COMPRESSION_HDR*/ 0;
        bitmap_data.bitmap_length = bitmap.bmp_size();

        this->RDPDrawable::draw(bitmap_data, bitmap);
    }
}

/*
    *
    *            +----+----+
    *            |\   |   /|  4 cases.
    *            | \  |  / |  > Case 1 is the normal case
    *            |  \ | /  |  > Case 2 has a negative coeff
    *            | 3 \|/ 2 |  > Case 3 and 4 are the same as
    *            +----0---->x    Case 1 and 2 but one needs to
    *            | 4 /|\ 1 |     exchange begin and end.
    *            |  / | \  |
    *            | /  |  \ |
    *            |/   |   \|
    *            +----v----+
    *                 y
    *  Anyway, we base the line drawing on bresenham's algorithm
    */
void RDPDrawable::draw(const RDPLineTo & lineto, Rect clip, gdi::ColorCtx color_ctx)
{
    this->drawable.draw_line(
        lineto.back_mode,
        lineto.startx, lineto.starty,
        lineto.endx, lineto.endy,
        lineto.rop2, u32rgb_to_color(this->drawable, color_ctx, lineto.pen.color), clip
    );
    this->last_update_index++;
}

void RDPDrawable::draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
    Rect screen_rect = clip.intersect(this->drawable.width(), this->drawable.height());
    if (screen_rect.isempty()){
        return ;
    }

    Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
    if (clipped_glyph_fragment_rect.isempty()) {
        return;
    }

    // set a background color
    {
        Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
        if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
            ajusted.cy--;
            this->drawable.opaquerect(ajusted.intersect(screen_rect), u32rgb_to_color(this->drawable, color_ctx, cmd.fore_color));
        }
    }

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.back_color);
    const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

    auto set_point = [&](int16_t x, int16_t y){ this->drawable.draw_pixel(x, y, color); };

    uint16_t draw_pos = 0;

    rdp_draw_glyphs(
        set_point, this->fragment_cache, {cmd.data, cmd.data_len},
        has_delta_bytes, cmd.ui_charinc, draw_pos,
        offset_y, cmd.bk.x + offset_x, cmd.bk.y,
        clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
    this->last_update_index++;
}

void RDPDrawable::draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    const Color color = u32rgb_to_color(this->drawable, color_ctx, cmd.PenColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        int16_t const endx = startx + cmd.deltaEncodedPoints[i].xDelta;
        int16_t const endy = starty + cmd.deltaEncodedPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, color, clip);

        startx = endx;
        starty = endy;
    }
    this->last_update_index++;
}

// TODO this functions only draw polygon borders but do not fill them with solid color.
void RDPDrawable::draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    int16_t endx;
    int16_t endy;

    const Color BrushColor = u32rgb_to_color(this->drawable, color_ctx, cmd.BrushColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        endx = startx + cmd.deltaPoints[i].xDelta;
        endy = starty + cmd.deltaPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);

        startx = endx;
        starty = endy;
    }
    endx = cmd.xStart;
    endy = cmd.yStart;

    this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);
    this->last_update_index++;
}

// TODO this functions only draw polygon borders but do not fill them with brush color.
void RDPDrawable::draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    int16_t startx = cmd.xStart;
    int16_t starty = cmd.yStart;

    int16_t endx;
    int16_t endy;

    const Color foreColor = u32rgb_to_color(this->drawable, color_ctx, cmd.foreColor);

    for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
        endx = startx + cmd.deltaPoints[i].xDelta;
        endy = starty + cmd.deltaPoints[i].yDelta;

        this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);

        startx = endx;
        starty = endy;
    }
    endx = cmd.xStart;
    endy = cmd.yStart;

    this->drawable.draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
{
    const Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                      , bitmap_data.dest_right - bitmap_data.dest_left + 1
                      , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

    const Rect trect = rectBmp.intersect(this->drawable.width(), this->drawable.height());

    this->drawable.draw_bitmap(trect, bmp);
    this->last_update_index++;
}

void RDPDrawable::draw(const RDP::FrameMarker & order)
{
    this->frame_start_count += ((order.action == RDP::FrameMarker::FrameStart) ? 1 : -1);
    assert(this->frame_start_count >= 0);
    this->drawable.logical_frame_ended = (this->frame_start_count == 0);
    this->last_update_index++;
}

void RDPDrawable::trace_mouse()
{
    this->save_mouse_x = this->mouse_cursor_pos_x;
    this->save_mouse_y = this->mouse_cursor_pos_y;
    int x = this->save_mouse_x - this->mouse_cursor_hotspot_x;
    int y = this->save_mouse_y - this->mouse_cursor_hotspot_y;
    return this->drawable.trace_mouse(this->current_pointer, x, y, this->save_mouse);
}

void RDPDrawable::clear_mouse()
{
    int x = this->save_mouse_x - this->mouse_cursor_hotspot_x;
    int y = this->save_mouse_y - this->mouse_cursor_hotspot_y;
    return this->drawable.clear_mouse(this->current_pointer, x, y, this->save_mouse);
}

void RDPDrawable::set_pointer(uint16_t /*cache_idx*/, Pointer const& cursor, SetPointerMode /*mode*/)
{
    if (cursor.get_native_xor_bpp() != BitsPerPixel{0})
    {
        // Is a native pointer
        const auto hotspot    = cursor.get_hotspot();
        const auto dimensions = cursor.get_dimensions();

        auto av_and_mask = cursor.get_monochrome_and_mask();
        auto av_xor_mask = cursor.get_native_xor_mask();

        const Pointer imported_cursor =
            decode_pointer(cursor.get_native_xor_bpp(),
                           dimensions.width,
                           dimensions.height,
                           hotspot.x,
                           hotspot.y,
                           av_xor_mask.size(),
                           av_xor_mask.data(),
                           av_and_mask.size(),
                           av_and_mask.data());

        this->mouse_cursor_hotspot_x = hotspot.x;
        this->mouse_cursor_hotspot_y = hotspot.y;

        this->current_pointer.set_cursor(imported_cursor);

        return;
    }

    const auto hotspot = cursor.get_hotspot();

    this->mouse_cursor_hotspot_x = hotspot.x;
    this->mouse_cursor_hotspot_y = hotspot.y;

    this->current_pointer.set_cursor(cursor);
}
