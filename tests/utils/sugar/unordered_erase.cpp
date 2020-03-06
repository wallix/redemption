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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/unordered_erase.hpp"


RED_AUTO_TEST_CASE(erase_elem)
{
    std::vector<char> v{'a', 'b', 'c', 'd', 'e'};

    RED_TEST(unordered_erase_element(v, &v[2]) == "abed"_av);
    RED_TEST(unordered_erase_element(v, &v[0]) == "dbe"_av);
    RED_TEST(unordered_erase_element(v, &v[2]) == "db"_av);
    RED_TEST(unordered_erase_element(v, &v[1]) == "d"_av);
    RED_TEST(unordered_erase_element(v, &v[0]) == ""_av);
}

RED_AUTO_TEST_CASE(erase_if)
{
    std::vector<char>
    v = {'a', 'b', 'c', 'd', 'e'};

    RED_TEST(unordered_erase_if(v, [](auto){ return false; }) == "abcde"_av);
    RED_TEST(unordered_erase_if(v, [](auto c){ return c == 'e'; }) == "abcd"_av);
    RED_TEST(unordered_erase_if(v, [](auto c){ return c == 'a'; }) == "dbc"_av);
    RED_TEST(unordered_erase_if(v, [](auto c){ return c != 'c'; }) == "c"_av);
    RED_TEST(unordered_erase_if(v, [](auto){ return true; }) == ""_av);

    v = {'a', 'b', 'c', 'd', 'e'};
    RED_TEST(unordered_erase_if(v, [](auto c){ return (c & 1) != ('a' & 1); }) == "aec"_av);

    v = {'a', 'b', 'c', 'd', 'e'};
    RED_TEST(unordered_erase_if(v, [](auto c){ return (c & 1) == ('a' & 1); }) == "db"_av);
}
