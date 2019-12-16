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


RED_TEST_DELEGATE_PRINT(std::chrono::seconds, x.count() << "s");


RED_AUTO_TEST_CASE(serialize_unserialize)
{
    using namespace Mwrm3;
    using namespace std::literals;

    static auto serialized_to_string = [](auto /*type*/, auto... bytes){
        return str_concat(bytes.as_chars()...);
    };

    // warning if a value of Type is missing :)
    switch (Type::None)
    {
        case Type::None:

#define CASE(serial, buffer, ...)                                          \
    [[fallthrough]]; case serial.type:                                     \
    serial.unserialize(buffer.drop_front(2) /* ignore type */, [](         \
        Type type, bytes_view remaining_bytes, auto... xs                  \
    ){                                                                     \
        RED_TEST(type == serial.type);                                     \
        RED_TEST(remaining_bytes.size() == 0);                             \
        __VA_ARGS__ (xs...);                                               \
                                                                           \
        RED_TEST(serial.serialize(xs..., serialized_to_string) == buffer); \
    }, [](){                                                               \
        RED_FAIL("error on " #serial ".unserialize");                      \
    })

        CASE(mwrm_header_compatibility,
            "v3\n"_av, [](){});

        CASE(wrm_new,
            "\x01\x00\x0f\x00my_filename.wrm"_av,
            [](bytes_view filename){
                RED_TEST(filename == "my_filename.wrm"_av);
            });

        CASE(wrm_state,
            "\x02\x00\xdc\x04\x00\x00\x00\x00\x00\x00}\x00\x00\x00\x00\x00\x00\x00\x03"
            "01234567890123456789012345678901"
            "abcdefghijabcdefghijabcdefghijab"_av,
            [](FileSize file_size, std::chrono::seconds seconds, QuickHash qhash, FullHash fhash){
               RED_TEST(file_size == FileSize(1244));
               RED_TEST(seconds == 125s);
               RED_TEST(qhash.hash == "01234567890123456789012345678901"_av);
               RED_TEST(fhash.hash == "abcdefghijabcdefghijabcdefghijab"_av);
            });

        CASE(fdx_new,
            "\x03\x00\x08\x00xxxx.fdx"_av,
            [](bytes_view filename){
                RED_TEST(filename == "xxxx.fdx"s);
            });

        CASE(tfl_new,
            "\x04\x00"
            "\x2a\x00\x00\x00\x00\x00\x00\x00"
            "\x0c\x00\x00\x00\x00\x00\x00\x00"
            "\x4f"
            "\x0f\x00"
            "\x10\x00"
            "my_secret_file!"
            "_SID_,000001.tfl"
            "01234567890123456789012345678901"
            "abcdefghijabcdefghijabcdefghijab"
            "ABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"_av,
            [](
                FileId file_id, FileSize file_size,
                Direction direction, TransferedStatus transfered_status,
                bytes_view original_filename, bytes_view tfl_filename,
                QuickHash qhash, FullHash fhash, Sha256Signature sig
            ){
               RED_TEST(file_id == FileId(42));
               RED_TEST(file_size == FileSize(12));
               RED_TEST(direction == Direction::ClientToServer);
               RED_TEST(transfered_status == TransferedStatus::Broken);
               RED_TEST(original_filename == "my_secret_file!"_av);
               RED_TEST(tfl_filename == "_SID_,000001.tfl"_av);
               RED_TEST(qhash.hash == "01234567890123456789012345678901"_av);
               RED_TEST(fhash.hash == "abcdefghijabcdefghijabcdefghijab"_av);
               RED_TEST(sig.sig == "ABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"_av);
            });
    }


#undef PARSE_TEST
}

RED_AUTO_TEST_CASE(mwrm3_parser)
{
    auto data =
        "v3\n"
        "\x04\x00"
        "\x04\x00\x00\x00\x00\x00\x00\x00"
        "\x03\x00\x00\x00\x00\x00\x00\x00"
        "\x08"
        "\x05\x00"
        "\x18\x00"
        "file4"
        "my_session_id,000004.tfl"_av;

    bytes_view remaining = data.first(33);

    using namespace Mwrm3;
    using namespace std::string_view_literals;

#define PARSE_TEST(vtype, ...) RED_TEST(parse_packet(remaining, [&]( \
    auto type, auto remaining_bytes, auto... xs                      \
){                                                                   \
    if constexpr (type == vtype) { __VA_ARGS__ (xs...); }            \
    else RED_TEST(type.value == vtype);                              \
    remaining = remaining_bytes;                                     \
}) == ParserResult::Ok)

    PARSE_TEST(Type::MwrmHeaderCompatibility, [](){});

    RED_TEST(parse_packet(remaining, [](auto...){}) == ParserResult::NeedMoreData);
    remaining = data.drop_front(remaining.data() - byte_ptr(data.data()));

    PARSE_TEST(Type::TflNew,
        [](
            FileId file_id, FileSize file_size,
            Direction direction, TransferedStatus transfered_status,
            bytes_view original_filename, bytes_view tfl_filename,
            QuickHash qhash, FullHash fhash, Sha256Signature sig
        ){
            RED_TEST(file_id == FileId(4));
            RED_TEST(file_size == FileSize(3));
            RED_TEST(transfered_status == TransferedStatus::Unknown);
            RED_TEST(direction == Direction::ClientToServer);
            RED_TEST(original_filename == "file4"sv);
            RED_TEST(tfl_filename == "my_session_id,000004.tfl"sv);
            RED_TEST(qhash.hash.empty());
            RED_TEST(fhash.hash.empty());
            RED_TEST(sig.sig.empty());
        });

    RED_REQUIRE(remaining.size() == 0);

    RED_TEST(parse_packet(remaining, [](auto...){}) == ParserResult::NeedMoreData);

#undef PARSE_TEST

    RED_TEST(parse_packet("\xff\xff"_av, [](auto...){}) == ParserResult::UnknownType);
}
