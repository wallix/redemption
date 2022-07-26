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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#pragma once

#include "gdi/graphic_api.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/not_null_ptr.hpp"

#include <vector>


namespace gdi
{

struct GraphicDispatcher : GraphicApi
{
    GraphicDispatcher() = default;

    GraphicDispatcher(std::vector<not_null_ptr<gdi::GraphicApi>> gds)
    : gds(std::move(gds))
    {}

    void add_graphic(gdi::GraphicApi& gd)
    {
        gds.emplace_back(&gd);
    }

    void remove_graphic(gdi::GraphicApi& gd)
    {
        for (auto it = gds.begin(); it != gds.end(); ++it) {
            if (*it == &gd) {
                gds.erase(it);
                break;
            }
        }
    }

    void clear()
    {
        gds.clear();
    }

    array_view<not_null_ptr<gdi::GraphicApi>> graphics() const noexcept
    {
        return gds;
    }

    void set_palette(BGRPalette const& palette) override
    {
        for (auto& gd : gds) {
            gd->set_palette(palette);
        }
    }

    void draw(RDP::FrameMarker const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDPDstBlt const& cmd, Rect clip) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip);
        }
    }

    void draw(RDPMultiDstBlt const& cmd, Rect clip) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip);
        }
    }

    void draw(RDPScrBlt const& cmd, Rect clip) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip);
        }
    }

    void draw(RDP::RDPMultiScrBlt const& cmd, Rect clip) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip);
        }
    }

#ifdef __EMSCRIPTEN__
    void set_bmp_cache_entries(std::array<CacheEntry, 3> const& cache_entries) override
    {
        for (auto& gd : gds) {
            gd->set_bmp_cache_entries(cache_entries);
        }
    }

    void draw(RDPBmpCache const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDPMemBlt const& cmd, Rect clip) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip);
        }
    }

    void draw(RDPMem3Blt const& cmd, Rect clip, ColorCtx color_ctx) override override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }
#else
    void draw(RDPMemBlt const& cmd, Rect clip, Bitmap const& bmp) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, bmp);
        }
    }

    void draw(RDPMem3Blt const& cmd, Rect clip, ColorCtx color_ctx, Bitmap const& bmp) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx, bmp);
        }
    }
#endif

    void draw(RDPBitmapData const& cmd, Bitmap const& bmp) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, bmp);
        }
    }


    void draw(RDPPatBlt const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDP::RDPMultiPatBlt const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPOpaqueRect const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPMultiOpaqueRect const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPLineTo const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPPolygonSC const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPPolygonCB const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPPolyline const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPEllipseSC const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPEllipseCB const& cmd, Rect clip, ColorCtx color_ctx) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx);
        }
    }

    void draw(RDPGlyphIndex const& cmd, Rect clip, ColorCtx color_ctx, GlyphCache const& gly_cache) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, clip, color_ctx, gly_cache);
        }
    }

    void draw(RDPSetSurfaceCommand const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDPSetSurfaceCommand const& cmd, RDPSurfaceContent const& content) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd, content);
        }
    }


    // NOTE maybe in an other interface
    void draw(RDP::RAIL::NewOrExistingWindow const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::WindowIcon const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::CachedIcon const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::DeletedWindow const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::NewOrExistingNotificationIcons const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::DeletedNotificationIcons const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::ActivelyMonitoredDesktop const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDP::RAIL::NonMonitoredDesktop const& cmd) override
    {
        for (auto& gd : gds) {
            gd->draw(cmd);
        }
    }

    void draw(RDPColCache const& cache) override
    {
        for (auto& gd : gds) {
            gd->draw(cache);
        }
    }

    void draw(RDPBrushCache const& cache) override
    {
        for (auto& gd : gds) {
            gd->draw(cache);
        }
    }

    void begin_update() override
    {
        for (auto& gd : gds) {
            gd->begin_update();
        }
    }

    void end_update() override
    {
        for (auto& gd : gds) {
            gd->end_update();
        }
    }

    void sync() override
    {
        for (auto& gd : gds) {
            gd->sync();
        }
    }

    void cached_pointer(CachePointerIndex cache_idx) override
    {
        for (auto& gd : gds) {
            gd->cached_pointer(cache_idx);
        }
    }

    /// \pre cache_idx.is_predefined_pointer() == false
    void new_pointer(CachePointerIndex cache_idx, RdpPointerView const& cursor) override
    {
        for (auto& gd : gds) {
            gd->new_pointer(cache_idx, cursor);
        }
    }

    void set_row(std::size_t rownum, bytes_view data) override
    {
        for (auto& gd : gds) {
            gd->set_row(rownum, data);
        }
    }

private:
    std::vector<not_null_ptr<gdi::GraphicApi>> gds;
};

}
