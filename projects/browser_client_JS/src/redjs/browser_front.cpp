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

#include "redjs/image_data.hpp"
#include "redjs/browser_front.hpp"

#include "red_emscripten/em_asm.hpp"

#include "core/RDP/bitmapupdate.hpp"
#include "utils/log.hpp"


namespace redjs
{

BrowserFront::BrowserFront(ScreenInfo& screen_info, bool verbose)
: width(screen_info.width)
, height(screen_info.height)
, verbose(verbose)
, screen_info(screen_info)
{}

bool BrowserFront::can_be_start_capture()
{
    return false;
}

bool BrowserFront::must_be_stop_capture()
{
    return false;
}

// Rect BrowserFront::intersect(Rect const& a, Rect const& b)
// {
//     return a.intersect(width, height).intersect(b);
// }

void BrowserFront::draw(RDPOpaqueRect const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }

void BrowserFront::draw(const RDPScrBlt & /*cmd*/, Rect /*clip*/) { }
void BrowserFront::draw(const RDPDestBlt & /*cmd*/, Rect /*clip*/) { }
void BrowserFront::draw(const RDPMultiDstBlt & /*cmd*/, Rect /*clip*/) { }
void BrowserFront::draw(RDPMultiOpaqueRect const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDP::RDPMultiPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(const RDP::RDPMultiScrBlt & /*cmd*/, Rect /*clip*/) { }
void BrowserFront::draw(RDPPatBlt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }

void BrowserFront::draw(const RDPMemBlt & /*cmd*/, Rect /*clip*/, const Bitmap & /*bmp*/) { }

void BrowserFront::draw(RDPMem3Blt const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, const Bitmap & /*bmp*/) { }
void BrowserFront::draw(RDPLineTo const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDPGlyphIndex const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, const GlyphCache & /*gly_cache*/) { }
void BrowserFront::draw(RDPPolygonSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDPPolygonCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDPPolyline const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDPEllipseSC const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(RDPEllipseCB const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/) { }
void BrowserFront::draw(const RDPColCache   & /*unused*/) { }
void BrowserFront::draw(const RDPBrushCache & /*unused*/) { }
void BrowserFront::draw(const RDP::FrameMarker & /*cmd*/) { }
void BrowserFront::draw(const RDP::RAIL::NewOrExistingWindow & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::WindowIcon & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::CachedIcon & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::DeletedWindow & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::DeletedNotificationIcons & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::ActivelyMonitoredDesktop & /*unused*/) { }
void BrowserFront::draw(const RDP::RAIL::NonMonitoredDesktop & /*unused*/) { }

void BrowserFront::draw(const RDPBitmapData & cmd, const Bitmap & bmp)
{
    // LOG(LOG_INFO, "JsFront::RDPBitmapData");

    // void ctx.drawImage(image, dx, dy);
    // void ctx.drawImage(image, dx, dy, dWidth, dHeight);
    // void ctx.drawImage(image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight);

    redjs::ImageData image(bmp);

    RED_EM_ASM(
        {
            Module.RdpClientEventTable[$0].drawImage($1, $2, $3, $4, $5, 0, 0, $6, $7);
        },
        this,
        image.data(),
        image.width(),
        image.height(),
        cmd.dest_left,
        cmd.dest_top,
        cmd.dest_right - cmd.dest_left + 1,
        cmd.dest_bottom - cmd.dest_top + 1
    );
}

void BrowserFront::set_palette(const BGRPalette& /*unused*/) { }
void BrowserFront::draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const & /*unused*/) {}
void BrowserFront::draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) { }


BrowserFront::ResizeResult BrowserFront::server_resize(int width, int height, BitsPerPixel bpp)
{
    this->width = width;
    this->height = height;
    this->screen_info.width = width;
    this->screen_info.height = height;
    this->screen_info.bpp = bpp;
    if (this->verbose) {
        LOG(LOG_INFO, "JsFront::server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
    }
    return ResizeResult::instant_done;
}

void BrowserFront::set_pointer(const Pointer & /*unused*/) { }

void BrowserFront::begin_update() { }
void BrowserFront::end_update() { }

void BrowserFront::send_to_channel(
    const CHANNELS::ChannelDef & /*channel*/, const uint8_t * /*data*/,
    std::size_t /*length*/, std::size_t /*chunk_size*/, int /*flags*/)
{
    if (this->verbose) {
        LOG(LOG_INFO, "JsFront::send_to_channel");
    }
}

void BrowserFront::update_pointer_position(uint16_t /*unused*/, uint16_t /*unused*/)
{
    if (this->verbose) {
        LOG(LOG_INFO, "JsFront::update_pointer_position");
    }
}

}
