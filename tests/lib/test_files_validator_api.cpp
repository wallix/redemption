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
#include "test_only/test_framework/file.hpp"
#include "lib/files_validator_api.hpp"
#include "utils/log.hpp"


#include "core/error.hpp"
#include "utils/netutils.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"

#include <fstream>


RED_AUTO_TEST_CASE(testFileValid)
{
    std::string socket_path("tools/ICAP_socket/redemption-icap-service-sock");

    ICAPService * service = validator_open_session(socket_path.c_str());
    bool mod_local_server = false;

    int n = -1;

    if (validator_session_is_open(service)) {
        mod_local_server = true;

        std::string file_name("README.md");
        std::string file_content(tu::get_file_contents("README.md"));
        int file_size = 30; /*file_content.length();*/

        int file_id = validator_open_file(service, file_name.c_str());
        RED_CHECK_EQUAL(file_id, 1);

        n = validator_send_data(service, file_id, file_content.c_str(), file_size);
        RED_CHECK(n>0);

        n = validator_end_of_file(service, file_id);
        RED_CHECK(n>0);

        validator_receive_result(service);
        RED_CHECK_EQUAL(validator_get_result(service), 0x00);
        std::string result_content(validator_get_content(service));
        RED_CHECK_EQUAL(result_content, "");
    }

    n = validator_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}



RED_AUTO_TEST_CASE(testFileInvalid)
{
    std::string socket_path("tools/ICAP_socket/redemption-icap-service-sock");

    ICAPService * service = validator_open_session(socket_path.c_str());

    int n = -1;

    bool mod_local_server = false;
    if (validator_session_is_open(service)) {
        mod_local_server = true;

        std::string file_path("../ICAPService/python/tests/the_zeus_binary_chapros");
        std::string file_name("the_zeus_binary_chapros");
        int file_size = 227328;

        int file_id = validator_open_file(service, file_name.c_str());
        RED_CHECK_EQUAL(file_id, 1);

        int sent_data = 0;
        char buff[1024] = {0};
        std::ifstream iFile(file_path, std::ios::binary | std::ios::app);

        if (iFile.is_open()) {
            while (sent_data < file_size) {

                iFile.read(buff, 1024);
                sent_data += 1024;

                n = validator_send_data(service, file_id, buff, 1024);
                RED_CHECK(n>0);
            }

            n = validator_end_of_file(service, file_id);
            RED_CHECK(n>0);
        }

        validator_receive_result(service);
        RED_CHECK_EQUAL(validator_get_result(service), 0x01);

        std::string expected_content =
        "\xaVIRUS FOUND\xa"
        "\xa"
        "\xa"
        "You tried to upload/download a file that contains the virus: \xa"
        "    Win.Trojan.Agent-1810289 \xa"
        "\xa"
        "The Http location is: \xa"
        "  http://127.0.1.1/the_zeus_binary_chapros \xa"
        "\xa"
        "\xa"
        "  For more information contact your system administrator\xa\xa";

        std::string result_content(validator_get_content(service));
        result_content = result_content.substr(0, expected_content.length());
        RED_CHECK_EQUAL(result_content, expected_content);

    }

    n = validator_close_session(service);

    if (mod_local_server) {
        RED_CHECK(n>0);
    }
}

