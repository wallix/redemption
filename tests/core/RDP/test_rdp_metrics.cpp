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



constexpr const char * rdp_metrics_path_file = "/tmp/";

RED_AUTO_TEST_CASE(TestRDPMetricsConstructor)
{
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");

    // Should create rdp_metrics files if they do not exist
    ClientInfo info;
    uint8_t key[32] = {0};
    time_t epoch = 1533211681; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    RDPMetrics metrics( epoch
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , hmac_user("primaryuser", key)
                      , hmac_account("secondaryuser", key)
                      , hmac_device_service("device1", "service1", key)
                      , hmac_client_info("10.10.13.12", info, key)
                      , std::chrono::hours{24}
                      , true
                      , 5
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));

    metrics.rotate(epoch + (3600*1));
    metrics.rotate(epoch + (3600*2));
    metrics.rotate(epoch + (3600*3));
    metrics.rotate(epoch + (3600*4));
    metrics.rotate(epoch + (3600*5));
    metrics.rotate(epoch + (3600*6));

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));



    metrics.rotate(epoch + (3600*24));

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-03.logindex"));

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-03.logindex");

}

RED_AUTO_TEST_CASE(TestRDPMetricsConstructorHoursRotation)
{
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");

    // Should create rdp_metrics files if they do not exist
    ClientInfo info;
    uint8_t key[32] = {0};
    time_t epoch = 0; // 2018-08-02 12:08:01 = 1533168000 + 12*3600 + 8*60 + 1
//     LOG(LOG_INFO, "%s", text_gmdatetime(1533193200-24*3600));

    RDPMetrics metrics( epoch
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , hmac_user("primaryuser", key)
                      , hmac_account("secondaryuser", key)
                      , hmac_device_service("device1", "service1", key)
                      , hmac_client_info("10.10.13.12", info, key)
                      , std::chrono::hours{7}
                      , true
                      , 5
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01.logindex"));

    metrics.rotate(epoch + (3600*1));
    metrics.rotate(epoch + (3600*2));
    metrics.rotate(epoch + (3600*3));
    metrics.rotate(epoch + (3600*4));
    metrics.rotate(epoch + (3600*5));
    metrics.rotate(epoch + (3600*6));
    metrics.rotate(epoch + (3600*6)+3599);

    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(false, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics.rotate(epoch + (3600*7));

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex"));

    metrics.rotate(epoch + (24*3600*3));

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex"));

    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-01_07-00-00.logindex");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-1970-01-03_22-00-00.logindex");
}


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



RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle) {

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");

    ClientInfo info;
    uint8_t key[32] = {0};
    time_t epoch = 1533211681-6;
    RDPMetrics metrics( epoch
                      , rdp_metrics_path_file
                      , "164d89c1a56957b752540093e178"
                      , hmac_user("primaryuser", key)
                      , hmac_account("secondaryuser", key)
                      , hmac_device_service("device1", "service1", key)
                      , hmac_client_info("10.10.13.12", info, key)
                      , std::chrono::hours{24}
                      , true
                      , 5
                      );

    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"));
    RED_CHECK_EQUAL(true, file_exist("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"));



    {
        metrics.right_click_pressed();

        timeval now;
        now.tv_sec = 1533211681;
        now.tv_usec = 0;
        metrics.log(now);

        std::string expected_log_index("2018-08-02 12:07:55 164d89c1a56957b752540093e178 user=51614130003BD5522C94E637866E4D749DDA13706AC2610C6F77BBFE111F3A58 account=1C57BA616EEDA5C9D8FF2E0202BB087D0B5D865AC830F336CDB9804331095B31 target_service_device=EAF28B142E03FFC03A35676722BB99DBC21908F3CEA96A8DA6E3C2321056AC48 client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7\n");

        std::string expected_log_metrics("2018-08-02 12:08:01 164d89c1a56957b752540093e178 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics"), expected_log_metrics);
        RED_CHECK_EQUAL(get_file_contents("/tmp/rdp_metrics-v1.0-2018-08-02.logindex"), expected_log_index);
    }

    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logmetrics");
    unlink("/tmp/rdp_metrics-v1.0-2018-08-02.logindex");
}


// RED_AUTO_TEST_CASE(TestRDPMetricsCLIPRDRReadChunk) {
//
//     ClientInfo info;
//     uint8_t key[32] = {0};
//     RDPMetrics metrics( rdp_metrics_path_file
//                       , "1"
//                       , "user"
//                       , "admin"
//                       , "10.10.13.12"
//                       , info
//                       , "RDP1"
//                       , "device1"
//                       , key
//                       , 24
//                       , true);
//
//     char current_date[24] = {'\0'};
//
//     timeval now = tvtime();
//     metrics.set_current_formated_date(current_date, false, now.tv_sec);
//
//     char complete_file_path[4096] = {'\0'};
//     ::snprintf(complete_file_path, sizeof(complete_file_path), "%srdp_metrics-%s.log", rdp_metrics_path_file, current_date);
//
//     RED_CHECK(file_exist(complete_file_path));
//
//     RED_REQUIRE(file_exist(complete_file_path));
//
//     metrics.cliprdr_init_format_list_done = true;
//     {
//         StaticOutStream<1600> out_stream;
//         RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 26+4);
//         format_list_header.emit(out_stream);
//         RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILECONTENTS, 13);
//         format.emit(out_stream);
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }https://mail.google.com/mail/u/0/#inbox/164fc2580bb76585
//     {
//         StaticOutStream<1600> out_stream;
//         RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 4+2);
//         format_list_header.emit(out_stream);
//         RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_TEXT, "\0", 1);
//         format.emit(out_stream);
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }
//     {
//         StaticOutStream<1600> out_stream;
//         RDPECLIP::FileContentsRequestPDU fileContentsRequest( 0
//                                                             , RDPECLIP::FILECONTENTS_RANGE
//                                                             , 0
//                                                             , 8);
//         fileContentsRequest.emit(out_stream);
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }
//
//     metrics.log();
//
//     std::string expected_log_header;
//     char start_full_date_time[24];
//     metrics.set_current_formated_date(start_full_date_time, true, now.tv_sec);
//     expected_log_header += start_full_date_time;
//
//     expected_log_header += " 1";
// //         "user=8D5F8AEEB64E3CE20B537D04C486407EAF489646617CFCF493E76F5B794FA080 account=5544E527C72AAE51DF22438F3EBA7B8A545F2D2391E64C4CC706EFFACA99D3C1 target_service_device=567475896AE7361D47721A8D430BEC617DF225B9A253FA97FFB09906FB9D3A4E client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7";
//
//     std::string expected_log_data_2(expected_log_header+" clipboard_channel_data_from_server=84 total_data_paste_on_server=8 nb_copy_text_on_server=1 nb_copy_file_on_server=1\n");
//
//     int fd = ::open(complete_file_path, O_RDONLY);
//     char buff[4096] = {'\0'};
//     ::read(fd, buff, 4096);
//     std::string file_content(buff);
//
//     RED_CHECK_EQUAL(file_content, expected_log_data_2);
//     ::close(fd);
//
//     RED_CHECK_EQUAL(get_file_contents(complete_file_path), expected_log_header);
//     remove(complete_file_path);
// }
//
// RED_AUTO_TEST_CASE(TestRDPMetricsRDPDRReadChunk) {
//
//     ClientInfo info;
//     uint8_t key[32] = {0};
//     RDPMetrics metrics( rdp_metrics_path_file
//                       , "1"
//                       , "user"
//                       , "admin"
//                       , "10.10.13.12"
//                       , info
//                       , "RDP1"
//                       , "device1"
//                       , key
//                       , 24
//                       , true);
//
//     char current_date[24] = {'\0'};
//
//     timeval now = tvtime();
//     metrics.set_current_formated_date(current_date, false, now.tv_sec);
//
//     char complete_file_path[4096] = {'\0'};
//     ::snprintf(complete_file_path, sizeof(complete_file_path), "%srdp_metrics-%s.log", rdp_metrics_path_file, current_date);;
//
//     RED_REQUIRE(file_exist(complete_file_path));
//
//     metrics.cliprdr_init_format_list_done = true;int
//     {
//         StaticOutStream<1600> out_stream;
//         rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
//         header.emit(out_stream);
//         rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_READ, 0);
//         dior.emit(out_stream);
//
//         rdpdr::DeviceReadRequest drr(1001, 0);
//         drr.emit(out_stream);
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }
//     {
//         StaticOutStream<1600> out_stream;
//         rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
//         header.emit(out_stream);
//         rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_SET_INFORMATION, 0);
//         dior.emit(out_stream);
//
//         rdpdr::ServerDriveSetInformationRequest sdsir(rdpdr::FileRenameInformation, 0);
//         sdsir.emit(out_stream);
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }
//     {
//         StaticOutStream<1600> out_stream;
//         rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
//         header.emit(out_stream);
//         rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_WRITE, 0);
//         dior.emit(out_stream);
//
//         InStream chunk(out_stream.get_data(), out_stream.get_offset());
//
//         metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);
//     }
//
//     metrics.log();
//
//     std::string expected_log;
//     char start_full_date_time[24];
//     metrics.set_current_formated_date(start_full_date_time, true, now.tv_sec);
//     expected_log += start_full_date_time;
//     expected_log += " 1  disk_redirection_channel_data_from_server=136 nb_files_1k_read=1 nb_files_write=1 nb_files_rename=1\n";
//
//     //"user=8D5F8AEEB64E3CE20B537D04C486407EAF489646617CFCF493E76F5B794FA080 account=5544E527C72AAE51DF22438F3EBA7B8A545F2D2391E64C4CC706EFFACA99D3C1 target_service_device=567475896AE7361D47721A8D430BEC617DF225B9A253FA97FFB09906FB9D3A4E client_info=B079C9845904075BAC3DBE0A26CB7364CE0CC0A5F47DC082F44D221EBC6722B7""
//
//     RED_CHECK_EQUAL(get_file_contents(complete_file_path), expected_log);
//     remove(complete_file_path);
// }
