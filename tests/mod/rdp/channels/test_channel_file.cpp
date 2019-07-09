/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/file.hpp"

#include "test_only/test_framework/working_directory.hpp"

#include "mod/rdp/channels/channel_file.hpp"


RED_AUTO_TEST_CASE_WD(TestChannelFileWrite, wd)
{
    ValidatorParams params;
    params.save_files_directory = wd.dirname().string();
    params.enable_interrupting = false;
    params.enable_save_files = true;
    params.target_name = "avscan";
    ChannelFile file(nullptr, params);

    auto const file1 = wd.add_file("12345_54321_new_file1.txt");

    auto word1 = "hello "_av;
    auto word2 = "world!"_av;
    auto word3 = "again"_av;
    auto word4 = "goodbye "_av;

    uint8_t direction = ChannelFile::NONE;

    timeval sec_and_usec_time;
    sec_and_usec_time.tv_sec = 12345;
    sec_and_usec_time.tv_usec = 54321;

    file.new_file("new_file1.txt", 12, direction, sec_and_usec_time);
    RED_CHECK_WORKSPACE(wd);
    RED_CHECK(!file.is_complete());

    file.set_data(word1);
    RED_CHECK(!file.is_complete());
    file.set_data(word2);
    RED_CHECK(file.is_complete());
    file.set_data(word3);
    RED_CHECK(file.is_complete());

    RED_CHECK_FILE_CONTENTS(file1, "hello world!"_av);

    auto const file2 = wd.add_file("12345_54321_new_file2.txt");
    file.new_file("new_file2.txt", 14, direction, sec_and_usec_time);
    RED_CHECK_WORKSPACE(wd);

    RED_CHECK(!file.is_complete());

    file.set_data(word4);
    RED_CHECK(!file.is_complete());
    file.set_data(word2);
    RED_CHECK(file.is_complete());
    file.set_data(word3);
    RED_CHECK(file.is_complete());

    RED_CHECK_FILE_CONTENTS(file1, "hello world!"_av);

    RED_CHECK_FILE_CONTENTS(file2, "goodbye world!"_av);

    uint8_t buffer[14];
    file.read_data(buffer, 14);

    RED_CHECK_SMEM(array_view(buffer, 14), "goodbye world!"_av);
}
