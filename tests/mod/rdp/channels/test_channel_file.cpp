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
*1324
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include "mod/rdp/channels/channel_file.hpp"



RED_AUTO_TEST_CASE(TestChannelFileCtr)
{
    WorkingDirectory wd("channel_file");

    ChannelFile file(wd.dirname());

    uint32_t streamID = 0x00;
    uint8_t direction = ChannelFile::NONE;

    timeval sec_and_usec_time;
    sec_and_usec_time.tv_sec = 12345;
    sec_and_usec_time.tv_usec = 54321;

    auto const file1 = wd.add_file("12345_54321_new_file1.txt");
    file.new_file("new_file1.txt", 0, streamID, direction, sec_and_usec_time);

    auto const file2 = wd.add_file("12345_54321_new_file2.txt");
    file.new_file("new_file2.txt", 0, streamID, direction, sec_and_usec_time);

    RED_CHECK_WORKSPACE(wd);
}



RED_AUTO_TEST_CASE(TestChannelFileWrite)
{
    WorkingDirectory wd("channel_file");

    ChannelFile file(wd.dirname());

    auto const file1 = wd.add_file("12345_54321_new_file1.txt");

    const char * word1 = "hello ";
    const char * word2 = "world!";
    const char * word3 = "again";
    const char * word4 = "goodbye ";

    uint32_t streamID = 0x00;
    uint8_t direction = ChannelFile::NONE;

    timeval sec_and_usec_time;
    sec_and_usec_time.tv_sec = 12345;
    sec_and_usec_time.tv_usec = 54321;

    file.new_file("new_file1.txt", 12, streamID, direction, sec_and_usec_time);
    RED_CHECK_WORKSPACE(wd);
    RED_CHECK_EQUAL(file.is_complete(), false);


    file.set_data(reinterpret_cast<const uint8_t *>(word1), 6);
    RED_CHECK_EQUAL(file.is_complete(), false);
    file.set_data(reinterpret_cast<const uint8_t *>(word2), 6);
    RED_CHECK_EQUAL(file.is_complete(), true);
    file.set_data(reinterpret_cast<const uint8_t *>(word3), 5);
    RED_CHECK_EQUAL(file.is_complete(), true);

    RED_CHECK_EQUAL(get_file_contents(file1), "hello world!");

    auto const file2 = wd.add_file("12345_54321_new_file2.txt");
    file.new_file("new_file2.txt", 14, streamID, direction, sec_and_usec_time);
    RED_CHECK_WORKSPACE(wd);
    RED_CHECK_EQUAL(file.is_complete(), false);

    file.set_data(reinterpret_cast<const uint8_t *>(word4), 8);
    RED_CHECK_EQUAL(file.is_complete(), false);
    file.set_data(reinterpret_cast<const uint8_t *>(word2), 6);
    RED_CHECK_EQUAL(file.is_complete(), true);
    file.set_data(reinterpret_cast<const uint8_t *>(word3), 5);
    RED_CHECK_EQUAL(file.is_complete(), true);

    RED_CHECK_EQUAL(get_file_contents(file1), "hello world!");

    RED_CHECK_EQUAL(get_file_contents(file2), "goodbye world!");

    uint8_t buffer[15];
    file.read_data(buffer, 14);
    buffer[14] = 0;

    std::string file_read_data(reinterpret_cast<const char *>(buffer));
    std::string expected_data("goodbye world!");

    RED_CHECK_EQUAL(file_read_data, expected_data);

    RED_CHECK_WORKSPACE(wd);

}
