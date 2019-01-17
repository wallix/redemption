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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap class, compression performance

*/

#define RED_TEST_MODULE TestDfaRegex
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "regex/regex.hpp"

#include <vector>
#include <sstream>

using namespace re;

inline
void regex_test(Regex & p_regex,
                const char * p_str,
                bool p_exact_result_search,
                bool p_result_search,
                bool p_exact_result_match,
                const Regex::range_matches & p_exact_match_result,
                bool p_result_match,
                const Regex::range_matches & p_match_result)
{
    RED_CHECK_EQUAL(p_regex.exact_search(p_str), p_exact_result_search);
    RED_CHECK_EQUAL(p_regex.search(p_str), p_result_search);
    RED_CHECK_EQUAL(p_regex.exact_search_with_matches(p_str), p_exact_result_match);
    RED_CHECK(p_regex.match_result() == p_exact_match_result);
    RED_CHECK_EQUAL(p_regex.search_with_matches(p_str), p_result_match);
    RED_CHECK(p_regex.match_result() == p_match_result);
}

#define regex_test(p_regex,\
                   p_str,\
                   p_exact_result_search,\
                   p_result_search,\
                   p_exact_result_match,\
                   p_exact_match_result,\
                   p_result_match,\
                   p_match_result)\
regex_test(p_regex, p_str, p_exact_result_search, p_result_search, p_exact_result_match, p_exact_match_result, p_result_match, p_match_result)

inline void test_re(re::Regex::flag_t flags)
{
    typedef re::StateMachine2::range_t range_t;

    Regex::range_matches matches;
    Regex::range_matches matches2;

    const char * str_regex = "a";
    Regex regex(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "ba", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "abc", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "dabc", 0, 1, 0, matches, 1, matches);

    str_regex = "^";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);

    str_regex = "$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);

    str_regex = "^$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaa", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);

    str_regex = "$^";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaa", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);

    str_regex = "(^$)";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    const char * str = "";
    matches.push_back(range_t(str, str));
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);
    matches.clear();

    str_regex = "^a";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "abc", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "a^b";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);

    str_regex = "a$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "aaaab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ba", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "abc", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "^a$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "abc", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "^a|^.?";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "ba", 1, 1, 1, matches, 1, matches);

    str_regex = "a+";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "ba", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "abc", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "dabc", 0, 1, 0, matches, 1, matches);

    str_regex = ".*a.*$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "ba", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "abc", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "dabc", 1, 1, 1, matches, 1, matches);

    str_regex = "aa+$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "baa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "ba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "ab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "aa*b?a";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "aaaa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "ab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baabaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "baab", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "bba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "aba", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "[a-cd]";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "b", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "c", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "d", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "lka", 0, 1, 0, matches, 1, matches);

    str_regex = "[^a-cd]";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "c", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "d", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "lka", 0, 1, 0, matches, 1, matches);


    str_regex = "\\a";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "a";
    regex_test(regex, str, 1, 1, 1, matches2, 1, matches);


    str_regex = "[\\a]";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "a";
    regex_test(regex, str, 1, 1, 1, matches2, 1, matches);


    str_regex = "\\[a]";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "[a]";
    regex_test(regex, str, 1, 1, 1, matches2, 1, matches);


    str_regex = "(.*)";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "abcd";
    matches.push_back(range_t(str, str+1));
    matches2.push_back(range_t(str, str+4));
    regex_test(regex, str, 1, 1, 1, matches2, 1, matches);

    str_regex = "(a?b?c?)d(.*)";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "abcdefg";
    matches.clear();
    matches.push_back(range_t(str, str+3));
    matches.push_back(range_t(str+4, str+7));
    matches2.clear();
    matches2.push_back(range_t(str, str+3));
    matches2.push_back(range_t(nullptr, nullptr));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches2);

    str_regex = "u(a?b?c?)d(.*)";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "uabcdefg";
    matches.clear();
    matches.push_back(range_t(str+1, str+4));
    matches.push_back(range_t(str+5, str+8));
    matches2.clear();
    matches2.push_back(range_t(str+1, str+4));
    matches2.push_back(range_t(nullptr, nullptr));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches2);

    str_regex = "(a?b?c?)d(.*)h";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "abcdefgh";
    matches.clear();
    matches.push_back(range_t(str, str+3));
    matches.push_back(range_t(str+4, str+7));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);

    str_regex = "(a?b?c?)d(.*)h$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);

    str_regex = "(a?b?c?)d(.*)gh";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "abcdefggh";
    matches.clear();
    matches.push_back(range_t(str, str+3));
    matches.push_back(range_t(str+4, str+7));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);

    str_regex = "(a?b?c?)d(.*)gh$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);

    str_regex = "(a?b?c?)d(.*)gh$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    str_regex = "(?:.)?";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "a";
    matches.clear();
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);
    str = "";
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    str_regex = "(?:a(bv)|(av))(d)";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "abvd";
    matches.clear();
    matches.push_back(range_t(str+1, str+3));
    matches.push_back(range_t(nullptr, nullptr));
    matches.push_back(range_t(str+3, str+4));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);
    matches.erase(matches.begin()+1);
    RED_CHECK(regex.match_result(false) == matches);

    str = "avd";
    matches.clear();
    matches.push_back(range_t(nullptr, nullptr));
    matches.push_back(range_t(str, str+2));
    matches.push_back(range_t(str+2, str+3));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    str_regex = "((.)(.))";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "ab";
    matches.clear();
    matches.push_back(range_t(str, str+2));
    matches.push_back(range_t(str, str+1));
    matches.push_back(range_t(str+1, str+2));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    str_regex = " *|(?:.)?";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "";
    matches.clear();
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    str_regex = "(\\d){3}";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    str = "012";
    matches.clear();
    matches.push_back(range_t(str+2, str+3));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);


    regex.reset("a{0}");
    if (!regex.message_error()) {
        RED_CHECK_MESSAGE(false, "fail");
    }

    regex.reset("a{2,1}");
    if (!regex.message_error()) {
        RED_CHECK_MESSAGE(false, "fail");
    }

    regex.reset("a**");
    if (!regex.message_error()) {
        RED_CHECK_MESSAGE(false, "fail");
    }

    regex.reset("a+*");
    if (!regex.message_error()) {
        RED_CHECK_MESSAGE(false, "fail");
    }

    regex.reset("a+{2}");
    if (!regex.message_error()) {
        RED_CHECK_MESSAGE(false, "fail");
    }

   matches.clear();

    str_regex = "b?a{,1}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "a{,1}c", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,1}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "ac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,3}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "ac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{2,4}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{2,}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "aaaaaac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,}c";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, "c", 1, 1, 1, matches, 1, matches);

    str_regex = "^ +\\d+.*bc.*$";
    regex.reset(str_regex, flags);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        RED_CHECK_MESSAGE(false, os.str().c_str());
    }
    regex_test(regex, " 24abcd", 1, 1, 1, matches, 1, matches);
}


RED_AUTO_TEST_CASE(TestRegex)
{
    test_re(re::Regex::DEFAULT_FLAG);
}

RED_AUTO_TEST_CASE(TestRegexOptimize)
{
    test_re(re::Regex::MINIMAL_MEMORY|re::Regex::OPTIMIZE_MEMORY);
}

RED_AUTO_TEST_CASE(TestRegexPartOfText)
{
    const char * str_regex = "a";
    Regex regex(str_regex);

    {
        const char * str = "a";
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*str);
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined ==  part_rgx.next(str)) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        const char * str = "dsqdaaz";
        Regex::PartOfText part_rgx = regex.part_of_text_search(!*str);
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined ==  part_rgx.next(str)) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    regex.reset("abc");

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.state());
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.next("a"));
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.next("b"));
        RED_CHECK_EQUAL(Regex::match_fail, part_rgx.next("h"));
        RED_CHECK_EQUAL(Regex::match_fail, part_rgx.finish());
    }

    {
        Regex::PartOfText part_rgx = regex.part_of_text_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("w")
             && Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("b")
             && Regex::match_success == part_rgx.next("c")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::PartOfText part_rgx = regex.part_of_text_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("wa")
             && Regex::match_success == part_rgx.next("bc")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    regex.reset("abc[0-9]");

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("b")
             && Regex::match_undetermined == part_rgx.next("c")
             && Regex::match_success == part_rgx.next("0")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.state());
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.next("ab"));
        RED_CHECK_EQUAL(Regex::match_undetermined, part_rgx.next("c"));
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.next("0"));
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.finish());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("abc")
             && Regex::match_undetermined == part_rgx.next("0")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("abc0")) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("bc")
             && Regex::match_undetermined == part_rgx.next("0")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("bc0")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("b")
             && Regex::match_undetermined == part_rgx.next("c0")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_success, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("b")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_fail, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"a");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("bca")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_fail, part_rgx.state());
    }

    {
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(!*"");
        if (Regex::match_undetermined == part_rgx.state()) {
            if (Regex::match_undetermined == part_rgx.next("a")
             && Regex::match_undetermined == part_rgx.next("bca")
            ) {
                part_rgx.finish();
            }
        }
        RED_CHECK_EQUAL(Regex::match_fail, part_rgx.state());
    }
}


RED_AUTO_TEST_CASE(TestRegexLimit)
{
    re::Regex regex("aaa");
    RED_CHECK(regex.search("bbbbbbbbbbbbbbbbbbbbaaaaa"));
    regex.step_limit = 40;
    RED_CHECK( ! regex.search("bbbbbbbbbbbbbbbbbbbbaaaaa"));
}


RED_AUTO_TEST_CASE(TestRegexLastPos)
{
    re::Regex regex("aaa|abcde|o*r");
    RED_CHECK(regex.search("abaabaaabcd"));
    RED_CHECK_EQUAL(regex.last_index(), 8);
    RED_CHECK(regex.search("r"));
    RED_CHECK_EQUAL(regex.last_index(), 1);
    RED_CHECK(regex.search("ooor"));
    RED_CHECK_EQUAL(regex.last_index(), 4);
    RED_CHECK( ! regex.search("xxxxxx"));
    RED_CHECK_EQUAL(regex.last_index(), 6);
}

