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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/log_id.hpp"
#include "core/report_message_api.hpp"
#include "capture/utils/match_finder.hpp"
#include <string_view>

using namespace std::string_view_literals;



constexpr bool verbosity = false;
constexpr auto without_capture = utils::MatchFinder::WithCapture(false);

RED_AUTO_TEST_CASE(configure_regexes)
{
    utils::MatchFinder::NamedRegexArray regexes;

    const char * pattern =
        "$kbd:a b \x01"
        " $kbd: c\x01"
        "$kbd:ee\x01"
        "$ocr:ocr\x01"
        "$ocr-kbd:ocr and kbd\x01"
        "$kbd-ocr:other ocr and kbd\x01"
        "$exact-content,kbd,ocr:exact str\x01"
        "$exact-regex,kbd,ocr:exact regex\x01"
        "$content,kbd,ocr:str\x01"
        "$content,ocr:title\x01"
    ;

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, pattern, regexes, verbosity, without_capture);

    RED_REQUIRE_EQUAL(regexes.size(), 8);
    RED_CHECK_EQUAL(regexes[0].name, "a b ");
    RED_CHECK_EQUAL(regexes[1].name, " c");
    RED_CHECK_EQUAL(regexes[2].name, "ee");
    RED_CHECK_EQUAL(regexes[3].name, "ocr and kbd");
    RED_CHECK_EQUAL(regexes[4].name, "other ocr and kbd");
    RED_CHECK_EQUAL(regexes[5].name, "exact str");
    RED_CHECK_EQUAL(regexes[6].name, "exact regex");
    RED_CHECK_EQUAL(regexes[7].name, "str");
    RED_CHECK(not regexes[0].is_exact_search);
    RED_CHECK(not regexes[1].is_exact_search);
    RED_CHECK(not regexes[2].is_exact_search);
    RED_CHECK(not regexes[3].is_exact_search);
    RED_CHECK(not regexes[4].is_exact_search);
    RED_CHECK(not regexes[5].is_exact_search);
    RED_CHECK(not regexes[6].is_exact_search);
    RED_CHECK(not regexes[7].is_exact_search);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, pattern, regexes, verbosity, without_capture);

    RED_REQUIRE_EQUAL(regexes.size(), 7);
    RED_CHECK_EQUAL(regexes[0].name, "ocr");
    RED_CHECK_EQUAL(regexes[1].name, "ocr and kbd");
    RED_CHECK_EQUAL(regexes[2].name, "other ocr and kbd");
    RED_CHECK_EQUAL(regexes[3].name, "exact str");
    RED_CHECK_EQUAL(regexes[4].name, "exact regex");
    RED_CHECK_EQUAL(regexes[5].name, "str");
    RED_CHECK_EQUAL(regexes[6].name, "title");
    RED_CHECK(not regexes[0].is_exact_search);
    RED_CHECK(not regexes[1].is_exact_search);
    RED_CHECK(not regexes[2].is_exact_search);
    RED_CHECK(regexes[3].is_exact_search);
    RED_CHECK(regexes[4].is_exact_search);
    RED_CHECK(not regexes[5].is_exact_search);
    RED_CHECK(not regexes[6].is_exact_search);

    regexes.resize(0);
    RED_REQUIRE_EQUAL(regexes.size(), 0);
    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "$ocr:abc", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "$ocr:abc", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 1);
    RED_CHECK_EQUAL(regexes[0].name, "abc");
    RED_CHECK(not regexes[0].is_exact_search);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "cba", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 1);
    RED_CHECK_EQUAL(regexes[0].name, "cba");
    RED_CHECK(not regexes[0].is_exact_search);

    regexes.resize(0);
    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "  abc", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 1);
    RED_CHECK_EQUAL(regexes[0].name, "abc"); // left space is stripped
    RED_CHECK(not regexes[0].is_exact_search);

    regexes.resize(0);
    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "cba", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, "$other", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 0);

    utils::MatchFinder::configure_regexes(utils::MatchFinder::KBD_INPUT, "$other", regexes, verbosity, without_capture);
    RED_REQUIRE_EQUAL(regexes.size(), 0);
}

RED_AUTO_TEST_CASE(search)
{
    utils::MatchFinder::NamedRegexArray regexes;

    const char * pattern =
        "$:(a)\x01"
        "$regex:(b)\x01"
        "$content:(c)\x01"
        "$exact-regex:(d)\x01"
        "$exact-content:(e)\x01"
    ;

    utils::MatchFinder::configure_regexes(utils::MatchFinder::OCR, pattern, regexes, verbosity, without_capture);

    RED_REQUIRE_EQUAL(regexes.size(), 5);
    RED_CHECK(regexes[0].search("(a)"));
    RED_CHECK(regexes[1].search("(b)"));
    RED_CHECK(regexes[2].search("(c)"));
    RED_CHECK(not regexes[3].search("(d)"));
    RED_CHECK(regexes[4].search("(e)"));

    RED_CHECK(regexes[0].search("-(a)-"));
    RED_CHECK(regexes[1].search("-(b)-"));
    RED_CHECK(regexes[2].search("-(c)-"));
    RED_CHECK(not regexes[3].search("-(d)-"));
    RED_CHECK(not regexes[4].search("-(e)-"));
}

RED_AUTO_TEST_CASE(report_notify)
{
    struct : NullReportMessage {
        bool has_log = false;
        bool has_report = false;

        void log6(LogId id, KVList kv_list) override {
            RED_CHECK(id == LogId::NOTIFY_PATTERN_DETECTED);
            RED_REQUIRE(kv_list.size() > 0);
            RED_CHECK(kv_list[0].key == "pattern"_av);
            RED_CHECK(kv_list[0].value == "$kbd:c| cacao"_av);
            this->has_log = true;
        }

        void report(const char* reason, const char* message) override {
            RED_CHECK(reason == "FINDPATTERN_NOTIFY"sv);
            RED_CHECK(message == "$kbd:c| cacao"sv);
            this->has_report = true;
        }
    } report_message;
    utils::MatchFinder::report(report_message, false, utils::MatchFinder::KBD_INPUT, "c", " cacao");
    RED_CHECK(report_message.has_log);
    RED_CHECK(report_message.has_report);
}

RED_AUTO_TEST_CASE(report_kill)
{
    struct : NullReportMessage {
        bool has_log = false;
        bool has_report = false;

        void log6(LogId id, KVList kv_list) override {
            RED_CHECK(id == LogId::KILL_PATTERN_DETECTED);
            RED_REQUIRE(kv_list.size() > 0);
            RED_CHECK(kv_list[0].key == "pattern"_av);
            RED_CHECK(kv_list[0].value == "$ocr:c| cacao"_av);
            this->has_log = true;
        }

        void report(const char* reason, const char* message) override {
            RED_CHECK(reason == "FINDPATTERN_KILL"sv);
            RED_CHECK(message == "$ocr:c| cacao"sv);
            this->has_report = true;
        }
    } report_message;
    utils::MatchFinder::report(report_message, true, utils::MatchFinder::OCR, "c", " cacao");
    RED_CHECK(report_message.has_log);
    RED_CHECK(report_message.has_report);
}
