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

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(reg.search("ab"));
        RED_CHECK(reg.search("abc"));
        RED_CHECK(reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(not reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("^a");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(not reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(reg.search("ab"));
        RED_CHECK(reg.search("abc"));
        RED_CHECK(not reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(not reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("a$");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(not reg.search("ab"));
        RED_CHECK(not reg.search("abc"));
        RED_CHECK(not reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(not reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("^a$");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(not reg.search("aaaa"));
        RED_CHECK(not reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(not reg.search("ab"));
        RED_CHECK(not reg.search("abc"));
        RED_CHECK(not reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(not reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("a+");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(reg.search("ab"));
        RED_CHECK(reg.search("abc"));
        RED_CHECK(reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg(".*a.*$");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(reg.search("ab"));
        RED_CHECK(reg.search("abc"));
        RED_CHECK(reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(reg.exact_search("a"));
        RED_CHECK(reg.exact_search("aaaa"));
        RED_CHECK(reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(reg.exact_search("ab"));
        RED_CHECK(reg.exact_search("abc"));
        RED_CHECK(reg.exact_search("dabc"));
    }

    {
        Reg reg("aa+$");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(not reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(not reg.search("ab"));
        RED_CHECK(not reg.search("abc"));
        RED_CHECK(not reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(not reg.exact_search("a"));
        RED_CHECK(reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("aa*b?a");

        RED_CHECK(reg.root());

        RED_CHECK(not reg.search(""));
        RED_CHECK(not reg.search("a"));
        RED_CHECK(reg.search("aaaa"));
        RED_CHECK(reg.search("baaaa"));
        RED_CHECK(reg.search("baaba"));
        RED_CHECK(reg.search("abaaba"));
        RED_CHECK(not reg.search("b"));
        RED_CHECK(not reg.search("ab"));
        RED_CHECK(not reg.search("abc"));
        RED_CHECK(not reg.search("dabc"));

        RED_CHECK(not reg.exact_search(""));
        RED_CHECK(not reg.exact_search("a"));
        RED_CHECK(reg.exact_search("aaaa"));
        RED_CHECK(not reg.exact_search("baaaa"));
        RED_CHECK(not reg.exact_search("abaaba"));
        RED_CHECK(not reg.exact_search("baaba"));
        RED_CHECK(reg.exact_search("aaba"));
        RED_CHECK(not reg.exact_search("b"));
        RED_CHECK(not reg.exact_search("ab"));
        RED_CHECK(not reg.exact_search("abc"));
        RED_CHECK(not reg.exact_search("dabc"));
    }

    {
        Reg reg("[a-ce]");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("a"));
        RED_CHECK(reg.search("b"));
        RED_CHECK(reg.search("c"));
        RED_CHECK(not reg.search("d"));
        RED_CHECK(reg.search("e"));
        RED_CHECK(not reg.search(""));
        RED_CHECK(reg.search("lka"));
        RED_CHECK(not reg.search("lkd"));
    }

    {
        Reg reg("[^a-cd]");

        RED_CHECK(reg.root());

        RED_CHECK_EQUAL(reg.search("a"), !true);
        RED_CHECK_EQUAL(reg.search("b"), !true);
        RED_CHECK_EQUAL(reg.search("c"), !true);
        RED_CHECK_EQUAL(reg.search("d"), !true);
        RED_CHECK(not reg.search(""));
        RED_CHECK_EQUAL(reg.search("lka"), !false);
    }

    {
        Reg reg("(a?b?c?)d(.*)$");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("adefg"));
    }

    {
        Reg reg("b?a{,1}c");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("a{,1}c"));
        RED_CHECK(reg.exact_search("a{,1}c"));
    }

    {
        Reg reg("b?a{0,3}c");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("ac"));
        RED_CHECK(reg.exact_search("ac"));
    }

    {
        Reg reg("b?a{2,4}c");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("aaac"));
        RED_CHECK(reg.exact_search("aaac"));
    }

    {
        Reg reg("b?a{2,}c");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("aaaaaaaac"));
        RED_CHECK(reg.exact_search("aaaaaaac"));
    }

    {
        Reg reg("b?a{0,}c");

        RED_CHECK(reg.root());

        RED_CHECK(reg.search("c"));
        RED_CHECK(reg.exact_search("c"));
    }
}
