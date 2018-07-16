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
    const char * templace_path_file = "/tmp/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
              , 1, "user", info, "10.10.13.12", "admin", 0);

    char current_date[24] {};
    timeval now = tvtime();
    metrics.set_current_formated_date(current_date, false, now.tv_sec);

    char complete_file_path[4096] {};
    ::snprintf(complete_file_path, sizeof(complete_file_path), "%s%s.log", templace_path_file, current_date);
    RED_CHECK(unique_fd(complete_file_path, O_RDWR | O_APPEND).is_open());
    remove(complete_file_path);

    RED_CHECK(!unique_fd(complete_file_path, O_RDWR | O_APPEND).is_open());

    time_t last_date_save = metrics.last_date;
    metrics.last_date -= 3600*24;
    metrics.log();
    RED_CHECK(last_date_save <= metrics.last_date);

    RED_CHECK(unique_fd(complete_file_path, O_RDWR | O_APPEND).is_open());
    remove(complete_file_path);
}


RED_AUTO_TEST_CASE(TestRDPMetricsOutputLogHeader) {

}
