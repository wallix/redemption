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

#define RED_TEST_MODULE TestRDPMetrics
#include "system/redemption_unit_tests.hpp"

#include "utils/fileutils.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include "mod/rdp/rdp_metrics.hpp"
#include "mod/metrics_hmac.hpp"


RED_AUTO_TEST_CASE(TestRDPMetricsH)
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

RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle1)
{
    WorkingDirectory wd("metrics_log_cycle1");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;

    RDPMetrics metrics( true
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
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        metrics.log(to_timeval(epoch+5s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);;
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        metrics.log(to_timeval(epoch+7s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        metrics.log(to_timeval(epoch+10s));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCycle2)
{
    WorkingDirectory wd("metrics_log_cycle2");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    {
        constexpr auto epoch = 1533211681s;

        RDPMetrics metrics( true
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
            std::string expected_log_metrics("2018-08-02 12:08:04 164d89c1a56957b752540093e178 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
            metrics.log(to_timeval(epoch+3s));
            RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
        }
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogBasicIncrement)
{
    WorkingDirectory wd("metrics_log_basic_inc");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;

    RDPMetrics metrics( true
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

    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {
        epoch += 5s;
        metrics.right_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.right_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 3 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_main_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 3 2 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.left_click_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 3 2 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.key_pressed();
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:31 164d89c1a56957b752540093e178 3 2 1 1 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.mouse_mouve(0, 0);
        metrics.mouse_mouve(2, 2);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:36 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_main_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:41 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_rail_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:46 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_rail_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:51 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 0\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.client_other_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
    {
        epoch += 5s;
        std::string expected_log_metrics_next("2018-08-02 12:08:56 164d89c1a56957b752540093e178 3 2 1 1 4 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3 3 3\n");
        expected_log_metrics += expected_log_metrics_next;
        metrics.server_other_channel_data(3);
        metrics.log(to_timeval(epoch));
        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerImageCopy_PasteOnClient)
{
    WorkingDirectory wd("metrics_log_clipcopypaste");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics;

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_METAFILEPICT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 0 1 0 12 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_METAFILEPICT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 64 0 0 0 0 0 1 0 12 0 1 0 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerFileCopy_PasteOnClient)
{
    WorkingDirectory wd("metrics_log_filecopy");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics;

    {  // FORMAT LIST FILE COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT FILE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 12 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(49562);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FILE CONTENT REQUEST SIZE FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 54 0 0 0 0 0 0 1 48 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsRequestPDU fcrq_size(0, RDPECLIP::FILECONTENTS_SIZE, 0, 42, 42);
        fcrq_size.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FILE CONTENT RESPONSE SIZE FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 78 0 0 0 0 0 0 1 48 0 0 1 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsResponse_Size fcrp_size(0, 42);
        fcrp_size.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIServerTextCopy_PasteOnClient)
{
    WorkingDirectory wd("metrics_log_textcopy");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics;

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_TEXT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 14 0 0 0 0 1 0 0 12 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_TEXT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 64 0 0 0 0 1 0 0 12 1 0 0 42 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}


RED_AUTO_TEST_CASE(TestRDPMetricsRDPDRReadChunk)
{
    WorkingDirectory wd("metrics_log_read_chunck");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 0 0 0 0 0 0 0 0 0 0 0\n");

     { // CLIENT PDU
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
        sharedHeader.emit(out_stream);

        rdpdr::ClientAnnounceReply clientAnnounceReply( 1, 1, 1);
        clientAnnounceReply.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // READ FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 56 1 0 0 0 42 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_READ, 0);
        dior.emit(out_stream);

        rdpdr::DeviceReadRequest drr(42, 0);
        drr.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // WRITE FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 154 1 0 1 0 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
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

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // RENAME FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 210 1 0 1 1 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_SET_INFORMATION, 0);
        dior.emit(out_stream);

        rdpdr::ServerDriveSetInformationRequest sdsir(rdpdr::FileRenameInformation, 0);
        sdsir.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // DELETE FILE
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 12 266 1 1 1 1 42 42 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        rdpdr::SharedHeader header(rdpdr::RDPDR_CTYP_CORE, rdpdr::PAKID_CORE_DEVICE_IOREQUEST);
        header.emit(out_stream);
        rdpdr::DeviceIORequest dior(0, 0, 0, rdpdr::IRP_MJ_SET_INFORMATION, 0);
        dior.emit(out_stream);

        rdpdr::ServerDriveSetInformationRequest sdsir(rdpdr::FileDispositionInformation, 0);
        sdsir.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_rdpdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientImageCopy_PasteOnServer)
{
    WorkingDirectory wd("metrics_log_imgcopy");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    { // FORMAT LIST INITIALISATION
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT LIST IMAGE COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 68 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_METAFILEPICT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT IMAGE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 1 0 0 0 0 0 68 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_METAFILEPICT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT IMAGE DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 1 0 42 0 0 0 118 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientFileCopy_PasteOnServer)
{
    WorkingDirectory wd("metrics_log_filecopy_paste_on_server");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {  // FORMAT LIST INITIALISATION
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT LIST FILE COPY ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT FILE DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 0 0 1 0 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(49562);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FILE CONTENT REQUEST FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 48 0 0 1 0 0 0 0 108 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsRequestPDU format(0, RDPECLIP::FILECONTENTS_SIZE, 0, 42, 42);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FILE CONTENT RESPONSE SIZE FROM CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:26 164d89c1a56957b752540093e178 0 0 0 0 0 0 48 0 0 1 42 0 0 0 132 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;                                                    //
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FileContentsResponse_Size fcrp_size(0, 42);
        fcrp_size.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}


RED_AUTO_TEST_CASE(TestRDPMetricsLogCLIPRDRIClientTextCopy_PasteOnServer)
{
    WorkingDirectory wd("metrics_log_clienttextcopy");
    auto logmetrics1 = wd.add_file("rdp_metrics-v1.0-2018-08-02.logmetrics");
    auto logindex1   = wd.add_file("rdp_metrics-v1.0-2018-08-02.logindex");

    auto epoch = 1533211681s;
    RDPMetrics metrics( true
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
    std::string expected_log_metrics("2018-08-02 12:08:06 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 54 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

    {  // FORMAT LIST INITIALISATION
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 42+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(49562, RDPECLIP::FILEGROUPDESCRIPTORW.data(), RDPECLIP::FILEGROUPDESCRIPTORW.size());
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT LIST TEXT COPY ON SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:11 164d89c1a56957b752540093e178 0 0 0 0 0 0 0 0 0 0 0 0 0 0 68 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 2+4);
        format_list_header.emit(out_stream);
        RDPECLIP::FormatListPDU_LongName format(RDPECLIP::CF_TEXT, "\0", 1);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA REQUEST PASTE ON CLIENT
        std::string expected_log_metrics_next("2018-08-02 12:08:16 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 1 0 0 0 0 0 0 68 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        RDPECLIP::FormatDataRequestPDU format(RDPECLIP::CF_TEXT);
        format.emit(out_stream);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_server_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    {  // FORMAT TEXT DATA RESPONSE FROM SERVER
        std::string expected_log_metrics_next("2018-08-02 12:08:21 164d89c1a56957b752540093e178 0 0 0 0 0 0 12 1 0 0 42 0 0 0 118 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
        expected_log_metrics += expected_log_metrics_next;
        epoch += 5s;
        StaticOutStream<1600> out_stream;
        size_t size = 42;
        RDPECLIP::FormatDataResponsePDU format(size);
        const uint8_t data[42] = { 0 };
        format.emit(out_stream, data, size);
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        metrics.set_client_cliprdr_metrics(chunk, out_stream.get_offset(), CHANNELS::CHANNEL_FLAG_FIRST);

        metrics.log(to_timeval(epoch));

        RED_CHECK_EQUAL(get_file_contents(wd[logmetrics1]), expected_log_metrics);
    }

    RED_CHECK_WORKSPACE(wd);
}
