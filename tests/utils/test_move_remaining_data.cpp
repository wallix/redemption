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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean

   Unit test for char parse class

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/move_remaining_data.hpp"

#include <cstring>

static bool operator == (RemainingDataResult const& a, RemainingDataResult const& b)
{
    return a.status == b.status
        && a.data.size() == b.data.size()
        && a.data.data()
        && b.data.data()
        && 0 == std::memcmp(a.data.data(), b.data.data(), a.data.size())
        ;
}

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "utils/sugar/int_to_chars.hpp"
# include "test_only/test_framework/compare_collection.hpp"

namespace
{

static ut::assertion_result test_comp_remaining_result(RemainingDataResult const& a, RemainingDataResult const& b)
{
    ut::assertion_result ar(true);

    if (REDEMPTION_UNLIKELY(!(a == b))) {
        ar = false;

        auto put = [&](std::ostream& out, RemainingDataResult const& r){
            out << "{" << r.status << ", \"";
            out.write(r.data.as_charp(), long(r.data.size()));
            out << "\"}";
        };

        auto& out = ar.message().stream();
        out << "[";
        ut::put_data_with_diff(out, a, "!=", b, put);
        out << "]";
    }

    return ar;
}

}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::RemainingDataResult), (::RemainingDataResult), ::test_comp_remaining_result)
#endif

RED_AUTO_TEST_CASE(Test_move_remaining_data)
{
    using reader_n = std::size_t;

    std::array<uint8_t, 5> buffer;
    auto move_data = [&](bytes_view data, std::size_t n) {
        buffer.fill(0);
        auto reader = [n](writable_bytes_view av) {
            auto len = std::min(av.size(), n);
            memcpy(av.data(), "abcdefghijklmnopqrst", len);
            return len;
        };
        return move_remaining_data(data, buffer, reader);
    };

    using St = RemainingDataResult::Status;
    auto result = [](St status, chars_view av) {
        auto* p = const_cast<char*>(av.data()); /* NOLINT */
        return RemainingDataResult{status, {p, av.size()}};
    };

    RED_CHECK(move_data(""_av, reader_n(0)) == result(St::Eof, ""_av));
    RED_CHECK(move_data(""_av, reader_n(1)) == result(St::Ok, "a"_av));
    RED_CHECK(move_data(""_av, reader_n(11)) == result(St::Ok, "abcde"_av));

    RED_CHECK(move_data("1"_av, reader_n(0)) == result(St::Eof, ""_av));
    RED_CHECK(move_data("1"_av, reader_n(1)) == result(St::Ok, "1a"_av));
    RED_CHECK(move_data("1"_av, reader_n(11)) == result(St::Ok, "1abcd"_av));

    RED_CHECK(move_data("1234"_av, reader_n(0)) == result(St::Eof, ""_av));
    RED_CHECK(move_data("1234"_av, reader_n(1)) == result(St::Ok, "1234a"_av));
    RED_CHECK(move_data("1234"_av, reader_n(11)) == result(St::Ok, "1234a"_av));

    RED_CHECK(move_data("12345"_av, reader_n(11)) == result(St::DataToLarge, ""_av));
}
