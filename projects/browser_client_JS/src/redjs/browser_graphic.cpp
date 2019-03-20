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

#include "gdi/screen_info.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "utils/log.hpp"

#include <numeric>


namespace
{
    constexpr char char_hex[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };

    std::array<char, 8> css_color(RDPColor c, gdi::ColorCtx color_ctx) noexcept
    {
        std::array<char, 8> r;

        auto* p = r.data();
        *p++ = '#';

        auto write_hex = [&p](uint8_t byte){
            *p++ = char_hex[byte >> 4];
            *p++ = char_hex[byte & 0xf];
        };

        const BGRColor color = color_decode(c, color_ctx);
        write_hex(color.red());
        write_hex(color.green());
        write_hex(color.blue());

        *p++ = '\0';
        return r;
    }

    Rect intersect(uint16_t w, uint16_t h, Rect const& a, Rect const& b)
    {
        return a.intersect(w, h).intersect(b);
    }

    // TODO removed when RDPMultiDstBlt and RDPMultiOpaqueRect contains a rect member
    //@{
    Rect to_rect(RDPMultiOpaqueRect const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    Rect to_rect(RDP::RDPMultiScrBlt const & cmd)
    { return cmd.rect; }
    //@}

    template<class RDPMulti, class FRect>
    void draw_multi(uint16_t w, uint16_t h, const RDPMulti & cmd, Rect clip, FRect f)
    {
        const Rect clip_drawable_cmd_intersect = intersect(w, h, clip, to_rect(cmd));

        Rect cmd_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
            f(clip_drawable_cmd_intersect.intersect(cmd_rect));
        }
    }
}

namespace redjs
{

BrowserGraphic::BrowserGraphic(redjs::JsTableId id, uint16_t width, uint16_t height, OrderCaps& order_caps)
: width(width)
, height(height)
, id(id)
{
    order_caps.orderSupport[TS_NEG_POLYLINE_INDEX] = 1;
    order_caps.orderSupport[TS_NEG_LINETO_INDEX] = 1;
    order_caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] = 1;
    order_caps.orderSupport[TS_NEG_PATBLT_INDEX] = 1;
    order_caps.orderSupport[TS_NEG_SCRBLT_INDEX] = 1;
    order_caps.orderSupport[TS_NEG_MEMBLT_INDEX] = 1;
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

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawRect($1, $2, $3, $4, Pointer_stringify($5));
        },
        this->id.raw(),
        trect.x,
        trect.y,
        trect.cx,
        trect.cy,
        css_color(cmd.color, color_ctx).data()
    );
}

void BrowserGraphic::draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPMultiOpaqueRect");

    const auto color = css_color(cmd._Color, color_ctx);
    draw_multi(this->width, this->height, cmd, clip, [&color, this](const Rect & trect) {
        RED_EM_ASM(
            {
                Module.RdpClientEventTable[$0].drawRect($1, $2, $3, $4, Pointer_stringify($5));
            },
            this->id.raw(),
            trect.x,
            trect.y,
            trect.cx,
            trect.cy,
            color.data()
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

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawSrcBlt($1, $2, $3, $4, $5, $6, $7);
        },
        this->id.raw(),
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

    const signed int deltax = cmd.nXSrc - cmd.rect.x;
    const signed int deltay = cmd.nYSrc - cmd.rect.y;

    draw_multi(this->width, this->height, cmd, clip, [&](const Rect & trect) {
        RED_EM_ASM(
            {
                Module.RdpClientEventTable[$0].drawSrcBlt($1, $2, $3, $4, $5, $6, $7);
            },
            this->id.raw(),
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

void BrowserGraphic::draw(const RDPDestBlt & /*cmd*/, Rect /*clip*/) { }
void BrowserGraphic::draw(const RDPMultiDstBlt & /*cmd*/, Rect /*clip*/) { }

void BrowserGraphic::draw(RDPPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserGraphic::draw(RDP::RDPMultiPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }

void BrowserGraphic::set_bmp_cache_entries(std::array<uint16_t, 3> const & nb_entries)
{
    this->image_data_index[0] = 0;
    this->image_data_index[1] = nb_entries[0];
    this->image_data_index[2] = this->image_data_index[1] + nb_entries[1];
    this->nb_image_datas = this->image_data_index[2] + nb_entries[2];
    this->image_datas = std::make_unique<ImageData[]>(this->nb_image_datas);
}

void BrowserGraphic::draw(RDPBmpCache const & cmd)
{
    size_t const image_idx = this->image_data_index[cmd.id & 0b11] + cmd.idx;
    if (image_idx >= this->nb_image_datas) {
        LOG(LOG_INFO, "BrowserGraphic::RDPBmpCache: out of range");
        return ;
    }
    image_datas[image_idx] = image_data_from_bitmap(cmd.bmp);
}

void BrowserGraphic::draw(RDPMemBlt const & cmd_, Rect clip)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPMemBlt");

    RDPMemBlt cmd(cmd_);

    cmd.rect = cmd_.rect.intersect(this->width, this->height);
    cmd.srcx += (cmd.rect.x - cmd_.rect.x);
    cmd.srcy += (cmd.rect.y - cmd_.rect.y);

    Rect const rect = clip.intersect(cmd.rect);
    uint16_t const srcx = cmd.srcx + (rect.x - cmd.rect.x);
    uint16_t const srcy = cmd.srcy + (rect.y - cmd.rect.y);

    size_t const image_idx = this->image_data_index[cmd.cache_id & 0b11] + cmd.cache_idx;
    if (image_idx >= this->nb_image_datas) {
        LOG(LOG_ERR, "BrowserGraphic::RDPMemBlt: out of range (%" PRIu16 " %" PRIu16 ")",
            cmd.cache_id, cmd.cache_idx);
        return ;
    }

    ImageData const& image = this->image_datas[image_idx];

    if (image.width() < srcx || image.height() < srcy) {
        return ;
    }

    const int mincx = std::min<int>(image.width() - srcx, std::min<int>(this->width - rect.x, rect.cx));
    const int mincy = std::min<int>(image.height() - srcy, std::min<int>(this->height - rect.y, rect.cy));

    if (mincx <= 0 || mincy <= 0) {
        return;
    }

    // cmd.rop == 0xCC
    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawImage($1, $2, $3, $4, $5, $6, $7, $8, $9);
        },
        this->id.raw(),
        image.data(),
        image.width(),
        image.height(),
        rect.x,
        rect.y,
        srcx,
        srcy,
        mincx,
        mincy
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

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawLineTo(
                $1, $2, $3, $4, $5, Pointer_stringify($6), $7, $8, Pointer_stringify($9));
        },
        cmd.back_mode,
        equa.segin.a.x,
        equa.segin.a.y,
        equa.segin.b.x,
        equa.segin.b.y,
        css_color(cmd.back_color, color_ctx).data(),
        cmd.pen.style,
        cmd.pen.width,
        css_color(cmd.pen.color, color_ctx).data()
    );
}

void BrowserGraphic::draw(RDPGlyphIndex const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, const GlyphCache & /*gly_cache*/) { }

void BrowserGraphic::draw(RDPPolygonSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserGraphic::draw(RDPPolygonCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }

void BrowserGraphic::draw(RDPPolyline const & cmd, Rect /*clip*/, gdi::ColorCtx color_ctx)
{
    // LOG(LOG_INFO, "BrowserGraphic::RDPPolyline");

    const auto color = css_color(cmd.PenColor, color_ctx);

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawPolyline($1, $2, $3, $4, Pointer_stringify($5));
        },
        cmd.xStart,
        cmd.yStart,
        cmd.NumDeltaEntries,
        cmd.deltaEncodedPoints,
        color.data()
    );
}

void BrowserGraphic::draw(RDPEllipseSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserGraphic::draw(RDPEllipseCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserGraphic::draw(const RDPColCache   & /*unused*/) { }
void BrowserGraphic::draw(const RDPBrushCache & /*unused*/) { }
void BrowserGraphic::draw(const RDP::FrameMarker & /*cmd*/) { }
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

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawImage($1, $2, $3, $4, $5, 0, 0, $6, $7);
        },
        this->id.raw(),
        image.data(),
        image.width(),
        image.height(),
        cmd.dest_left,
        cmd.dest_top,
        cmd.dest_right - cmd.dest_left + 1,
        cmd.dest_bottom - cmd.dest_top + 1
    );
}

void BrowserGraphic::set_palette(const BGRPalette& /*unused*/) { }
void BrowserGraphic::draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) {}
void BrowserGraphic::draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) { }


void BrowserGraphic::set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode)
{
    // LOG(LOG_INFO, "BrowserGraphic::Pointer %d", mode);

    switch (mode) {
    case SetPointerMode::Cached:
        RED_EM_ASM(
            {
                Module.RdpClientEventTable[$0].cachedPointer($1);
            },
            this->id.raw(),
            cache_idx
        );
        break;
    case SetPointerMode::New: {
        const redjs::ImageData image = redjs::image_data_from_pointer(cursor);
        const auto hotspot = cursor.get_hotspot();

        RED_EM_ASM(
            {
                Module.RdpClientEventTable[$0].newPointer($1, $2, $3, $4, $5, $6);
            },
            this->id.raw(),
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

        RED_EM_ASM(
            {
                Module.RdpClientEventTable[$0].setPointer($1, $2, $3, $4, $5);
            },
            this->id.raw(),
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

void BrowserGraphic::begin_update() { }
void BrowserGraphic::end_update() { }

bool BrowserGraphic::resize_canvas(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].resizeCanvas($1, $2);
        },
        this->id.raw(),
        width,
        height
    );

    return true;
}

} // namespace redjs
