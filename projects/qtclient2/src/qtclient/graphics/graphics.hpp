/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "gdi/graphic_api.hpp"

#include <QtGui/QPixmap>
#include <QtGui/QPainter>


namespace qtclient
{

class Graphics : public gdi::GraphicApi
{
public:
    Graphics() noexcept = default;

    void resize(int width, int height);

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPBitmapData & cmd, Bitmap const& bmp) override;
    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPScrBlt & cmd, Rect clip) override;
    void draw(const RDPMemBlt & cmd, Rect clip, Bitmap const& bmp) override;
    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp) override;
    void draw(const RDPDstBlt & cmd, Rect clip) override;
    void draw(const RDPMultiDstBlt & cmd, Rect clip) override;
    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override;
    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override;
    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override;

    void draw(const RDP::FrameMarker & order) override
    {
        (void)order;
    }

    void draw(const RDPSetSurfaceCommand & cmd) override;
    void draw(const RDPSetSurfaceCommand & cmd, RDPSurfaceContent const & content) override;

    void draw(const RDP::RAIL::NewOrExistingWindow &) override {}
    void draw(const RDP::RAIL::WindowIcon &) override {}
    void draw(const RDP::RAIL::CachedIcon &) override {}
    void draw(const RDP::RAIL::DeletedWindow &) override {}
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons &) override {}
    void draw(const RDP::RAIL::DeletedNotificationIcons &) override {}
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop &) override {}
    void draw(const RDP::RAIL::NonMonitoredDesktop &) override {}

    void new_pointer(gdi::CachePointerIndex cache_idx, const RdpPointerView & cursor) override
    {
        (void)cache_idx;
        (void)cursor;
    }

    void cached_pointer(gdi::CachePointerIndex cache_idx) override
    {
        (void)cache_idx;
    }

    Rect get_and_reset_updated_rect()
    {
        auto tmp = updated_rect;
        updated_rect = Rect();
        return tmp;
    }

    QPixmap& get_pixmap() noexcept { return this->cache; }
    QPainter& get_painter() noexcept { return this->painter; }

private:
    uint16_t width = 0;
    uint16_t height = 0;
    Rect updated_rect;
    QPixmap cache;
    QPainter painter;
};

}
