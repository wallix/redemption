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

    template<std::size_t n>
    struct SerializeResult
    {
        Mwrm3::Type type;
        std::array<std::vector<uint8_t>, n> buffers;
    };

    template<class... Ts>
    struct UnserializeResult
    {
        Mwrm3::Type type;
        std::size_t remaining;
        std::tuple<Ts...> values;
    };

    struct CommonType
    {
        template<class T>
        operator T () const { return T(); }
    };

    inline constexpr auto make_serialize_result = [](Mwrm3::Type type, auto... bytes_views){
        return SerializeResult<sizeof...(bytes_views)>{
            type,
            {std::vector<uint8_t>(bytes_views.begin(), bytes_views.end())...}
        };
    };

    inline constexpr auto make_unserialize_result = [](Mwrm3::Type type, auto remaining_bytes, auto... values){
        return UnserializeResult<std::decay_t<decltype(ramify(values))>...>{
            type,
            remaining_bytes.size(),
            {ramify(values)...}
        };
    };
}

RED_AUTO_TEST_CASE(serialize_unserialize)
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
#define CASE(type_test, fn, params, /*output_buffer*/...) [[fallthrough]];                    \
    case type_test: do {                                                                      \
        const bytes_view output_buffers[]{__VA_ARGS__};                                       \
        const auto input_buffer = str_concat(__VA_ARGS__);                                    \
        const auto input_buffer_without_type = array_view(input_buffer).drop_front(2);        \
                                                                                              \
        const auto unserialize_error = [](){                                                  \
            RED_FAIL("error on un" #fn);                                                      \
            return CommonType{};                                                              \
        };                                                                                    \
                                                                                              \
        auto serialize_result = fn(CASE_UNPACK params,                                        \
            make_serialize_result);                                                           \
        auto unserialize_result = un##fn(input_buffer_without_type,                           \
            make_unserialize_result, unserialize_error);                                      \
                                                                                              \
        /* test serialize */                                                                  \
                                                                                              \
        {                                                                                     \
            RED_CHECK(type_test == serialize_result.type);                                    \
            RED_CHECK(type_test == Type(InStream(serialize_result.buffers[0]).in_uint8()));   \
            RED_CHECK(serialize_result.buffers.size() == std::size(output_buffers));          \
            auto size = std::min(serialize_result.buffers.size(), std::size(output_buffers)); \
            for (unsigned i = 0; i < size; ++i) {                                             \
                RED_TEST_CONTEXT("i = " << i) {                                               \
                    RED_CHECK(output_buffers[i] == serialize_result.buffers[i]);              \
                }                                                                             \
            }                                                                                 \
        }                                                                                     \
                                                                                              \
        /* test unserialize */                                                                \
                                                                                              \
        {                                                                                     \
            RED_CHECK(type_test == unserialize_result.type);                                  \
            RED_CHECK(0 == unserialize_result.remaining);                                     \
            int i = 0;                                                                        \
            auto cmp = [&i](auto x, auto y){                                                  \
                RED_TEST_CONTEXT("i = " << i++) {                                             \
                    RED_CHECK(ramify(y) == ramify(x));                                        \
                }                                                                             \
            };                                                                                \
            /* fix for clang < 9 */                                                           \
            REDEMPTION_DIAGNOSTIC_PUSH                                                        \
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-lambda-capture")                     \
            std::apply([&](auto... xs){                                                       \
                [&, xs...](auto... ys){ (cmp(ys, xs), ...); }(CASE_UNPACK params);            \
            }, unserialize_result.values);                                                    \
            REDEMPTION_DIAGNOSTIC_POP                                                         \
        }                                                                                     \
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
