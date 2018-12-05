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

#define RED_TEST_MODULE TestRecordingProgress
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/recording_progress.hpp"
#include "test_only/get_file_contents.hpp"


RED_AUTO_TEST_CASE(TestRecordingProgress)
{
    std::string contents;

    time_t const start_time = 123456789;
    time_t const end_time = start_time + 100;
    char const * filename = "/tmp/test_progress_123.test";

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::JSON_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":0,"eta":-1,"videos":0})");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":10,"eta":0,"videos":0})");
//         {"percentage":0,"eta":-1,"videos":0}
//         {"percentage":10,"eta":0,"videos":0}

        p(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":90,"eta":0,"videos":0})");

        p(start_time + 100);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":99,"eta":0,"videos":0})");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, R"({"percentage":100,"eta":0,"videos":1})");

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::JSON_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":0,"eta":-1,"videos":0})");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":10,"eta":0,"videos":0})");

        p.next_video(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":90,"eta":0,"videos":1})");

        p(start_time + 100);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":99,"eta":0,"videos":1})");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, R"({"percentage":100,"eta":0,"videos":2})");

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::JSON_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":0,"eta":-1,"videos":0})");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":10,"eta":0,"videos":0})");

        p.next_video(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, R"({"percentage":90,"eta":0,"videos":1})");

        p.raise_error(2, "plouf");
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents,
            R"({"percentage":90,"eta":0,"videos":1,"error":{"code":2,"message":"plouf"}})");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, R"({"percentage":90,"eta":0,"videos":1,"error":{"code":2,"message":"plouf"}})");

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::OLD_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "0 -1");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "10 0");

        p(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "90 0");

        p(start_time + 100);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "99 0");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, "100 0");

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::OLD_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "0 -1");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "10 0");

        p.next_video(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "90 0");

        p(start_time + 100);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "99 0");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, "100 0");

    {
        unlink(filename);
        UpdateProgressData p(
            UpdateProgressData::OLD_FORMAT,
            filename,
            start_time,
            end_time,
            0, 0
        );

        RED_REQUIRE(p.is_valid());

        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "0 -1");

        p(start_time + 10);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "10 0");

        p.next_video(start_time + 90);
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "90 0");

        p.raise_error(2, "plouf");
        append_file_contents(contents, filename);
        RED_CHECK_EQUAL(contents, "-1 plouf (2)");
    }
    append_file_contents(contents, filename);
    RED_CHECK_EQUAL(contents, "-1 plouf (2)");

    unlink(filename);
}
