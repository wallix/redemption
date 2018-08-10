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

#include "utils/log.hpp"

#define RED_TEST_MODULE TestRDPMetrics
#include "system/redemption_unit_tests.hpp"

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"

#include "core/RDP/rdp_metrics.hpp"


RED_AUTO_TEST_CASE(TestRDPMetricsH)
{
    uint8_t key[32] = {0};

    RED_CHECK_EQUAL(std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"),
    hmac_user("primaryuser", key));

    RED_CHECK_EQUAL(std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"),
    hmac_account("secondaryuser", key));

    RED_CHECK_EQUAL(std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"),
    hmac_device_service("device1", "service1", key));

    ClientInfo info;
    RED_CHECK_EQUAL(std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"),
    hmac_client_info("10.10.13.12", info, key));

}


constexpr const char * rdp_metrics_path_file = "/tmp";

RED_AUTO_TEST_CASE(TestRDPMetricsConstructor)
{
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");

    // Should create rdp_metrics files if they do not exist
    time_t epoch = 1533211681; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));

    metrics_rotate(epoch + (3600*1), metrics.metrics);
    metrics_rotate(epoch + (3600*2), metrics.metrics);
    metrics_rotate(epoch + (3600*3), metrics.metrics);
    metrics_rotate(epoch + (3600*4), metrics.metrics);
    metrics_rotate(epoch + (3600*5), metrics.metrics);
    metrics_rotate(epoch + (3600*6), metrics.metrics);

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));

    metrics_rotate(epoch + (3600*24), metrics.metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");

}
/*
RED_AUTO_TEST_CASE(TestRDPMetricsConstructorHoursRotation)
{
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");

    // Should create rdp_metrics files if they do not exist
    time_t epoch = 0; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{7}
                      , std::chrono::seconds{5}
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logindex"));

    metrics_rotate(epoch + (3600*1), metrics.metrics);
    metrics_rotate(epoch + (3600*2), metrics.metrics);
    metrics_rotate(epoch + (3600*3), metrics.metrics);
    metrics_rotate(epoch + (3600*4), metrics.metrics);
    metrics_rotate(epoch + (3600*5), metrics.metrics);
    metrics_rotate(epoch + (3600*6), metrics.metrics);
    metrics_rotate(epoch + (3600*6)+3599, metrics.metrics);

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics_rotate(epoch + (3600*7), metrics.metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics_rotate(epoch + (24*3600*3), metrics.metrics);

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex"));

    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle1) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));

    std::string expected_log_index("2018-08-02 12:08:01 connection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));

    {
        metrics.right_click_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        timeval now = {epoch+1, 0};
        metrics.log(now);

        metrics.right_click_pressed();

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        timeval now = {epoch+3, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        timeval now = {epoch+5, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        timeval now = {epoch+7, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        timeval now = {epoch+10, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle2) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    std::string expected_log_index("2018-08-02 12:08:01 connection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    {

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{3}
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));



    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));

    {
        metrics.right_click_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        timeval now = {epoch+1, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        timeval now = {epoch+2, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), std::string(""));
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:04 164d89c1a56957b752540093e178 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        timeval now = {epoch+3, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    }

    }

    std::string expected_disconnected_index("2018-08-02 12:08:07 disconnection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index+expected_disconnected_index);

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogBasicIncrement) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogBasicIncrement");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {
        epoch += 5;
        metrics.right_click_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.right_click_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 3 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_main_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 3 2 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.left_click_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 3 2 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.key_pressed();

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:31 164d89c1a56957b752540093e178 3 2 1 1 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.mouse_mouve(0, 0);
        metrics.mouse_mouve(2, 2);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:36 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_main_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:41 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_rail_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:46 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_rail_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:51 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_other_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
    {
        epoch += 5;
        std::string expected_log_metrics_next("2018-08-02 12:08:56 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 3\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_other_channel_data(3);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerImageCopy_PasteOnClient) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIServerImageCopy_PasteOnClient");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics;

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_METAFILEPICT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 0 1 0 12 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_METAFILEPICT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 64 0 0 0 0 0 1 0 12 0 1 0 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerFileCopy_PasteOnClient) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIServerFileCopy_PasteOnClient");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics;

    {  // FORMAT LIST FILE COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT FILE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 12 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(49562);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FILE CONTENT REQUEST FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 44 0 0 1 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsRequestPDU format(0, RDPECLIP::FILECONTENTS_RANGE, 0, 42);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }
}



RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerTextCopy_PasteOnClient) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIServerTextCopy_PasteOnClient");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics;

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_UNICODETEXT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 1 0 0 12 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_OEMTEXT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 64 0 0 0 0 1 0 0 12 1 0 0 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}



RED_AUTO_TEST_CASE(TestRDPMetricsRDPDRReadChunk) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsRDPDRReadChunk");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 0 0 0 0 0 0 0 0 0 0 0\n");

     { // CLIENT PDU
        epoch += 5;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
        sharedHeader.emit(out_stream);

        rdpdr::ClientAnnounceReply clientAnnounceReply( 1, 1, 1);
        clientAnnounceReply.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // READ FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 56 1 0 0 0 42 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_READ, 0);
        dior.emit(out_stream);

        rdpdr::DeviceReadRequest drr(42, 0);
        drr.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // WRITE FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 154 1 0 1 0 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_WRITE, 0);
        dior.emit(out_stream);
        const uint8_t data[42] = { 0 };
        rdpdr::DeviceWriteRequest dwr(42, 0, data);
        dwr.emit(out_stream);

        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // RENAME FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 210 1 0 1 1 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_SET_INFORMATION, 0);
        dior.emit(out_stream);

        rdpdr::ServerDriveSetInformationRequest sdsir(rdpdr::FileRenameInformation, 0);
        sdsir.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // DELETE FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 266 1 1 1 1 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_SET_INFORMATION, 0);
        dior.emit(out_stream);

        rdpdr::ServerDriveSetInformationRequest sdsir(rdpdr::FileDispositionInformation, 0);
        sdsir.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientImageCopy_PasteOnServer) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIClientImageCopy_PasteOnServer");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    { // FORMAT LIST INITIALISATION
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT LIST IMAGE COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 68 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_METAFILEPICT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT IMAGE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 1 0 0 0 0 0 68 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_METAFILEPICT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT IMAGE DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 1 0 42 0 0 0 118 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientFileCopy_PasteOnServer) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIClientTextCopy_PasteOnServer");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {  // FORMAT LIST INITIALISATION
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT LIST FILE COPY ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT FILE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 0 1 0 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(49562);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FILE CONTENT REQUEST FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 44 0 0 1 42 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsRequestPDU format(0, RDPECLIP::FILECONTENTS_RANGE, 0, 42);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientTextCopy_PasteOnServer) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    LOG(LOG_INFO, "\n\nTestRDPMetricsLogCLIPRDRIClientTextCopy_PasteOnServer");

    time_t epoch = 1533211681;
    RDPMetrics metrics( true
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , std::string("51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58")
                      , std::string("1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31")
                      , std::string("EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48")
                      , std::string("B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7")
                      , epoch
                      , std::chrono::hours{24}
                      , std::chrono::seconds{5}
                      , true
                      );

   std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {  // FORMAT LIST INITIALISATION
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 68 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_UNICODETEXT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 1 0 0 0 0 0 0 68 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_OEMTEXT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 1 0 0 42 0 0 0 118 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        timeval now = {epoch, 0};
        metrics.log(now);

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}*/
