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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_authorisation.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPDownDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_down_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPUpDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_up_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
        clipboard_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, clipboard_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPFullDenied)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);

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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

    #include "fixtures/test_cliprdr_channel_xfreerdp_full_denied.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

    NullSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, front,
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

    clipboard_virtual_channel_params.log_only_relevant_clipboard_activities = false;

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
