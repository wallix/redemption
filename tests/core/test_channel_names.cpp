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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE TestChannelNames
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/channel_names.hpp"

RED_AUTO_TEST_CASE(TestChannelName)
{
    switch (CHANNELS::ChannelNameId("ABc"))
    {
        case CHANNELS::ChannelNameId("abc"):
            break;
        case CHANNELS::ChannelNameId("abcdef"):
        default:
            RED_CHECK(false);
    }

    RED_CHECK_EQ(CHANNELS::ChannelNameId("abcdefg").c_str(), "abcdefg");
    RED_CHECK_EQ(CHANNELS::ChannelNameId("aBC").c_str(), "abc");
    RED_CHECK_EQ(channel_names::rdpdr.c_str(), "rdpdr");
}
