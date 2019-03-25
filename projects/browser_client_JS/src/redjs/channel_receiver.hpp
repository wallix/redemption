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

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "core/channel_names.hpp"

#include <functional>


namespace redjs
{

struct ChannelReceiver
{
    using receiver_type = std::function<void(cbytes_view data, int channel_flags)>;

    ChannelReceiver(CHANNELS::ChannelNameId name_id, receiver_type receiver);

    CHANNELS::ChannelNameId name() const noexcept { return this->name_id; }

    void operator()(cbytes_view data, int channel_flags);

private:
    CHANNELS::ChannelNameId name_id;
    receiver_type do_receive;
};

} // namespace redjs
