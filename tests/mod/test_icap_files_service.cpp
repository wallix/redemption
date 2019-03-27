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



RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPService) {
    std::string socket_fake_path("tools/ICAPService/fake_socket/redemption-icap-service-sock");

    ICAPService service(socket_fake_path);
    int id_got = service.generate_id();
    RED_CHECK_EQUAL(id_got, 1);
    RED_CHECK_EQUAL(service.fd.is_open(), false);
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_icap_open_session) {
    std::string socket_fake_path("tools/ICAPService/fake_socket/redemption-icap-service-sock");

    ICAPService * service = icap_open_session(socket_fake_path);
    int id_got = service->generate_id();
    RED_CHECK_EQUAL(id_got, 1);
    RED_CHECK_EQUAL(service->fd.is_open(), false);

    int n = icap_close_session(service);
    RED_CHECK_EQUAL(n, -1);
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPHeader) {
    const uint8_t msg_type = LocalICAPServiceProtocol::NEW_FILE_FLAG;
    const uint32_t msg_len = 12;

    StaticOutStream<10> message;

    LocalICAPServiceProtocol::ICAPHeader header(msg_type, msg_len);
    header.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x00\x0c");

    RED_CHECK_EQUAL(message.get_offset(), 5);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPNewFile) {

    const int file_id = 1;
    const std::string file_name = "file_name.file";

    StaticOutStream<512> message;

    LocalICAPServiceProtocol::ICAPNewFile icap_new_file(file_id, file_name);
    icap_new_file.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x01\x00\x00\x00\x0e\x66\x69\x6c\x65\x5f\x6e\x61\x6d" //........file_nam
        "\x65\x2e\x66\x69\x6c\x65"                                         //e.file
    );

    RED_CHECK_EQUAL(message.get_offset(), 22);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPFileDataHeader) {

    const int file_id = 1;

    StaticOutStream<512> message;

    LocalICAPServiceProtocol::ICAPFileDataHeader icap_file_data(file_id);
    icap_file_data.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x01"                         //....
    );

    RED_CHECK_EQUAL(message.get_offset(), 4);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPEndOfFile) {

    const int file_id = 1;

    StaticOutStream<16> message;

    LocalICAPServiceProtocol::ICAPEndOfFile icap_end_of_file(file_id);
    icap_end_of_file.emit(message);

    auto exp_data = cstr_array_view(
        "\x00\x00\x00\x01"                         //....
    );

    RED_CHECK_EQUAL(message.get_offset(), 4);
    RED_CHECK_MEM(exp_data, message.get_bytes());
}

RED_AUTO_TEST_CASE(TestICAPLocalProtocol_ICAPResult) {

    const uint8_t expected_result = LocalICAPServiceProtocol::ACCEPTED_FLAG;
    const int expected_file_id = 1;
    const std::string expected_content = "eg";

    auto data = cstr_array_view(
        "\x00\x00\x00\x00\x01\x00\x00\x00\x02\x65\x67"                     //.....
    );

    InStream stream(data);

    LocalICAPServiceProtocol::ICAPResult icap_result;
    icap_result.receive(stream);

    RED_CHECK_EQUAL(icap_result.id, expected_file_id);
    RED_CHECK_EQUAL(icap_result.result, expected_result);
    RED_CHECK_EQUAL(icap_result.content, expected_content);
}



RED_AUTO_TEST_CASE(testFileValid)
{
    std::string socket_path("tools/ICAP_socket/redemption-icap-service-sock");

    ICAPService * service = icap_open_session(socket_path);
    bool mod_local_server = false;

    int n = -1;

    if (service->fd.is_open()) {
        mod_local_server = true;

        std::string file_name("README.md");
        std::string file_content(get_file_contents("README.md"));
        int file_size = 30; /*file_content.length();*/

        int file_id = icap_open_file(service, file_name);
        RED_CHECK_EQUAL(file_id, 1);

        n = icap_send_data(service, file_id, file_content.c_str(), file_size);
        RED_CHECK(n>0);

        n = icap_end_of_file(service, file_id);
        RED_CHECK(n>0);

        int result = icap_get_result(service);
        RED_CHECK_EQUAL(result, LocalICAPServiceProtocol::ACCEPTED_FLAG);
    }

    n = icap_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}



RED_AUTO_TEST_CASE(testFileInvalid)
{
    std::string socket_path("tools/ICAP_socket/redemption-icap-service-sock");

    ICAPService * service = icap_open_session(socket_path);

    int n = -1;

    bool mod_local_server = false;
    if (service->fd.is_open()) {
        mod_local_server = true;

        std::string file_path("../ICAPService/python/tests/the_zeus_binary_chapros");
        std::string file_name("the_zeus_binary_chapros");
        int file_size = 227328;

        int file_id = icap_open_file(service, file_name);
        RED_CHECK_EQUAL(file_id, 1);

        int sent_data = 0;
        char buff[1024] = {0};
        std::ifstream iFile(file_path, std::ios::binary | std::ios::app);

        if (iFile.is_open()) {
            while (sent_data < file_size) {

                iFile.read(buff, 1024);
                sent_data += 1024;

                n = icap_send_data(service, file_id, buff, 1024);
                RED_CHECK(n>0);
            }

            n = icap_end_of_file(service, file_id);
            RED_CHECK(n>0);
        }

        int result = icap_get_result(service);
        RED_CHECK_EQUAL(result, LocalICAPServiceProtocol::REJECTED_FLAG);
    }

    n = icap_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}

