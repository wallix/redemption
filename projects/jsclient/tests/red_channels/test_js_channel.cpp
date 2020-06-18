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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "binding_channel.hpp"

#include "red_channels/js_channel.hpp"

#include "core/channel_list.hpp"

namespace
{

using redjs::JsChannel;

MAKE_BINDING_CALLBACKS(
    JsChannel,
    BasicChannelData,
    ((d, receiveData, uint8_t, uint32_t total_len, uint32_t flags))
    ((c, free))
)

}

RED_AUTO_TEST_CASE(TestJsChannel)
{
    auto chan_flags1 = CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST;
    auto chan_flags2 = CHANNELS::CHANNEL_FLAG_FIRST;
    auto name_id_ref = CHANNELS::ChannelNameId("test");

    auto p = JsChannel_ctx(name_id_ref);

#define CALL_CB(...) p->channel_ptr->__VA_ARGS__; CTX_CHECK_DATAS(p)

    using namespace test_channel_data::JsChannel_structs;

    CALL_CB(receive("abc"_av, 3, chan_flags1))
    {
        CHECK_NEXT_DATA(receiveData("abc"_av, 3, chan_flags1));
    };

    CALL_CB(receive("def"_av, 3, chan_flags2))
    {
        CHECK_NEXT_DATA(receiveData("def"_av, 3, chan_flags2));
    };

    CALL_CB(send("xyz"_av, 3, chan_flags2))
    {
        CHECK_NEXT_DATA(BasicChannelData(name_id_ref, "xyz"_av, 3, chan_flags2));
    };

    p->channel_ptr.reset();
    CTX_CHECK_DATAS(p)
    {
        CHECK_NEXT_DATA(test_channel_data::JsChannel_structs::free());
    };
}
