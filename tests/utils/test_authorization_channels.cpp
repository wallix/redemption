/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthorizationChannels
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "authorization_channels.hpp"

BOOST_AUTO_TEST_CASE(TestAuthorizationChannels)
{
    AuthorizationChannels authorization_channels = make_authorization_channels("a,b,c", "b,d");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("e"), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsAllDeny)
{
    AuthorizationChannels authorization_channels = make_authorization_channels("a,b,c", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(1), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsAllAllow)
{
    AuthorizationChannels authorization_channels = make_authorization_channels("*", "a,b,c");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), !false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), !false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(1), !false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsCliprdr)
{
    AuthorizationChannels authorization_channels = make_authorization_channels("cliprdr", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);

    authorization_channels = make_authorization_channels("cliprdr_up,cliprdr_down", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);

    authorization_channels = make_authorization_channels("cliprdr_down", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);

    authorization_channels = make_authorization_channels("cliprdr_down", "cliprdr_up");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsRdpdr)
{
    AuthorizationChannels authorization_channels = make_authorization_channels("*", "rdpdr_printer");
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(1), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(2), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(3), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), true);
    authorization_channels = make_authorization_channels("rdpdr_port,rdpdr_general", "rdpdr_printer");
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(1), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(2), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(3), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr_port"), false);
}


BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels)
{
    auto pair = update_authorized_channels(
      "cliprdr,drdynvc,cliprdr_down,rdpdr_general,rdpdr,rdpsnd",
      "rdpdr_port",
      ""
    );
    BOOST_CHECK_EQUAL(std::get<0>(pair), "drdynvc,rdpsnd");
    BOOST_CHECK_EQUAL(std::get<1>(pair), "cliprdr_up,cliprdr_down,rdpdr");

    pair = update_authorized_channels(
      "cliprdr,drdynvc,cliprdr_down,rdpdr_general,rdpdr,rdpsnd",
      "",
      "RDP_DEVICE_REDIRECTION"
    );
    BOOST_CHECK_EQUAL(std::get<0>(pair), "drdynvc,rdpsnd,rdpdr");
    BOOST_CHECK_EQUAL(std::get<1>(pair), "cliprdr_up,cliprdr_down");

    pair = update_authorized_channels(
      "",
      "",
      "RDP_CLIPBOARD_DOWN"
    );
    BOOST_CHECK_EQUAL(std::get<0>(pair), "cliprdr_down");
    BOOST_CHECK_EQUAL(std::get<1>(pair), "cliprdr_up,rdpdr");
}
