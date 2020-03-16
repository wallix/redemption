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

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/bind.hpp"
#include "red_channels/js_channel.hpp"

#include "core/channel_list.hpp"

#include <tuple>
#include <vector>
#include <variant>
#include <string_view>

namespace
{

CHANNELS::ChannelNameId name_id_ref("test");

using DataChan = DataChanPrintable;

MAKE_BINDING_CALLBACKS(
    DataChan,
    (JS_d(receiveData, uint8_t, int flags))
    (JS_c(free))
)

auto test_init_channel(Callback& cb, emscripten::val&& v)
{
    return redjs::JsChannel{cb, std::move(v), name_id_ref};
}

}

#define RECEIVE_DATAS(...) js_channel.receive(__VA_ARGS__); CTX_CHECK_DATAS()
#define CALL_CB(...) js_channel.__VA_ARGS__; CTX_CHECK_DATAS()

RED_AUTO_TEST_CHANNEL(TestJsChannel, test_init_channel, js_channel)
{
    auto vec = [](bytes_view av) { return std::vector<uint8_t>(av.begin(), av.end()); };
    auto chan_flags1 = CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST;
    auto chan_flags2 = CHANNELS::CHANNEL_FLAG_FIRST;

    RECEIVE_DATAS("abc"_av, chan_flags1)
    {
        CHECK_NEXT_DATA(receiveData("abc"_av, chan_flags1));
    };

    RECEIVE_DATAS("def"_av, chan_flags2)
    {
        CHECK_NEXT_DATA(receiveData("def"_av, chan_flags2));
    };

    CALL_CB(send_data("xyz"_av, 3, chan_flags2))
    {
        CHECK_NEXT_DATA(DataChan(name_id_ref, vec("xyz"_av), 3, chan_flags2));
    };
}
