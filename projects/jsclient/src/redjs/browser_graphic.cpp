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

#include "redjs/browser_graphic.hpp"

#include "redjs/image_data_from_bitmap.hpp"
#include "redjs/image_data_from_pointer.hpp"

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/val.hpp"

#include "gdi/screen_info.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/rdp_draw_glyphs.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "utils/log.hpp"

#include <numeric>
#include <cinttypes>


namespace
{
    Rect intersect(uint16_t w, uint16_t h, Rect const& a, Rect const& b)
    {
        return a.intersect(w, h).intersect(b);
    }

    // TODO removed when RDPMultiScrBlt and RDPMultiOpaqueRect contains a rect member
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

    namespace jsnames
    {
        constexpr char const* draw_rect = "drawRect";
        constexpr char const* draw_scr_blt = "drawSrcBlt";
        constexpr char const* draw_line_to = "drawLineTo";
        constexpr char const* draw_polyline = "drawPolyline";
        constexpr char const* draw_pat_blt = "drawPatBlt";
        constexpr char const* draw_pat_blt_ex = "drawPatBltEx";
        constexpr char const* draw_dest_blt = "drawDestBlt";

        constexpr char const* draw_image = "drawImage";
        constexpr char const* draw_memblt = "drawCachedImage";
        constexpr char const* set_cached_image = "cachedImage";
        constexpr char const* set_cached_image_size = "setCachedImageSize";

        constexpr char const* cached_pointer = "cachedPointer";
        constexpr char const* new_pointer = "newPointer";
        constexpr char const* set_pointer = "setPointer";

        constexpr char const* resize_canvas = "resizeCanvas";
    }
}


inline uint32_t emval_call_arg(BGRColor const& bgr) noexcept
{
    return BGRColor(BGRasRGBColor(bgr)).as_u32();
}

namespace redjs
{

BrowserGraphic::BrowserGraphic(emscripten::val callbacks, uint16_t width, uint16_t height)
: width(width)
, height(height)
, callbacks(std::move(callbacks))
{
}

PrimaryDrawingOrdersSupport BrowserGraphic::get_supported_orders() const
{
    PrimaryDrawingOrdersSupport supported {};
    auto set = [&](char const* name, auto f){
        supported |= !!callbacks[name] ? f : PrimaryDrawingOrdersSupport{};
    };

    supported |= PrimaryDrawingOrdersSupport{}
        | TS_NEG_OPAQUERECT_INDEX       // mendatory support
        | TS_NEG_MULTIOPAQUERECT_INDEX; // based on opaque rect

    set(jsnames::draw_scr_blt, PrimaryDrawingOrdersSupport{}
        | TS_NEG_SCRBLT_INDEX
        | TS_NEG_MULTISCRBLT_INDEX);

    set(jsnames::draw_memblt, TS_NEG_MEMBLT_INDEX);
    set(jsnames::draw_line_to, TS_NEG_LINETO_INDEX);
    set(jsnames::draw_polyline, TS_NEG_POLYLINE_INDEX);

    return supported;
}

BrowserGraphic::~BrowserGraphic() = default;

Rect BrowserGraphic::intersect(Rect const& a, Rect const& b)
{
    return a.intersect(width, height).intersect(b);
}

void BrowserGraphic::draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPOpaqueRect");

    const Rect trect = intersect(clip, cmd.rect);

    emval_call(this->callbacks, jsnames::draw_rect,
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        color_decode(cmd.color, color_ctx)
    );
}

void BrowserGraphic::draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPMultiOpaqueRect");

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

void BrowserGraphic::draw(const RDPScrBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPScrBlt");

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

void BrowserGraphic::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPMultiScrBlt");

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

void BrowserGraphic::draw(const RDPDestBlt & cmd, Rect clip)
{
    const Rect trect = intersect(clip, cmd.rect);
    emval_call(this->callbacks, jsnames::draw_dest_blt,
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        cmd.rop
    );
}

void BrowserGraphic::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
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

void BrowserGraphic::draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const Rect trect = intersect(clip, cmd.rect);

    auto back_color = color_decode(cmd.back_color, color_ctx);
    if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {
        auto fore_color = color_decode(cmd.fore_color, color_ctx);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;

        emval_call(this->callbacks, jsnames::draw_pat_blt_ex,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            cmd.rop,
            back_color,
            fore_color,
            brush_data
        );
    }
    else {
        emval_call(this->callbacks, jsnames::draw_pat_blt,
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            cmd.rop,
            back_color
        );
    }
}

void BrowserGraphic::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    auto back_color = color_decode(cmd.BackColor, color_ctx);
    if (cmd.brush.style == 0x03 && (cmd.bRop == 0xF0 || cmd.bRop == 0x5A)) {
        auto fore_color = color_decode(cmd.ForeColor, color_ctx);
        uint8_t brush_data[8];
        memcpy(brush_data, cmd.brush.extra, 7);
        brush_data[7] = cmd.brush.hatch;
        draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
            emval_call(this->callbacks, jsnames::draw_pat_blt_ex,
                trect.x,
                trect.y,
                trect.cx,
                trect.cy,
                cmd.bRop,
                back_color,
                fore_color,
                brush_data
            );
        });
    }
    else {
        draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
            emval_call(this->callbacks, jsnames::draw_pat_blt,
                trect.x,
                trect.y,
                trect.cx,
                trect.cy,
                cmd.bRop,
                back_color
            );
        });
    }
}

void BrowserGraphic::set_bmp_cache_entries(std::array<uint16_t, 3> const & nb_entries)
{
    this->image_data_index[0] = 0;
    this->image_data_index[1] = nb_entries[0];
    this->image_data_index[2] = this->image_data_index[1] + nb_entries[1];
    this->nb_image_datas = this->image_data_index[2] + nb_entries[2];
    emval_call(this->callbacks, jsnames::set_cached_image_size, this->nb_image_datas);
}

void BrowserGraphic::draw(RDPBmpCache const & cmd)
{
    uint32_t const image_idx = this->image_data_index[cmd.id & 0b11] + cmd.idx;
    if (image_idx >= this->nb_image_datas) {
        LOG(LOG_INFO, "BrowserGraphic::RDPBmpCache: out of range");
        return ;
    }

    auto img = image_data_from_bitmap(cmd.bmp);
    emval_call(this->callbacks, jsnames::set_cached_image,
        img.data(),
        img.width(),
        img.height(),
        image_idx
    );
}

void BrowserGraphic::draw(RDPMemBlt const & cmd_, Rect clip)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPMemBlt");

    uint32_t const image_idx = this->image_data_index[cmd_.cache_id & 0b11] + cmd_.cache_idx;
    if (image_idx >= this->nb_image_datas) {
        LOG(LOG_ERR, "BrowserGraphic::RDPMemBlt: out of range (%" PRIu16 " %" PRIu16 ")",
            cmd_.cache_id, cmd_.cache_idx);
        return ;
    }

    RDPMemBlt cmd(cmd_);

    cmd.rect = cmd_.rect.intersect(this->width, this->height);
    cmd.srcx += (cmd.rect.x - cmd_.rect.x);
    cmd.srcy += (cmd.rect.y - cmd_.rect.y);

    Rect const rect = clip.intersect(cmd.rect);
    uint16_t const srcx = cmd.srcx + (rect.x - cmd.rect.x);
    uint16_t const srcy = cmd.srcy + (rect.y - cmd.rect.y);

    // ImageData const& image = this->image_datas[image_idx];
    //
    // if (image.width() < srcx || image.height() < srcy) {
    //     return ;
    // }
    //
    // const int mincx = std::min<int>(image.width() - srcx, std::min<int>(this->width - rect.x, rect.cx));
    // const int mincy = std::min<int>(image.height() - srcy, std::min<int>(this->height - rect.y, rect.cy));
    //
    // if (mincx <= 0 || mincy <= 0) {
    //     return;
    // }

    // cmd.rop == 0xCC
    emval_call(this->callbacks, jsnames::draw_memblt,
        image_idx,
        cmd_.rop,
        rect.x,
        rect.y,
        srcx,
        srcy,
        rect.cx,
        rect.cy
    );

    // switch (cmd.rop) {
    // case 0xCC:  // dest
    // case 0x55:  // dest = NOT source
    // case 0x22:  // dest = dest AND (NOT source)
    // case 0x66:  // dest = source XOR dest (SRCINVERT)
    // case 0x88:  // dest = source AND dest (SRCAND)
    // case 0xBB:  // dest = (NOT source) OR dest (MERGEPAINT)
    // case 0xEE:  // dest = source OR dest (SRCPAINT)
    //     break;
    // default:
    //     // should not happen
    //     //LOG(LOG_INFO, "Unsupported Rop=0x%02X", cmd.rop);
    // break;
    // }
}

void BrowserGraphic::draw(RDPMem3Blt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) {}

void BrowserGraphic::draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPLineTo");

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

void BrowserGraphic::draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
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
        clipped_glyph_fragment_rect.width(),
        clipped_glyph_fragment_rect.height(),
        0xCC,
        clipped_glyph_fragment_rect.x,
        clipped_glyph_fragment_rect.y
    );
}

void BrowserGraphic::draw(RDPPolygonSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/)
{
    LOG(LOG_DEBUG, "RDPPolygonSC unsupported");
}

void BrowserGraphic::draw(RDPPolygonCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/)
{
    LOG(LOG_DEBUG, "RDPPolygonCB unsupported");
}

void BrowserGraphic::draw(RDPPolyline const & cmd, Rect /*clip*/, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPPolyline");

    const auto color = color_decode(cmd.PenColor, color_ctx);

    emval_call(this->callbacks, jsnames::draw_polyline,
        cmd.xStart,
        cmd.yStart,
        cmd.NumDeltaEntries,
        reinterpret_cast<void const*>(cmd.deltaEncodedPoints),
        color
    );
}

void BrowserGraphic::draw(RDPEllipseSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/)
{
    LOG(LOG_DEBUG, "RDPEllipseSC unsupported");
}

void BrowserGraphic::draw(RDPEllipseCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/)
{
    LOG(LOG_DEBUG, "RDPEllipseCB unsupported");
}

void BrowserGraphic::draw(const RDPColCache   & /*unused*/) { }

void BrowserGraphic::draw(const RDPBrushCache & /*unused*/) { }

void BrowserGraphic::draw(const RDP::FrameMarker & /*cmd*/) {}

void BrowserGraphic::draw(const RDP::RAIL::NewOrExistingWindow & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::WindowIcon & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::CachedIcon & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::DeletedWindow & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::DeletedNotificationIcons & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*unused*/) { }
void BrowserGraphic::draw(const RDP::RAIL::NonMonitoredDesktop & /*unused*/) { }


void BrowserGraphic::draw(const RDPBitmapData & cmd, const Bitmap & bmp)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPBitmapData");

    redjs::ImageData image = image_data_from_bitmap(bmp);

    emval_call(this->callbacks, jsnames::draw_image,
        image.data(),
        image.width(),
        image.height(),
        0xCC,
        cmd.dest_left,
        cmd.dest_top,
        0,
        0,
        cmd.dest_right - cmd.dest_left + 1,
        cmd.dest_bottom - cmd.dest_top + 1
    );
}

void BrowserGraphic::set_palette(const BGRPalette& /*unused*/) { }

void BrowserGraphic::draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) { }

void BrowserGraphic::draw(RDPSetSurfaceCommand const & /*cmd*/) { }

void BrowserGraphic::draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) { }


void BrowserGraphic::set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode)
{
    // LOG(LOG_INFO, "BrowserGraphic::Pointer %d", mode);

    switch (mode) {
    case SetPointerMode::Cached:
        emval_call(this->callbacks, jsnames::cached_pointer, cache_idx);
        break;
    case SetPointerMode::New: {
        const redjs::ImageData image = redjs::image_data_from_pointer(cursor);
        const auto hotspot = cursor.get_hotspot();

        emval_call(this->callbacks, jsnames::new_pointer,
            image.data(),
            image.width(),
            image.height(),
            cache_idx,
            hotspot.x,
            hotspot.y
        );
        break;
    }
    case SetPointerMode::Insert: {
        const redjs::ImageData image = redjs::image_data_from_pointer(cursor);
        const auto hotspot = cursor.get_hotspot();

        emval_call(this->callbacks, jsnames::set_pointer,
            image.data(),
            image.width(),
            image.height(),
            hotspot.x,
            hotspot.y
        );
        break;
    }
    }
}

void BrowserGraphic::begin_update() {}

void BrowserGraphic::end_update() {}


bool BrowserGraphic::resize_canvas(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;

    emval_call(this->callbacks, jsnames::resize_canvas,
        width,
        height
    );

    return true;
}

} // namespace redjs
