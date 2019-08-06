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
#include "lib/files_validator_api.hpp"
#include "mod/file_validatior_service.hpp"

#include "utils/sugar/algostring.hpp"

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


RED_AUTO_TEST_CASE_WF(testFileValid, wf)
{
    auto data =
        "\x07"                  // invalid msg_type
        "\x00\x00\x00\x05"      // len
        "\x01"                  // flag
        "\x00\x00\x00\x01"      // max_connection_number
        "\x05\x00\x00\x00\x00"  // msg_type, len(ignored)
        "\x01"                  // result
        "\x00\x00\x00\x08"      // file id
        "\x00\x00\x00\x02"      // size
        "ok"_av;
    std::ofstream(wf.c_str()).write(data.data(), data.size());

    int fd = open(wf, O_RDWR);
    auto* validator = validator_open_fd_session(fd);
    RED_REQUIRE(validator);

    using R = FileValidatorService::ResponseType;

    RED_CHECK(validator_get_response_type(validator) == underlying_cast(R::WaitingData));

    RED_CHECK(validator_receive_response(validator) == underlying_cast(R::Error));

    RED_CHECK(validator_receive_response(validator) == underlying_cast(R::HasContent));
    RED_CHECK(validator_get_content(validator) == "ok");
    RED_CHECK(validator_get_response_type(validator) == underlying_cast(R::HasContent));
    RED_CHECK(validator_get_result_file_id(validator) == 8);
    RED_CHECK(validator_get_result_flag(validator) == 1);

    RED_CHECK(validator_receive_response(validator) == underlying_cast(R::WaitingData));

    RED_CHECK(validator_close_session(validator) == 0);

    RED_REQUIRE_MEM_FILE_CONTENTS(wf, str_concat(data, "\x02\x00\x00\x00\x04\x00\x00\x00\x00"_av));
}
