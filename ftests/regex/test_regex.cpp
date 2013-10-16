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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDfaRegex
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "regex.hpp"

#include <vector>
#include <sstream>

using namespace re;

BOOST_AUTO_TEST_CASE(TestRegexSt)
{
    struct Reg {
        Reg(const char * s)
        {
            st_compile(this->stw, s);
        }

        bool search(const char * s)
        {
            this->stw.reset_num();
            return st_search(this->stw, s);
        }

        bool exact_search(const char * s)
        {
            this->stw.reset_num();
            return st_exact_search(this->stw, s);
        }

        StatesWrapper stw;
    };

    {
        Reg reg("a");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), true);
        BOOST_CHECK_EQUAL(reg.search("abc"), true);
        BOOST_CHECK_EQUAL(reg.search("dabc"), true);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("^a");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), true);
        BOOST_CHECK_EQUAL(reg.search("abc"), true);
        BOOST_CHECK_EQUAL(reg.search("dabc"), false);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("a$");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), false);
        BOOST_CHECK_EQUAL(reg.search("abc"), false);
        BOOST_CHECK_EQUAL(reg.search("dabc"), false);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("^a$");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), false);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), false);
        BOOST_CHECK_EQUAL(reg.search("abc"), false);
        BOOST_CHECK_EQUAL(reg.search("dabc"), false);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("a+");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), true);
        BOOST_CHECK_EQUAL(reg.search("abc"), true);
        BOOST_CHECK_EQUAL(reg.search("dabc"), true);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg(".*a.*$");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), true);
        BOOST_CHECK_EQUAL(reg.search("abc"), true);
        BOOST_CHECK_EQUAL(reg.search("dabc"), true);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), true);
    }

    {
        Reg reg("aa+$");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), false);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), false);
        BOOST_CHECK_EQUAL(reg.search("abc"), false);
        BOOST_CHECK_EQUAL(reg.search("dabc"), false);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("aa*b?a");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("a"), false);
        BOOST_CHECK_EQUAL(reg.search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaaa"), true);
        BOOST_CHECK_EQUAL(reg.search("baaba"), true);
        BOOST_CHECK_EQUAL(reg.search("abaaba"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), false);
        BOOST_CHECK_EQUAL(reg.search("ab"), false);
        BOOST_CHECK_EQUAL(reg.search("abc"), false);
        BOOST_CHECK_EQUAL(reg.search("dabc"), false);

        BOOST_CHECK_EQUAL(reg.exact_search(""), false);
        BOOST_CHECK_EQUAL(reg.exact_search("a"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abaaba"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("baaba"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("aaba"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("b"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("ab"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("abc"), false);
        BOOST_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("[a-ce]");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("a"), true);
        BOOST_CHECK_EQUAL(reg.search("b"), true);
        BOOST_CHECK_EQUAL(reg.search("c"), true);
        BOOST_CHECK_EQUAL(reg.search("d"), false);
        BOOST_CHECK_EQUAL(reg.search("e"), true);
        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("lka"), true);
        BOOST_CHECK_EQUAL(reg.search("lkd"), false);
    }

    {
        Reg reg("[^a-cd]");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("a"), !true);
        BOOST_CHECK_EQUAL(reg.search("b"), !true);
        BOOST_CHECK_EQUAL(reg.search("c"), !true);
        BOOST_CHECK_EQUAL(reg.search("d"), !true);
        BOOST_CHECK_EQUAL(reg.search(""), false);
        BOOST_CHECK_EQUAL(reg.search("lka"), !false);
    }

    {
        Reg reg("(a?b?c?)d(.*)$");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("adefg"), true);
    }

    {
        Reg reg("b?a{,1}c");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("a{,1}c"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("a{,1}c"), true);
    }

    {
        Reg reg("b?a{0,3}c");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("ac"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("ac"), true);
    }

    {
        Reg reg("b?a{2,4}c");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("aaac"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaac"), true);
    }

    {
        Reg reg("b?a{2,}c");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("aaaaaaaac"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("aaaaaaac"), true);
    }

    {
        Reg reg("b?a{0,}c");

        BOOST_CHECK(reg.stw.root);

        BOOST_CHECK_EQUAL(reg.search("c"), true);
        BOOST_CHECK_EQUAL(reg.exact_search("c"), true);
    }
}



inline void regex_test(Regex & p_regex,
                       const char * p_str,
                       const int p_exact_result_search,
                       const int p_result_search,
                       const int p_exact_result_match,
                       const Regex::range_matches & p_exact_match_result,
                       const bool p_result_match,
                       const Regex::range_matches & p_match_result)
{
    BOOST_CHECK_EQUAL(p_regex.exact_search(p_str), p_exact_result_search);
    BOOST_CHECK_EQUAL(p_regex.search(p_str), p_result_search);
    BOOST_CHECK_EQUAL(p_regex.exact_search_with_matches(p_str), p_exact_result_match);
    BOOST_CHECK(p_regex.match_result() == p_exact_match_result);
    BOOST_CHECK_EQUAL(p_regex.search_with_matches(p_str), p_result_match);
    BOOST_CHECK(p_regex.match_result() == p_match_result);
}

BOOST_AUTO_TEST_CASE(TestRegex)
{
    Regex::range_matches matches;

    const char * str_regex = "a";
    Regex regex(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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

    str_regex = "^a";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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

    str_regex = "a$";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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

    str_regex = "a+";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
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
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }

    regex_test(regex, "aa", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "aaaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "ab", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "baabaa", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "baab", 0, 1, 0, matches, 1, matches);
    regex_test(regex, "bba", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "aba", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "dabc", 0, 0, 0, matches, 0, matches);

    str_regex = "[a-cd]";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }

    regex_test(regex, "a", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "b", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "c", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "d", 1, 1, 1, matches, 1, matches);
    regex_test(regex, "", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "lka", 0, 1, 0, matches, 1, matches);

    str_regex = "[^a-cd]";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }

    regex_test(regex, "a", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "b", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "c", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "d", 0, 0, 0, matches, 0, matches);
    regex_test(regex, "lka", 0, 1, 0, matches, 1, matches);

    str_regex = "(a?b?c?)d(.*)$";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }

    const char * str = "abcdefg";
    typedef re::StateMachine2::range_t range_t;
    matches.push_back(range_t(str, str+3));
    matches.push_back(range_t(str+4, str+6));
    regex_test(regex, str, 1, 1, 1, matches, 1, matches);

    regex.reset("a{0}");
    if (!regex.message_error()) {
        BOOST_CHECK_MESSAGE(false, "fail");
    }

    regex.reset("a{2,1}");
    if (!regex.message_error()) {
        BOOST_CHECK_MESSAGE(false, "fail");
    }

   matches.clear();

    str_regex = "b?a{,1}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "a{,1}c", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,1}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "ac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,3}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "ac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{2,4}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "aaac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{2,}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "aaaaaac", 1, 1, 1, matches, 1, matches);

    str_regex = "b?a{0,}c";
    regex.reset(str_regex);
    if (regex.message_error()) {
        std::ostringstream os;
        os << str_regex << (regex.message_error())
        << " at offset " << regex.position_error();
        BOOST_CHECK_MESSAGE(false, os.str());
    }
    regex_test(regex, "c", 1, 1, 1, matches, 1, matches);
}
