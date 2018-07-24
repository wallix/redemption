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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Clément Moroldo
*/

#include "utils/log.hpp"

#define RED_TEST_MODULE TestRDPMetrics
#include "system/redemption_unit_tests.hpp"

#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <linux/hdreg.h>

#include "utils/fileutils.hpp"
#include "utils/sugar/unique_fd.hpp"

#include "core/RDP/rdp_metrics.hpp"



RED_AUTO_TEST_CASE(TestRDPMetricsOutputFileTurnOver) {

    ClientInfo info;
    // tmp/rdp_metrics_file_test
    const char * templace_path_file = "tests/core/RDP/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
                      , 1
                      , "user"
                      , "admin"
                      , "10.10.13.12"
                      , info
                      , "RDP1");

    char current_date[24] {};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

    char complete_file_path[4096] = {'\0'};
    ::snprintf(complete_file_path, sizeof(complete_file_path), "%s-%s.log", templace_path_file, current_date);
    int fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd > 0);
    ::close(fd);
    remove(complete_file_path);

    time_t yesterday_time = metrics.utc_last_date - 3600*24;
    metrics.utc_last_date = yesterday_time;
    char yesterday_date[24] = {};
    metrics.set_current_formated_date(yesterday_date, false, yesterday_time);
    char yesterday_complete_path[4096] = {'\0'};
    ::snprintf(yesterday_complete_path, sizeof(yesterday_complete_path), "%s-%s.log", templace_path_file, yesterday_date);
    remove(yesterday_complete_path);

    metrics.log();
    metrics.log();

    fd = ::open(yesterday_complete_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd == -1);
    RED_CHECK(yesterday_time <= metrics.utc_last_date);

    fd = ::open(complete_file_path, O_RDONLY | O_APPEND);
    RED_CHECK(fd > 0);

    ::close(fd);
    remove(complete_file_path);
}

RED_AUTO_TEST_CASE(TestRDPMetricsOutputLogHeader) {

    ClientInfo info;
    const char * templace_path_file = "tests/core/RDP/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
                      , 1
                      , "user"
                      , "admin"
                      , "10.10.13.12"
                      , info
                      , "RDP1");

    char current_date[24] = {'\0'};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

    char complete_file_path[4096] = {'\0'};
    ::snprintf(complete_file_path, sizeof(complete_file_path), "%s-%s.log", templace_path_file, current_date);
    int fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd > 0);
    metrics.log();

    std::string expected_log("Session_starting_time=");
    expected_log += metrics.start_full_date_time;
    expected_log += " Session_id=1 user=8C6976E5B5410415BDE908BD4DEE15DFB167A9C873FC4BB8A81F6F2AB448A918 account=04F8996DA763B7A969B1028EE3007569EAF3A635486DDAB211D512C85B9DF8FB hostname=E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855 target_service=92161923FF7D1373CB331803671170CAC898037707ECA11F1D457EA47006A93C session_info=C1CBF9693611D913838B15A665E22B070228FF42402EB6553FA6EBF9F743CDF3 delta_time(s)=0";

    char log_read[512] = {'\0'};
    ::read(fd, log_read, expected_log.length());
    std::string str_log_read(log_read);
    RED_CHECK_EQUAL(str_log_read, expected_log);
    ::close(fd);
    remove(complete_file_path);
}


RED_AUTO_TEST_CASE(TestRDPMetricsOutputData) {

    ClientInfo info;
    const char * templace_path_file = "tests/core/RDP/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
                      , 1
                      , "user"
                      , "admin"
                      , "10.10.13.12"
                      , info
                      , "RDP1");

    char current_date[24] = {'\0'};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

    char complete_file_path[4096] = {'\0'};
    ::snprintf(complete_file_path, sizeof(complete_file_path), "%s-%s.log", templace_path_file, current_date);
    int fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd > 0);

    for (int i = 0; i < 33; i++) {
        metrics.current_data[i] = i+1;
    }
    metrics.log();

    std::string expected_log_header("Session_starting_time=");
    expected_log_header += metrics.start_full_date_time;
    expected_log_header += " Session_id=1 user=8C6976E5B5410415BDE908BD4DEE15DFB167A9C873FC4BB8A81F6F2AB448A918 account=04F8996DA763B7A969B1028EE3007569EAF3A635486DDAB211D512C85B9DF8FB hostname=E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855 target_service=92161923FF7D1373CB331803671170CAC898037707ECA11F1D457EA47006A93C session_info=C1CBF9693611D913838B15A665E22B070228FF42402EB6553FA6EBF9F743CDF3 delta_time(s)=0";
    RED_CHECK(fd > 0);

    std::string expected_log_data(" main_channel_data_from_client=1 right_click=2 left_click=3 keys_pressed=4 mouse_move=5 main_channel_data_from_serveur=6 cliprdr_channel_data_from_server=7 nb_text_paste_on_server=8 nb_image_paste_on_server=9 nb_file_paste_on_server=10 total_data_past_from_server11 nb_text_copy_on_server=12 nb_image_copy_on_server=13 nb_file_copy_on_server=14 cliprdr_channel_data_from_client=15 nb_text_paste_on_client=16 nb_image_paste_on_client=17 nb_file_paste_on_client=18 total_data_past_from_client19 nb_text_copy_on_client=20 nb_image_copy_on_client=21 nb_file_copy_on_client=22 rdpdr_channel_data_from_client=23 rdpdr_channel_data_from_server=24 nb_more_1k_byte_read_file=25 nb_deleted_file_or_folder=26 nb_write_file=27 nb_rename_file=28 nb_open_folder=29 rail_channel_data_from_client=30 rail_channel_data_from_server=31 other_channel_data_from_client=32 other_channel_data_from_server=33");

    char log_read[4096] = {'\0'};
    ::read(fd, log_read, expected_log_header.length()+expected_log_data.length()+1);
    std::string str_log_read(log_read);
    std::string str_log_data = str_log_read.substr(expected_log_header.length(), str_log_read.length());

    RED_CHECK_EQUAL(expected_log_data, str_log_data);
    ::close(fd);
    remove(complete_file_path);

    time_t yesterday_time = metrics.utc_last_date - 3600*24;
    metrics.utc_last_date = yesterday_time;
    for (int i = 0; i < 16; i++) {
        metrics.current_data[i] += i+1;
    }
    metrics.log();

    fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd > 0);

    std::string expected_log_data_2(" main_channel_data_from_client=2 right_click=4 left_click=6 keys_pressed=8 mouse_move=10 main_channel_data_from_serveur=12 cliprdr_channel_data_from_server=14 nb_text_paste_on_server=16 nb_image_paste_on_server=18 nb_file_paste_on_server=20 total_data_past_from_server22 nb_text_copy_on_server=24 nb_image_copy_on_server=26 nb_file_copy_on_server=28 cliprdr_channel_data_from_client=30 nb_text_paste_on_client=32");

    char log_read_2[2048] = {'\0'};
    ::read(fd, log_read_2, expected_log_header.length()+expected_log_data_2.length()+1);
    std::string str_log_read_2(log_read_2);
    std::string str_log_data_2 = str_log_read_2.substr(expected_log_header.length(), str_log_read_2.length());

    RED_CHECK_EQUAL(expected_log_data_2, str_log_data_2);
    ::close(fd);
    remove(complete_file_path);

    //LOG(LOG_INFO, "%s !!", str_log_data_2);


}


