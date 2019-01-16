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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean

   Fake Graphic class for Unit Testing
*/

#include "test_only/front/fake_front.hpp"
#include "test_only/gdi/test_graphic.hpp"

#include "core/channel_list.hpp"


struct FakeFront::D
{
    ScreenInfo& screen_info;
    TestGraphic gd;
    CHANNELS::ChannelDefArray cl;
};

void FakeFront::draw(RDP::FrameMarker    const & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(RDPDestBlt          const & cmd, Rect clip)
{
    d->gd->draw(cmd, clip);
}

void FakeFront::draw(RDPMultiDstBlt      const & cmd, Rect clip)
{
    d->gd->draw(cmd, clip);
}

void FakeFront::draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPScrBlt           const & cmd, Rect clip)
{
    d->gd->draw(cmd, clip);
}

void FakeFront::draw(RDP::RDPMultiScrBlt const & cmd, Rect clip)
{
    d->gd->draw(cmd, clip);
}

void FakeFront::draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    d->gd->draw(cmd, clip, color_ctx);
}

void FakeFront::draw(RDPBitmapData       const & cmd, Bitmap const & bmp)
{
    d->gd->draw(cmd, bmp);
}

void FakeFront::draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp)
{
    d->gd->draw(cmd, clip, bmp);
}

void FakeFront::draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp)
{
    d->gd->draw(cmd, clip, color_ctx, bmp);
}

void FakeFront::draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache)
{
    d->gd->draw(cmd, clip, color_ctx, gly_cache);
}

void FakeFront::draw(const RDP::RAIL::NewOrExistingWindow            & cmd)
{
    d->gd->draw(cmd);
}
void FakeFront::draw(const RDP::RAIL::WindowIcon                     & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::CachedIcon                     & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::DeletedWindow                  & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::DeletedNotificationIcons       & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(const RDP::RAIL::NonMonitoredDesktop            & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(RDPColCache   const & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::draw(RDPBrushCache const & cmd)
{
    d->gd->draw(cmd);
}

void FakeFront::set_palette(const BGRPalette &)
{
}

void FakeFront::set_pointer(const Pointer & cursor)
{
    d->gd->set_pointer(cursor);
}

void FakeFront::sync()
{
}

const CHANNELS::ChannelDefArray & FakeFront::get_channel_list(void) const
{
    return d->cl;
}

void FakeFront::send_to_channel(
    const CHANNELS::ChannelDef &, uint8_t const * /*data*/, size_t /*length*/,
    size_t /*chunk_size*/, int /*flags*/)
{
}

void FakeFront::begin_update()
{
}

void FakeFront::end_update()
{
}

FakeFront::ResizeResult FakeFront::server_resize(int width, int height, BitsPerPixel bpp)
{
    this->d->gd.resize(width, height);
    this->d->screen_info.bpp = bpp;
    return ResizeResult::done;
}

FakeFront::operator ConstImageDataView() const
{
    return d->gd;
}

FakeFront::FakeFront(ScreenInfo& screen_info)
: d(new D{
    screen_info,
    TestGraphic(screen_info.width, screen_info.height),
    {}
})
{}

FakeFront::~FakeFront() = default;
