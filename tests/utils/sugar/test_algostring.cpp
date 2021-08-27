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

#include "utils/sugar/algostring.hpp"


RED_AUTO_TEST_CASE(TestTrim)
{
    char s[] = " \t abcd   ";
    auto av = make_writable_array_view(s).drop_back(1);

    RED_CHECK_EQUAL(ltrim(chars_view(av).last(0)), ""_av);
    RED_CHECK_EQUAL(ltrim(chars_view(av).last(3)), ""_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av).last(0)), ""_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av).last(3)), ""_av);
    RED_CHECK_EQUAL(ltrim(chars_view(av)), "abcd   "_av);
    RED_CHECK_EQUAL(rtrim(chars_view(av)), " \t abcd"_av);
    RED_CHECK_EQUAL(trim(chars_view(av)), "abcd"_av);

    RED_CHECK_EQUAL(ltrim(av.last(0)), ""_av);
    RED_CHECK_EQUAL(ltrim(av.last(3)), ""_av);
    RED_CHECK_EQUAL(rtrim(av.last(0)), ""_av);
    RED_CHECK_EQUAL(rtrim(av.last(3)), ""_av);
    RED_CHECK_EQUAL(ltrim(av), "abcd   "_av);
    RED_CHECK_EQUAL(rtrim(av), " \t abcd"_av);
    RED_CHECK_EQUAL(trim(av), "abcd"_av);
}
