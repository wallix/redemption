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

RED_AUTO_TEST_CASE(testFileValid)
{
    std::string session_id("0000");
    std::string socket_path("../ICAPService/socket/redemption-icap-service-sock");
    std::string file_name("README.md");
    std::string file_content(get_file_contents("../ICAPService/README.md"));
    int file_size = file_content.length();


    ICAPService * service = icap_open_session(socket_path, session_id);
    int fd = service->fd.fd();
    if (service->fd.is_open()) {
        RED_CHECK(fd>0);
        std::string session_id_ = service->session_id;
        RED_CHECK_EQUAL(session_id_, "0000");


        std::string file_id = icap_open_file(service, file_name, file_size);
        RED_CHECK_EQUAL(file_id, "0000-1");


        icap_send_data(service, file_id, const_byte_ptr(file_content.c_str()), file_size);


        ICAPResult result = icap_get_result(service);
        RED_CHECK_EQUAL(result.res, 0x03);
        RED_CHECK_EQUAL(result.id, "0000-1");

        int n = icap_close_session(service);

        RED_CHECK(n>0);
    }
}


RED_AUTO_TEST_CASE(testFileInvalid)
{
    std::string session_id("0001");
    std::string socket_path("../ICAPService/socket/redemption-icap-service-sock");
    std::string file_path("../ICAPService/test/the_zeus_binary_chapros");
    std::string file_name("the_zeus_binary_chapros");
    int file_size = 227328;

    ICAPService * service = icap_open_session(socket_path, session_id);
    int fd = service->fd.fd();
    if (service->fd.is_open()) {
        RED_CHECK(fd>0);
        std::string session_id_ = service->session_id;
        RED_CHECK_EQUAL(session_id_, "0001");


        std::string file_id = icap_open_file(service, file_name, file_size);
        RED_CHECK_EQUAL(file_id, "0001-1");

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
        RED_CHECK_EQUAL(result.res, 0x04);
        RED_CHECK_EQUAL(result.id, "0001-1");

        int n = icap_close_session(service);

        RED_CHECK(n>0);
    }
}
