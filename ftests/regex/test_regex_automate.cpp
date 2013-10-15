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
#define BOOST_TEST_MODULE TestDfaRegexAutomate
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL


#include "regex_automate.hpp"

using namespace re;

BOOST_AUTO_TEST_CASE(TestRegexCheck)
{
    {
        StateBase st(NORMAL, multi_char("Þ"));
        BOOST_CHECK(st.check(multi_char("Þ")));
        BOOST_CHECK( ! st.check('a'));
    }
    {
        StateCharacters st("aÎbps");
        BOOST_CHECK(st.check('a'));
        BOOST_CHECK(st.check(multi_char("Î")));
        BOOST_CHECK(st.check('b'));
        BOOST_CHECK(st.check('p'));
        BOOST_CHECK(st.check('s'));
        BOOST_CHECK( ! st.check('t'));
    }
    {
        StateRange st('e','g');
        BOOST_CHECK(st.check('e'));
        BOOST_CHECK(st.check('f'));
        BOOST_CHECK(st.check('g'));
        BOOST_CHECK( ! st.check('d'));
        BOOST_CHECK( ! st.check('h'));
    }
    {
        StateSpace st;
        BOOST_CHECK(st.check(' '));
        BOOST_CHECK(st.check('\t'));
        BOOST_CHECK(st.check('\n'));
        BOOST_CHECK( ! st.check('a'));
    }
    {
        StateNoSpace st;
        BOOST_CHECK( ! st.check(' '));
        BOOST_CHECK( ! st.check('\t'));
        BOOST_CHECK( ! st.check('\n'));
        BOOST_CHECK(st.check('a'));
    }
    {
        StateDigit st;
        BOOST_CHECK(st.check('0'));
        BOOST_CHECK(st.check('1'));
        BOOST_CHECK(st.check('2'));
        BOOST_CHECK(st.check('3'));
        BOOST_CHECK(st.check('4'));
        BOOST_CHECK(st.check('5'));
        BOOST_CHECK(st.check('6'));
        BOOST_CHECK(st.check('7'));
        BOOST_CHECK(st.check('8'));
        BOOST_CHECK(st.check('9'));
        BOOST_CHECK( ! st.check('a'));
    }
    {
        StateWord st;
        BOOST_CHECK(st.check('0'));
        BOOST_CHECK(st.check('9'));
        BOOST_CHECK(st.check('a'));
        BOOST_CHECK(st.check('z'));
        BOOST_CHECK(st.check('A'));
        BOOST_CHECK(st.check('Z'));
        BOOST_CHECK(st.check('_'));
        BOOST_CHECK( ! st.check('-'));
        BOOST_CHECK( ! st.check(':'));
    }
    {
        StateMultiTest st;
        st.push_checker(new CheckerDigit);
        st.push_checker(new CheckerString("abc"));
        BOOST_CHECK(st.check('0'));
        BOOST_CHECK(st.check('1'));
        BOOST_CHECK(st.check('9'));
        BOOST_CHECK(st.check('a'));
        BOOST_CHECK(st.check('b'));
        BOOST_CHECK(st.check('c'));
        BOOST_CHECK( ! st.check('d'));
    }
}


