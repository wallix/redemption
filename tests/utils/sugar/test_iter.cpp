/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#define RED_TEST_MODULE TestIter
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/iter.hpp"
#include <string>

RED_AUTO_TEST_CASE(TestIter)
{
    std::string g;
    {
        char s[] = "0123456789";
        for (char c : iter(s, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        const char s[] = "0123456789";
        for (char c : iter(s, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        char s[] = "0123456789";
        char * p = s;
        for (char c : iter(p, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        char s[] = "0123456789";
        for (char c : iter(s, s+3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        char s[] = "0123456789";
        char * p = s + 3;
        for (char c : iter(s, p)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        const char * s = "0123456789";
        for (char c : iter(s, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "012");
    }
    g.clear();
    {
        const char * s = "0123456789";
        for (char c : iter(s + 2, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "234");
    }
    g.clear();
    {
        std::string s("abcdef");
        for (char c : iter(s.begin(), 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "abc");
    }
    g.clear();
    {
        std::string s("abcdef");
        for (char c : iter(s, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "abc");
    }
    g.clear();
    {
        const std::string s("abcdef");
        for (char c : iter(s, 3)) {
            g.push_back(c);
        }
        RED_CHECK_EQUAL(g, "abc");
    }
    g.clear();
    {
        const std::string s("abcdef");
        for (char c : iter(s.begin(), s.end())) {
            g.push_back(c);
          }
        RED_CHECK_EQUAL(g, s);
    }
}
