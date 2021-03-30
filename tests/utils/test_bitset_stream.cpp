/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/bitset_stream.hpp"

RED_AUTO_TEST_CASE(TestBitsetIt)
{
    using int_type = BitsetOutStream::underlying_type;
    const int rep = 6;
    const int n = 11;

    int_type data[n*rep / sizeof(int_type) + 2] {};

    BitsetOutStream out(data);
    for (int i = 0; i < rep; ++i) {
        RED_TEST_CONTEXT(i) {
            out.write(true);
            out.write(true);
            out.write(false);
            out.write(false);
            out.write(false);
            out.write(true);
            out.write(true);
            out.write(true);
            out.write(true);
            out.write(true);
            out.write(true);
        }
        RED_CHECK(out.is_partial());
    }

    BitsetInStream in(data);
    for (int i = 0; i < rep; ++i) {
        RED_TEST_CONTEXT(i) {
            RED_CHECK(in.read());
            RED_CHECK(in.read());
            RED_CHECK(!in.read());
            RED_CHECK(!in.read());
            RED_CHECK(!in.read());
            RED_CHECK(in.read());
            RED_CHECK(in.read());
            RED_CHECK(in.read());
            RED_CHECK(in.read());
            RED_CHECK(in.read());
            RED_CHECK(in.read());
        }
        RED_CHECK(out.is_partial());
    }
    for (int i = 0; i < int(sizeof(data) * 8) - rep*n; ++i) {
        RED_TEST_CONTEXT(i) {
            RED_CHECK(std::end(data) != in.current());
            RED_CHECK(!in.read());
        }
    }
    RED_CHECK(std::end(data) == in.current());
    RED_CHECK(!in.is_partial());
}
