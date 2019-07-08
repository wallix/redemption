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
   Copyright (C) Wallix 2019
   Author(s): Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"
#include "test_only/transport/test_transport.hpp"

#include "mod/icap_files_service.hpp"
#include "utils/sugar/algostring.hpp"


RED_AUTO_TEST_CASE(icapSend)
{
    BufTransport trans;
    ICAPService icap{trans};

    auto filename = FIXTURES_PATH "/test_infile.txt";
    auto content = tu::get_file_contents(filename);

    ICAPFileId id = icap.open_file(filename, "clamav");
    RED_CHECK(id == ICAPFileId(1));

    icap.send_data(id, content);
    icap.send_eof(id);

    const auto data_ref =
        "\x00\x00\x00\x00\x3b\x00\x00\x00\x01\x00\x00\x00\x29\x2e\x2f\x74" //....;.......)./t
        "\x65\x73\x74\x73\x2f\x69\x6e\x63\x6c\x75\x64\x65\x73\x2f\x66\x69" //ests/includes/fi
        "\x78\x74\x75\x72\x65\x73\x2f\x74\x65\x73\x74\x5f\x69\x6e\x66\x69" //xtures/test_infi
        "\x6c\x65\x2e\x74\x78\x74\x00\x00\x00\x06\x63\x6c\x61\x6d\x61\x76" //le.txt....clamav
        "\x01\x00\x00\x00\x1c\x00\x00\x00\x01\x57\x65\x20\x72\x65\x61\x64" //.........We read
        "\x20\x77\x68\x61\x74\x20\x77\x65\x20\x70\x72\x6f\x76\x69\x64\x65" // what we provide
        "\x21\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av;
    RED_CHECK_MEM(trans.data(), data_ref);

    RED_CHECK(icap.open_file(filename, "clamav") == ICAPFileId(2));
}

RED_AUTO_TEST_CASE(icapReceive)
{
    BufTransport trans;
    ICAPService icap{trans};

    auto setbuf = [&](cbytes_view data){
        trans.buf.assign(data.as_charp(), data.size());
    };

    // init header
    RED_CHECK(!icap.service_is_up());
    RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    setbuf("\x07\x00\x00\x00"_av); // msg_type, len(3)
    RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    setbuf("\x05\x01"_av);         // len(1), flag
    RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    setbuf("\x00\x00\x00\x01"_av); // max_connection_number
    RED_CHECK(icap.receive_response() == ICAPService::ResponseType::HasPacket);
    RED_REQUIRE(icap.service_is_up());
    RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);

    // message 1
    {
        auto response =
            "\x05\x00\x00\x00\x00"  // msg_type, len(ignored)
            "\x00"                  // result
            "\x00\x00\x00\x01"      // file id
            "\x00\x00\x00\x02"      // size
            ""_av;
        auto pos = response.size() / 2;
        setbuf(response.first(pos));
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
        setbuf(response.array_from_offset(pos));
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
        setbuf("o"_av);
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
        setbuf("k"_av);
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::HasContent);
        RED_CHECK(icap.last_file_id() == ICAPFileId(1));
        RED_CHECK(icap.get_content() == "ok");
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    }

    // message 2
    {
        auto response =
            "\x05\x00\x00\x00\x00"  // msg_type, len(ignored)
            "\x01"                  // result
            "\x00\x00\x00\x04"      // file id
            "\x00\x00\x00\x02"      // size
            "ko"_av;
        setbuf(response);
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::HasContent);
        RED_CHECK(icap.last_file_id() == ICAPFileId(4));
        RED_CHECK(icap.get_content() == "ko");
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    }

    // message 3
    {
        auto response =
            "\x05\x00\x00\x00\x00"  // msg_type, len(ignored)
            "\x01"                  // result
            "\x00\x00\x00\x03"      // file id
            "\x00\x00\x00\x02"      // size
            "ok"
            "\x05\x00\x00\x00\x00"  // msg_type, len(ignored)
            "\x01"                  // result
            "\x00\x00\x00\x05"      // file id
            "\x00\x00\x00\x02"      // size
            "ko"_av;
        setbuf(response);
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::HasContent);
        RED_CHECK(icap.last_file_id() == ICAPFileId(3));
        RED_CHECK(icap.get_content() == "ok");
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::HasContent);
        RED_CHECK(icap.last_file_id() == ICAPFileId(5));
        RED_CHECK(icap.get_content() == "ko");
        RED_CHECK(icap.receive_response() == ICAPService::ResponseType::WaitingData);
    }
}
