/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#define RED_TEST_MODULE TestCLIPRDRChannel
#include "system/redemption_unit_tests.hpp"


#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "utils/sugar/make_unique.hpp"
#include "core/RDP/clipboard.hpp"
#include "test_only/transport/test_transport.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"

#include "test_only/front/fake_front.hpp"

class TestToClientSender : public VirtualChannelDataSender {
    Transport& transport;

public:
    TestToClientSender(Transport& transport) : transport(transport) {}

    virtual void operator() (uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length) override {
        LOG(LOG_INFO,
            "TestToClientSender: "
                "total_length=%u flags=0x%X chunk_data_length=%u",
            total_length, flags, chunk_data_length);

        const uint32_t dest = 0;    // Client
        this->transport.send(reinterpret_cast<const uint8_t*>(&dest),
            sizeof(dest));
        this->transport.send(reinterpret_cast<uint8_t*>(&total_length),
            sizeof(total_length));
        this->transport.send(reinterpret_cast<uint8_t*>(&flags),
            sizeof(flags));
        this->transport.send(reinterpret_cast<uint8_t*>(&chunk_data_length),
            sizeof(chunk_data_length));

        this->transport.send(chunk_data, chunk_data_length);
    }
};

class TestToServerSender : public VirtualChannelDataSender {
    Transport& transport;

public:
    TestToServerSender(Transport& transport) : transport(transport) {}

    virtual void operator() (uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length) override {
        LOG(LOG_INFO,
            "TestToServerSender: "
                "total_length=%u flags=0x%X chunk_data_length=%u",
            total_length, flags, chunk_data_length);

        const uint32_t dest = 1;    // Server
        this->transport.send(reinterpret_cast<const uint8_t*>(&dest),
            sizeof(dest));
        this->transport.send(reinterpret_cast<uint8_t*>(&total_length),
            sizeof(total_length));
        this->transport.send(reinterpret_cast<uint8_t*>(&flags),
            sizeof(flags));
        this->transport.send(reinterpret_cast<uint8_t*>(&chunk_data_length),
            sizeof(chunk_data_length));

        this->transport.send(chunk_data, chunk_data_length);
    }
};

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPFullAuthrisation)
{
    ClientInfo info;
    info.keylayout             = 0x04C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 24;
    info.width                 = 800;
    info.height                = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname, sizeof(info.hostname), "test");
    FakeFront front(info,
                    511 // verbose
                   );

    int verbose = static_cast<int>(RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump);

    NullReportMessage report_message;
    ClipboardVirtualChannel::Params clipboard_virtual_channel_params(report_message);

    clipboard_virtual_channel_params.exchanged_data_limit      = 0;
    clipboard_virtual_channel_params.verbose                   = to_verbose_flags(verbose);

    clipboard_virtual_channel_params.clipboard_down_authorized = true;
    clipboard_virtual_channel_params.clipboard_up_authorized   = true;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    clipboard_virtual_channel_params.dont_log_data_into_syslog = false;
    clipboard_virtual_channel_params.dont_log_data_into_wrm    = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_authorisation.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);

    auto test = [&]{
        while (true) {
            auto end = virtual_channel_data;
            t.recv_boom(end,
                   16    // dest(4) + total_length(4) + flags(4) +
                         //     chunk_length(4)
                );

            const uint32_t dest              =
                virtual_channel_stream.in_uint32_le();
            const uint32_t total_length      =
                virtual_channel_stream.in_uint32_le();
            const uint32_t flags             =
                virtual_channel_stream.in_uint32_le();
            const uint32_t chunk_data_length =
                virtual_channel_stream.in_uint32_le();

            //std::cout << "dest=" << dest <<
            //    ", total_length=" << total_length <<
            //    ", flags=" <<  flags <<
            //    ", chunk_data_length=" << chunk_data_length <<
            //    std::endl;

            end = virtual_channel_data;
            uint8_t * chunk_data = end;

            t.recv_boom(end, chunk_data_length);

            //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

            if (!dest)  // Client
            {
                clipboard_virtual_channel.process_client_message(
                    total_length, flags, chunk_data, chunk_data_length);
            }
            else
            {
                std::unique_ptr<AsynchronousTask> out_asynchronous_task;

                clipboard_virtual_channel.process_server_message(
                    total_length, flags, chunk_data, chunk_data_length,
                    out_asynchronous_task);

                RED_CHECK(false == bool(out_asynchronous_task));
            }

            virtual_channel_stream.rewind();
        }
    };
    RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPDownDenied)
{
    ClientInfo info;
    info.keylayout             = 0x04C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 24;
    info.width                 = 800;
    info.height                = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname, sizeof(info.hostname), "test");
    FakeFront front(info,
                    511 // verbose
                   );

    int verbose = static_cast<int>(RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump);

    NullReportMessage report_message;
    ClipboardVirtualChannel::Params clipboard_virtual_channel_params(report_message);

    clipboard_virtual_channel_params.exchanged_data_limit      = 0;
    clipboard_virtual_channel_params.verbose                   = to_verbose_flags(verbose);

    clipboard_virtual_channel_params.clipboard_down_authorized = false;
    clipboard_virtual_channel_params.clipboard_up_authorized   = true;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    clipboard_virtual_channel_params.dont_log_data_into_syslog = false;
    clipboard_virtual_channel_params.dont_log_data_into_wrm    = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_down_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);

    auto test = [&]{
        while (true) {
            auto end = virtual_channel_data;
            t.recv_boom(end,
                   16    // dest(4) + total_length(4) + flags(4) +
                         //     chunk_length(4)
                );

            const uint32_t dest              =
                virtual_channel_stream.in_uint32_le();
            const uint32_t total_length      =
                virtual_channel_stream.in_uint32_le();
            const uint32_t flags             =
                virtual_channel_stream.in_uint32_le();
            const uint32_t chunk_data_length =
                virtual_channel_stream.in_uint32_le();

            //std::cout << "dest=" << dest <<
            //    ", total_length=" << total_length <<
            //    ", flags=" <<  flags <<
            //    ", chunk_data_length=" << chunk_data_length <<
            //    std::endl;

            end = virtual_channel_data;
            uint8_t * chunk_data = end;

            t.recv_boom(end, chunk_data_length);

            //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

            if (!dest)  // Client
            {
                clipboard_virtual_channel.process_client_message(
                    total_length, flags, chunk_data, chunk_data_length);
            }
            else
            {
                std::unique_ptr<AsynchronousTask> out_asynchronous_task;

                clipboard_virtual_channel.process_server_message(
                    total_length, flags, chunk_data, chunk_data_length,
                    out_asynchronous_task);

                RED_CHECK(false == bool(out_asynchronous_task));
            }

            virtual_channel_stream.rewind();
        }
    };
    RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPUpDenied)
{
    ClientInfo info;
    info.keylayout             = 0x04C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 24;
    info.width                 = 800;
    info.height                = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname, sizeof(info.hostname), "test");
    FakeFront front(info,
                    511 // verbose
                   );

    int verbose = static_cast<int>(RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump);

    NullReportMessage report_message;
    ClipboardVirtualChannel::Params clipboard_virtual_channel_params(report_message);

    clipboard_virtual_channel_params.exchanged_data_limit      = 0;
    clipboard_virtual_channel_params.verbose                   = to_verbose_flags(verbose);

    clipboard_virtual_channel_params.clipboard_down_authorized = true;
    clipboard_virtual_channel_params.clipboard_up_authorized   = false;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    clipboard_virtual_channel_params.dont_log_data_into_syslog = false;
    clipboard_virtual_channel_params.dont_log_data_into_wrm    = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_up_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);

    auto test = [&]{
        while (true) {
            auto end = virtual_channel_data;
            t.recv_boom(end,
                   16    // dest(4) + total_length(4) + flags(4) +
                         //     chunk_length(4)
                );

            const uint32_t dest              =
                virtual_channel_stream.in_uint32_le();
            const uint32_t total_length      =
                virtual_channel_stream.in_uint32_le();
            const uint32_t flags             =
                virtual_channel_stream.in_uint32_le();
            const uint32_t chunk_data_length =
                virtual_channel_stream.in_uint32_le();

            //std::cout << "dest=" << dest <<
            //    ", total_length=" << total_length <<
            //    ", flags=" <<  flags <<
            //    ", chunk_data_length=" << chunk_data_length <<
            //    std::endl;

            end = virtual_channel_data;
            uint8_t * chunk_data = end;

            t.recv_boom(end, chunk_data_length);

            //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

            if (!dest)  // Client
            {
                clipboard_virtual_channel.process_client_message(
                    total_length, flags, chunk_data, chunk_data_length);
            }
            else
            {
                std::unique_ptr<AsynchronousTask> out_asynchronous_task;

                clipboard_virtual_channel.process_server_message(
                    total_length, flags, chunk_data, chunk_data_length,
                    out_asynchronous_task);

                RED_CHECK(false == bool(out_asynchronous_task));
            }

            virtual_channel_stream.rewind();
        }
    };
    RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPFullDenied)
{
    ClientInfo info;
    info.keylayout             = 0x04C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 24;
    info.width                 = 800;
    info.height                = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname, sizeof(info.hostname), "test");
    FakeFront front(info,
                    511 // verbose
                   );

    int verbose = static_cast<int>(RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump);
    NullReportMessage report_message;

    ClipboardVirtualChannel::Params clipboard_virtual_channel_params(report_message);

    clipboard_virtual_channel_params.exchanged_data_limit      = 0;
    clipboard_virtual_channel_params.verbose                   = to_verbose_flags(verbose);

    clipboard_virtual_channel_params.clipboard_down_authorized = false;
    clipboard_virtual_channel_params.clipboard_up_authorized   = false;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    clipboard_virtual_channel_params.dont_log_data_into_syslog = false;
    clipboard_virtual_channel_params.dont_log_data_into_wrm    = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);

    auto test = [&]{
        while (true) {
            auto end = virtual_channel_data;
            t.recv_boom(end,
                   16    // dest(4) + total_length(4) + flags(4) +
                         //     chunk_length(4)
                );

            const uint32_t dest              =
                virtual_channel_stream.in_uint32_le();
            const uint32_t total_length      =
                virtual_channel_stream.in_uint32_le();
            const uint32_t flags             =
                virtual_channel_stream.in_uint32_le();
            const uint32_t chunk_data_length =
                virtual_channel_stream.in_uint32_le();

            //std::cout << "dest=" << dest <<
            //    ", total_length=" << total_length <<
            //    ", flags=" <<  flags <<
            //    ", chunk_data_length=" << chunk_data_length <<
            //    std::endl;

            end = virtual_channel_data;
            uint8_t * chunk_data = end;

            t.recv_boom(end, chunk_data_length);

            //hexdump_c(chunk_data, virtual_channel_stream.in_remain());

            if (!dest)  // Client
            {
                clipboard_virtual_channel.process_client_message(
                    total_length, flags, chunk_data, chunk_data_length);
            }
            else
            {
                std::unique_ptr<AsynchronousTask> out_asynchronous_task;

                clipboard_virtual_channel.process_server_message(
                    total_length, flags, chunk_data, chunk_data_length,
                    out_asynchronous_task);

                RED_CHECK(false == bool(out_asynchronous_task));
            }

            virtual_channel_stream.rewind();
        }
    };
    RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
}

class NullSender : public VirtualChannelDataSender {
public:
    virtual void operator() (uint32_t, uint32_t, const uint8_t*, uint32_t) override {}
};

RED_AUTO_TEST_CASE(TestCliprdrChannelMalformedFormatListPDU)
{
    ClientInfo info;
    info.keylayout             = 0x04C;
    info.console_session       = 0;
    info.brush_cache_code      = 0;
    info.bpp                   = 24;
    info.width                 = 800;
    info.height                = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    snprintf(info.hostname, sizeof(info.hostname), "test");
    FakeFront front(info,
                    511 // verbose
                   );

    int verbose = static_cast<int>(RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump);

    NullReportMessage report_message;
    ClipboardVirtualChannel::Params clipboard_virtual_channel_params(report_message);

    clipboard_virtual_channel_params.exchanged_data_limit      = 0;
    clipboard_virtual_channel_params.verbose                   = to_verbose_flags(verbose);

    clipboard_virtual_channel_params.clipboard_down_authorized = true;
    clipboard_virtual_channel_params.clipboard_up_authorized   = true;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    clipboard_virtual_channel_params.dont_log_data_into_syslog = false;
    clipboard_virtual_channel_params.dont_log_data_into_wrm    = false;

    NullSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);


    RDPECLIP::FormatListPDU format_list_pdu;
    StaticOutStream<256>    out_s;

    const bool unicodetext           = false;
    const bool use_long_format_names = true;    // Malformation

    format_list_pdu.emit_2(out_s, unicodetext, use_long_format_names);

    const size_t totalLength = out_s.get_offset();

    clipboard_virtual_channel.process_client_message(
            totalLength,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            out_s.get_data(),
            totalLength);
}
