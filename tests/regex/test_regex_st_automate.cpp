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

#define RED_TEST_MODULE TestNdfaRegex
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "regex/regex_parser.hpp"
#include "regex/regex_st_automate.hpp"


using namespace re;

RED_AUTO_TEST_CASE(TestRegexSt)
{
    struct Reg {
        Reg(const char * s)
        {
            this->stparser.compile(s);
        }

        bool search(const char * s)
        {
            return st_search(this->stparser, s);
        }

        bool exact_search(const char * s)
        {
            return st_exact_search(this->stparser, s);
        }

        const State * root() const
        {
            return this->stparser.root();
        }

        StateParser stparser;
    };

    {
        Reg reg("a");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), true);
        RED_CHECK_EQUAL(reg.search("abc"), true);
        RED_CHECK_EQUAL(reg.search("dabc"), true);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("^a");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), false);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), true);
        RED_CHECK_EQUAL(reg.search("abc"), true);
        RED_CHECK_EQUAL(reg.search("dabc"), false);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("a$");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), false);
        RED_CHECK_EQUAL(reg.search("abc"), false);
        RED_CHECK_EQUAL(reg.search("dabc"), false);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("^a$");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), false);
        RED_CHECK_EQUAL(reg.search("baaaa"), false);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), false);
        RED_CHECK_EQUAL(reg.search("abc"), false);
        RED_CHECK_EQUAL(reg.search("dabc"), false);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("a+");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), true);
        RED_CHECK_EQUAL(reg.search("abc"), true);
        RED_CHECK_EQUAL(reg.search("dabc"), true);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg(".*a.*$");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), true);
        RED_CHECK_EQUAL(reg.search("abc"), true);
        RED_CHECK_EQUAL(reg.search("dabc"), true);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), true);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), true);
        RED_CHECK_EQUAL(reg.exact_search("abc"), true);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), true);
    }

    {
        Reg reg("aa+$");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), false);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), false);
        RED_CHECK_EQUAL(reg.search("abc"), false);
        RED_CHECK_EQUAL(reg.search("dabc"), false);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), false);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("aa*b?a");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("a"), false);
        RED_CHECK_EQUAL(reg.search("aaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaaa"), true);
        RED_CHECK_EQUAL(reg.search("baaba"), true);
        RED_CHECK_EQUAL(reg.search("abaaba"), true);
        RED_CHECK_EQUAL(reg.search("b"), false);
        RED_CHECK_EQUAL(reg.search("ab"), false);
        RED_CHECK_EQUAL(reg.search("abc"), false);
        RED_CHECK_EQUAL(reg.search("dabc"), false);

        RED_CHECK_EQUAL(reg.exact_search(""), false);
        RED_CHECK_EQUAL(reg.exact_search("a"), false);
        RED_CHECK_EQUAL(reg.exact_search("aaaa"), true);
        RED_CHECK_EQUAL(reg.exact_search("baaaa"), false);
        RED_CHECK_EQUAL(reg.exact_search("abaaba"), false);
        RED_CHECK_EQUAL(reg.exact_search("baaba"), false);
        RED_CHECK_EQUAL(reg.exact_search("aaba"), true);
        RED_CHECK_EQUAL(reg.exact_search("b"), false);
        RED_CHECK_EQUAL(reg.exact_search("ab"), false);
        RED_CHECK_EQUAL(reg.exact_search("abc"), false);
        RED_CHECK_EQUAL(reg.exact_search("dabc"), false);
    }

    {
        Reg reg("[a-ce]");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("a"), true);
        RED_CHECK_EQUAL(reg.search("b"), true);
        RED_CHECK_EQUAL(reg.search("c"), true);
        RED_CHECK_EQUAL(reg.search("d"), false);
        RED_CHECK_EQUAL(reg.search("e"), true);
        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("lka"), true);
        RED_CHECK_EQUAL(reg.search("lkd"), false);
    }

    {
        Reg reg("[^a-cd]");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("a"), !true);
        RED_CHECK_EQUAL(reg.search("b"), !true);
        RED_CHECK_EQUAL(reg.search("c"), !true);
        RED_CHECK_EQUAL(reg.search("d"), !true);
        RED_CHECK_EQUAL(reg.search(""), false);
        RED_CHECK_EQUAL(reg.search("lka"), !false);
    }

    {
        Reg reg("(a?b?c?)d(.*)$");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("adefg"), true);
    }

    {
        Reg reg("b?a{,1}c");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("a{,1}c"), true);
        RED_CHECK_EQUAL(reg.exact_search("a{,1}c"), true);
    }

    {
        Reg reg("b?a{0,3}c");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("ac"), true);
        RED_CHECK_EQUAL(reg.exact_search("ac"), true);
    }

    {
        Reg reg("b?a{2,4}c");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("aaac"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaac"), true);
    }

    {
        Reg reg("b?a{2,}c");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("aaaaaaaac"), true);
        RED_CHECK_EQUAL(reg.exact_search("aaaaaaac"), true);
    }

    {
        Reg reg("b?a{0,}c");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("c"), true);
        RED_CHECK_EQUAL(reg.exact_search("c"), true);
    }
}
