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

const CHANNELS::ChannelDefArray & FakeFront::get_channel_list() const
{
    return d->cl;
}

void FakeFront::send_to_channel(
    const CHANNELS::ChannelDef &, uint8_t const * /*data*/, size_t /*length*/,
    size_t /*chunk_size*/, int /*flags*/)
{
}

FakeFront::ResizeResult FakeFront::server_resize(ScreenInfo screen_server)
{
    this->d->gd.resize(screen_server.width, screen_server.height);
    this->d->screen_info.bpp = screen_server.bpp;
    return ResizeResult::done;
}

FakeFront::operator ConstImageDataView() const
{
    return d->gd;
}

gdi::GraphicApi& FakeFront::gd() noexcept
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
