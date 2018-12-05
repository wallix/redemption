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

#define RED_TEST_MODULE TestNonCopyable
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/noncopyable.hpp"
#include <type_traits>

struct Copyable {

};
struct NonCopyable : noncopyable{

};

RED_AUTO_TEST_CASE(TestSplitter)
{
    RED_CHECK_EQUAL(std::is_copy_constructible<Copyable>::value, true);
    RED_CHECK_EQUAL(std::is_copy_assignable<Copyable>::value, true);

    RED_CHECK_EQUAL(std::is_copy_constructible<NonCopyable>::value, false);
    RED_CHECK_EQUAL(std::is_copy_assignable<NonCopyable>::value, false);

    // test default constructible
    NonCopyable x; (void)x;
}
