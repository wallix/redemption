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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include "core/RDP/clipboard.hpp"
#include "test_only/transport/test_transport.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"

#include "./test_channel.hpp"
#include "test_only/front/fake_front.hpp"


RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPFullAuthrisation)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    SessionReactor session_reactor;
    NullReportMessage report_message;

    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_authorisation.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
                base_params,
                clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPDownDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    SessionReactor session_reactor;
    NullReportMessage report_message;

    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    
    ClipboardVirtualChannelParams clipboard_virtual_channel_params;
    clipboard_virtual_channel_params.clipboard_down_authorized = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_down_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPUpDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

    SessionReactor session_reactor;

    NullReportMessage report_message;
    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;
    clipboard_virtual_channel_params.clipboard_up_authorized   = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_up_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPFullDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

    SessionReactor session_reactor;

    NullReportMessage report_message;

    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;
    clipboard_virtual_channel_params.clipboard_down_authorized = false;
    clipboard_virtual_channel_params.clipboard_up_authorized   = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

class NullSender : public VirtualChannelDataSender {
public:
    virtual void operator() (uint32_t, uint32_t, const uint8_t*, uint32_t) override {}
};

RED_AUTO_TEST_CASE(TestCliprdrChannelMalformedFormatListPDU)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    SessionReactor session_reactor;
    NullReportMessage report_message;
    
    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;

    NullSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);


    RDPECLIP::FormatListPDUEx format_list_pdu;
    format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

    const bool use_long_format_names = true;    // Malformation
    const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

    RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
        RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
        format_list_pdu.size(use_long_format_names));

    StaticOutStream<256> out_s;

    clipboard_header.emit(out_s);
    format_list_pdu.emit(out_s, use_long_format_names);

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
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    SessionReactor session_reactor;
    NullReportMessage report_message;
    
    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;

    NullSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, false, "", session_reactor,
        base_params,
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


 class TestResponseSender : public VirtualChannelDataSender
{

public:
    std::vector<InStream> streams;


    explicit TestResponseSender()
    {}

    void operator()(uint32_t /*total_length*/, uint32_t /*flags*/,
        const uint8_t* chunk_data, uint32_t chunk_data_length)
            override
    {
//         InStream stream();
        this->streams.push_back(InStream(chunk_data, chunk_data_length));
    }
};


RED_AUTO_TEST_CASE(TestCliprdrChannelFilterServerDataFile) {
    WorkingDirectory wd("TestCliprdrChannelFilterServerDataFile");

    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

    SessionReactor session_reactor;
    timeval time_test;
    time_test.tv_sec = 12345;
    time_test.tv_usec = 54321;
    session_reactor.set_current_time(time_test);

    NullReportMessage report_message;
    
    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;
    
    ClipboardVirtualChannelParams clipboard_virtual_channel_params;

    TestResponseSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, true, wd.dirname(), session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    std::unique_ptr<AsynchronousTask> out_asynchronous_task;

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

// INFO (4749/4749) --      CliprdrHeader: MsgType=0x2(2 bytes):CB_FORMAT_LIST MsgFlags=0x0(2 bytes):CB_RESPONSE__NONE_ DataLen=122Byte(s)(4 bytes)
// INFO (4749/4749) -- FormatListPDU: {formatId=<unknown>(49280) formatName="FileGroupDescriptorW"} {formatId=<unknown>(49282) formatName="FileContents"} {formatId=<unknown>(49309) formatName="Preferred DropEffect"}
// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: total_length=134 flags=0x00000003 chunk_data_length=134
// INFO (4749/4749) -- Recv done on channel (134) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x00\x00\x00\x86
// INFO (4749/4749) -- \x00\x00\x00\x03
// INFO (4749/4749) -- \x00\x00\x00\x86
// INFO (4749/4749) -- /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x80\xc0\x00\x00\x46\x00\x69\x00" // ....z.......F.i.
// INFO (4749/4749) -- /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
// INFO (4749/4749) -- /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
// INFO (4749/4749) -- /* 0030 */ "\x72\x00\x57\x00\x00\x00\x82\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
// INFO (4749/4749) -- /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
// INFO (4749/4749) -- /* 0050 */ "\x73\x00\x00\x00\x9d\xc0\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
// INFO (4749/4749) -- /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
// INFO (4749/4749) -- /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
// INFO (4749/4749) -- /* 0080 */ "\x00\x00\x00\x00\x00\x00"                                         // ......
// INFO (4749/4749) -- Dump done on channel (134) n bytes

    clipboard_virtual_channel.process_server_message(
            134,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x80\xc0\x00\x00\x46\x00\x69\x00" // ....z.......F.i.
    /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
    /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
    /* 0030 */ "\x72\x00\x57\x00\x00\x00\x82\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
    /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
    /* 0050 */ "\x73\x00\x00\x00\x9d\xc0\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
    /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
    /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
    /* 0080 */ "\x00\x00\x00\x00\x00\x00"
                ),
            134,
            out_asynchronous_task);

// INFO (4749/4749) --      CliprdrHeader: MsgType=0x3(2 bytes):CB_FORMAT_LIST_RESPONSE MsgFlags=0x1(2 bytes):CB_RESPONSE_OK DataLen=0Byte(s)(4 bytes)
// INFO (4749/4749) -- FormatListResponsePDU
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=8 flags=0x00000013 chunk_data_length=8
// INFO (4749/4749) -- Recv done on channel (8) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x8
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x8
// INFO (4749/4749) -- /* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"                                 // ........
// INFO (4749/4749) -- Dump done on channel (8) n bytes

        clipboard_virtual_channel.process_client_message(
            8,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"
                ),
            8);

// INFO (4749/4749) -- LockClipboardDataPDU: streamDataID=0x00000000(4 bytes)
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=12 flags=0x00000013 chunk_data_length=12
// INFO (4749/4749) -- Recv done on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x0a\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"                 // ............
// INFO (4749/4749) -- Dump done on channel (12) n bytes

        clipboard_virtual_channel.process_client_message(
            12,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x0a\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
                ),
            12);

// INFO (4749/4749) -- FormatDataRequestPDU: requestedFormatId=0x0000c080(4 bytes):<unknown>
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=12 flags=0x00000013 chunk_data_length=12
// INFO (4749/4749) -- Recv done on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x80\xc0\x00\x00"                 // ............
// INFO (4749/4749) -- Dump done on channel (12) n bytes

        clipboard_virtual_channel.process_client_message(
            12,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x80\xc0\x00\x00"
                ),
            12);

// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: Format Data Response PDU
// INFO (4749/4749) -- Sending FileGroupDescriptorW(49280) clipboard data to client. cItems=1
// INFO (4749/4749) -- FileDescriptor: flags=0x4064 fileAttributes=0x20 lastWriteTime=131853846090163395 fileSizeHigh=0x0 fileSizeLow=0xa fileName="test.txt"
// INFO (4749/4749) -- type="CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION" format="FileGroupDescriptorW(49280)" size="596"
// INFO (4749/4749) -- Sending on channel (608) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x2\x60
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x2\x60
// INFO (4749/4749) -- /* 0000 */ "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x64\x40\x00\x00" // ....T.......d@..
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0030 */ "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //  ...............
// INFO (4749/4749) -- /* 0040 */ "\x00\x00\x00\x00\xc3\x24\x79\xda\x5f\x70\xd4\x01\x00\x00\x00\x00" // .....$y._p......
// INFO (4749/4749) -- /* 0050 */ "\x0a\x00\x00\x00\x74\x00\x65\x00\x73\x00\x74\x00\x2e\x00\x74\x00" // ....t.e.s.t...t.
// INFO (4749/4749) -- /* 0060 */ "\x78\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // x.t.............
// INFO (4749/4749) -- /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- Sent dumped on channel (608) n bytes

    clipboard_virtual_channel.process_server_message(
            608,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x64\x40\x00\x00" // ....T.......d@..
    /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0030 */ "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //  ...............
    /* 0040 */ "\x00\x00\x00\x00\xc3\x24\x79\xda\x5f\x70\xd4\x01\x00\x00\x00\x00" // .....$y._p......
    /* 0050 */ "\x0a\x00\x00\x00\x74\x00\x65\x00\x73\x00\x74\x00\x2e\x00\x74\x00" // ....t.e.s.t...t.
    /* 0060 */ "\x78\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // x.t.............
    /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
                ),
            608,
            out_asynchronous_task);

    auto const file_test = wd.add_file("12345_54321_test.txt");

// INFO (4749/4749) -- FileContentsRequestPDU: streamId=1(4 bytes) lindex=0(4 bytes) dwFlags=1(4 bytes) nPositionLow=0(4 bytes) nPositionHigh=0(4 bytes) cbRequested=8(4 bytes) clipDataId=0
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=36 flags=0x00000013 chunk_data_length=36
// INFO (4749/4749) -- Recv done on channel (36) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Dump done on channel (36) n bytes

        clipboard_virtual_channel.process_client_message(
            36,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0010 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
                ),
            36);

// INFO (4749/4749) -- File Contents Response Size: streamID = 0X00000001(4 bytes) size=26(8 bytes) Padding - (4 byte) NOT USED
// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: total_length=24 flags=0x00000003 chunk_data_length=24
// INFO (4749/4749) -- Recv done on channel (24) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\x18
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x0\x18
// INFO (4749/4749) -- /* 0000 */ "\x09\x00\x01\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00"                                 // ........
// INFO (4749/4749) -- Dump done on channel (24) n bytes

        clipboard_virtual_channel.process_server_message(
            24,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x09\x00\x01\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x1a\x00\x00\x00" // ................
        /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00"                                 // ........
                ),
            24,
            out_asynchronous_task);

// INFO (4749/4749) -- FileContentsRequestPDU: streamId=1(4 bytes) lindex=0(4 bytes) dwFlags=2(4 bytes) nPositionLow=0(4 bytes) nPositionHigh=0(4 bytes) cbRequested=65535(4 bytes) clipDataId=0
// INFO (4749/4749) -- Sending on channel (36) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Sent dumped on channel (36) n bytes

        clipboard_virtual_channel.process_client_message(
            36,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
                ),
            36);

    RED_CHECK_EQUAL(to_client_sender.streams.size(), 5);

// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: File Contents Response PDU
// INFO (4749/4749) -- Sending on channel (20) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x14
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x0\x14
// INFO (4749/4749) -- /* 0000 */ "\x09\x00\x01\x00\x0b\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" // ............test
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Sent dumped on channel (20) n bytes

        clipboard_virtual_channel.process_server_message(
            22,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x09\x00\x01\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" // ............test
                ),
            16,
            out_asynchronous_task);

    RED_CHECK_EQUAL(get_file_contents(file_test), "test");

        clipboard_virtual_channel.process_server_message(
            22,
              CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x20\x20\x74\x65\x73\x74"                         //   test....
                ),
            6,
            out_asynchronous_task);

    RED_CHECK_EQUAL(to_client_sender.streams.size(), 5);

    RED_CHECK_EQUAL(get_file_contents(file_test), "test  test");

    clipboard_virtual_channel.DLP_antivirus_check_channels_files();

    RED_CHECK_EQUAL(to_client_sender.streams.size(), 6);

    auto expected_pdu =
    /* 0000 */ "\x09\x00\x01\x00\x0e\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" //............test
    /* 0001 */ "\x20\x20\x74\x65\x73\x74"                         //   test....
    ""_av
    ;

//    RED_CHECK_MEM(expected_pdu, to_client_sender.streams[5].get_bytes());


// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: Unlock Clipboard Data PDU
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: clipDataId=1
// INFO (4749/4749) -- Sending on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x0b\x00\x00\x00\x04\x00\x00\x00\x01\x00\x00\x00"                 // ............
// INFO (4749/4749) -- Sent dumped on channel (12) n bytes

    RED_CHECK_WORKSPACE(wd);
}









RED_AUTO_TEST_CASE(TestCliprdrChannelFilterClientDataFile) {
    WorkingDirectory wd("TestCliprdrChannelFilterClientDataFile");

    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

    SessionReactor session_reactor;
    timeval time_test;
    time_test.tv_sec = 12345;
    time_test.tv_usec = 54321;
    session_reactor.set_current_time(time_test);

    NullReportMessage report_message;
    
    BaseVirtualChannel::Params base_params(report_message);
    base_params.exchanged_data_limit      = 0;
    base_params.verbose                   = RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;

    NullSender to_client_sender;
    TestResponseSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front, true, wd.dirname(), session_reactor,
        base_params,
        clipboard_virtual_channel_params);

    std::unique_ptr<AsynchronousTask> out_asynchronous_task;


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

// INFO (4749/4749) --      CliprdrHeader: MsgType=0x2(2 bytes):CB_FORMAT_LIST MsgFlags=0x0(2 bytes):CB_RESPONSE__NONE_ DataLen=122Byte(s)(4 bytes)
// INFO (4749/4749) -- FormatListPDU: {formatId=<unknown>(49280) formatName="FileGroupDescriptorW"} {formatId=<unknown>(49282) formatName="FileContents"} {formatId=<unknown>(49309) formatName="Preferred DropEffect"}
// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: total_length=134 flags=0x00000003 chunk_data_length=134
// INFO (4749/4749) -- Recv done on channel (134) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x00\x00\x00\x86
// INFO (4749/4749) -- \x00\x00\x00\x03
// INFO (4749/4749) -- \x00\x00\x00\x86
// INFO (4749/4749) -- /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x80\xc0\x00\x00\x46\x00\x69\x00" // ....z.......F.i.
// INFO (4749/4749) -- /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
// INFO (4749/4749) -- /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
// INFO (4749/4749) -- /* 0030 */ "\x72\x00\x57\x00\x00\x00\x82\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
// INFO (4749/4749) -- /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
// INFO (4749/4749) -- /* 0050 */ "\x73\x00\x00\x00\x9d\xc0\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
// INFO (4749/4749) -- /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
// INFO (4749/4749) -- /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
// INFO (4749/4749) -- /* 0080 */ "\x00\x00\x00\x00\x00\x00"                                         // ......
// INFO (4749/4749) -- Dump done on channel (134) n bytes

    clipboard_virtual_channel.process_client_message(
            134,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x80\xc0\x00\x00\x46\x00\x69\x00" // ....z.......F.i.
    /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
    /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
    /* 0030 */ "\x72\x00\x57\x00\x00\x00\x82\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
    /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
    /* 0050 */ "\x73\x00\x00\x00\x9d\xc0\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
    /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
    /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
    /* 0080 */ "\x00\x00\x00\x00\x00\x00"
                ),
            134);

// INFO (4749/4749) --      CliprdrHeader: MsgType=0x3(2 bytes):CB_FORMAT_LIST_RESPONSE MsgFlags=0x1(2 bytes):CB_RESPONSE_OK DataLen=0Byte(s)(4 bytes)
// INFO (4749/4749) -- FormatListResponsePDU
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=8 flags=0x00000013 chunk_data_length=8
// INFO (4749/4749) -- Recv done on channel (8) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x8
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x8
// INFO (4749/4749) -- /* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"                                 // ........
// INFO (4749/4749) -- Dump done on channel (8) n bytes

        clipboard_virtual_channel.process_server_message(
            8,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x03\x00\x01\x00\x00\x00\x00\x00"
                ),
            8,
            out_asynchronous_task);

// INFO (4749/4749) -- LockClipboardDataPDU: streamDataID=0x00000000(4 bytes)
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=12 flags=0x00000013 chunk_data_length=12
// INFO (4749/4749) -- Recv done on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x0a\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"                 // ............
// INFO (4749/4749) -- Dump done on channel (12) n bytes

        clipboard_virtual_channel.process_server_message(
            12,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x0a\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
                ),
            12,
            out_asynchronous_task);

// INFO (4749/4749) -- FormatDataRequestPDU: requestedFormatId=0x0000c080(4 bytes):<unknown>
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=12 flags=0x00000013 chunk_data_length=12
// INFO (4749/4749) -- Recv done on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x80\xc0\x00\x00"                 // ............
// INFO (4749/4749) -- Dump done on channel (12) n bytes

        clipboard_virtual_channel.process_server_message(
            12,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x80\xc0\x00\x00"
                ),
            12,
            out_asynchronous_task);

// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: Format Data Response PDU
// INFO (4749/4749) -- Sending FileGroupDescriptorW(49280) clipboard data to client. cItems=1
// INFO (4749/4749) -- FileDescriptor: flags=0x4064 fileAttributes=0x20 lastWriteTime=131853846090163395 fileSizeHigh=0x0 fileSizeLow=0xa fileName="test.txt"
// INFO (4749/4749) -- type="CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION" format="FileGroupDescriptorW(49280)" size="596"
// INFO (4749/4749) -- Sending on channel (608) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x2\x60
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x2\x60
// INFO (4749/4749) -- /* 0000 */ "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x64\x40\x00\x00" // ....T.......d@..
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0030 */ "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //  ...............
// INFO (4749/4749) -- /* 0040 */ "\x00\x00\x00\x00\xc3\x24\x79\xda\x5f\x70\xd4\x01\x00\x00\x00\x00" // .....$y._p......
// INFO (4749/4749) -- /* 0050 */ "\x0a\x00\x00\x00\x74\x00\x65\x00\x73\x00\x74\x00\x2e\x00\x74\x00" // ....t.e.s.t...t.
// INFO (4749/4749) -- /* 0060 */ "\x78\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // x.t.............
// INFO (4749/4749) -- /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- Sent dumped on channel (608) n bytes

    clipboard_virtual_channel.process_client_message(
            608,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x64\x40\x00\x00" // ....T.......d@..
    /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0030 */ "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //  ...............
    /* 0040 */ "\x00\x00\x00\x00\xc3\x24\x79\xda\x5f\x70\xd4\x01\x00\x00\x00\x00" // .....$y._p......
    /* 0050 */ "\x0a\x00\x00\x00\x74\x00\x65\x00\x73\x00\x74\x00\x2e\x00\x74\x00" // ....t.e.s.t...t.
    /* 0060 */ "\x78\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // x.t.............
    /* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
                ),
            608);

    auto const file_test = wd.add_file("12345_54321_test.txt");

// INFO (4749/4749) -- FileContentsRequestPDU: streamId=1(4 bytes) lindex=0(4 bytes) dwFlags=1(4 bytes) nPositionLow=0(4 bytes) nPositionHigh=0(4 bytes) cbRequested=8(4 bytes) clipDataId=0
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: total_length=36 flags=0x00000013 chunk_data_length=36
// INFO (4749/4749) -- Recv done on channel (36) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Dump done on channel (36) n bytes

        clipboard_virtual_channel.process_server_message(
            36,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0010 */ "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
                ),
            36,
            out_asynchronous_task);

// INFO (4749/4749) -- File Contents Response Size: streamID = 0X00000001(4 bytes) size=26(8 bytes) Padding - (4 byte) NOT USED
// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: total_length=24 flags=0x00000003 chunk_data_length=24
// INFO (4749/4749) -- Recv done on channel (24) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\x18
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x0\x18
// INFO (4749/4749) -- /* 0000 */ "\x09\x00\x01\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00"                                 // ........
// INFO (4749/4749) -- Dump done on channel (24) n bytes

        clipboard_virtual_channel.process_client_message(
            24,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x09\x00\x01\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x1a\x00\x00\x00" // ................
        /* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00"                                 // ........
                ),
            24);

// INFO (4749/4749) -- FileContentsRequestPDU: streamId=1(4 bytes) lindex=0(4 bytes) dwFlags=2(4 bytes) nPositionLow=0(4 bytes) nPositionHigh=0(4 bytes) cbRequested=65535(4 bytes) clipDataId=0
// INFO (4749/4749) -- Sending on channel (36) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\x24
// INFO (4749/4749) -- /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
// INFO (4749/4749) -- /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00" // ................
// INFO (4749/4749) -- /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Sent dumped on channel (36) n bytes

        clipboard_virtual_channel.process_server_message(
            36,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
    /* 0000 */ "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" // ................
    /* 0010 */ "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00" // ................
    /* 0020 */ "\x00\x00\x00\x00"                                                 // ....
                ),
            36,
            out_asynchronous_task);

    RED_CHECK_EQUAL(to_server_sender.streams.size(), 6);

// INFO (4749/4749) -- ClipboardVirtualChannel::process_server_message: File Contents Response PDU
// INFO (4749/4749) -- Sending on channel (20) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x00
// INFO (4749/4749) -- \x0\x0\x0\x14
// INFO (4749/4749) -- \x0\x0\x0\x3
// INFO (4749/4749) -- \x0\x0\x0\x14
// INFO (4749/4749) -- /* 0000 */ "\x09\x00\x01\x00\x0b\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" // ............test
// INFO (4749/4749) -- /* 0010 */ "\x00\x00\x00\x00"                                                 // ....
// INFO (4749/4749) -- Sent dumped on channel (20) n bytes

        clipboard_virtual_channel.process_client_message(
            22,
              CHANNELS::CHANNEL_FLAG_FIRST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x09\x00\x01\x00\x0e\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" // ............test
                ),
            16);

    RED_CHECK_EQUAL(get_file_contents(file_test), "test");

        clipboard_virtual_channel.process_client_message(
            22,
              CHANNELS::CHANNEL_FLAG_LAST
            | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            byte_ptr_cast(
        /* 0000 */ "\x20\x20\x74\x65\x73\x74"                         //   test....
                ),
            6);

    RED_CHECK_EQUAL(to_server_sender.streams.size(), 6);

    RED_CHECK_EQUAL(get_file_contents(file_test), "test  test");

    clipboard_virtual_channel.DLP_antivirus_check_channels_files();

    RED_CHECK_EQUAL(to_server_sender.streams.size(), 7);

//    auto expected_pdu =
//    /* 0000 */ "\x09\x00\x01\x00\x0e\x00\x00\x00\x01\x00\x00\x00\x74\x65\x73\x74" //............test
//    /* 0001 */ "\x20\x20\x74\x65\x73\x74"                         //   test....
//    ""_av
//    ;

//    RED_CHECK_MEM(expected_pdu, make_array_view(to_server_sender.streams[6].get_data(), to_server_sender.streams[6].in_remain()));


// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: Unlock Clipboard Data PDU
// INFO (4749/4749) -- ClipboardVirtualChannel::process_client_message: clipDataId=1
// INFO (4749/4749) -- Sending on channel (12) n bytes
// INFO (4749/4749) -- \x00\x00\x00\x01
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- \x0\x0\x0\x13
// INFO (4749/4749) -- \x0\x0\x0\xc
// INFO (4749/4749) -- /* 0000 */ "\x0b\x00\x00\x00\x04\x00\x00\x00\x01\x00\x00\x00"                 // ............
// INFO (4749/4749) -- Sent dumped on channel (12) n bytes

    RED_CHECK_WORKSPACE(wd);
}

