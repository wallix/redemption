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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Meng Tan

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include "utils/translation.hpp"
#include <string_view>

using namespace std::string_view_literals;


RED_AUTO_TEST_CASE(TestTranslation)
{
    Language lang = Language::en;

    {
        constexpr std::size_t n = 128;
        char s[n]{};
        TR_fmt(s, n, trkeys::fmt_field_required, lang, "'XY'");
        RED_CHECK(s == "Error: 'XY' field is required."sv);
    }

    RED_CHECK_EQUAL(TR(trkeys::login, lang),             "Login");
    RED_CHECK_EQUAL(TR(trkeys::password, lang),          "Password");
    RED_CHECK_EQUAL(TR(trkeys::diagnostic, lang),        "Diagnostic");
    RED_CHECK_EQUAL(TR(trkeys::connection_closed, lang), "Connection closed");
    RED_CHECK_EQUAL(TR(trkeys::OK, lang),                "OK");
    RED_CHECK_EQUAL(TR(trkeys::cancel, lang),            "Cancel");
    RED_CHECK_EQUAL(TR(trkeys::help, lang),              "Help");
    RED_CHECK_EQUAL(TR(trkeys::close, lang),             "Close");
    RED_CHECK_EQUAL(TR(trkeys::refused, lang),           "Refused");
    RED_CHECK_EQUAL(TR(trkeys::username, lang),          "Username");
    RED_CHECK_EQUAL(TR(trkeys::password_expire, lang),   "Your Bastion password will expire soon. Please change it.");
    RED_CHECK_EQUAL(TR(trkeys::protocol, lang),          "Protocol");
    RED_CHECK_EQUAL(TR(trkeys::target, lang),            "Target");
    RED_CHECK_EQUAL(TR(trkeys::close_time, lang),        "Close Time");
    RED_CHECK_EQUAL(TR(trkeys::logout, lang),            "Logout");
    RED_CHECK_EQUAL(TR(trkeys::apply, lang),             "Apply");
    RED_CHECK_EQUAL(TR(trkeys::connect, lang),           "Connect");
    RED_CHECK_EQUAL(TR(trkeys::timeleft, lang),          "Time left");
    RED_CHECK_EQUAL(TR(trkeys::second, lang),            "second");
    RED_CHECK_EQUAL(TR(trkeys::minute, lang),            "minute");
    RED_CHECK_EQUAL(TR(trkeys::before_closing, lang),    "before closing");
    RED_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang), "Connection closed by manager.");
    RED_CHECK_EQUAL(TR(trkeys::end_connection, lang),    "End of connection");
    RED_CHECK_EQUAL(TR(trkeys::selector, lang),           "Selector");
    RED_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   "Session is out of allowed timeframe");
    RED_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     "Missed keepalive from ACL");
    RED_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   "Connection closed on inactivity");
    RED_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           "Authentifier service failed");
}
