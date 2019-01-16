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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Clément Moroldo

*/

#define RED_TEST_MODULE Testiametrics
#include "test_only/test_framework/redemption_unit_tests.hpp"
#include <chrono>

using namespace std::chrono_literals;

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include "lib/iametrics.hpp"


constexpr const char * fields_rdp_metrics_version = "v1.0";
constexpr const char * protocol_name = "rdp";

RED_AUTO_TEST_CASE(TestRDPMetricsConstructor)
{
    WorkingDirectory wd("metrics_ctor");

    std::chrono::seconds epoch = 1533211681s; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1

    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, 34
                      , wd.dirname().c_str()
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                      , epoch.count()
                      , 24
                      , 5
                      );

    RED_CHECK_WORKSPACE(wd.add_files({
        "rdp_metrics-v1.0-2018-08-02.logmetrics",
        "rdp_metrics-v1.0-2018-08-02.logindex"}));

    metrics_log(metrics, std::chrono::milliseconds(epoch + 1h).count());
    metrics_log(metrics, std::chrono::milliseconds(epoch + 2h).count());
    metrics_log(metrics, std::chrono::milliseconds(epoch + 3h).count());
    metrics_log(metrics, std::chrono::milliseconds(epoch + 4h).count());
    metrics_log(metrics, std::chrono::milliseconds(epoch + 5h).count());
    metrics_log(metrics, std::chrono::milliseconds(epoch + 6h).count());

    RED_CHECK_WORKSPACE(wd);

    metrics_log(metrics, std::chrono::milliseconds(epoch + 26h).count());

    RED_CHECK_WORKSPACE(wd.add_files({
        "rdp_metrics-v1.0-2018-08-03.logmetrics",
        "rdp_metrics-v1.0-2018-08-03.logindex"}));

    metrics_delete(metrics);
}

RED_AUTO_TEST_CASE(TestRDPMetricsConstructorHoursRotation)
{
    WorkingDirectory wd("metrics_hours_rotation");

    // Should create rdp_metrics files if they do not exist
    time_t epoch = 0; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, 34
                      , wd.dirname().c_str()
                      , "164d89c1a56957b752540093e178"
                      , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                      , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                      , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                      , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                      , epoch
                      , 7
                      , 5
                      );

    RED_CHECK_WORKSPACE(wd.add_files({
        "rdp_metrics-v1.0-1970-01-01.logmetrics",
        "rdp_metrics-v1.0-1970-01-01.logindex"}));

    metrics_log(metrics, (epoch + (3600*1)) * 1000);
    metrics_log(metrics, (epoch + (3600*2)) * 1000);
    metrics_log(metrics, (epoch + (3600*3)) * 1000);
    metrics_log(metrics, (epoch + (3600*4)) * 1000);
    metrics_log(metrics, (epoch + (3600*5)) * 1000);
    metrics_log(metrics, (epoch + (3600*6)) * 1000);
    metrics_log(metrics, (epoch + (3600*6)+3599) * 1000);

    RED_CHECK_WORKSPACE(wd);

    metrics_log(metrics, (epoch + (3600*7))*1000);

    RED_CHECK_WORKSPACE(wd.add_files({
        "rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics",
        "rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"}));

    metrics_log(metrics, (epoch + (24*3600*3))*1000);

    RED_CHECK_WORKSPACE(wd.add_files({
        "rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics",
        "rdp_metrics-v1.0-1970-01-03_22-00-00.logindex"}));

    metrics_delete(metrics);
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle1)
{
    WorkingDirectory wd("metrics_log_cycle1");

    time_t epoch = 1533211681;
    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, 34
                    , wd.dirname().c_str()
                    , "164d89c1a56957b752540093e178"
                    , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                    , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                    , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                    , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                    , epoch
                    , 24
                    , 5
                    );

    auto const logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto const logindex1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");
    RED_CHECK_WORKSPACE(wd);

    std::string expected_log_index("2018-08-02 12:08:01 connection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    RED_CHECK_EQUAL(get_file_contents(logmetrics1), "");
    RED_CHECK_EQUAL(get_file_contents(logindex1), expected_log_index);

    metrics_delete(metrics);

    RED_CHECK_WORKSPACE(wd);

    std::string expected_disconnected_index("2018-08-02 12:08:06 disconnection 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

    std::string expected_log_metrics1("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    RED_CHECK_EQUAL(get_file_contents(logmetrics1), expected_log_metrics1);
    RED_CHECK_EQUAL(get_file_contents(logindex1), expected_log_index+expected_disconnected_index);
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogAndIncrementations)
{
    WorkingDirectory wd("metrics_log_and_incrementatins");

    time_t epoch = 1533211681;
    const unsigned int index_len = 10;

    Metrics * metrics = metrics_new(fields_rdp_metrics_version, protocol_name, index_len
                    , wd.dirname().c_str()
                    , "164d89c1a56957b752540093e178"
                    , "51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58"
                    , "1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31"
                    , "EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48"
                    , "B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7"
                    , epoch
                    , 24
                    , 5
                    );

    auto const logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto const logindex1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");
    RED_CHECK_WORKSPACE(wd);

    std::string expected_log_metrics1("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0\n");

    metrics_log(metrics, (epoch + 4) * 1000);
    RED_CHECK_EQUAL(get_file_contents(logmetrics1), "");

    metrics_log(metrics, (epoch + 5) * 1000);
    RED_CHECK_EQUAL(get_file_contents(logmetrics1), expected_log_metrics1);

    std::string expected_log_metrics2("2018-08-02 12:08:11 164d89c1a56957b752540093e178 1 2 3 4 5 6 7 8 9 10\n");

    for (unsigned index = 0; index < index_len; index++) {
        unsigned val = index+1u;
        metrics_add_to_current_data(metrics, index, val);
    }
    metrics_log(metrics, (epoch + 7) * 1000);
    RED_CHECK_EQUAL(get_file_contents(logmetrics1), expected_log_metrics1);

    metrics_log(metrics, (epoch + 10) * 1000);
    RED_CHECK_EQUAL(get_file_contents(logmetrics1), expected_log_metrics1+expected_log_metrics2);

    metrics_delete(metrics);

    RED_CHECK_WORKSPACE(wd);
}
