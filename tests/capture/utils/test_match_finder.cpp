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

RED_AUTO_TEST_CASE(MatchFinder_configure_regexes)
{
    utils::MatchFinder::NamedRegexArray regexes;

    const char * pattern =
        "$kbd:a b \x01"
        " $kbd: c\x01"
        "$kbd:ee\x01"
        "$ocr:ocr\x01"
        "$ocr-kbd:ocr and kbd\x01"
        "$kbd-ocr:other ocr and kbd";
        
    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, pattern, regexes, 0);

    RED_REQUIRE_EQUAL(regexes.size(), 5);
    RED_REQUIRE_EQUAL(regexes[0].name, "a b ");
    RED_REQUIRE_EQUAL(regexes[1].name, " c");
    RED_REQUIRE_EQUAL(regexes[2].name, "ee");
    RED_REQUIRE_EQUAL(regexes[3].name, "ocr and kbd");
    RED_REQUIRE_EQUAL(regexes[4].name, "other ocr and kbd");

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, pattern, regexes, 0);

    RED_REQUIRE_EQUAL(regexes.size(), 3);
    RED_REQUIRE_EQUAL(regexes[0].name, "ocr");
    RED_REQUIRE_EQUAL(regexes[1].name, "ocr and kbd");
    RED_REQUIRE_EQUAL(regexes[2].name, "other ocr and kbd");

    regexes.resize(0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);
    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "$ocr:abc", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "$ocr:abc", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 1);
    RED_REQUIRE_EQUAL(regexes[0].name, "abc");

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "cba", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 1);
    RED_REQUIRE_EQUAL(regexes[0].name, "cba");

    regexes.resize(0);
    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "cba", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "$other", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "$other", regexes, 0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);
}

RED_AUTO_TEST_CASE(MatchFinder_report_notify)
{
    struct Auth : auth_api {
        bool has_log = false;
        bool has_report = false;
        void log4(bool duplicate_with_pid, const char* type, const char* extra) override {
            RED_CHECK_EQUAL(duplicate_with_pid, false);
            RED_CHECK_EQUAL(type, "NOTIFY_PATTERN_DETECTED");
            RED_CHECK_EQUAL(extra, "pattern=\"$kbd:c| cacao\"");
            this->has_log = true;
        }
        void report(const char* reason, const char* message) override {
            RED_CHECK_EQUAL(reason, "FINDPATTERN_NOTIFY");
            RED_CHECK_EQUAL(message, "$kbd:c| cacao");
            this->has_report = true;
        }
        void set_auth_channel_target(const char*) override { RED_CHECK(false); }
        void set_auth_error_message(const char*) override { RED_CHECK(false); }
    };

    Auth auth;
    utils::MatchFinder::report(auth, false, utils::MatchFinder::KBD_INPUT, "c", " cacao");
    RED_CHECK(auth.has_log);
    RED_CHECK(auth.has_report);
}

RED_AUTO_TEST_CASE(MatchFinder_report_kill)
{
    struct Auth : auth_api {
        bool has_log = false;
        bool has_report = false;
        void log4(bool duplicate_with_pid, const char* type, const char* extra) override {
            RED_CHECK_EQUAL(duplicate_with_pid, false);
            RED_CHECK_EQUAL(type, "KILL_PATTERN_DETECTED");
            RED_CHECK_EQUAL(extra, "pattern=\"$ocr:c| cacao\"");
            this->has_log = true;
        }
        void report(const char* reason, const char* message) override {
            RED_CHECK_EQUAL(reason, "FINDPATTERN_KILL");
            RED_CHECK_EQUAL(message, "$ocr:c| cacao");
            this->has_report = true;
        }
        void set_auth_channel_target(const char*) override { RED_CHECK(false); }
        void set_auth_error_message(const char*) override { RED_CHECK(false); }
    };

    Auth auth;
    utils::MatchFinder::report(auth, true, utils::MatchFinder::OCR, "c", " cacao");
    RED_CHECK(auth.has_log);
    RED_CHECK(auth.has_report);
}
