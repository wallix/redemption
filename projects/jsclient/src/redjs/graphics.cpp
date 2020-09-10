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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "redjs/graphics.hpp"

#include "red_emscripten/val.hpp"
#include "red_emscripten/constants.hpp"

#include "gdi/screen_info.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/rdp_draw_glyphs.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"

#include "client_redemption/pointer_to_rgba8888.hpp"

// include "utils/log.hpp"

#include <cinttypes>


namespace
{
    Rect intersect(uint16_t w, uint16_t h, Rect const& a, Rect const& b)
    {
        return a.intersect(w, h).intersect(b);
    }

    //@{
    Rect to_rect(RDPMultiDstBlt const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    Rect to_rect(RDPMultiOpaqueRect const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    Rect to_rect(RDP::RDPMultiScrBlt const & cmd)
    { return cmd.rect; }

    Rect to_rect(RDP::RDPMultiPatBlt const & cmd)
    { return cmd.rect; }
    //@}

    template<class RDPMulti, class FRect>
    void draw_multi(uint16_t w, uint16_t h, const RDPMulti & cmd, Rect clip, FRect f)
    {
        const Rect clip_drawable_cmd_intersect = intersect(w, h, clip, to_rect(cmd));

        Rect cmd_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx = cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy = cmd.deltaEncodedRectangles[i].height;
            f(clip_drawable_cmd_intersect.intersect(cmd_rect));
        }
    }

    std::array<uint8_t, 8> to_brush_data(RDPBrush const& brush)
    {
        return std::array<uint8_t, 8>{{
            brush.extra[0],
            brush.extra[1],
            brush.extra[2],
            brush.extra[3],
            brush.extra[4],
            brush.extra[5],
            brush.extra[6],
            brush.hatch,
        }};
    }

    template<class Delta>
    emscripten::val make_emval_delta_points(Delta const (&delta)[128], uint8_t num_entries)
    {
        static_assert(alignof(Delta) == 2);
        static_assert(sizeof(Delta) == 2*2);

        return redjs::emval_from_view(array_view{
            reinterpret_cast<uint16_t const*>(delta), /* NOLINT */
            num_entries * 2u
        });
    }

    struct MemBltPoints
    {
        Rect rect;
        uint16_t srcx;
        uint16_t srcy;

        MemBltPoints(Rect rect, uint16_t srcx, uint16_t srcy, Rect clip)
        : rect(clip.intersect(rect))
        , srcx(srcx + rect.x - rect.x + (this->rect.x - rect.x))
        , srcy(srcy + rect.y - rect.y + (this->rect.y - rect.y))
        {}
    };

    namespace jsnames
    {
        constexpr char const* draw_rect = "drawRect";
        constexpr char const* draw_scr_blt = "drawScrBlt";
        constexpr char const* draw_line_to = "drawLineTo";
        constexpr char const* draw_polyline = "drawPolyline";
        constexpr char const* draw_polygone_sc = "drawPolygoneCB";
        constexpr char const* draw_polygone_cb = "drawPolygoneSC";
        constexpr char const* draw_ellipse_sc = "drawEllipseCB";
        constexpr char const* draw_ellipse_cb = "drawEllipseSC";
        constexpr char const* draw_pat_blt = "drawPatBlt";
        constexpr char const* draw_dest_blt = "drawDstBlt";

        constexpr char const* draw_image = "drawImage";
        constexpr char const* draw_memblt = "drawMemBlt";
        constexpr char const* draw_mem3blt = "drawMem3Blt";

        constexpr char const* set_bmp_cache = "setBmpCacheIndex";
        constexpr char const* set_bmp_cache_entries = "setBmpCacheEntries";

        constexpr char const* cached_pointer = "cachedPointer";
        constexpr char const* new_pointer = "newPointer";
        constexpr char const* set_pointer = "setPointer";

        constexpr char const* frame_marker = "frameMarker";

        constexpr char const* resize_canvas = "resizeCanvas";
        constexpr char const* update_pointer_position = "updatePointerPosition";
    }
}


inline uint32_t emval_call_arg(BGRColor const& bgr) noexcept
{
    return BGRColor(BGRasRGBColor(bgr)).as_u32();
}

namespace redjs
{

Graphics::Graphics(emscripten::val callbacks, uint16_t width, uint16_t height)
: width(width)
, height(height)
, callbacks(std::move(callbacks))
{
}

PrimaryDrawingOrdersSupport Graphics::get_supported_orders() const
{
    PrimaryDrawingOrdersSupport supported = TS_NEG_GLYPH_INDEX;
    auto set = [&](char const* name, auto f){
        supported |= !!callbacks[name] ? f : PrimaryDrawingOrdersSupport{};
    };

    set(jsnames::draw_scr_blt, PrimaryDrawingOrdersSupport{}
        | TS_NEG_SCRBLT_INDEX
        | TS_NEG_MULTISCRBLT_INDEX);

    // enable draw_rect and draw_pat_blt
    set(jsnames::draw_pat_blt, PrimaryDrawingOrdersSupport{}
        | TS_NEG_PATBLT_INDEX
        | TS_NEG_MULTIPATBLT_INDEX);

    set(jsnames::draw_rect, PrimaryDrawingOrdersSupport{}
        // OpaqueRect depends of PatBlt
        | TS_NEG_MULTIOPAQUERECT_INDEX);

    set(jsnames::draw_dest_blt, PrimaryDrawingOrdersSupport{}
        | TS_NEG_DSTBLT_INDEX
        | TS_NEG_MULTIDSTBLT_INDEX);

    set(jsnames::draw_memblt, TS_NEG_MEMBLT_INDEX);
    set(jsnames::draw_mem3blt, TS_NEG_MEM3BLT_INDEX);
    set(jsnames::draw_line_to, TS_NEG_LINETO_INDEX);
    set(jsnames::draw_polyline, TS_NEG_POLYLINE_INDEX);
    set(jsnames::draw_polygone_cb, TS_NEG_POLYGON_CB_INDEX);
    set(jsnames::draw_polygone_sc, TS_NEG_POLYGON_SC_INDEX);
    set(jsnames::draw_ellipse_cb, TS_NEG_ELLIPSE_CB_INDEX);
    set(jsnames::draw_ellipse_sc, TS_NEG_ELLIPSE_SC_INDEX);

    auto supported_orders_impl = this->callbacks["supportedOrders"];
    if (!!supported_orders_impl) {
        supported &= PrimaryDrawingOrdersSupport(supported_orders_impl.as<uint32_t>());
    }

    return supported;
}

Graphics::~Graphics() = default;

Rect Graphics::intersect(Rect const& a, Rect const& b)
{
    return a.intersect(width, height).intersect(b);
}

void Graphics::draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPOpaqueRect");

    const Rect trect = intersect(clip, cmd.rect);

    emval_call(this->callbacks, jsnames::draw_rect,
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        color_decode(cmd.color, color_ctx)
    );
}

void Graphics::draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPMultiOpaqueRect");

    const auto color = color_decode(cmd._Color, color_ctx);
    draw_multi(this->width, this->height, cmd, clip, [color, this](const Rect & trect) {
        emval_call(this->callbacks, jsnames::draw_rect,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            color
        );
    });
}

void Graphics::draw(const RDPScrBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "Graphics::RDPScrBlt");

    const Rect drect = intersect(clip, cmd.rect);
    // adding delta move dest to source
    const auto deltax = cmd.srcx - cmd.rect.x;
    const auto deltay = cmd.srcy - cmd.rect.y;

    emval_call(this->callbacks, jsnames::draw_scr_blt,
        drect.x + deltax,
        drect.y + deltay,
        drect.cx,
        drect.cy,
        drect.x,
        drect.y,
        cmd.rop
    );
}

void Graphics::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "Graphics::RDPMultiScrBlt");

    const int deltax = cmd.nXSrc - cmd.rect.x;
    const int deltay = cmd.nYSrc - cmd.rect.y;

    draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
        emval_call(this->callbacks, jsnames::draw_scr_blt,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            trect.x + deltax,
            trect.y + deltay,
            cmd.bRop
        );
    });
}

void Graphics::draw(const RDPDstBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "Graphics::RDPDstBlt");

    const Rect trect = intersect(clip, cmd.rect);
    emval_call(this->callbacks, jsnames::draw_dest_blt,
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        cmd.rop
    );
}

void Graphics::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "Graphics::RDPMultiDstBlt");

    draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
        emval_call(this->callbacks, jsnames::draw_dest_blt,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            cmd.bRop
        );
    });
}

void Graphics::draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPPatBlt");

    const Rect trect = intersect(clip, cmd.rect);

    emval_call(this->callbacks, jsnames::draw_pat_blt,
        cmd.brush.org_x,
        cmd.brush.org_y,
        cmd.brush.style,
        to_brush_data(cmd.brush),
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        cmd.rop,
        color_decode(cmd.back_color, color_ctx),
        color_decode(cmd.fore_color, color_ctx)
    );
}

void Graphics::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPMultiPatBlt");

    auto back_color = color_decode(cmd.BackColor, color_ctx);
    auto fore_color = color_decode(cmd.ForeColor, color_ctx);

    draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
        emval_call(this->callbacks, jsnames::draw_pat_blt,
            to_brush_data(cmd.brush),
            cmd.brush.org_x,
            cmd.brush.org_y,
            cmd.brush.style,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            cmd.bRop,
            back_color,
            fore_color
        );
    });
}

void Graphics::set_bmp_cache_entries(std::array<CacheEntry, 3> const & cache_entries)
{
    this->image_data_index[0] = 0;
    this->image_data_index[1] = cache_entries[0].nb_entries;
    this->image_data_index[2] = this->image_data_index[1] + cache_entries[1].nb_entries;

    auto f = this->callbacks[jsnames::set_bmp_cache_entries];
    if (!!f) {
        f(
            cache_entries[0].nb_entries,
            cache_entries[0].bmp_size,
            cache_entries[0].is_persistent,
            cache_entries[1].nb_entries,
            cache_entries[1].bmp_size,
            cache_entries[1].is_persistent,
            cache_entries[2].nb_entries,
            cache_entries[2].bmp_size,
            cache_entries[2].is_persistent
        );
    }
}

void Graphics::draw(RDPBmpCache const & cmd)
{
    // LOG(LOG_INFO, "Graphics::RDPBmpCache");

    uint32_t const image_idx = this->image_data_index[cmd.id & 0b11] + cmd.idx;

    emval_call(this->callbacks, jsnames::set_bmp_cache,
        cmd.bmp.data(),
        cmd.bmp.bpp(),
        cmd.bmp.cx(),
        cmd.bmp.cy(),
        uint32_t(cmd.bmp.line_size()),
        image_idx
    );
}

void Graphics::draw(RDPMemBlt const & cmd, Rect clip)
{
    // LOG(LOG_INFO, "Graphics::RDPMemBlt");

    const uint32_t image_idx = this->image_data_index[cmd.cache_id & 0b11] + cmd.cache_idx;

    MemBltPoints ps(cmd.rect.intersect(this->width, this->height), cmd.srcx, cmd.srcy, clip);

    emval_call(this->callbacks, jsnames::draw_memblt,
        image_idx,
        cmd.rop,
        ps.srcx,
        ps.srcy,
        ps.rect.x,
        ps.rect.y,
        ps.rect.cx,
        ps.rect.cy
    );
}

void Graphics::draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPMem3Blt");

    const uint32_t image_idx = this->image_data_index[cmd.cache_id & 0b11] + cmd.cache_idx;

    MemBltPoints ps(cmd.rect.intersect(this->width, this->height), cmd.srcx, cmd.srcy, clip);

    emval_call(this->callbacks, jsnames::draw_mem3blt,
        to_brush_data(cmd.brush),
        cmd.brush.org_x,
        cmd.brush.org_y,
        cmd.brush.style,
        image_idx,
        cmd.rop,
        ps.srcx,
        ps.srcy,
        ps.rect.x,
        ps.rect.y,
        ps.rect.cx,
        ps.rect.cy,
        color_decode(cmd.back_color, color_ctx),
        color_decode(cmd.fore_color, color_ctx)
    );
}

void Graphics::draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPLineTo");

    LineEquation equa(cmd.startx, cmd.starty, cmd.endx, cmd.endy);

    if (not equa.resolve(clip)) {
        return;
    }

    emval_call(this->callbacks, jsnames::draw_line_to,
        cmd.back_mode,
        equa.segin.a.x,
        equa.segin.a.y,
        equa.segin.b.x,
        equa.segin.b.y,
        color_decode(cmd.back_color, color_ctx),
        cmd.pen.style,
        cmd.pen.width,
        color_decode(cmd.pen.color, color_ctx)
    );
}

void Graphics::draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
    // LOG(LOG_INFO, "Graphics::RDPGlyphIndex");

    Rect screen_rect = clip.intersect(this->width, this->height);
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
            emval_call(this->callbacks, jsnames::draw_rect,
                ajusted.x,
                ajusted.y,
                ajusted.cx,
                ajusted.cy,
                color_decode(cmd.fore_color, color_ctx)
            );
        }
    }

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
    const BGRColor color = color_decode(cmd.back_color, color_ctx);
    const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

    const size_t len = clipped_glyph_fragment_rect.width() * clipped_glyph_fragment_rect.height();
    auto img_data = std::make_unique<uint8_t[]>(len * 4);

    auto set_point = [&](int16_t x, int16_t y) {
        x -= clipped_glyph_fragment_rect.x;
        y -= clipped_glyph_fragment_rect.y;
        const auto i = y * clipped_glyph_fragment_rect.cx + x;
        img_data[i+0] = color.red();
        img_data[i+1] = color.green();
        img_data[i+2] = color.blue();
        img_data[i+3] = 255;
    };

    uint16_t draw_pos = 0;

    rdp_draw_glyphs(
        set_point, this->fragment_cache, {cmd.data, cmd.data_len},
        has_delta_bytes, cmd.ui_charinc,
        draw_pos, offset_y, cmd.bk.x + offset_x, cmd.bk.y,
        clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);

    emval_call(this->callbacks, jsnames::draw_image,
        img_data.get(),
        BitsPerPixel::BitsPP32,
        clipped_glyph_fragment_rect.width(),
        clipped_glyph_fragment_rect.height(),
        clipped_glyph_fragment_rect.width(),
        clipped_glyph_fragment_rect.x,
        clipped_glyph_fragment_rect.y
    );
}

void Graphics::draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPPolygonSC");

    emval_call(this->callbacks, jsnames::draw_polygone_sc,
        cmd.xStart,
        cmd.yStart,
        make_emval_delta_points(cmd.deltaPoints, cmd.NumDeltaEntries),
        clip.x,
        clip.y,
        clip.cx,
        clip.cy,
        color_decode(cmd.BrushColor, color_ctx),
        cmd.fillMode
    );
}

void Graphics::draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPPolygonCB");

    emval_call(this->callbacks, jsnames::draw_polygone_cb,
        cmd.xStart,
        cmd.yStart,
        make_emval_delta_points(cmd.deltaPoints, cmd.NumDeltaEntries),
        clip.x,
        clip.y,
        clip.cx,
        clip.cy,
        to_brush_data(cmd.brush),
        cmd.brush.org_x,
        cmd.brush.org_y,
        cmd.brush.style,
        color_decode(cmd.backColor, color_ctx),
        color_decode(cmd.foreColor, color_ctx),
        cmd.fillMode
    );
}

void Graphics::draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPPolyline");

    emval_call(this->callbacks, jsnames::draw_polyline,
        cmd.xStart,
        cmd.yStart,
        make_emval_delta_points(cmd.deltaEncodedPoints, cmd.NumDeltaEntries),
        clip.x,
        clip.y,
        clip.cx,
        clip.cy,
        color_decode(cmd.PenColor, color_ctx)
    );
}

void Graphics::draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPEllipseSC");

    emval_call(this->callbacks, jsnames::draw_ellipse_sc,
        cmd.el.ileft(),
        cmd.el.itop(),
        cmd.el.eright(),
        cmd.el.ebottom(),
        clip.x,
        clip.y,
        clip.cx,
        clip.cy,
        cmd.bRop2,
        color_decode(cmd.color, color_ctx),
        cmd.fillMode
    );
}

void Graphics::draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "Graphics::RDPEllipseCB");

    emval_call(this->callbacks, jsnames::draw_ellipse_cb,
        cmd.el.ileft(),
        cmd.el.itop(),
        cmd.el.eright(),
        cmd.el.ebottom(),
        clip.x,
        clip.y,
        clip.cx,
        clip.cy,
        cmd.brop2,
        color_decode(cmd.back_color, color_ctx),
        color_decode(cmd.fore_color, color_ctx),
        cmd.fill_mode
    );
}

void Graphics::draw(const RDPColCache   & /*unused*/) { }

void Graphics::draw(const RDPBrushCache & /*unused*/) { }

void Graphics::draw(const RDP::FrameMarker & cmd)
{
    // LOG(LOG_INFO, "Graphics::FrameMarker");

    emval_call(this->callbacks, jsnames::frame_marker,
        cmd.action == RDP::FrameMarker::FrameStart);
}

void Graphics::draw(const RDP::RAIL::NewOrExistingWindow & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::WindowIcon & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::CachedIcon & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::DeletedWindow & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::DeletedNotificationIcons & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*unused*/) { }
void Graphics::draw(const RDP::RAIL::NonMonitoredDesktop & /*unused*/) { }


void Graphics::draw(const RDPBitmapData & cmd, const Bitmap & bmp)
{
    // LOG(LOG_INFO, "Graphics::RDPBitmapData");

    const uint16_t dw = cmd.dest_right - cmd.dest_left + 1;
    const uint16_t dh = cmd.dest_bottom - cmd.dest_top + 1;

    emval_call(this->callbacks, jsnames::draw_image,
        bmp.data(),
        bmp.bpp(),
        std::min(bmp.cx(), dw),
        std::min(bmp.cy(), dh),
        uint32_t(bmp.line_size()),
        cmd.dest_left,
        cmd.dest_top
    );
}

void Graphics::set_palette(const BGRPalette& /*unused*/) { }

void Graphics::draw(RDPSetSurfaceCommand const & /*cmd*/) { }

void Graphics::draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) { }


void Graphics::set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode)
{
    // LOG(LOG_INFO, "Graphics::Pointer %d", mode);

    switch (mode) {
    case SetPointerMode::Cached:
        emval_call(this->callbacks, jsnames::cached_pointer, cache_idx);
        break;
    case SetPointerMode::New: {
        const redclient::RGBA8888Image image = redclient::pointer_to_rgba8888(cursor);
        const auto hotspot = cursor.get_hotspot();

        emval_call(this->callbacks, jsnames::new_pointer,
            image.data(),
            image.width,
            image.height,
            cache_idx,
            hotspot.x,
            hotspot.y
        );
        break;
    }
    case SetPointerMode::Insert: {
        const redclient::RGBA8888Image image = redclient::pointer_to_rgba8888(cursor);
        const auto hotspot = cursor.get_hotspot();

        emval_call(this->callbacks, jsnames::set_pointer,
            image.data(),
            image.width,
            image.height,
            hotspot.x,
            hotspot.y
        );
        break;
    }
    }
}

void Graphics::begin_update()
{
    emval_call(this->callbacks, jsnames::frame_marker, true);
}

void Graphics::end_update()
{
    emval_call(this->callbacks, jsnames::frame_marker, false);
}

bool Graphics::resize_canvas(ScreenInfo screen)
{
    this->width = screen.width;
    this->height = screen.height;

    emval_call(this->callbacks, jsnames::resize_canvas,
        screen.width,
        screen.height,
        screen.bpp
    );

    return true;
}

void Graphics::update_pointer_position(uint16_t x, uint16_t y)
{
    emval_call(this->callbacks, jsnames::update_pointer_position, x, y);
}

} // namespace redjs


RED_JS_BINDING_CONSTANTS(
    GraphicOrders,

    (("MultiOpaqueRect", PrimaryDrawingOrdersSupport(TS_NEG_MULTIOPAQUERECT_INDEX).as_uint()))

    (("DstBlt", PrimaryDrawingOrdersSupport(TS_NEG_DSTBLT_INDEX).as_uint()))
    (("MultiDstBlt", PrimaryDrawingOrdersSupport(TS_NEG_MULTIDSTBLT_INDEX).as_uint()))

    (("PatBltAndOpaqueRect", PrimaryDrawingOrdersSupport(TS_NEG_PATBLT_INDEX).as_uint()))
    (("MultiPatBlt", PrimaryDrawingOrdersSupport(TS_NEG_MULTIPATBLT_INDEX).as_uint()))

    (("ScrBlt", PrimaryDrawingOrdersSupport(TS_NEG_SCRBLT_INDEX).as_uint()))
    (("MultiScrBlt", PrimaryDrawingOrdersSupport(TS_NEG_MULTISCRBLT_INDEX).as_uint()))

    (("MemBlt", PrimaryDrawingOrdersSupport(TS_NEG_MEMBLT_INDEX).as_uint()))
    (("Mem3Blt", PrimaryDrawingOrdersSupport(TS_NEG_MEM3BLT_INDEX).as_uint()))

    (("LineTo", PrimaryDrawingOrdersSupport(TS_NEG_LINETO_INDEX).as_uint()))
    (("Polyline", PrimaryDrawingOrdersSupport(TS_NEG_POLYLINE_INDEX).as_uint()))

    (("PolygonSC", PrimaryDrawingOrdersSupport(TS_NEG_POLYGON_SC_INDEX).as_uint()))
    (("PolygonCB", PrimaryDrawingOrdersSupport(TS_NEG_POLYGON_CB_INDEX).as_uint()))

    (("EllipseSC", PrimaryDrawingOrdersSupport(TS_NEG_ELLIPSE_SC_INDEX).as_uint()))
    (("EllipseCB", PrimaryDrawingOrdersSupport(TS_NEG_ELLIPSE_CB_INDEX).as_uint()))

    (("FastGlyph", PrimaryDrawingOrdersSupport(TS_NEG_FAST_GLYPH_INDEX).as_uint()))
    (("Glyph", PrimaryDrawingOrdersSupport(TS_NEG_GLYPH_INDEX).as_uint()))
);
