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
#include "mod/icap_files_service.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/log.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>



RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPService) {
    std::string socket_path("tools/ICAPService/fake_socket/redemption-icap-service-sock");
    std::string session_id("0000");

    ICAPService service(socket_path, session_id);
    std::string id_got = service.generate_id();
    RED_CHECK_EQUAL(id_got, "0000-1");
    RED_CHECK_EQUAL(service.fd.is_open(), false);
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_icap_open_session) {
    std::string socket_path("tools/ICAPService/fake_socket/redemption-icap-service-sock");
    std::string session_id("0001");


    ICAPService * service = icap_open_session(socket_path, session_id);
    std::string id_got = service->generate_id();
    RED_CHECK_EQUAL(id_got, "0001-1");
    RED_CHECK_EQUAL(service->fd.is_open(), false);

    int n = icap_close_session(service);
    RED_CHECK_EQUAL(n, -1);
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPHeader) {
    const uint8_t msg_type = ICAPService::NEW_FILE_FLAG;
    const uint32_t msg_len = 12;

    StaticOutStream<10> message;

    ICAPHeader header(msg_type, msg_len);
    header.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x00\x0c");

    RED_CHECK_EQUAL(message.get_offset(), 5);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPNewFile) {

    const std::string file_id = "0000-1";
    const std::string file_name = "file_name.file";
    const size_t file_size = 5658;

    StaticOutStream<512> message;

    ICAPNewFile icap_new_file(file_id, file_name, file_size);
    icap_new_file.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x06\x30\x30\x30\x30\x2d\x31\x00\x00\x00\x0e\x66\x69" //....0000-1....fi !
        "\x6c\x65\x5f\x6e\x61\x6d\x65\x2e\x66\x69\x6c\x65\x00\x00\x16\x1a" //le_name.file.... !]
    );

    RED_CHECK_EQUAL(message.get_offset(), 32);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPFileDataHeader) {
    const std::string file_id = "0000-1";

    StaticOutStream<512> message;

    ICAPFileDataHeader icap_file_data(file_id);
    icap_file_data.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x06\x30\x30\x30\x30\x2d\x31"
    );

    RED_CHECK_EQUAL(message.get_offset(), 10);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPResult) {

    const std::string expected_file_id = "0000-1";
    const uint8_t expected_result = ICAPService::ACCEPTED_FLAG;

    auto data = cstr_array_view(
        "\x00\x00\x00\x00\x06\x30\x30\x30\x30\x2d\x31"
    );

    InStream stream(data);

    ICAPResult icap_result;
    icap_result.receive(stream);

    RED_CHECK_EQUAL(icap_result.id, expected_file_id);
    RED_CHECK_EQUAL(icap_result.result,  expected_result);
}




RED_AUTO_TEST_CASE(testFileValid)
{
    std::string session_id("0002");
    std::string socket_path("tools/ICAPService/socket/redemption-icap-service-sock");

    ICAPService * service = icap_open_session(socket_path, session_id);
    bool mod_local_server = false;

    if (service->fd.is_open()) {
        mod_local_server = true;

        std::string file_name("README.md");
        std::string file_content(get_file_contents("README.md"));
        int file_size = file_content.length();

        std::string session_id_ = service->session_id;
        RED_CHECK_EQUAL(session_id_, "0002");

        std::string file_id = icap_open_file(service, file_name, file_size);
        RED_CHECK_EQUAL(file_id, "0002-1");

        icap_send_data(service, file_id, const_byte_ptr(file_content.c_str()), file_size);

        ICAPResult result = icap_get_result(service);
        RED_CHECK_EQUAL(result.result, 0x00);
        RED_CHECK_EQUAL(result.id, "0002-1");
    }

    int n = icap_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}



RED_AUTO_TEST_CASE(testFileInvalid)
{
    std::string session_id("0003");
    std::string socket_path("tools/ICAPService/socket/redemption-icap-service-sock");

    ICAPService * service = icap_open_session(socket_path, session_id);

    bool mod_local_server = false;
    if (service->fd.is_open()) {
        mod_local_server = true;

        std::string file_path("../ICAPService/test/the_zeus_binary_chapros");
        std::string file_name("the_zeus_binary_chapros");
        int file_size = 227328;

        std::string session_id_ = service->session_id;
        RED_CHECK_EQUAL(session_id_, "0003");

        std::string file_id = icap_open_file(service, file_name, file_size);
        RED_CHECK_EQUAL(file_id, "0003-1");

        int sent_data = 0;
        char buff[1024] = {0};
        std::ifstream iFile(file_path, std::ios::binary | std::ios::app);

        if (iFile.is_open()) {
            while (sent_data < file_size) {

                iFile.read(buff, 1024);
                sent_data += 1024;

                icap_send_data(service, file_id, buff, 1024);
            }
        }

        ICAPResult result = icap_get_result(service);
        RED_CHECK_EQUAL(result.result, 0x01);
        RED_CHECK_EQUAL(result.id, "0003-1");
    }

    int n = icap_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}

