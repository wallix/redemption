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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#define UNIT_TEST_MODULE TestUniquePtr
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/make_unique.hpp"
#include <type_traits>
#include <iostream>
#include <sstream>

struct A {
    A(int, int) {}
    A() = default;
};

RED_AUTO_TEST_CASE(TestMakeUnique)
{
    RED_CHECK((std::is_same<std::unique_ptr<A>, decltype(std::make_unique<A>(2, 3))>::value));
    RED_CHECK((std::is_same<std::unique_ptr<A[]>, decltype(std::make_unique<A[]>(2))>::value));

    std::stringbuf buf;
    auto * oldbuf = std::cout.rdbuf(&buf);

    struct D {
        D() { std::cout << "c"; }
        D(int) { std::cout << "i"; }
        ~D() { std::cout << "d"; }
    };

    std::make_unique<D>();
    RED_CHECK_EQUAL(buf.str(), "cd");

    buf.str("");
    std::make_unique<D>(1);
    RED_CHECK_EQUAL(buf.str(), "id");

    buf.str("");
    std::make_unique<D[]>(2);
    RED_CHECK_EQUAL(buf.str(), "ccdd");

    std::cout.rdbuf(oldbuf);
}
