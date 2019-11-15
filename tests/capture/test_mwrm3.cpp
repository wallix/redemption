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

#include "capture/mwrm3.hpp"

#include <iterator>

RED_AUTO_TEST_CASE(header_line)
{
    using namespace Mwrm3;

    char buffer[128];

    using std::size;
    using std::empty;

    Type type{1};
    DataSize data_size{5u};

    writable_buffer_view result;
    for (unsigned i = 0; i < size(buffer) && empty(result); ++i) {
        std::tie(result, std::ignore)
          = serialize_header_line(writable_bytes_view(buffer, i), type, data_size);
    }

    RED_CHECK_MEM(result, "\x01\x00\x05\x00"_av);
}

RED_AUTO_TEST_CASE(serialize)
{
    using namespace Mwrm3;

#define as_error() []([[maybe_unused]] auto... xs){ RED_FAIL("error on serialize_*"); }

    auto filename = "my_filename.wrm"_av;
    auto filename2 = "my_filename2.wrm"_av;

    auto qhash = "0123456789012345678901234567890123456789012345678901234567890123"_av;
    auto fhash = "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcd"_av;

    // warning if a value of Type is missing :)
    switch (Type::None)
    {
        case Type::None:
            [[fallthrough]];

        case Type::WrmNew:
            serialize_wrm_new(filename, [&](Type type, DataSize data_size, auto data){
                RED_CHECK(type == Type::WrmNew);
                RED_CHECK(data_size == DataSize(filename.size()));
                RED_CHECK(data == filename);
            }, as_error());
            [[fallthrough]];

        case Type::WrmState:
            serialize_wrm_stat(
                FileSize(1244), std::chrono::seconds(125), QuickHash{qhash}, FullHash{fhash},
                [&](Type type, DataSize data_size,
                    bytes_view prefix, bytes_view qhash, bytes_view fhash
                ){
                    RED_CHECK(type == Type::WrmState);
                    RED_CHECK(data_size == DataSize(144));
                    RED_CHECK(prefix == "\xdc\x04\x00\x00\x00\x00\x00\x00}\x00\x00\x00\x00\x00\x00\x00"_av);
                    RED_CHECK(qhash == qhash);
                    RED_CHECK(fhash == fhash);
                }
            );
            [[fallthrough]];

        case Type::FdxNew:
            serialize_fdx_new(filename, [&](Type type, DataSize data_size, auto data){
                RED_CHECK(type == Type::FdxNew);
                RED_CHECK(data_size == DataSize(filename.size()));
                RED_CHECK(data == filename);
            }, as_error());
            [[fallthrough]];

        case Type::TflNew:
            serialize_tfl_new(
                filename, filename2, [&](Type type, DataSize data_size,
                    bytes_view header, bytes_view file1, bytes_view file2
                ){
                    RED_CHECK(type == Type::TflNew);
                    RED_CHECK(data_size == DataSize(33));
                    RED_CHECK(header == "\x0f\x00"_av);
                    RED_CHECK(file1 == filename);
                    RED_CHECK(file2 == filename2);
                },
                as_error()
            );
            [[fallthrough]];

        case Type::TflState:
            serialize_tfl_stat(
                FileSize(12), QuickHash{qhash}, FullHash{fhash},
                [&](Type type, DataSize data_size,
                    bytes_view prefix, bytes_view qhash, bytes_view fhash
                ){
                    RED_CHECK(type == Type::TflState);
                    RED_CHECK(data_size == DataSize(136));
                    RED_CHECK(prefix == "\x0c\x00\x00\x00\x00\x00\x00\x00"_av);
                    RED_CHECK(qhash == qhash);
                    RED_CHECK(fhash == fhash);
                }
            );
    }
}
