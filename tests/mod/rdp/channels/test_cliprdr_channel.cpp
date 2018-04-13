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

RED_AUTO_TEST_CASE(TestCliprdrChannelFailedFormatDataResponsePDU)
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

// ClipboardVirtualChannel::process_server_message: total_length=28 flags=0x00000003 chunk_data_length=28
// Recv done on channel (28) n bytes
// /* 0000 */ "\x01\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x1c\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x03\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x1c\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
// /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 // ............
// Dump done on channel (28) n bytes
// ClipboardVirtualChannel::process_server_message: Clipboard Capabilities PDU
// ClipboardVirtualChannel::process_server_clipboard_capabilities_pdu: General Capability Set
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E

    std::unique_ptr<AsynchronousTask> out_asynchronous_task;

    clipboard_virtual_channel.process_server_message(
            28,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
        /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 // ............
                ),
            28,
            out_asynchronous_task);

// ClipboardVirtualChannel::process_client_message: total_length=24 flags=0x00000013 chunk_data_length=24
// Recv done on channel (24) n bytes
// /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x18\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x13\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x18\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
// /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00"                                 // ........
// Dump done on channel (24) n bytes
// ClipboardVirtualChannel::process_client_message: Clipboard Capabilities PDU
// ClipboardVirtualChannel::process_client_clipboard_capabilities_pdu: General Capability Set
// RDPECLIP::GeneralCapabilitySet: capabilitySetType=CB_CAPSTYPE_GENERAL(1) lengthCapability=12 version=CB_CAPS_VERSION_2(0x00000002) generalFlags=0x0000001E

    clipboard_virtual_channel.process_client_message(
            24,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
    /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00"                                 // ........
                ),
            24);

// ClipboardVirtualChannel::process_client_message: total_length=130 flags=0x00000013 chunk_data_length=130
// Recv done on channel (130) n bytes
// /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x82\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x13\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x82\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x6e\xc0\x00\x00\x46\x00\x69\x00" // ....z...n...F.i.
// /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
// /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
// /* 0030 */ "\x72\x00\x57\x00\x00\x00\x94\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
// /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
// /* 0050 */ "\x73\x00\x00\x00\x10\xc1\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
// /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
// /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
// /* 0080 */ "\x00\x00"                                                         // ..
// Dump done on channel (130) n bytes
// ClipboardVirtualChannel::process_client_message: Format List PDU
// ClipboardVirtualChannel::process_client_format_list_pdu: Long Format Name variant of Format List PDU is used for exchanging updated format names.
// ClipboardVirtualChannel::process_client_format_list_pdu: formatId=<unknown>(49262) wszFormatName="FileGroupDescriptorW"
// ClipboardVirtualChannel::process_client_format_list_pdu: formatId=<unknown>(49300) wszFormatName="FileContents"
// ClipboardVirtualChannel::process_client_format_list_pdu: formatId=<unknown>(49424) wszFormatName="Preferred DropEffect"

    clipboard_virtual_channel.process_client_message(
            130,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x6e\xc0\x00\x00\x46\x00\x69\x00" // ....z...n...F.i.
        /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
        /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
        /* 0030 */ "\x72\x00\x57\x00\x00\x00\x94\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
        /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
        /* 0050 */ "\x73\x00\x00\x00\x10\xc1\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
        /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
        /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
        /* 0080 */ "\x00\x00"                                                         // ..
                ),
            130);

// ClipboardVirtualChannel::process_server_format_data_request_pdu: requestedFormatId=<unknown>(49262)
// Sending on channel (16) n bytes
// /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x10\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x03\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x10\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x6e\xc0\x00\x00\x00\x00\x00\x00" // ........n.......
// Sent dumped on channel (16) n bytes
// CLIPRDR CB_FORMAT_DATA_RESPONSE format data id = 49262 <unknown>

    clipboard_virtual_channel.process_server_message(
            16,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x6e\xc0\x00\x00\x00\x00\x00\x00"
                ),
            16,
            out_asynchronous_task);

// ClipboardVirtualChannel::process_client_message: total_length=8 flags=0x00000013 chunk_data_length=8
// Recv done on channel (8) n bytes
// /* 0000 */ "\x00\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x08\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x13\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x08\x00\x00\x00"                                                 // ....
// /* 0000 */ "\x05\x00\x02\x00\x00\x00\x00\x00"                                 // ........
// Dump done on channel (8) n bytes
// ClipboardVirtualChannel::process_client_message: Format Data Response PDU

    clipboard_virtual_channel.process_client_message(
            8,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x05\x00\x02\x00\x00\x00\x00\x00"
                ),
            8);
}
