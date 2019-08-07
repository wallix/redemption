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

#include "mod/file_validatior_service.hpp"
#include "utils/sugar/algostring.hpp"


RED_AUTO_TEST_CASE(file_validatorSend)
{
    BufTransport trans;
    FileValidatorService file_validator{trans};

    auto filename = FIXTURES_PATH "/test_infile.txt";
    auto content = RED_REQUIRE_GET_FILE_CONTENTS(filename);

    FileValidatorId id = file_validator.open_file(filename, "clamav");
    RED_CHECK(id == FileValidatorId(1));

    file_validator.send_data(id, content);
    file_validator.send_eof(id);

    const auto data_ref =
        "\x00\x00\x00\x00\x3b\x00\x00\x00\x01\x00\x00\x00\x29\x2e\x2f\x74" //....;.......)./t
        "\x65\x73\x74\x73\x2f\x69\x6e\x63\x6c\x75\x64\x65\x73\x2f\x66\x69" //ests/includes/fi
        "\x78\x74\x75\x72\x65\x73\x2f\x74\x65\x73\x74\x5f\x69\x6e\x66\x69" //xtures/test_infi
        "\x6c\x65\x2e\x74\x78\x74\x00\x00\x00\x06\x63\x6c\x61\x6d\x61\x76" //le.txt....clamav
        "\x01\x00\x00\x00\x1c\x00\x00\x00\x01\x57\x65\x20\x72\x65\x61\x64" //.........We read
        "\x20\x77\x68\x61\x74\x20\x77\x65\x20\x70\x72\x6f\x76\x69\x64\x65" // what we provide
        "\x21\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av;
    RED_CHECK_MEM(trans.data(), data_ref);

    RED_CHECK(file_validator.open_file(filename, "clamav") == FileValidatorId(2));
}

RED_AUTO_TEST_CASE(file_validatorSendInfos)
{
    BufTransport trans;
    FileValidatorService file_validator{trans};

    file_validator.send_infos({
        "key1"_av, "value1"_av,
        "key"_av, "v2"_av,
        "k3"_av, "value"_av,
    });

    const auto data_ref =
        "\x06\x00\x00\x00\x24"
        "\x00\x03"
        "\x00\x04key1" "\x00\x06value1"
        "\x00\x03key"  "\x00\x02v2"
        "\x00\x02k3"   "\x00\x05value"
        ""_av;
    RED_CHECK_MEM(trans.data(), data_ref);
}

RED_AUTO_TEST_CASE(file_validatorReceive)
{
    BufTransport trans;
    FileValidatorService file_validator{trans};

    auto setbuf = [&](cbytes_view data){
        trans.buf.assign(data.as_charp(), data.size());
    };

    RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);

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
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
        setbuf(response.from_at(pos));
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
        setbuf("o"_av);
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
        setbuf("k"_av);
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::HasContent);
        RED_CHECK(file_validator.last_file_id() == FileValidatorId(1));
        RED_CHECK(file_validator.get_content() == "ok");
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
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
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::HasContent);
        RED_CHECK(file_validator.last_file_id() == FileValidatorId(4));
        RED_CHECK(file_validator.get_content() == "ko");
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
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
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::HasContent);
        RED_CHECK(file_validator.last_file_id() == FileValidatorId(3));
        RED_CHECK(file_validator.get_content() == "ok");
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::HasContent);
        RED_CHECK(file_validator.last_file_id() == FileValidatorId(5));
        RED_CHECK(file_validator.get_content() == "ko");
        RED_CHECK(file_validator.receive_response() == FileValidatorService::ResponseType::WaitingData);
    }
}
