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

#define RED_TEST_MODULE Testmatch_finder
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "capture/utils/match_finder.hpp"

RED_AUTO_TEST_CASE(Testmatch_finder)
{
    utils::MatchFinder::NamedRegexArray regexes;

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "$kbd:a b \x01 $kbd: c", regexes, 0);

    RED_REQUIRE_EQUAL(regexes.size(), 2);
    RED_REQUIRE_EQUAL(regexes.begin()->name, "a b ");
    RED_REQUIRE_EQUAL(regexes.begin()[1].name, " c");

    struct Auth : auth_api {
        void log4(bool duplicate_with_pid, const char* type, const char* extra) override {
            RED_CHECK_EQUAL(duplicate_with_pid, false);
            RED_CHECK_EQUAL(type, "NOTIFY_PATTERN_DETECTED");
            RED_CHECK_EQUAL(extra, "pattern=\"$kbd:c| cacao\"");
        }
        void report(const char* reason, const char* message) override {
            RED_CHECK_EQUAL(reason, "FINDPATTERN_NOTIFY");
            RED_CHECK_EQUAL(message, "$kbd:c| cacao");
        }
        void set_auth_channel_target(const char*) override { RED_CHECK(false); }
        void set_auth_error_message(const char*) override { RED_CHECK(false); }
    };

    Auth auth;
    utils::MatchFinder::report(auth, false, utils::MatchFinder::KBD_INPUT, "c", " cacao");
}
