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
        (void)f; // disable warning if ints... is empty
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

#define PARSE_TEST_UNPACK(...) __VA_ARGS__
#define PARSE_TEST(type_test, fn, params, /*output_buffer*/...) [[fallthrough]];                 \
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
        }(PARSE_TEST_UNPACK params);                                                             \
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
            auto t = std::make_tuple(PARSE_TEST_UNPACK params);                                  \
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

        PARSE_TEST(Type::MwrmHeaderCompatibility, serialize_mwrm_header_compatibility, (), "v3\n"_av);

        PARSE_TEST(Type::WrmNew, serialize_wrm_new, (filename), "\x01\x00\x0f\x00"_av, filename);

        PARSE_TEST(Type::WrmState, serialize_wrm_stat,
            (FileSize(1244), std::chrono::seconds(125), QuickHash{qhash}, FullHash{fhash}),
            "\x02\x00\xdc\x04\x00\x00\x00\x00\x00\x00}\x00\x00\x00\x00\x00\x00\x00\x01"_av,
            qhash, fhash);

        PARSE_TEST(Type::FdxNew, serialize_fdx_new, (filename), "\x03\x00\x0f\x00"_av, filename);

        PARSE_TEST(Type::TflNew, serialize_tfl_new, (FileId(42), filename, filename2),
            "\x04\x00*\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x10\x00"_av, filename, filename2);

        PARSE_TEST(Type::TflState, serialize_tfl_stat,
            (FileId(42), FileSize(12), QuickHash{qhash}, FullHash{fhash}),
            "\x05\x00*\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x01"_av,
            qhash, fhash);
    }

#undef PARSE_TEST
}

RED_AUTO_TEST_CASE(mwrm3_parser)
{
    auto data =
        "v3\n\x04\x00\x04\x00\x00\x00\x00\x00\x00\x00\x05\x00\x18\x00""file"
        "4my_session_id,000004.tfl\x05\x00\x04\x00\x00\x00\x00\x00\x00\x00"
        "\x03\x00\x00\x00\x00\x00\x00\x00\x00"_av;

    bytes_view remaining = data.first(33);

    using namespace Mwrm3;
    using namespace std::string_view_literals;

#define PARSE_TEST(vtype, ...) RED_TEST(parse_packet(remaining, [&]( \
    auto type, auto remaining_bytes, auto... xs                      \
){                                                                   \
    if constexpr (type == vtype) { __VA_ARGS__ (xs...); }            \
    else RED_TEST(type.value == vtype);                              \
    remaining = remaining_bytes;                                     \
}) == ParserResult::Ok);

    PARSE_TEST(Type::MwrmHeaderCompatibility, [](){});

    RED_TEST(parse_packet(remaining, [](auto...){}) == ParserResult::NeedMoreData);
    remaining = data.drop_front(remaining.data() - byte_ptr(data.data()));

    PARSE_TEST(Type::TflNew,
        [](FileId file_id, bytes_view original_filename, bytes_view tfl_filename){
            RED_TEST(file_id == FileId(4));
            RED_TEST(original_filename == "file4"sv);
            RED_TEST(tfl_filename == "my_session_id,000004.tfl"sv);
        });

    PARSE_TEST(Type::TflState,
        [](FileId file_id, FileSize file_size, QuickHash qhash, FullHash fhash){
            RED_TEST(file_id == FileId(4));
            RED_TEST(file_size == FileSize(3));
            RED_TEST(qhash.hash.empty());
            RED_TEST(fhash.hash.empty());
        });

    RED_REQUIRE(remaining.size() == 0);

#undef PARSE_TEST

    RED_TEST(parse_packet("\xff\xff"_av, [](auto...){}) == ParserResult::UnknownType);
}
