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

namespace gdi
{

struct UpdatableGraphics : gdi::GraphicApi
{
    bool has_drawing_event() const
    {
        return this->has_drawing_event_;
    }

    void set_drawing_event(bool has_drawing_event)
    {
        this->has_drawing_event_ = has_drawing_event;
    }

    void draw(RDP::FrameMarker const & cmd) override
    {
        (void)cmd;
    }

    void draw(RDPDstBlt const & cmd, Rect clip) override
    {
        (void)cmd;
        (void)clip;
        this->has_drawing_event_ = true;
    }

    void draw(RDPMultiDstBlt const & cmd, Rect clip) override
    {
        (void)cmd;
        (void)clip;
        this->has_drawing_event_ = true;
    }

    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPScrBlt const & cmd, Rect clip) override
    {
        (void)cmd;
        (void)clip;
        this->has_drawing_event_ = true;
    }

    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override
    {
        (void)cmd;
        (void)clip;
        this->has_drawing_event_ = true;
    }

    void draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

    void draw(RDPBitmapData const & cmd, Bitmap const & bmp) override
    {
        (void)cmd;
        (void)bmp;
        this->has_drawing_event_ = true;
    }

#ifdef __EMSCRIPTEN__

    void set_bmp_cache_entries(std::array<uint16_t, 3> const & nb_entries) override
    {
        (void)nb_entries;
    }

    void draw(RDPBmpCache const & cmd) override
    {
        (void)cmd;
    }

    void draw(RDPMemBlt const & cmd, Rect clip) override
    {
        (void)cmd;
        (void)clip;
        this->has_drawing_event_ = true;
    }

    void draw(RDPMem3Blt const & cmd, Rect clip, ColorCtx color_ctx) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        this->has_drawing_event_ = true;
    }

#else

    void draw(RDPMemBlt const & cmd, Rect clip, Bitmap const & bmp) override
    {
        (void)cmd;
        (void)clip;
        (void)bmp;
        this->has_drawing_event_ = true;
    }

    void draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        (void)bmp;
        this->has_drawing_event_ = true;
    }

#endif

    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override
    {
        (void)cmd;
        (void)clip;
        (void)color_ctx;
        (void)gly_cache;
        this->has_drawing_event_ = true;
    }

    void draw(RDPSetSurfaceCommand const & cmd) override
    {
        (void)cmd;
        this->has_drawing_event_ = true;
    }

    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override
    {
        (void)cmd;
        (void)content;
        this->has_drawing_event_ = true;
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::WindowIcon & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::CachedIcon & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::DeletedWindow & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & cmd) override
    {
        (void)cmd;
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & cmd) override
    {
        (void)cmd;
    }

    void draw(RDPColCache const & cmd) override
    {
        (void)cmd;
    }

    void draw(RDPBrushCache const & cmd) override
    {
        (void)cmd;
    }

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override
    {
        (void)cache_idx;
        (void)cursor;
        (void)mode;
        this->has_drawing_event_ = true;
    }

    void set_palette(BGRPalette const & palette) override
    {
        (void)palette;
    }

    void sync() override
    {}

    void set_row(std::size_t rownum, bytes_view data) override
    {
        (void)rownum;
        (void)data;
        this->has_drawing_event_ = true;
    }

    void begin_update() override
    {}

    void end_update() override
    {}

private:
    bool has_drawing_event_ = false;
};

} // namespace gdi
