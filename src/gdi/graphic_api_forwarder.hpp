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
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "gdi/graphic_api.hpp"

namespace gdi
{

template<class ForwardTo>
class GraphicApiForwarder : public gdi::GraphicApi
{
protected:
    ForwardTo sink;

public:
    template<class... Ts>
    GraphicApiForwarder(Ts&&... xs)
    : sink(static_cast<Ts&&>(xs)...)
    {}

    void draw(RDP::FrameMarker const & cmd)
            override { this->sink.draw(cmd); }
    void draw(RDPDstBlt const & cmd, Rect clip)
            override { this->sink.draw(cmd, clip); }
    void draw(RDPMultiDstBlt const & cmd, Rect clip)
            override { this->sink.draw(cmd, clip); }
    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPScrBlt const & cmd, Rect clip)
            override { this->sink.draw(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip)
            override { this->sink.draw(cmd, clip); }
    void draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx)
            override { this->sink.draw(cmd, clip, color_ctx); }
    void draw(RDPBitmapData const & cmd, Bitmap const & bmp)
            override { this->sink.draw(cmd, bmp); }
#ifdef __EMSCRIPTEN__
    void set_bmp_cache_entries(std::array<uint16_t, 3> const & nb_entries)
        override { this->sink.set_bmp_cache_entries(nb_entries);}
    void draw(RDPBmpCache const & cmd)
        override { this->sink.draw(cmd); }
    void draw(RDPMemBlt const & cmd, Rect clip)
        override { this->sink.draw(cmd, clip); }
    void draw(RDPMem3Blt const & cmd, Rect clip, ColorCtx color_ctx)
        override { this->sink.draw(cmd, clip, color_ctx); }
#else
    void draw(RDPMemBlt const & cmd, Rect clip, Bitmap const & bmp)
            override { this->sink.draw(cmd, clip, bmp);}
    void draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp)
            override { this->sink.draw(cmd, clip, color_ctx, bmp); }
#endif
    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache)
            override { this->sink.draw(cmd, clip, color_ctx, gly_cache); }
    void draw(RDPSetSurfaceCommand const & cmd) override { this->sink.draw(cmd); }
    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content)
            override { this->sink.draw(cmd, content); }
    void draw(const RDP::RAIL::NewOrExistingWindow & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::WindowIcon & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::CachedIcon & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::DeletedWindow & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & cmd)
            override { this->sink.draw(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop & cmd)
            override { this->sink.draw(cmd); }
    void draw(RDPColCache const & cmd)
            override { this->sink.draw(cmd); }
    void draw(RDPBrushCache const & cmd)
            override { this->sink.draw(cmd); }

    void set_palette(BGRPalette const & palette)
            override { this->sink.set_palette(palette); }
    void sync()
            override {this->sink.sync();}
    void set_row(std::size_t rownum, bytes_view data)
            override {this->sink.set_row(rownum, data);}
    void begin_update()
            override {this->sink.begin_update();}
    void end_update()
            override {this->sink.end_update();}
};

}  // namespace gdi
