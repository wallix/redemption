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
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "core/channel_names.hpp"

#include <functional>


namespace redjs
{

struct ChannelReceiver
{
    using receiver_func_t = void(*)(
        void* ctx, bytes_view data,
        uint32_t total_data_len, uint32_t channel_flags
    );

    void operator()(bytes_view data, std::size_t total_data_len, uint32_t channel_flags)
    {
        this->do_receive(this->ctx, data, checked_cast<uint32_t>(total_data_len), channel_flags);
    }

    CHANNELS::ChannelNameId channel_name;
    void * ctx = nullptr;
    receiver_func_t do_receive = [](auto...){};
};

template<auto MemFn, class Ctx>
ChannelReceiver make_channel_receiver(CHANNELS::ChannelNameId channel_name, Ctx* ctx)
{
    return ChannelReceiver{channel_name, ctx, [](
        void* pctx, bytes_view data, uint32_t total_data_len, uint32_t channel_flags
    ){
        (static_cast<Ctx*>(pctx)->*MemFn)(data, total_data_len, channel_flags);
    }};
}

} // namespace redjs
