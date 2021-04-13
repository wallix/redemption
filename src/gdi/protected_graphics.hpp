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

#pragma once

#include "gdi/graphic_api.hpp"

class RdpInput;


namespace gdi
{

class ProtectedGraphics : public gdi::GraphicApi
{
    Rect protected_rect;
    gdi::GraphicApi & drawable;
    RdpInput & rdp_input; // only for rdp_input_invalide2(array_view<Rect>)

public:
    ProtectedGraphics(gdi::GraphicApi & drawable, RdpInput & rdp_input, Rect rect)
    : protected_rect(rect)
    , drawable(drawable)
    , rdp_input(rdp_input)
    {}

    [[nodiscard]] Rect get_protected_rect() const
    {
        return this->protected_rect;
    }

    void set_protected_rect(Rect const rect)
    {
        this->protected_rect = rect;
    }

    void draw(RDP::FrameMarker    const & cmd) override { this->drawable.draw(cmd); }
    void draw(RDPDstBlt           const & cmd, Rect clip) override;
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override;
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPScrBlt           const & cmd, Rect clip) override;
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override;
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override;
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override;
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override;
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override;

    // TODO unimplemented
    void draw(RDPSetSurfaceCommand const & cmd) override { this->drawable.draw(cmd); }
    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override { this->drawable.draw(cmd, content); }


    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->drawable.draw(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->drawable.draw(cmd); }

    void draw(RDPColCache   const & cmd) override { this->drawable.draw(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->drawable.draw(cmd); }

    void new_pointer(gdi::CachePointerIndex cache_idx, const RdpPointerView & cursor) override
    {
        this->drawable.new_pointer(cache_idx, cursor);
    }

    void cached_pointer(gdi::CachePointerIndex cache_idx) override
    {
        this->drawable.cached_pointer(cache_idx);
    }

    void set_palette(BGRPalette const & palette) override
    {
        this->drawable.set_palette(palette);
    }

    void sync() override
    {
        this->drawable.sync();
    }

    void set_row(std::size_t rownum, bytes_view data) override
    {
        this->drawable.set_row(rownum, data);
    }

    void begin_update() override
    {
        this->drawable.begin_update();
    }

    void end_update() override
    {
        this->drawable.end_update();
    }

    [[nodiscard]] gdi::GraphicApi & get_graphic_proxy() const
    {
        return this->drawable;
    }
};

}  // namespace gdi
