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

    template<class F, std::size_t... ints>
    void each_index(std::integer_sequence<std::size_t, ints...>, F f)
    {
        (f(std::integral_constant<std::size_t, ints>()), ...);
    }
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

#define CASE2_UNPACK(...) __VA_ARGS__
#define CASE2(type_test, fn, params, results, /*output_buffer*/...) [[fallthrough]];                 \
    case type_test: do {                                                                   \
        const bytes_view output_buffers[]{__VA_ARGS__};                                    \
        const auto input_buffer = str_concat(__VA_ARGS__);                                 \
        const auto input_buffer_without_type = array_view(input_buffer).drop_front(2);     \
                                                                                           \
        const auto unserialize_error = [](){                                               \
            RED_FAIL("error on un" #fn);                                                   \
            return CommonType{};                                                           \
        };                                                                                 \
                                                                                           \
        auto serialize_result = [&](auto&&... xs) {                                        \
            return fn(xs..., make_serialize_result);                                       \
        }(CASE2_UNPACK params);                                                             \
                                                                                           \
        auto unserialize_result = un##fn(input_buffer_without_type,                        \
            make_unserialize_result, unserialize_error);                                   \
                                                                                           \
        /* test serialize */                                                               \
                                                                                           \
        {                                                                                  \
            RED_CHECK(type_test == serialize_result.type);                                 \
            RED_CHECK(type_test == Type(InStream(                                          \
                serialize_result.buffers[0]).in_uint16_le()));                             \
            RED_CHECK(serialize_result.buffers.size() == std::size(output_buffers));       \
            auto size = std::min(serialize_result.buffers.size(),                          \
                                 std::size(output_buffers));                               \
            for (unsigned i = 0; i < size; ++i) {                                          \
                RED_TEST_CONTEXT("i = " << i) {                                            \
                    RED_CHECK(output_buffers[i] == serialize_result.buffers[i]);           \
                }                                                                          \
            }                                                                              \
        }                                                                                  \
                                                                                           \
        /* test unserialize */                                                             \
                                                                                           \
        {                                                                                  \
            results;                                                                       \
            int counter = 0;                                                               \
            auto cmp = [&](auto i){                                                        \
                RED_TEST_CONTEXT("i = " << counter++) {                                    \
                    RED_CHECK(ramify(std::get<i>(t)) ==                                    \
                        std::get<i>(unserialize_result.values));                           \
                }                                                                          \
            };                                                                             \
            RED_CHECK(type_test == unserialize_result.type);                               \
            RED_CHECK(0 == unserialize_result.remaining);                                  \
            constexpr auto unserialize_tuple_len                                           \
                = std::tuple_size<decltype(unserialize_result.values)>();                  \
            constexpr auto param_count = std::tuple_size<decltype(t)>();                   \
            constexpr auto min = std::min(unserialize_tuple_len.value, param_count.value); \
            RED_CHECK(unserialize_tuple_len == param_count);                               \
            each_index(std::make_index_sequence<min>(), cmp);                              \
        }                                                                                  \
    } while(0)

        CASE2(Type::MwrmHeaderCompatibility, serialize_mwrm_header_compatibility, (),
            auto t = std::forward_as_tuple()
            ,
            "v3\n"_av);

        CASE2(Type::WrmNew, serialize_wrm_new, (filename),
            auto a = filename;
            auto t = std::forward_as_tuple(a)
            ,
             "\x01\x00\x0f\x00"_av, filename);

        CASE2(Type::WrmState, serialize_wrm_stat,
            (FileSize(1244), std::chrono::seconds(125), QuickHash{qhash}, FullHash{fhash}),
            auto a = FileSize(1244);
            auto b = std::chrono::seconds(125);
            auto c = QuickHash{qhash};
            auto d = FullHash{fhash};
            auto t = std::forward_as_tuple(a, b, c, d)
            ,
            "\x02\x00\xdc\x04\x00\x00\x00\x00\x00\x00}\x00\x00\x00\x00\x00\x00\x00\x01"_av,
            qhash, fhash);

        CASE2(Type::FdxNew, serialize_fdx_new, (filename), 
            auto a = filename;
            auto t = std::forward_as_tuple(a)
            ,
            "\x03\x00\x0f\x00"_av, filename);

        CASE2(Type::TflNew, serialize_tfl_new, (42, filename, filename2),
            auto a = 42;
            auto b = filename;
            auto c = filename2;
            auto t = std::forward_as_tuple(a, b, c)
            ,
            "\x04\x00*\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x10\x00"_av, filename, filename2);

        CASE2(Type::TflState, serialize_tfl_stat,
            (42, FileSize(12), QuickHash{qhash}, FullHash{fhash}),
            auto a = 42;
            auto b = FileSize(12);
            auto c = QuickHash{qhash};
            auto d = FullHash{fhash};
            auto t = std::forward_as_tuple(a, b, c, d)
            ,
            "\x05\x00*\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x01"_av,
            qhash, fhash);
    }
}
