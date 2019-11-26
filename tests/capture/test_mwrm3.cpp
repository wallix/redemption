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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/algostring.hpp"
#include "capture/mwrm3.hpp"

#include <iterator>


namespace
{
    template<unsigned> class should_be_same_size{};

    template<class T> T const& ramify(T const& x) { return x; }
    bytes_view ramify(Mwrm3::QuickHash h) { return h.hash; }
    bytes_view ramify(Mwrm3::FullHash h) { return h.hash; }
    auto ramify(std::chrono::seconds seconds) { return seconds.count(); }
}

RED_AUTO_TEST_CASE(serialize)
{
    using namespace Mwrm3;

    auto filename = "my_filename.wrm"_av;
    auto filename2 = "my_filename2.wrm"_av;

    auto qhash = "01234567890123456789012345678901"_av;
    auto fhash = "abcdefghijabcdefghijabcdefghijab"_av;

    // warning if a value of Type is missing :)
    switch (Type::None)
    {
        case Type::None:

#define CASE_UNPACK(...) __VA_ARGS__
#define CASE(type_test, fn, params, ...) [[fallthrough]];                 \
    case type_test: do {                                                  \
        bytes_view rhs[]{__VA_ARGS__};                                    \
        auto checker = [&](Type type, bytes_view av, auto... avs){        \
            RED_CHECK(type == type_test);                                 \
            RED_CHECK(type == Type(InStream(av).in_uint8()));             \
            bytes_view lhs[]{av, avs...};                                 \
            should_be_same_size<sizeof(lhs)/sizeof(lhs[0])>{} =           \
                should_be_same_size<sizeof(rhs)/sizeof(rhs[0])>{};        \
            for (unsigned i = 0; i < std::size(lhs); ++i) {               \
                RED_TEST_CONTEXT("i = " << i) {                           \
                    RED_CHECK(lhs[i] == rhs[i]);                          \
                }                                                         \
            }                                                             \
            return str_concat(av.as_chars(), avs.as_chars()...);          \
        };                                                                \
        std::string data = fn(CASE_UNPACK params, checker);               \
        un##fn(array_view(data).drop_front(2), [&](                       \
            Type type, bytes_view av, auto... xs                          \
        ){                                                                \
            RED_CHECK(type == type_test);                                 \
            RED_CHECK(av.empty());                                        \
            /* fix for clang < 9 */                                       \
            REDEMPTION_DIAGNOSTIC_PUSH                                    \
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-lambda-capture") \
            [&xs...](auto... ys){                                         \
                int i = 0;                                                \
                ([&](){                                                   \
                    RED_TEST_CONTEXT("i = " << i++) {                     \
                        RED_CHECK(ramify(ys) == ramify(xs));              \
                    }                                                     \
                }(), ...);                                                \
            }(CASE_UNPACK params);                                        \
            REDEMPTION_DIAGNOSTIC_POP                                     \
        }, [](){ RED_FAIL("error on un" #fn); });                         \
    } while(0)

        CASE(Type::WrmNew, serialize_wrm_new, (filename), "\x01\x00\x0f\x00"_av, filename);

        CASE(Type::WrmState, serialize_wrm_stat,
            (FileSize(1244), std::chrono::seconds(125), QuickHash{qhash}, FullHash{fhash}),
            "\x02\x00\xdc\x04\x00\x00\x00\x00\x00\x00}\x00\x00\x00\x00\x00\x00\x00"_av,
            qhash, fhash);

        CASE(Type::FdxNew, serialize_fdx_new, (filename), "\x03\x00\x0f\x00"_av, filename);

        CASE(Type::TflNew, serialize_tfl_new, (42, filename, filename2),
            "\x04\x00*\x00\x00\x00\x00\x00\x00\x00\x0f\x00"_av, filename, "\x10\x00"_av, filename2);

        CASE(Type::TflState, serialize_tfl_stat,
            (42, FileSize(12), QuickHash{qhash}, FullHash{fhash}),
            "\x05\x00*\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00"_av,
            qhash, fhash);
    }
}
