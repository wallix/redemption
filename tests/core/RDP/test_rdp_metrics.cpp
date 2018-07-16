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

#include "core/RDP/rdp_metrics.hpp"



RED_AUTO_TEST_CASE(TestRDPMetricsOutputFileTurnOver) {

    ClientInfo info;
    const char * templace_path_file = "tests/core/RDP/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
                      , 1
                      , "user"
                      , "admin"
                      , "10.10.13.12"
                      , info
                      , 0
                      , "RDP1");

    char current_date[24] = {'\0'};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

    char complete_file_path[4096] = {'\0'};
    ::snprintf(complete_file_path, sizeof(complete_file_path), "%s-%s.log", templace_path_file, current_date);
    int fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
    RED_CHECK(fd > 0);

    remove(complete_file_path);
    fd = ::open(complete_file_path, O_RDONLY | O_APPEND);
    RED_CHECK(fd == -1);

    time_t last_date_save = metrics.last_date;
    metrics.last_date -= 3600*24;
    metrics.log();
    metrics.log();
    RED_CHECK(last_date_save <= metrics.last_date);

    fd = ::open(complete_file_path, O_RDONLY | O_APPEND);
    RED_CHECK(fd > 0);
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
                      , 0
                      , "RDP1");

    char current_date[24] = {'\0'};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

//     char complete_file_path[4096] = {'\0'};
//     ::snprintf(complete_file_path, sizeof(complete_file_path), "%s-%s.log", templace_path_file, current_date);
//     int fd = ::open(complete_file_path, O_RDONLY| O_APPEND);
//     RED_CHECK(fd > 0);
//     metrics.log();
//
//     std::string expected_log("Session_starting_time=");
//     expected_log += metrics.start_full_date_time;
//     expected_log += " delta_time(s)=0 Session_id=1 user=D033E22AE348AEB5660 account=12DEA96FEC20593566A hostname=DA39A3EE5E6B4B0D325 target_service=EE5D8A196324C9649DC session_info=1709919F0A4B52AE2A7";
//
//     char log_read[512] = {'\0'};
//     ::read(fd, log_read, expected_log.length());
//     std::string str_log_read(log_read);
//     RED_CHECK_EQUAL(str_log_read, expected_log);
//     remove(complete_file_path);
}


RED_AUTO_TEST_CASE(TestRDPMetricsOutputData) {



}


