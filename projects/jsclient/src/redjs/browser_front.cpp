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

#include "redjs/browser_front.hpp"
#include "red_emscripten/em_asm.hpp"

#include "gdi/screen_info.hpp"
#include "utils/log.hpp"


namespace redjs
{

BrowserFront::BrowserFront(emscripten::val callbacks, uint16_t width, uint16_t height, RDPVerbose verbose)
: gd(std::move(callbacks), width, height)
, verbose(verbose)
{
    this->cl.push_back(CHANNELS::ChannelDef(channel_names::cliprdr, 0, 0));
}

BrowserFront::~BrowserFront() = default;

PrimaryDrawingOrdersSupport BrowserFront::get_supported_orders() const
{
    return this->gd.get_supported_orders();
}

void BrowserFront::add_channel_receiver(ChannelReceiver&& channel_receiver)
{
    this->cl.push_back(CHANNELS::ChannelDef(channel_receiver.name(), 0, 0));
    this->channel_receivers.emplace_back(std::move(channel_receiver));
}

bool BrowserFront::can_be_start_capture()
{
    return false;
}

bool BrowserFront::must_be_stop_capture()
{
    return false;
}

BrowserFront::ResizeResult BrowserFront::server_resize(ScreenInfo screen_server)
{
    if (bool(this->verbose & RDPVerbose::graphics)) {
        LOG(LOG_INFO, "BrowserFront::server_resize(width=%d, height=%d, bpp=%d",
        screen_server.width, screen_server.height, screen_server.bpp);
    }

    return this->gd.resize_canvas(screen_server.width, screen_server.height)
        ? ResizeResult::instant_done
        : ResizeResult::fail;
}

void BrowserFront::send_to_channel(
    const CHANNELS::ChannelDef & channel_def, bytes_view chunk_data,
        std::size_t /*total_data_len*/, int flags)
{
    LOG_IF(bool(this->verbose & RDPVerbose::channels),
        LOG_INFO, "BrowserFront::send_to_channel");

    for (ChannelReceiver& receiver : this->channel_receivers)
    {
        if (receiver.name() == channel_def.name)
        {
            receiver(chunk_data, flags);
            break;
        }
    }
}

void BrowserFront::update_pointer_position(uint16_t /*x*/, uint16_t /*y*/)
{
    LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
        LOG_INFO, "BrowserFront::update_pointer_position");
}

} // namespace redjs
