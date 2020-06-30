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

#include "redjs/browser_front.hpp"

#include <bitset>


RED_AUTO_TEST_CASE(TestBrowserFront)
{
    redjs::BrowserFront front{emscripten::val::object(), 800, 600, RDPVerbose(0)};

    using bitset = std::bitset<PrimaryDrawingOrdersSupport::max>;
    RED_CHECK(bitset{(
        PrimaryDrawingOrdersSupport{}
        | TS_NEG_GLYPH_INDEX
        | TS_NEG_MULTIOPAQUERECT_INDEX
        | TS_NEG_OPAQUERECT_INDEX
    ).as_uint()} == bitset{front.get_supported_orders().as_uint()});

    struct Chann
    {
        void test(bytes_view data, uint32_t total_data_len, uint32_t channel_flags)
        {
            this->data = data;
            this->total_data_len = total_data_len;
            this->channel_flags = channel_flags;
        }

        bytes_view data {};
        uint32_t total_data_len {};
        uint32_t channel_flags {};
    };
    Chann chann;

    CHANNELS::ChannelNameId channel_name{"test"};

    front.add_channel_receiver(redjs::make_channel_receiver<&Chann::test>(channel_name, &chann));

    CHANNELS::ChannelDefArray const& cl = front.get_channel_list();
    CHANNELS::ChannelDef const* chann_ptr = cl.get_by_name(channel_name);

    RED_REQUIRE(chann_ptr);

    front.send_to_channel(*chann_ptr, "plop"_av, 12, CHANNELS::CHANNEL_FLAG_FIRST);

    RED_CHECK("plop"_av == chann.data);
    RED_CHECK(12 == chann.total_data_len);
    RED_CHECK(CHANNELS::CHANNEL_FLAG_FIRST == chann.channel_flags);
}
