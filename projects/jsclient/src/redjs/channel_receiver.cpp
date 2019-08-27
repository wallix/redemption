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

#include "redjs/channel_receiver.hpp"
#include "utils/stream.hpp"
#include "core/callback.hpp"


redjs::ChannelReceiver::ChannelReceiver(CHANNELS::ChannelNameId name_id, receiver_type receiver)
: name_id(name_id)
, do_receive(std::move(receiver))
{
    assert(bool(receiver));
}

void redjs::ChannelReceiver::operator()(bytes_view data, int channel_flags)
{
    this->do_receive(data, channel_flags);
}


#include "red_emscripten/bind.hpp"

EMSCRIPTEN_BINDINGS(channel_receiver)
{
    redjs::class_<redjs::ChannelReceiver>("ChannelReceiver");
}
