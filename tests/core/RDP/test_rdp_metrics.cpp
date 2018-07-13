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

    const char * templace_path_file = "tests/core/RDP/rdp_metrics_file_test";
    RDPMetrics metrics( templace_path_file
              , 1, "user", "10.10.13.12", "admin");

    std::string str_path(templace_path_file+ metrics.get_current_formated_date() + ".log");
    int fd = ::open(str_path.c_str(), O_RDWR | O_APPEND);
    RED_CHECK(fd > 0);

    remove(str_path.c_str());
    fd = ::open(str_path.c_str(), O_RDWR | O_APPEND);
    RED_CHECK(fd == -1);


    const char * new_date = "-2018-Jul-xx";
    std::string str_path_xx(templace_path_file);
    str_path_xx += new_date;
    str_path_xx += ".log";
    memcpy(metrics.last_date, new_date, 12);

    metrics.log();

    fd = ::open(str_path_xx.c_str(), O_RDWR | O_APPEND);
    RED_CHECK(fd == -1);

    fd = ::open(str_path.c_str(), O_RDWR | O_APPEND);
    RED_CHECK(fd > 0);

    remove(str_path.c_str());

}
