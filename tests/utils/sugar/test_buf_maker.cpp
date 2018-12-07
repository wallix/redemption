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

#define RED_TEST_MODULE TestBufMaker
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/buf_maker.hpp"

RED_AUTO_TEST_CASE(TestBufMaker)
{
    using P = void const*;
    BufMaker<16> buf_maker;
    RED_CHECK_EQ(buf_maker.static_array().size(), 16u);

    auto av = buf_maker.dyn_array(12);
    RED_CHECK_EQ(av.size(), 12u);
    RED_CHECK_EQ(P(av.data()), P(buf_maker.static_array().data()));

    av = buf_maker.dyn_array(20);
    RED_CHECK_EQ(av.size(), 20u);
    RED_CHECK_NE(P(av.data()), P(buf_maker.static_array().data()));
}
