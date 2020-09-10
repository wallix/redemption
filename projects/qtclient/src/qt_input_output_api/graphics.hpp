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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
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
    Graphics() = default;

    void resize(int width, int height);

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPBitmapData & cmd, Bitmap const& bmp) override;
    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPScrBlt & cmd, Rect clip) override;
    void draw(const RDPMemBlt & cmd, Rect clip, Bitmap const& bmp) override;
    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp) override;
    void draw(const RDPDestBlt & cmd, Rect clip) override;
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
    void draw(const RDP::FrameMarker & order) override;
    void draw(const RDPSetSurfaceCommand & cmd) override;
    void draw(const RDPSetSurfaceCommand & cmd, RDPSurfaceContent const & content) override;

    QPixmap& getPixmap() noexcept { return this->cache; }
    QPainter& getPainter() noexcept { return this->painter; }

private:
    QPixmap cache;
    QPainter painter;
};

}
