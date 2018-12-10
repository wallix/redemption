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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#define RED_TEST_MODULE TestSubRectGenerator
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/contiguous_sub_rect_f.hpp"
#include <array>

template<std::size_t N>
struct AccuRect
{
    std::array<Rect, N> const ref;
    std::array<Rect, N> computed;
    std::size_t i;

    void operator()(Rect r)
    {
        RED_REQUIRE_NE(this->i, N);
        this->computed[this->i++] = r;
    }
};

template<class... Rects>
AccuRect<sizeof...(Rects)>
rects(Rects... rects)
{
    return {{{rects...}}, {}, 0};
}

RED_AUTO_TEST_CASE(Testcontiguous_sub_rect_f1)
{
    auto f = rects();
    contiguous_sub_rect_f(CxCy{0, 0}, SubCxCy{32, 32}, std::ref(f));
    RED_CHECK_EQUAL_RANGES(f.ref, f.computed);
}

RED_AUTO_TEST_CASE(Testcontiguous_sub_rect_f2)
{
    auto f = rects(Rect(0, 0, 50, 50));
    contiguous_sub_rect_f(CxCy{50, 50}, SubCxCy{50, 50}, std::ref(f));
    RED_CHECK_EQUAL_RANGES(f.ref, f.computed);
}

RED_AUTO_TEST_CASE(Testcontiguous_sub_rect_f3)
{
    auto f = rects(
        Rect( 0,  0, 25, 25),
        Rect(25,  0, 25, 25),
        Rect( 0, 25, 25, 25),
        Rect(25, 25, 25, 25));
    contiguous_sub_rect_f(CxCy{50, 50}, SubCxCy{25, 25}, std::ref(f));
    RED_CHECK_EQUAL_RANGES(f.ref, f.computed);
}

RED_AUTO_TEST_CASE(Testcontiguous_sub_rect_f4)
{
    auto f = rects(
        Rect( 0,  0, 32, 32),
        Rect(32,  0, 32, 32),
        Rect(64,  0, 32, 32),
        Rect(96,  0,  4, 32),
        Rect( 0, 32, 32, 32),
        Rect(32, 32, 32, 32),
        Rect(64, 32, 32, 32),
        Rect(96, 32,  4, 32),
        Rect( 0, 64, 32, 32),
        Rect(32, 64, 32, 32),
        Rect(64, 64, 32, 32),
        Rect(96, 64,  4, 32),
        Rect( 0, 96, 32,  4),
        Rect(32, 96, 32,  4),
        Rect(64, 96, 32,  4),
        Rect(96, 96,  4,  4));
    contiguous_sub_rect_f(CxCy{100, 100}, SubCxCy{32, 32}, std::ref(f));
    RED_CHECK_EQUAL_RANGES(f.ref, f.computed);
}
