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

#pragma once

#ifdef IN_IDE_PARSER
# define __EMSCRIPTEN__
#endif

#include "gdi/graphic_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "redjs/js_table_id.hpp"

#include <emscripten/val.h>

#include <vector>
#include <memory>


class ScreenInfo;
class OrderCaps;

namespace redjs
{

class ImageData;

class BrowserGraphic : public gdi::GraphicApi
{
public:
    BrowserGraphic(emscripten::val callbacks, JsTableId id, uint16_t width, uint16_t height, OrderCaps& order_caps);
    ~BrowserGraphic();

    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDPScrBlt & cmd, Rect clip) override;
    void draw(const RDPDestBlt & cmd, Rect clip) override;
    void draw(const RDPMultiDstBlt & cmd, Rect clip) override;
    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override;
    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void set_bmp_cache_entries(std::array<uint16_t, 3> const & /*nb_entries*/) override;
    void draw(RDPBmpCache const & /*cmd*/) override;
    void draw(RDPMemBlt const & /*cmd*/, Rect /*clip*/) override;
    void draw(RDPMem3Blt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) override;

    void draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override;
    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPColCache   & /*unused*/) override;
    void draw(const RDPBrushCache & /*unused*/) override;
    void draw(const RDP::FrameMarker & cmd) override;
    void draw(const RDP::RAIL::NewOrExistingWindow & /*unused*/) override;
    void draw(const RDP::RAIL::WindowIcon & /*unused*/) override;
    void draw(const RDP::RAIL::CachedIcon & /*unused*/) override;
    void draw(const RDP::RAIL::DeletedWindow & /*unused*/) override;
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) override;
    void draw(const RDP::RAIL::DeletedNotificationIcons & /*unused*/) override;
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*unused*/) override;
    void draw(const RDP::RAIL::NonMonitoredDesktop & /*unused*/) override;

    void draw(const RDPBitmapData & cmd, const Bitmap & bmp) override;

    void set_palette(const BGRPalette& /*unused*/) override;
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) override;
    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & /*content*/) override;

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override;

    void begin_update() override;
    void end_update() override;

    bool resize_canvas(uint16_t width, uint16_t height);

private:
    Rect intersect(Rect const& a, Rect const& b);

    uint16_t width;
    uint16_t height;
    JsTableId id;
    emscripten::val callbacks;
    std::unique_ptr<ImageData[]> image_datas;
    std::array<size_t, 3> image_data_index {0};
    size_t nb_image_datas {0};
};

}
