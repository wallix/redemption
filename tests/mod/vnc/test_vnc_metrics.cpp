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
*   Product name: redemption, a FLOSS VNC proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#define RED_TEST_MODULE TestVNCMetrics
#include "system/redemption_unit_tests.hpp"

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include "mod/vnc/vnc_metrics.hpp"


RED_AUTO_TEST_CASE(TestVNCMetricsH)
{
    std::array<uint8_t, 32> key{0};

    RED_CHECK_MEM("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"_av,
        hmac_user("primaryuser"_av, key));

    RED_CHECK_MEM("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"_av,
        hmac_account("secondaryuser"_av, key));

    RED_CHECK_MEM("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"_av,
        hmac_device_service("device1"_av, "service1", key));

    ClientInfo info;
    RED_CHECK_MEM("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"_av,
        hmac_client_info("10.10.13.12", info, key));
}


using namespace std::literals::chrono_literals;

RED_AUTO_TEST_CASE(TestVNCMetricsLogCycle1)
{
    WorkingDirectory wd("metrics_log_cycle1");
    auto logmetrics1 = wd.add_file("vnc_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("vnc_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;

    VNCMetrics metrics( true
                      , wd.dirname().c_str()
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"_av
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"_av
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"_av
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"_av
                      , epoch
                      , 24h
                      , 5s
                      );

    RED_CHECK_FILE_EXISTS(wd[logmetrics1]);
    RED_CHECK_FILE_EXISTS(wd[logindex1]);

    std::string expected_log_index("2018-08-02 12:08:01 connection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
    RED_CHECK_EQUAL(get_file_contents(wd[logindex1]), expected_log_index);

    {
        metrics.right_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
    }
    {
        metrics.log(to_timeval(epoch+1s));
        metrics.right_click_pressed();
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
    }
    {
        metrics.log(to_timeval(epoch+3s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 2 0\n");
        metrics.log(to_timeval(epoch+5s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);;
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 2 0\n");
        metrics.log(to_timeval(epoch+7s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 2 0\n2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 2 0\n");
        metrics.log(to_timeval(epoch+10s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestVNCMetricsLogCycle2)
{
    WorkingDirectory wd("metrics_log_cycle2");
    auto logmetrics1 = wd.add_file("vnc_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("vnc_metrics-v1.0-2018-08-02.logindex");

    {
        constexpr auto epoch = 1533211681s;

        VNCMetrics metrics( true
                        , wd.dirname().c_str()
                        , "164d89c1a56957b752540093e178"
                        , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"_av
                        , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"_av
                        , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"_av
                        , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"_av
                        , epoch
                        , 24h
                        , 3s
                        );

        RED_CHECK_FILE_EXISTS(wd[logindex1]);
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");

        {
            metrics.right_click_pressed();
            metrics.log(to_timeval(epoch+0s));
            RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
        }
        {
            metrics.log(to_timeval(epoch+1s));
            RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
        }
        {
            metrics.log(to_timeval(epoch+2s));
            RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), "");
        }
        {
            std::string expected_log_metrics("2018-08-02 12:08:04 164d89c1a56957b752540093e178 0 0 1 0\n");
            metrics.log(to_timeval(epoch+3s));
            RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
        }
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestVNCMetricsLogBasicIncrement)
{
    WorkingDirectory wd("metrics_log_basic_inc");
    auto logmetrics1 = wd.add_file("vnc_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("vnc_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;

    VNCMetrics metrics( true
                      , wd.dirname().c_str()
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"_av
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"_av
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"_av
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"_av
                      , epoch
                      , 24h
                      , 5s
                      );

    RED_CHECK_FILE_EXISTS(wd[logindex1]);

    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 1 0\n");

    {
        epoch += 5s;
        metrics.right_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 2 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.right_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 2 1\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.left_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 1 2 1\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.key_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 4 1 2 1\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.mouse_mouve(0, 0);
        metrics.mouse_mouve(2, 2);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}



