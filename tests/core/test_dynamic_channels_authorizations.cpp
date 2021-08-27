/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/dynamic_channels_authorizations.hpp"

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        ""_zv, ""_zv);

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("b"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations1)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        ", a, b ,, c"_zv, ""_zv);

    RED_CHECK( dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("d"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations2)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        " a ,b , c d  "_zv, "*"_zv);

    RED_CHECK( dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("d"));


    RED_CHECK( dynamic_channels_authorizations.is_authorized("c d"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations3)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        "*"_zv, "  a ,b, c"_zv);

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("d"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("e"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations4)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        "*"_zv, "  *, "_zv);

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations5)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        "*, b"_zv, "  a, *, "_zv);

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations6)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        " a,b"_zv, "  c,d "_zv);

    RED_CHECK( dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("d"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("e"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations7)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        " * "_zv, "  a,b "_zv);

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("d"));
}

RED_AUTO_TEST_CASE(TestDynamicChannelsAuthorizations8)
{
    DynamicChannelsAuthorizations dynamic_channels_authorizations(
        "a,b"_zv, "*"_zv);

    RED_CHECK( dynamic_channels_authorizations.is_authorized("a"));

    RED_CHECK( dynamic_channels_authorizations.is_authorized("b"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("c"));

    RED_CHECK(!dynamic_channels_authorizations.is_authorized("d"));
}
