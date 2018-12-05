/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestChannelList
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/channel_list.hpp"

RED_AUTO_TEST_CASE(TestChannelDefArray)
{
    CHANNELS::ChannelDefArray channels;
    channels.push_back({CHANNELS::ChannelNameId("abc"), 0, 0});
    channels.push_back({CHANNELS::ChannelNameId("def"), 0, 1});
    CHANNELS::ChannelDef const * channel = channels.get_by_id(1);
    RED_REQUIRE(channel);
    RED_CHECK_EQ(channel->name.c_str(), "def");
    channel = channels.get_by_name(CHANNELS::ChannelNameId("abc"));
    RED_REQUIRE(channel);
    RED_CHECK_EQ(channel->name.c_str(), "abc");
}
