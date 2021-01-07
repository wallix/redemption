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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "test_only/transport/test_transport.hpp"
#include "test_only/fake_stat.hpp"
#include "test_only/lcg_random.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "core/RDP/clipboard.hpp"
#include "utils/timebase.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "utils/sugar/algostring.hpp"
#include "capture/cryptofile.hpp"
#include "core/log_id.hpp"
#include "capture/fdx_capture.hpp"
#include "mod/file_validator_service.hpp"
#include "acl/auth_api.hpp"
#include "gdi/osd_api.hpp"

#include "./test_channel.hpp"

namespace
{
    namespace data_full_auth
    {
        #include "fixtures/test_cliprdr_channel_xfreerdp_full_authorisation.hpp"
        const auto cb_params = []{ /*NOLINT*/
            ClipboardVirtualChannelParams clipboard_virtual_channel_params;
            clipboard_virtual_channel_params.clipboard_up_authorized   = true;
            clipboard_virtual_channel_params.clipboard_down_authorized = true;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            return clipboard_virtual_channel_params;
        }();
    } // namespace data_full_auth

    namespace data_down_denied
    {
        #include "fixtures/test_cliprdr_channel_xfreerdp_down_denied.hpp"
        const auto cb_params = []{ /*NOLINT*/
            ClipboardVirtualChannelParams clipboard_virtual_channel_params;
            clipboard_virtual_channel_params.clipboard_up_authorized   = true;
            clipboard_virtual_channel_params.clipboard_down_authorized = false;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            return clipboard_virtual_channel_params;
        }();
    } // namespace data_down_denied

    namespace data_up_denied
    {
        #include "fixtures/test_cliprdr_channel_xfreerdp_up_denied.hpp"
        const auto cb_params = []{ /*NOLINT*/
            ClipboardVirtualChannelParams clipboard_virtual_channel_params;
            clipboard_virtual_channel_params.clipboard_up_authorized   = false;
            clipboard_virtual_channel_params.clipboard_down_authorized = true;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            return clipboard_virtual_channel_params;
        }();
    } // namespace data_up_denied

    namespace data_full_denied
    {
        #include "fixtures/test_cliprdr_channel_xfreerdp_full_denied.hpp"
        const auto cb_params = []{ /*NOLINT*/
            ClipboardVirtualChannelParams clipboard_virtual_channel_params;
            clipboard_virtual_channel_params.clipboard_up_authorized   = false;
            clipboard_virtual_channel_params.clipboard_down_authorized = false;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            return clipboard_virtual_channel_params;
        }();
    } // namespace data_full_denied

    constexpr uint32_t first_last_flags
        = CHANNELS::CHANNEL_FLAG_FIRST
        | CHANNELS::CHANNEL_FLAG_LAST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    constexpr uint32_t first_flags
        = CHANNELS::CHANNEL_FLAG_FIRST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    constexpr uint32_t last_flags
        = CHANNELS::CHANNEL_FLAG_LAST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    gdi::NullOsd osd;
} // anonymous namespace

RED_AUTO_TEST_CASE(TestCliprdrChannelXfreeRDPAuthorisation)
{
    FileValidatorService * ipca_service = nullptr;
    NullSessionLog session_log;
    EventContainer events;

    struct D
    {
        char const* name;
        ClipboardVirtualChannelParams cb_params;
        bytes_view indata;
        bytes_view outdata;
    };

    #define F(name) D{#name,           \
        name::cb_params,               \
        cstr_array_view(name::indata), \
        cstr_array_view(name::outdata) \
    }

    RED_TEST_CONTEXT_DATA(D const& d, d.name, {
        F(data_full_auth),
        F(data_down_denied),
        F(data_up_denied),
        F(data_full_denied)
    })
    {
        TestTransport t(d.indata, d.outdata);

        TestToClientSender to_client_sender(t);
        TestToServerSender to_server_sender(t);

        ClipboardVirtualChannel clipboard_virtual_channel(
            &to_client_sender, &to_server_sender, events, osd,
            d.cb_params, ipca_service, {nullptr, false, std::string()},
            session_log, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

        RED_CHECK_EXCEPTION_ERROR_ID(
            CHECK_CHANNEL(t, clipboard_virtual_channel),
            ERR_TRANSPORT_NO_MORE_DATA);
    }
}


class NullSender : public VirtualChannelDataSender
{
public:
    void operator() (uint32_t /*total_length*/, uint32_t /*flags*/, bytes_view /*chunk_data*/) override {}
};

RED_AUTO_TEST_CASE(TestCliprdrChannelMalformedFormatListPDU)
{
    NullSessionLog session_log;
    FileValidatorService * ipca_service = nullptr;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;
    clipboard_virtual_channel_params.clipboard_down_authorized = true;
    clipboard_virtual_channel_params.clipboard_up_authorized   = true;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    NullSender to_client_sender;
    NullSender to_server_sender;

    EventContainer events;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, events, osd, clipboard_virtual_channel_params, ipca_service, {nullptr, false, std::string()},
        session_log, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

    uint8_t  virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
    InStream virtual_channel_stream(virtual_channel_data);

    StaticOutStream<256> out_s;
    Cliprdr::format_list_serialize_with_header(
        out_s, Cliprdr::IsLongFormat(true),
        std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

    const size_t totalLength = out_s.get_offset();

    clipboard_virtual_channel.process_client_message(
            totalLength, first_last_flags, out_s.get_produced_bytes());
}

RED_AUTO_TEST_CASE(TestCliprdrChannelFailedFormatDataResponsePDU)
{
    EventContainer events;
    NullSessionLog session_log;
    FileValidatorService * ipca_service = nullptr;

    ClipboardVirtualChannelParams clipboard_virtual_channel_params;
    clipboard_virtual_channel_params.clipboard_down_authorized = true;
    clipboard_virtual_channel_params.clipboard_up_authorized   = true;
    clipboard_virtual_channel_params.clipboard_file_authorized = true;

    NullSender to_client_sender;
    NullSender to_server_sender;

    ClipboardVirtualChannel clipboard_virtual_channel(
        &to_client_sender, &to_server_sender, events, osd,
        clipboard_virtual_channel_params, ipca_service, {nullptr, false, std::string()},
        session_log, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

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

    clipboard_virtual_channel.process_server_message(28, first_last_flags,
        /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
        /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00"                 // ............
                ""_av);

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
            24, first_last_flags,
    /* 0000 */ "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" // ................
    /* 0010 */ "\x02\x00\x00\x00\x1e\x00\x00\x00"                                 // ........
                ""_av);

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
            130, first_last_flags,
        /* 0000 */ "\x02\x00\x00\x00\x7a\x00\x00\x00\x6e\xc0\x00\x00\x46\x00\x69\x00" // ....z...n...F.i.
        /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
        /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
        /* 0030 */ "\x72\x00\x57\x00\x00\x00\x94\xc0\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
        /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
        /* 0050 */ "\x73\x00\x00\x00\x10\xc1\x00\x00\x50\x00\x72\x00\x65\x00\x66\x00" // s.......P.r.e.f.
        /* 0060 */ "\x65\x00\x72\x00\x72\x00\x65\x00\x64\x00\x20\x00\x44\x00\x72\x00" // e.r.r.e.d. .D.r.
        /* 0070 */ "\x6f\x00\x70\x00\x45\x00\x66\x00\x66\x00\x65\x00\x63\x00\x74\x00" // o.p.E.f.f.e.c.t.
        /* 0080 */ "\x00\x00"                                                         // ..
                ""_av);

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
            16, first_last_flags,
        /* 0000 */ "\x04\x00\x00\x00\x04\x00\x00\x00\x6e\xc0\x00\x00\x00\x00\x00\x00"_av);

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
            8, first_last_flags,
        /* 0000 */ "\x05\x00\x02\x00\x00\x00\x00\x00"_av);
}


namespace
{
    struct Msg
    {
        struct Av : ut::flagged_bytes_view
        {
            Av(bytes_view bytes)
            : ut::flagged_bytes_view{bytes, ut::default_pattern_view, ut::default_ascii_min_len}
            {}

            template<class... Ts>
            Av(Ts&&... xs)
            : ut::flagged_bytes_view{xs...}
            {}
        };


        struct ToMod
        {
            uint32_t total_length;
            uint32_t flags;
            Av av;
        };

        struct ToFront
        {
            uint32_t total_length;
            uint32_t flags;
            Av av;
        };

        struct ToValidator
        {
            Av av;
        };

        struct Log6
        {
            Av av;
        };

        struct Nothing {};
        struct Missing {};

        int type;
        union
        {
            Log6 log6;
            ToMod to_mod;
            ToFront to_front;
            ToValidator to_validator;
        };

        struct OutputData
        {
            std::size_t other_av_len = 0;
            std::size_t mismatch_position = 0;
            ut::PatternView pattern_view = ut::PatternView::deduced;
            bool show_mismatch_position = false;
        };
        mutable OutputData output_data;

        Msg(Log6 log6) : type(0), log6(log6) {}
        Msg(ToMod to_mod) : type(1), to_mod(to_mod) {}
        Msg(ToFront to_front) : type(2), to_front(to_front) {}
        Msg(ToValidator to_validator) : type(3), to_validator(to_validator) {}
        Msg(Nothing) : type(4) {}
        Msg(Missing) : type(5) {}

        bool operator == (Msg const& other) const
        {
            other.output_data.show_mismatch_position = true;

            auto check_av = [&](Av const& lhs, Av const& rhs){
                this->output_data.other_av_len = rhs.bytes.size();
                other.output_data.other_av_len = lhs.bytes.size();
                this->output_data.pattern_view = rhs.pattern;
                other.output_data.pattern_view = lhs.pattern;
                bool r = ut::compare_bytes(
                    this->output_data.mismatch_position, lhs.bytes, rhs.bytes);
                other.output_data.mismatch_position = this->output_data.mismatch_position;
                return r;
            };

            if (this->type == other.type)
            {
                auto check_chann = [&](auto& lhs, auto& rhs){
                    return
                        check_av(lhs.av, rhs.av)
                     && lhs.total_length == rhs.total_length
                     && lhs.flags == rhs.flags
                     ;
                };

                switch (other.type)
                {
                    case 0: return check_av(this->log6.av, other.log6.av);
                    case 1: return check_chann(this->to_mod, other.to_mod);
                    case 2: return check_chann(this->to_front, other.to_front);
                    case 3: return check_av(this->to_validator.av, other.to_validator.av);
                }
            }

            auto get_av = [](Msg const& msg){
                switch (msg.type)
                {
                    case 0: return msg.log6.av;
                    case 1: return msg.to_mod.av;
                    case 2: return msg.to_front.av;
                    case 3: return msg.to_validator.av;
                }
                return Av{};
            };

            check_av(get_av(*this), get_av(other));

            return false;
        }

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
        friend std::ostream& operator<<(std::ostream& out, Msg const& msg)
        {
            char const* names[]{
                "Log6", "ToMod", "ToFront", "ToValidator", "Nothing", "Missing"
            };

            out << names[msg.type] << "{";

            auto const pos = msg.output_data.mismatch_position;
            std::size_t av_len = 0;

            auto put_av = [&](Av av){
                av_len = av.bytes.size();

                if (msg.output_data.other_av_len == av_len
                    && msg.output_data.mismatch_position == av_len
                ) {
                    out << "..._av";
                    return ;
                }

                if (av.pattern == ut::PatternView::deduced) {
                    av.pattern = msg.output_data.pattern_view;
                }
                ut::put_view(pos, out, av);
                out << "_av";
            };

            auto put_chann = [&](auto data){
                out << data.total_length;
                switch (data.flags)
                {
                    case first_last_flags: out << ", first_last_flags, "; break;
                    case first_flags: out << ", first_flags, "; break;
                    case last_flags: out << ", last_flags, "; break;
                    default: out << ", " << data.flags << ", "; break;
                }
                put_av(data.av);
            };

            switch (msg.type)
            {
                case 0: put_av(msg.log6.av); break;
                case 1: put_chann(msg.to_mod); break;
                case 2: put_chann(msg.to_front); break;
                case 3: put_av(msg.to_validator.av); break;
            }

            out << "}";

            if (msg.output_data.other_av_len != av_len
                && msg.output_data.show_mismatch_position
            ) {
                out << "]\nDatas size mismatch: "
                    << msg.output_data.other_av_len << " != " << av_len;
            }

            return out;
        }
#endif
    };

    inline auto ininit_msg_comparator_compare = +[](void*, Msg const&)
    {
        // maybe used with dtor of ClipboardVirtualChannel
        RED_REQUIRE(false);
        return true;
    };

    struct MsgComparator
    {
        using func_t = bool(*)(void*, Msg const&);

        struct ExceptionLock
        {
            MsgComparator& self;

            ~ExceptionLock()
            {
                if (std::uncaught_exceptions()) {
                    self.has_exception = true;
                }
            }
        };

        template<class Fn, class Process, class... Fns>
        void run(Fn fn, Process&& process, Fns... fns)
        {
            struct Tuple : Fn, Fns... {} t {fn, fns...};
            func_t table[]{
                func_t([](void* data, Msg const& msg){
                    return static_cast<Fn&>(*static_cast<Tuple*>(data))(msg);
                }),
                func_t([](void* data, Msg const& msg){
                    return static_cast<Fns&>(*static_cast<Tuple*>(data))(msg);
                })...
            };
            auto av_table = make_array_view(table);
            this->fn_table = av_table;
            this->index_table = 1;
            this->data = &t;

            ExceptionLock lock{*this};
            process();

            for (auto f : av_table.from_offset(this->index_table)) {
                f(&t, Msg::Nothing{});
            }

            this->fn_table = {&ininit_msg_comparator_compare, 1};
            this->index_table = 1;
        }

        array_view<func_t> fn_table {&ininit_msg_comparator_compare, 1};
        std::size_t index_table = 0;
        void* data;
        bool has_exception = false;

        void push(Msg const& msg)
        {
            if (this->has_exception) {
                return ;
            }

            for(;;) {
                if (this->index_table < this->fn_table.size()) {
                    bool r = this->fn_table[this->index_table](this->data, msg);
                    ++this->index_table;
                    if (r) {
                        return;
                    }
                }
                else {
                    this->fn_table[0](this->data, msg);
                    return;
                }
            }
        }
    };

#define TEST_PROCESS TEST_BUF(Msg::Missing{}), [&]

#define TEST_BUF(...) [&](Msg const& msg_) { \
    [&](Msg const& expected) {               \
        RED_CHECK(expected == msg_);         \
    }(Msg(__VA_ARGS__));                     \
    return true;                             \
}

#define TEST_BUF_IF(cond, ...) [&](Msg const& msg_) { \
    if (cond) {                                       \
        RED_CHECK(Msg(__VA_ARGS__) == msg_);          \
        return true;                                  \
    }                                                 \
    return false;                                     \
}


    class FrontSenderTest : public VirtualChannelDataSender
    {
        MsgComparator& msg_comparator;

    public:
        explicit FrontSenderTest(MsgComparator& msg_comparator)
        : msg_comparator(msg_comparator)
        {}

        void operator()(uint32_t total_length, uint32_t flags, bytes_view chunk_data) override
        {
            msg_comparator.push(Msg::ToFront{total_length, flags, chunk_data});
        }
    };

    class ModSenderTest : public VirtualChannelDataSender
    {
        MsgComparator& msg_comparator;

    public:
        explicit ModSenderTest(MsgComparator& msg_comparator)
        : msg_comparator(msg_comparator)
        {}

        void operator()(uint32_t total_length, uint32_t flags, bytes_view chunk_data) override
        {
            msg_comparator.push(Msg::ToMod{total_length, flags, chunk_data});
        }
    };

    class ReportMessageTest : public NullSessionLog
    {
        MsgComparator& msg_comparator;

    public:
        ReportMessageTest(MsgComparator& msg_comparator)
        : msg_comparator(msg_comparator)
        {}

        void log6(LogId id, KVLogList kv_list) override
        {
            std::string s = detail::log_id_string_map[int(id)].data();
            for (auto& kv : kv_list) {
                str_append(s, ' ', kv.key, '=', kv.value);
            }
            this->msg_comparator.push(Msg::Log6{bytes_view(s)});
        }
    };

    class ValidatorTransportTest : public Transport
    {
        MsgComparator& msg_comparator;

    public:
        bytes_view buf_reader {};

        ValidatorTransportTest(MsgComparator& msg_comparator)
        : msg_comparator(msg_comparator)
        {}

        void do_send(const uint8_t * buffer, std::size_t len) override
        {
            this->msg_comparator.push(Msg::ToValidator{bytes_view(buffer, len)});
        }

        size_t do_partial_read(uint8_t* buffer, size_t len) override
        {
            len = std::min(len, this->buf_reader.size());
            memcpy(buffer, this->buf_reader.data(), len);
            this->buf_reader = this->buf_reader.from_offset(len);
            return len;
        }
    };

    constexpr auto use_long_format = Cliprdr::IsLongFormat(true);
    constexpr auto file_group = Cliprdr::formats::file_group_descriptor_w.ascii_name;
    constexpr uint32_t file_group_id = 49262;

    struct Buffer
    {
        StaticOutStream<1600> out {};

        template<class F>
        bytes_view build(uint16_t msgType, uint16_t msgFlags, F f) &
        {
            auto av = out.out_skip_bytes(RDPECLIP::CliprdrHeader::size());
            f(this->out);
            uint32_t data_len = uint32_t(out.get_offset() - av.size());
            OutStream stream_header(av);
            RDPECLIP::CliprdrHeader(msgType, msgFlags, data_len).emit(stream_header);
            return out.get_produced_bytes();
        }

        template<class F>
        bytes_view build_ok(uint16_t msgType, F f) &
        {
            return this->build(msgType, RDPECLIP::CB_RESPONSE_OK, f);
        }

        template<class F>
        bytes_view build_ok(uint16_t msgType, uint32_t data_len, F f) &
        {
            auto av = out.out_skip_bytes(RDPECLIP::CliprdrHeader::size());
            f(this->out);
            OutStream stream_header(av);
            RDPECLIP::CliprdrHeader(msgType, RDPECLIP::CB_RESPONSE_OK, data_len).emit(stream_header);
            return out.get_produced_bytes();
        }

        template<class F>
        bytes_view build_fail(uint16_t msgType, F f) &
        {
            return this->build(msgType, RDPECLIP::CB_RESPONSE_FAIL, f);
        }

        bytes_view build_format_list(Cliprdr::FormatNameRef format_name) &
        {
            Cliprdr::format_list_serialize_with_header(
                out,
                use_long_format,
                std::array{format_name});
            return out.get_produced_bytes();
        }

        bytes_view build_format_list_with_file() &
        {
            return this->build_format_list(Cliprdr::FormatNameRef{file_group_id, file_group});
        }

        bytes_view build_format_list_with_text() &
        {
            return this->build_format_list(Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, nullptr});
        }
    };


    using namespace std::string_view_literals;

    inline constexpr auto fdx_basename = "sid,blabla.fdx"sv;
    inline constexpr auto sid = "my_session_id"sv;

    struct FdxTestCtx
    {
        FdxTestCtx(char const* name)
        : wd(name)
        {}

        WorkingDirectory wd;
        WorkingDirectory::SubDirectory record_path = wd.create_subdirectory("record");
        WorkingDirectory::SubDirectory hash_path = wd.create_subdirectory("hash");
        WorkingDirectory::SubDirectory fdx_record_path = record_path.create_subdirectory(sid);
        WorkingDirectory::SubDirectory fdx_hash_path = hash_path.create_subdirectory(sid);
        CryptoContext cctx;
        LCGRandom rnd;
        FakeFstat fstat;
        FdxCapture fdx = FdxCapture{
            record_path.dirname().string(),
            hash_path.dirname().string(),
            "sid,blabla", sid, -1, 0660, cctx, rnd, fstat,
            [](const Error & /*error*/){}};
    };

    auto add_file(FdxTestCtx& data_test, std::string_view suffix)
    {
        auto basename = str_concat(sid, suffix);
        auto path = data_test.fdx_record_path.add_file(basename);
        (void)data_test.fdx_hash_path.add_file(basename);
        return path;
    }

    using ValidationResult = LocalFileValidatorProtocol::ValidationResult;

    struct ClipDataTest
    {
        bool with_validator;
        bool with_fdx_capture;
        bool always_file_storage;
        bool verify_before_transfer = false;
        ValidationResult validation_result = ValidationResult::Wait;

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
        friend std::ostream& operator<<(std::ostream& out, ClipDataTest const& d)
        {
            out <<
                "with validator: " << d.with_validator <<
                "  with fdx: " << d.with_fdx_capture <<
                "  always_file_storage: " << d.always_file_storage <<
                "  verify_before_transfer: " << d.verify_before_transfer
            ;

            if (d.validation_result == ValidationResult::IsAccepted) {
                out << "  validation_result: IsAccepted";
            }
            if (d.validation_result == ValidationResult::IsRejected) {
                out << "  validation_result: IsRejected";
            }

            return out;
        }
#endif

        ClipboardVirtualChannelParams default_channel_params() const
        {
            ClipboardVirtualChannelParams clipboard_virtual_channel_params {};
            clipboard_virtual_channel_params.clipboard_down_authorized = true;
            clipboard_virtual_channel_params.clipboard_up_authorized   = true;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            clipboard_virtual_channel_params.validator_params.down_target_name = "down";
            clipboard_virtual_channel_params.validator_params.up_target_name = "up";
            clipboard_virtual_channel_params.validator_params.log_if_accepted = true;
            clipboard_virtual_channel_params.validator_params.enable_clipboard_text_up = true;
            clipboard_virtual_channel_params.validator_params.enable_clipboard_text_down = true;
            clipboard_virtual_channel_params.validator_params.block_invalid_file_down
                = this->verify_before_transfer;
            clipboard_virtual_channel_params.validator_params.block_invalid_file_up
                = this->verify_before_transfer;
            return clipboard_virtual_channel_params;
        }

        std::unique_ptr<FdxTestCtx> make_optional_fdx_ctx() const
        {
            if (this->with_fdx_capture) {
                return std::make_unique<FdxTestCtx>(
                    &"abcdefgh"[this->with_validator * 2 + this->always_file_storage]
                );
            }
            return std::unique_ptr<FdxTestCtx>();
        }

        class ChannelCtx
        {
            ReportMessageTest report_message;
            ValidatorTransportTest validator_transport;
            FileValidatorService file_validator_service{validator_transport};

            FrontSenderTest to_client_sender;
            ModSenderTest to_server_sender;

            EventContainer events;

            ClipboardVirtualChannel clipboard_virtual_channel;

        public:
            ChannelCtx(
                MsgComparator& msg_comparator,
                FdxTestCtx* fdx_ctx,
                ClipboardVirtualChannelParams clipboard_virtual_channel_params,
                ClipDataTest const& d, RDPVerbose verbose)
            : report_message(msg_comparator)
            , validator_transport(msg_comparator)
            , file_validator_service(validator_transport)
            , to_client_sender(msg_comparator)
            , to_server_sender(msg_comparator)
            , clipboard_virtual_channel(
                &to_client_sender, &to_server_sender,
                events, osd,
                clipboard_virtual_channel_params,
                d.with_validator ? &file_validator_service : nullptr,
                ClipboardVirtualChannel::FileStorage{
                    fdx_ctx ? &fdx_ctx->fdx : nullptr,
                    d.always_file_storage,
                    fdx_ctx ? fdx_ctx->wd.dirname().string() : std::string()
                },
                report_message,
                verbose
            )
            {}

            bool dlp_message_accept(FileValidatorId id)
            {
                return this->dlp_message(ValidationResult::IsAccepted, id);
            }

            bool dlp_message(ValidationResult result, FileValidatorId id)
            {
                StaticOutStream<256> out;
                auto status = (result == ValidationResult::IsAccepted)
                    ? "ok"_av
                    : "fail"_av;

                using namespace LocalFileValidatorProtocol;
                FileValidatorHeader(MsgType::Result, 0/*len*/).emit(out);
                FileValidatorResultHeader{result, id, checked_int(status.size())}.emit(out);
                out.out_copy_bytes(status);

                this->validator_transport.buf_reader = out.get_produced_bytes();
                this->clipboard_virtual_channel.DLP_antivirus_check_channels_files();
                return this->validator_transport.buf_reader.empty();
            }

            void process_server_message(
                bytes_view av,
                uint32_t total_len = -1u,
                uint32_t flags = first_last_flags)
            {
                if (total_len == -1u) {
                    total_len = uint32_t(av.size());
                }
                flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                this->clipboard_virtual_channel.process_server_message(total_len, flags, av);
            }

            void process_client_message(
                bytes_view av,
                uint32_t total_len = -1u,
                uint32_t flags = first_last_flags)
            {
                if (total_len == -1u) {
                    total_len = uint32_t(av.size());
                }
                flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                this->clipboard_virtual_channel.process_client_message(total_len, flags, av);
            }
        };
    };
} // anonymous namespace

// TODO move err count condition to RED_AUTO_TEST_CONTEXT_DATA
#define RED_AUTO_TEST_CLIPRDR(test_name, type_value, iocontext, ...) \
    void test_name##__case(type_value);                              \
    RED_AUTO_TEST_CASE(test_name) {                                  \
        auto l = __VA_ARGS__;                                        \
        auto p = l.begin();                                          \
        RED_TEST_CONTEXT_DATA(type_value, iocontext, l)              \
        {                                                            \
            auto const err_count = RED_ERROR_COUNT();                \
            test_name##__case(*p);                                   \
            if (err_count != RED_ERROR_COUNT()) {                    \
                break;                                               \
            };                                                       \
            ++p;                                                     \
        }                                                            \
    }                                                                \
    void test_name##__case(type_value)

namespace
{
    void initialize_channel(
        MsgComparator& msg_comparator,
        ClipDataTest::ChannelCtx& channel_ctx,
        uint16_t capabilities,
        bool format_list_by_server = true)
    {
        bytes_view temp_av;

        using namespace RDPECLIP;
        Buffer buf;
        temp_av = buf.build(CB_CLIP_CAPS, 0, [&](OutStream& out){
            out.out_uint16_le(CB_CAPSTYPE_GENERAL);
            out.out_clear_bytes(2);
            GeneralCapabilitySet{CB_CAPS_VERSION_1, capabilities}.emit(out);
        });

        msg_comparator.run(
            TEST_PROCESS { channel_ctx.process_server_message(temp_av); },
            TEST_BUF(Msg::ToFront{24, first_last_flags, temp_av})
        );

        msg_comparator.run(
            TEST_PROCESS { channel_ctx.process_client_message(temp_av); },
            TEST_BUF(Msg::ToMod{24, first_last_flags, temp_av})
        );

        msg_comparator.run(
            TEST_PROCESS {
                Buffer buf;
                temp_av = buf.build_format_list_with_file();
                if (format_list_by_server) {
                    channel_ctx.process_server_message(temp_av);
                }
                else {
                    channel_ctx.process_client_message(temp_av);
                }
            },
            TEST_BUF_IF(format_list_by_server, Msg::ToFront{54, first_last_flags, temp_av}),
            TEST_BUF_IF(not format_list_by_server, Msg::ToMod{54, first_last_flags, temp_av})
        );
    }
} // anonymous namespace

RED_AUTO_TEST_CLIPRDR(TestCliprdrChannelFilterDataFileWithoutLock, ClipDataTest const& d, d, {
    //           icap  fdx  storage verify
    ClipDataTest{true, false, true, false},
    ClipDataTest{true, true, false, false},
    ClipDataTest{true, true, true, false},

    ClipDataTest{false, false, true, false},
    ClipDataTest{false, true, false, false},
    ClipDataTest{false, true, true, false}
})
{
    bytes_view temp_av;
    MsgComparator msg_comparator;
    auto fdx_ctx = d.make_optional_fdx_ctx();
    auto channel_ctx = std::make_unique<ClipDataTest::ChannelCtx>(
        msg_comparator,
        fdx_ctx.get(),
        d.default_channel_params(),
        d, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

   initialize_channel(msg_comparator, *channel_ctx, RDPECLIP::CB_USE_LONG_FORMAT_NAMES, false);

   // skip format list response

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);    // count item
                out.out_uint32_le(RDPECLIP::FD_FILESIZE);    // flags
                out.out_clear_bytes(32); // reserved1
                out.out_uint32_le(0);    // attributes
                out.out_clear_bytes(16); // reserved2
                out.out_uint64_le(0);    // lastWriteTime
                out.out_uint32_le(0);    // file size high
                out.out_uint32_le(12);   // file size low
                auto filename = "abc"_utf16_le;
                out.out_copy_bytes(filename);
                out.out_clear_bytes(520u - filename.size());
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION"
            " format=FileGroupDescriptorW(49262) size=596"_av}),
        TEST_BUF(Msg::ToMod{604, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace RDPECLIP;
            Buffer buf;
            temp_av = buf.build_ok(CB_FILECONTENTS_REQUEST, [&](OutStream& out){
                FileContentsRequestPDU(0, 0, FILECONTENTS_RANGE, 0, 0, 12, 0, true).emit(out);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{36, first_last_flags, temp_av})
    );

    // file content

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
                out.out_copy_bytes("data_abcdefg"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x01\x00\x02up"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x10\x00\x00\x00\x01"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"data_abcdefg"_av}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av}),
        TEST_BUF_IF(!d.verify_before_transfer, Msg::ToMod{24, first_last_flags, temp_av}),
        // without "data_abcdefg"
        TEST_BUF_IF(d.verify_before_transfer, Msg::ToMod{24, first_flags, temp_av.first(12)})
    );

    if (d.with_validator) {
        msg_comparator.run(
            TEST_PROCESS { RED_TEST(channel_ctx->dlp_message_accept(FileValidatorId(1))); },
            TEST_BUF(Msg::Log6{
                "FILE_VERIFICATION direction=UP file_name=abc status=ok"_av}),
            TEST_BUF_IF(d.verify_before_transfer, Msg::ToMod{24, last_flags, "data_abcdefg"_av})
        );
    }

    if (fdx_ctx && d.always_file_storage) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000001.tfl"), "data_abcdefg"sv);
    }

    // check TEXT_VALIDATION

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_format_list_with_text();
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::ToMod{14, first_last_flags, temp_av})
    );

    // skip format list response

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(RDPECLIP::CF_UNICODETEXT);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_copy_bytes("a\0b\0c\0"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x02\x00\x02up"
            "\x00\x01\x00\x09""format_id\x00\x02""13"_av
        }),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"
            " format=CF_UNICODETEXT(13) size=6 partial_data=abc"_av
        }),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x0a\x00\x00\x00\x02"_av
        }),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"a\0b\0c\0"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x03\x00\x00\x00\x04\x00\x00\x00\x02"_av
        }),
        TEST_BUF(Msg::ToMod{14, first_last_flags, temp_av})
    );

    if (d.with_validator) {
        msg_comparator.run(
            TEST_PROCESS { RED_TEST(channel_ctx->dlp_message_accept(FileValidatorId(2))); },
            TEST_BUF(Msg::Log6{
                "TEXT_VERIFICATION direction=UP copy_id=2 status=ok"_av})
        );
    }

    msg_comparator.run(TEST_PROCESS {
        channel_ctx.reset();
    });

    if (fdx_ctx) {
        (void)fdx_ctx->hash_path.add_file(fdx_basename);
        auto fdx_path = fdx_ctx->record_path.add_file(fdx_basename);

        OutCryptoTransport::HashArray qhash;
        OutCryptoTransport::HashArray fhash;
        fdx_ctx->fdx.close(qhash, fhash);

        RED_CHECK_WORKSPACE(fdx_ctx->wd);

        if (d.always_file_storage) {
            RED_CHECK_FILE_CONTENTS(fdx_path,
                "v3\n\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00,\x03\x00\x26\x00""abcmy_session_id/my_session_id,000001.tfl\xd1\xb9\xc9"
                "\xdb""E\\p\xb7\xc6\xa7\x02%\xa0\x0f\x85\x99""1\xe4\x98\xf7\xf5\xe0"
                "\x7f,\x96.\x10x\xc0""5\x9f^"_av);
        }
        else {
            RED_CHECK_FILE_CONTENTS(fdx_path, "v3\n"_av);
        }
    }
}

RED_AUTO_TEST_CLIPRDR(TestCliprdrChannelFilterDataMultiFileWithLock, ClipDataTest const& d, d, {
    //                       always
    //           icap  fdx  storage verify=false
    ClipDataTest{true, false, true},
    ClipDataTest{true, true, false},
    ClipDataTest{true, true, true},
    ClipDataTest{false, false, true},
    ClipDataTest{false, true, false},
    ClipDataTest{false, true, true},
})
{
    bytes_view temp_av;
    MsgComparator msg_comparator;
    auto fdx_ctx = d.make_optional_fdx_ctx();
    auto channel_ctx = std::make_unique<ClipDataTest::ChannelCtx>(
        msg_comparator,
        fdx_ctx.get(),
        d.default_channel_params(),
        d, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

   initialize_channel(msg_comparator, *channel_ctx,
        RDPECLIP::CB_CAN_LOCK_CLIPDATA | RDPECLIP::CB_USE_LONG_FORMAT_NAMES, false);

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_LOCK_CLIPDATA, 0, [&](OutStream& out){
                out.out_uint32_le(0);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_LIST_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);    // count item
                out.out_uint32_le(RDPECLIP::FD_FILESIZE);    // flags
                out.out_clear_bytes(32); // reserved1
                out.out_uint32_le(0);    // attributes
                out.out_clear_bytes(16); // reserved2
                out.out_uint64_le(0);    // lastWriteTime
                out.out_uint32_le(0);    // file size high
                out.out_uint32_le(12);   // file size low
                auto filename = "abc"_utf16_le;
                out.out_copy_bytes(filename);
                out.out_clear_bytes(520u - filename.size());
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION"
            " format=FileGroupDescriptorW(49262) size=596"_av}),
        TEST_BUF(Msg::ToMod{604, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace RDPECLIP;
            Buffer buf;
            temp_av = buf.build_ok(CB_FILECONTENTS_REQUEST, [&](OutStream& out){
                FileContentsRequestPDU(0, 0, FILECONTENTS_RANGE, 0, 0, 5, 0, true).emit(out);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{36, first_last_flags, temp_av})
    );

    // (partial) file content (2 packets)

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
                out.out_copy_bytes("da"_av);
            });
            channel_ctx->process_client_message(temp_av, -1u, CHANNELS::CHANNEL_FLAG_FIRST);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x01\x00\x02up\x00\x01\x00\b"
            "filename\x00\x03""abc"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x06\x00\x00\x00\x01"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"da"_av}),
        TEST_BUF(Msg::ToMod{14, first_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_client_message("ta_"_av, -1u, CHANNELS::CHANNEL_FLAG_LAST);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x07\x00\x00\x00\x01"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"ta_"_av}),
        TEST_BUF(Msg::ToMod{3, last_flags, "ta_"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_format_list_with_file();
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::ToMod{54, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_LOCK_CLIPDATA, 0, [&](OutStream& out){
                out.out_uint32_le(1);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_ok(
                RDPECLIP::CB_FORMAT_LIST_RESPONSE, [&](OutStream& out){
                    out.out_uint32_le(file_group_id);
                });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(file_group_id);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);    // count item
                out.out_uint32_le(RDPECLIP::FD_FILESIZE);    // flags
                out.out_clear_bytes(32); // reserved1
                out.out_uint32_le(0);    // attributes
                out.out_clear_bytes(16); // reserved2
                out.out_uint64_le(0);    // lastWriteTime
                out.out_uint32_le(0);    // file size high
                out.out_uint32_le(10);   // file size low
                auto filename = "def"_utf16_le;
                out.out_copy_bytes(filename);
                out.out_clear_bytes(520u - filename.size());
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION"
            " format=FileGroupDescriptorW(49262) size=596"_av}),
        TEST_BUF(Msg::ToMod{604, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace RDPECLIP;
            Buffer buf;
            temp_av = buf.build_ok(CB_FILECONTENTS_REQUEST, [&](OutStream& out){
                FileContentsRequestPDU(1, 0, FILECONTENTS_RANGE, 0, 0, 99, 1, true).emit(out);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{36, first_last_flags, temp_av})
    );

    // file content

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes("plopploppl"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x02\x00\x02up\x00\x01\x00\b"
            "filename\x00\x03""def"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x0e\x00\x00\x00\x02"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"plopploppl"_av}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION file_name=def size=10"
            " sha256=dda098fc2804923ceeef1b65f26b78be8789535b7b9dddb6fda8de6a2dacf190"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x03\x00\x00\x00\x04\x00\x00\x00\x02"_av}),
        TEST_BUF(Msg::ToMod{22, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_UNLOCK_CLIPDATA, 0, [&](OutStream& out){
                out.out_uint32_le(1);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace RDPECLIP;
            Buffer buf;
            temp_av = buf.build_ok(CB_FILECONTENTS_REQUEST, [&](OutStream& out){
                FileContentsRequestPDU(0, 0, FILECONTENTS_RANGE, 5, 0, 99, 0, true).emit(out);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{36, first_last_flags, temp_av})
    );

    // (last partial) file content

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
                out.out_copy_bytes("abcdefg"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x01\x00\x00\x00\x0b\x00\x00\x00\x01"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{"abcdefg"_av}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION file_name=abc size=12"
            " sha256=d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
        TEST_BUF_IF(d.with_validator, Msg::ToValidator{
            "\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av}),
        TEST_BUF(Msg::ToMod{19, first_last_flags, temp_av})
    );

    if (d.with_validator) {
        msg_comparator.run(
            TEST_PROCESS { channel_ctx->dlp_message_accept(FileValidatorId(1)); },
            TEST_BUF(Msg::Log6{"FILE_VERIFICATION direction=UP file_name=abc status=ok"_av})
        );
    }

    msg_comparator.run(
        TEST_PROCESS { channel_ctx.reset(); },
        TEST_BUF_IF(d.with_validator, Msg::Log6{
            "FILE_VERIFICATION direction=UP file_name=def status=Connexion closed"_av})
    );

    if (fdx_ctx) {
        if (d.always_file_storage) {
            RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000001.tfl"), "data_abcdefg"sv);
        }

        if (d.with_validator || (d.with_fdx_capture && d.always_file_storage)) {
            RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000002.tfl"), "plopploppl"sv);
        }

        (void)fdx_ctx->hash_path.add_file(fdx_basename);
        auto fdx_path = fdx_ctx->record_path.add_file(fdx_basename);

        OutCryptoTransport::HashArray qhash;
        OutCryptoTransport::HashArray fhash;
        fdx_ctx->fdx.close(qhash, fhash);

        if (d.with_validator && d.always_file_storage) {
            RED_CHECK_FILE_CONTENTS(fdx_path,
                "v3\n\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00,\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000001.tfl\xd1\xb9\xc9"
                "\xdb""E\\p\xb7\xc6\xa7\x02%\xa0\x0f\x85\x99""1\xe4\x98"
                "\xf7\xf5\xe0\x7f,\x96.\x10x\xc0""5\x9f^\x04\x00\x02\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "L\x03\x00&\x00""defmy_session_id/my_session_id,000002.tfl"
                "\xdd\xa0\x98\xfc(\x04\x92<\xee\xef\x1b""e\xf2kx\xbe\x87"
                "\x89S[{\x9d\xdd\xb6\xfd\xa8\xdej-\xac\xf1\x90"_av);
        }
        else if (d.always_file_storage) {
            RED_CHECK_FILE_CONTENTS(fdx_path,
                "v3\n\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00,\x03\x00&\x00""defmy_session_id/my_session_id,000002.tfl"
                "\xdd\xa0\x98\xfc(\x04\x92<\xee\xef\x1b""e\xf2kx\xbe\x87\x89S[{\x9d"
                "\xdd\xb6\xfd\xa8\xdej-\xac\xf1\x90\x04\x00\x01\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00,\x03\x00&\x00"
                "abcmy_session_id/my_session_id,000001.tfl\xd1\xb9\xc9\xdb""E\\p\xb7"
                "\xc6\xa7\x02%\xa0\x0f\x85\x99""1\xe4\x98\xf7\xf5\xe0\x7f,\x96.\x10"
                "x\xc0""5\x9f^"_av);
        }
        else if (d.with_validator && d.with_fdx_capture) {
            RED_CHECK_FILE_CONTENTS(fdx_path,
                "v3\n\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00L\x03\x00&\x00""defmy_session_id/my_session_id,000002.tfl"
                "\xdd\xa0\x98\xfc(\x04\x92<\xee\xef\x1b""e\xf2kx\xbe\x87\x89S[{"
                "\x9d\xdd\xb6\xfd\xa8\xdej-\xac\xf1\x90"_av);
        }
        else {
            RED_CHECK_FILE_CONTENTS(fdx_path, "v3\n"_av);
        }

        RED_CHECK_WORKSPACE(fdx_ctx->wd);
    }
}

namespace
{
    struct RequestedRange
    {
        enum class StreamId : uint32_t;

        MsgComparator& msg_comparator;
        ClipDataTest::ChannelCtx& channel_ctx;
        bytes_view file_contents {};

        Buffer filecontens_request_buf {};
        bytes_view filecontents_request_av {};

        // data_request + data_response of 1 file
        void prepare_file(bytes_view file_contents)
        {
            this->file_contents = file_contents;

            bytes_view av;
            msg_comparator.run(
                TEST_PROCESS {
                    Buffer buf;
                    av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                        out.out_uint32_le(file_group_id);
                    });
                    channel_ctx.process_client_message(av);
                },
                TEST_BUF(Msg::ToMod{12, first_last_flags, av})
            );

            msg_comparator.run(
                TEST_PROCESS {
                    using namespace Cliprdr;
                    Buffer buf;
                    av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                        out.out_uint32_le(1);    // count item
                        out.out_uint32_le(RDPECLIP::FD_FILESIZE);    // flags
                        out.out_clear_bytes(32); // reserved1
                        out.out_uint32_le(0);    // attributes
                        out.out_clear_bytes(16); // reserved2
                        out.out_uint64_le(0);    // lastWriteTime
                        out.out_uint32_le(0);    // file size high
                        out.out_uint32_le(checked_int(file_contents.size())); // file size low
                        auto filename = "abc"_utf16_le;
                        out.out_copy_bytes(filename);
                        out.out_clear_bytes(520u - filename.size());
                    });
                    channel_ctx.process_server_message(av);
                },
                TEST_BUF(Msg::Log6{
                    "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION"
                    " format=FileGroupDescriptorW(49262) size=596"_av}),
                TEST_BUF(Msg::ToFront{604, first_last_flags, av})
            );
        }

        void client_full_file_request(StreamId stream_id = StreamId(0))
        {
            this->client_file_request(0, this->file_contents.size(), stream_id);
        }

        void client_file_request(
            size_t offset, size_t len, StreamId stream_id = StreamId(0))
        {
            using namespace RDPECLIP;

            this->filecontens_request_buf.out.rewind();
            this->filecontents_request_av = this->filecontens_request_buf
            .build_ok(CB_FILECONTENTS_REQUEST, [&](OutStream& out){
                FileContentsRequestPDU(
                     safe_int(stream_id), 0, FILECONTENTS_RANGE,
                     checked_int(offset), 0, checked_int(len), 0, false)
                .emit(out);
            });

            this->channel_ctx.process_client_message(this->filecontents_request_av);
        }
    };

    constexpr chars_view zeros
        = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"_av;
}

RED_AUTO_TEST_CLIPRDR(TestCliprdrValidationBeforeTransfer, ClipDataTest const& d, d, {
    //                       always
    //           icap  fdx  storage verify
    ClipDataTest{true, false, true, true, ValidationResult::IsAccepted},
    ClipDataTest{true, true, false, true, ValidationResult::IsAccepted},
    ClipDataTest{true, true, true, true, ValidationResult::IsAccepted},

    ClipDataTest{true, false, true, true, ValidationResult::IsRejected},
    ClipDataTest{true, true, false, true, ValidationResult::IsRejected},
    ClipDataTest{true, true, true, true, ValidationResult::IsRejected},
})
{
    RED_TEST(d.with_validator);
    RED_TEST(d.verify_before_transfer);

    bytes_view temp_av;
    MsgComparator msg_comparator;
    auto fdx_ctx = d.make_optional_fdx_ctx();

    auto cliprdr_params = d.default_channel_params();
    cliprdr_params.validator_params.max_file_size_rejected = 30;

    auto channel_ctx = std::make_unique<ClipDataTest::ChannelCtx>(
        msg_comparator,
        fdx_ctx.get(),
        cliprdr_params,
        d, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

    initialize_channel(msg_comparator, *channel_ctx, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

    // skip format list response

    const bool is_accepted = d.validation_result == ValidationResult::IsAccepted;
    const bool is_rejected = d.validation_result == ValidationResult::IsRejected;
    const auto file_verification_msg = Msg::Log6{
        is_accepted
          ? "FILE_VERIFICATION direction=DOWN file_name=abc status=ok"_av
          : "FILE_VERIFICATION direction=DOWN file_name=abc status=fail"_av
    };
    const auto file_blocked_msg = Msg::Log6{"FILE_BLOCKED direction=DOWN file_name=abc"_av};

    using StreamId = RequestedRange::StreamId;
    RequestedRange requested{msg_comparator, *channel_ctx};

    requested.prepare_file("data_abcdefg"_av);

    // full file with a packet.size() < 1600
    // req, response, validation_result
    //@{
    msg_comparator.run(
        TEST_PROCESS { requested.client_full_file_request(); },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
                out.out_copy_bytes(requested.file_contents);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x01\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x10\x00\x00\x00\x01"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
        TEST_BUF(Msg::ToValidator{"\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{24, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(1)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg),
        TEST_BUF(Msg::ToFront{24, last_flags, is_accepted
            ? requested.file_contents
            : zeros.first(requested.file_contents.size())})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000001.tfl"), requested.file_contents);
    }
    //@}

    // data already available
    // req, response, req, response
    //@{
    if (is_accepted) {
        msg_comparator.run(
            TEST_PROCESS {
                requested.client_file_request(0, 6, StreamId(1));
            },
            TEST_BUF(Msg::ToFront{18, first_last_flags,
                "\x09\x00\x01\x00\x0a\x00\x00\x00\x01\x00\x00\x00""data_a"_av})
        );
    }

    // big request
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(5, 10000, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{19, first_last_flags,
            "\x09\x00\x01\x00\x0b\x00\x00\x00\x01\x00\x00\x00""abcdefg"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );
    //@}

    requested.prepare_file("data_abcdefgf"_av);

    // full file with 2 requested ranges
    // (req, response) * 2, validation_result, random req, resp
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(2));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x02\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x02"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.from_offset(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0a\x00\x00\x00\x02"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.from_offset(7)}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=13 sha256="
            "67f40662fd7aae2942e02a35a519fa2cf628498df498ab3b9c3a74e69f572e4e"_av}),
        TEST_BUF(Msg::ToValidator{"\x03\x00\x00\x00\x04\x00\x00\x00\x02"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(2)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg),
        TEST_BUF(Msg::ToFront{19, last_flags, (is_accepted
            ? requested.file_contents
            : zeros
        ).first(7)})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000002.tfl"), requested.file_contents);
    }

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(7, 6, StreamId(2));
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{18, first_last_flags,
             "\x09\x00\x01\x00\x0a\x00\x00\x00\x02\x00\x00\x00""cdefgf"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
             "\x09\x00\x02\x00\x04\x00\x00\x00\x02\x00\x00\x00"_av})
    );
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // full file with 3 requested ranges
    // (req, response) * 3, validation_result, (random req, resp) * 2
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 4, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.first(4));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x03\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x08\x00\x00\x00\x03"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(4)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{16, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.subarray(4, 4));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x08\x00\x00\x00\x03"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.subarray(4, 4)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.last(4));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x08\x00\x00\x00\x03"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.last(4)}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
        TEST_BUF(Msg::ToValidator{"\x03\x00\x00\x00\x04\x00\x00\x00\x03"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(3)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg),
        TEST_BUF(Msg::ToFront{16, last_flags, (is_accepted
            ? requested.file_contents
            : zeros
        ).first(4)})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000003.tfl"), requested.file_contents);
    }

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(4, 4, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{16, first_last_flags,
             "\x09\x00\x01\x00\x08\x00\x00\x00\x01\x00\x00\x00""_abc"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
             "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(8, 4, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{16, first_last_flags,
             "\x09\x00\x01\x00\x08\x00\x00\x00\x01\x00\x00\x00""defg"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
             "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );
    //@}

    requested.prepare_file("data_abcdefgf"_av);

    // requested range with ko
    // req, response fail
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(2));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_fail(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );
    //@}

    requested.prepare_file("data_abcdefgf"_av);

    // 2 requested ranges with ko for the second
    // req, response, validator, req, response fail
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(2));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x04\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x04"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_fail(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=13 sha256="
            "0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToValidator{"\x04\x00\x00\x00\x04\x00\x00\x00\x04"_av}),
        TEST_BUF(Msg::ToFront{19, last_flags, zeros.first(7)})
    );

    if (fdx_ctx && d.always_file_storage) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000004.tfl"),
            requested.file_contents.first(7));
    }
    //@}


    requested.prepare_file("data_abcdefgf"_av);

    // full file with 2 requested ranges and validation_result before the second response
    // req, response, validator, req, response
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(2));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x05\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x05"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(5)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.from_offset(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=13 sha256="
            "67f40662fd7aae2942e02a35a519fa2cf628498df498ab3b9c3a74e69f572e4e"_av}),
        TEST_BUF(Msg::ToFront{19, last_flags, (is_accepted
            ? requested.file_contents
            : zeros
        ).first(7)})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000005.tfl"), requested.file_contents);
    }

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(7, 6, StreamId(2));
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{18, first_last_flags,
             "\x09\x00\x01\x00\x0a\x00\x00\x00\x02\x00\x00\x00""cdefgf"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
             "\x09\x00\x02\x00\x04\x00\x00\x00\x02\x00\x00\x00"_av})
    );
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // validation_result while response
    // req, response header, response data, validator, response data
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_full_file_request(StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, 16, [&](OutStream& out){
                out.out_uint32_le(1);
                // no data
            });
            channel_ctx->process_server_message(temp_av, 24, first_flags);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x06\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{24, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_server_message(
                requested.file_contents.first(6), 20, /*flag=*/0);
        },
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0a\x00\x00\x00\x06"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(6)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(6)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_server_message(
                requested.file_contents.from_offset(6), 20, last_flags);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
        TEST_BUF(Msg::ToFront{24, last_flags, is_accepted
            ? requested.file_contents
            : zeros.first(requested.file_contents.size())})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000006.tfl"), requested.file_contents);
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // 2 requested ranges + validation_result while response
    // req, response header, validator, response data, req, [response]
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, 11, [&](OutStream& out){
                out.out_uint32_le(1);
                // no data
            });
            channel_ctx->process_server_message(temp_av, 19, first_flags);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x07\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(7)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_server_message(
                requested.file_contents.first(7), 19, last_flags);
        },
        TEST_BUF_IF(is_rejected, Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToFront{19, last_flags, (is_accepted
            ? requested.file_contents
            : zeros
        ).first(7)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(7, 15, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::ToMod{32, first_last_flags,
            requested.filecontents_request_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );

    if (is_accepted) {
        msg_comparator.run(
            TEST_PROCESS {
                using namespace Cliprdr;
                Buffer buf;
                temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                    out.out_uint32_le(1);
                    out.out_copy_bytes(requested.file_contents.from_offset(7));
                });
                channel_ctx->process_server_message(temp_av);
            },
            TEST_BUF(Msg::Log6{
                "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                " file_name=abc size=12 sha256="
                "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av}),
            TEST_BUF(Msg::ToFront{17, first_last_flags, temp_av})
        );
    }

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000007.tfl"),
            is_accepted ? requested.file_contents : requested.file_contents.first(7));
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // 2 requested ranges + validation_result while response
    // req, response header, validator, response data, req(offet-1), [response]
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, 11, [&](OutStream& out){
                out.out_uint32_le(1);
                // no data
            });
            channel_ctx->process_server_message(temp_av, 19, first_flags);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x08\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(8)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_server_message(
                requested.file_contents.first(7), 19, last_flags);
        },
        TEST_BUF_IF(is_rejected, Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToFront{19, last_flags, (is_accepted
            ? requested.file_contents
            : zeros
        ).first(7)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(6, 15, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x0e\x00\x00\x00"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );

    if (is_accepted) {
        msg_comparator.run(
            TEST_PROCESS {
                using namespace Cliprdr;
                Buffer buf;
                temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                    out.out_uint32_le(1);
                    out.out_copy_bytes(requested.file_contents.from_offset(7));
                });
                channel_ctx->process_server_message(temp_av);
            },
            TEST_BUF(Msg::ToFront{18, first_last_flags,
                "\x09\x00\x01\x00\x0a\x00\x00\x00\x01\x00\x00\x00""bcdefg"_av}),
            TEST_BUF(Msg::Log6{
                "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                " file_name=abc size=12 sha256="
                "d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av})
        );
    }

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000008.tfl"),
            is_accepted ? requested.file_contents : requested.file_contents.first(7));
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // 2 requested ranges + validation_result while response
    // req, response header, validator, response data, req(offet+1), [response]
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, 11, [&](OutStream& out){
                out.out_uint32_le(1);
                // no data
            });
            channel_ctx->process_server_message(temp_av, 19, first_flags);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x09\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(9)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_server_message(
                requested.file_contents.first(7), 19, last_flags);
        },
        TEST_BUF_IF(is_accepted, Msg::ToFront{19, last_flags,
            requested.file_contents.first(7)}),
        TEST_BUF_IF(is_rejected, Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF_IF(is_rejected, Msg::ToFront{19, last_flags, zeros.first(7)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(8, 15, StreamId(1));
        },
        TEST_BUF_IF(is_accepted, Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12 sha256="
            "0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x01\x00\x00\x00"_av})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000009.tfl"),
            requested.file_contents.first(7));
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // break transfer by format_list
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(2));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(2);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x0a\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x0a"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_format_list_with_file();
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{19, last_flags, zeros.first(7)}),
        TEST_BUF(Msg::Log6{"CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                " file_name=abc size=12"
                " sha256=0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToValidator{"\x04\x00\x00\x00\x04\x00\x00\x00\x0a"_av}),
        TEST_BUF(Msg::ToFront{54, first_last_flags, temp_av})
    );

    if (fdx_ctx && d.always_file_storage) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000010.tfl"),
            requested.file_contents.first(7));
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // break transfer by format_list after first packet
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 4, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.first(4));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x0b\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x08\x00\x00\x00\x0b"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(4)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{16, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.subarray(4, 4));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x08\x00\x00\x00\x0b"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.subarray(4, 4)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_format_list_with_file();
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{16, last_flags, zeros.first(4)}),
        TEST_BUF(Msg::Log6{"CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                " file_name=abc size=12"
                " sha256=9ec505a2f479aca44465c9d226be14fde2e832fecae3536f49b885d92137094d"_av}),
        TEST_BUF(Msg::ToValidator{"\x04\x00\x00\x00\x04\x00\x00\x00\x0b"_av}),
        TEST_BUF(Msg::ToFront{54, first_last_flags, temp_av})
    );

    if (fdx_ctx && d.always_file_storage) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000011.tfl"),
            requested.file_contents.first(8));
    }
    //@}


    requested.prepare_file("data_abcdefg"_av);

    // break transfer by format_list (with validation_result)
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7, StreamId(1));
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(1);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x0c\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x0c"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToMod{32, first_last_flags,
            "\x08\x00\x01\x00\x18\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
            "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"_av}),
        // without file_contents
        TEST_BUF(Msg::ToFront{19, first_flags, temp_av.first(12)})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(12)));
        },
        TEST_BUF(file_verification_msg),
        TEST_BUF_IF(is_rejected, file_blocked_msg)
    );

    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build_format_list_with_file();
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{19, last_flags, zeros.first(7)}),
        TEST_BUF(Msg::Log6{"CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                " file_name=abc size=12"
                " sha256=0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToFront{54, first_last_flags, temp_av})
    );

    if (fdx_ctx && (is_rejected || d.always_file_storage)) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000012.tfl"),
            requested.file_contents.first(7));
    }
    //@}


    msg_comparator.run(TEST_PROCESS {
        channel_ctx.reset();
    });

    if (fdx_ctx) {
        (void)fdx_ctx->hash_path.add_file(fdx_basename);
        auto fdx_path = fdx_ctx->record_path.add_file(fdx_basename);

        OutCryptoTransport::HashArray qhash;
        OutCryptoTransport::HashArray fhash;
        fdx_ctx->fdx.close(qhash, fhash);

        RED_CHECK_WORKSPACE(fdx_ctx->wd);

        if (d.always_file_storage || is_rejected) {
            RED_CHECK_FILE_CONTENTS(fdx_path, ut::ascii(str_concat(
                "v3\n"

                "\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000001.tfl\xd1\xb9\xc9\xdb"
                "\x45\x5c\x70\xb7\xc6\xa7\x02\x25\xa0\x0f\x85\x99\x31\xe4"
                "\x98\xf7\xf5\xe0\x7f\x2c\x96\x2e\x10\x78\xc0\x35\x9f\x5e"

                "\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000002.tflg\xf4\x06\x62"
                "\xfd\x7a\xae\x29\x42\xe0\x2a\x35\xa5\x19\xfa\x2c\xf6\x28"
                "\x49\x8d\xf4\x98\xab\x3b\x9c\x3a\x74\xe6\x9f\x57\x2e\x4e"

                "\x04\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000003.tfl\xd1\xb9\xc9\xdb"
                "\x45\x5c\x70\xb7\xc6\xa7\x02\x25\xa0\x0f\x85\x99\x31\xe4"
                "\x98\xf7\xf5\xe0\x7f\x2c\x96\x2e\x10\x78\xc0\x35\x9f\x5e"_av,

                d.always_file_storage ?
                    "\x04\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                    "abcmy_session_id/my_session_id,000004.tfl\x04\x43\xb6\xdc"
                    "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                    "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"_av
                : ""_av,

                "\x04\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000005.tflg\xf4\x06\x62"
                "\xfd\x7a\xae\x29\x42\xe0\x2a\x35\xa5\x19\xfa\x2c\xf6\x28"
                "\x49\x8d\xf4\x98\xab\x3b\x9c\x3a\x74\xe6\x9f\x57\x2e\x4e"

                "\x04\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000006.tfl\xd1\xb9\xc9\xdb"
                "\x45\x5c\x70\xb7\xc6\xa7\x02\x25\xa0\x0f\x85\x99\x31\xe4"
                "\x98\xf7\xf5\xe0\x7f\x2c\x96\x2e\x10\x78\xc0\x35\x9f\x5e"

                ""_av,

                is_rejected ?
                "\x04\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000007.tfl\x04\x43\xb6\xdc"
                "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"

                "\x04\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000008.tfl\x04\x43\xb6\xdc"
                "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"

                ""_av
                    :
                "\x04\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000007.tfl\xd1\xb9\xc9\xdb"
                "\x45\x5c\x70\xb7\xc6\xa7\x02\x25\xa0\x0f\x85\x99\x31\xe4"
                "\x98\xf7\xf5\xe0\x7f\x2c\x96\x2e\x10\x78\xc0\x35\x9f\x5e"

                "\x04\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x34\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000008.tfl\xd1\xb9\xc9\xdb"
                "\x45\x5c\x70\xb7\xc6\xa7\x02\x25\xa0\x0f\x85\x99\x31\xe4"
                "\x98\xf7\xf5\xe0\x7f\x2c\x96\x2e\x10\x78\xc0\x35\x9f\x5e"

                ""_av
                ,

                "\x04\x00\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000009.tfl\x04\x43\xb6\xdc"
                "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"_av,

                d.always_file_storage ?
                    "\x04\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                    "abcmy_session_id/my_session_id,000010.tfl\x04\x43\xb6\xdc"
                    "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                    "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"

                    "\x04\x00\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                    "abcmy_session_id/my_session_id,000011.tfl\x9e\xc5\x05\xa2"
                    "\xf4\x79\xac\xa4\x44\x65\xc9\xd2\x26\xbe\x14\xfd\xe2\xe8"
                    "\x32\xfe\xca\xe3\x53\x6f\x49\xb8\x85\xd9\x21\x37\t\x4d"

                    ""_av
                : ""_av,

                "\x04\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000012.tfl\x04\x43\xb6\xdc"
                "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"_av

            ), 5));
        }
        else {
            RED_CHECK_FILE_CONTENTS(fdx_path, "v3\n"_av);
        }
    }
}


RED_AUTO_TEST_CLIPRDR(TestCliprdrValidationBeforeTransferAndMaxSize, ClipDataTest const& d, d, {
    //                       always
    //           icap  fdx  storage verify
    ClipDataTest{true, false, true, true},
    ClipDataTest{true, true, false, true},
    ClipDataTest{true, true, true, true},
})
{
    RED_TEST(d.with_validator);
    RED_TEST(d.verify_before_transfer);

    bytes_view temp_av;
    MsgComparator msg_comparator;
    auto fdx_ctx = d.make_optional_fdx_ctx();

    auto cliprdr_params = d.default_channel_params();
    cliprdr_params.validator_params.max_file_size_rejected = 10;

    auto channel_ctx = std::make_unique<ClipDataTest::ChannelCtx>(
        msg_comparator,
        fdx_ctx.get(),
        cliprdr_params,
        d, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

    initialize_channel(msg_comparator, *channel_ctx, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

    // skip format list response

    RequestedRange requested{msg_comparator, *channel_ctx};

    requested.prepare_file("data_abcdefg"_av);

    // request too big
    //@{
    msg_comparator.run(
        TEST_PROCESS { requested.client_full_file_request(); },
        TEST_BUF(Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x00\x00\x00\x00"_av})
    );
    //@}

    // file too big (start with unknown size)
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            requested.client_file_request(0, 7);
        },
        TEST_BUF(Msg::ToMod{32, first_last_flags, requested.filecontents_request_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FILECONTENTS_RESPONSE, [&](OutStream& out){
                out.out_uint32_le(0);
                out.out_copy_bytes(requested.file_contents.first(7));
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x1b\x00\x00\x00\x01\x00\x04""down"
            "\x00\x01\x00\bfilename\x00\x03""abc"_av}),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0b\x00\x00\x00\x01"_av}),
        TEST_BUF(Msg::ToValidator{requested.file_contents.first(7)}),
        TEST_BUF(Msg::ToFront{12, first_last_flags,
            "\x09\x00\x02\x00\x04\x00\x00\x00\x00\x00\x00\x00"_av}),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            " file_name=abc size=12"
            " sha256=0443b6dc17edffa55fb5705981b28c6b0786aa4a778e4f8088e5cc03d0802c4a"_av}),
        TEST_BUF(Msg::ToValidator{"\x04\x00\x00\x00\x04\x00\x00\x00\x01"_av})
    );

    if (fdx_ctx && d.always_file_storage) {
        RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000001.tfl"),
            requested.file_contents.first(7));
    }
    //@}

    msg_comparator.run(TEST_PROCESS {
        channel_ctx.reset();
    });

    if (fdx_ctx) {
        (void)fdx_ctx->hash_path.add_file(fdx_basename);
        auto fdx_path = fdx_ctx->record_path.add_file(fdx_basename);

        OutCryptoTransport::HashArray qhash;
        OutCryptoTransport::HashArray fhash;
        fdx_ctx->fdx.close(qhash, fhash);

        RED_CHECK_WORKSPACE(fdx_ctx->wd);

        if (d.always_file_storage) {
            RED_CHECK_FILE_CONTENTS(fdx_path, ut::ascii(
                "v3\n"

                "\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x54\x03\x00\x26\x00"
                "abcmy_session_id/my_session_id,000001.tfl\x04\x43\xb6\xdc"
                "\x17\xed\xff\xa5\x5f\xb5\x70\x59\x81\xb2\x8c\x6b\x07\x86"
                "\xaa\x4a\x77\x8e\x4f\x80\x88\xe5\xcc\x03\xd0\x80\x2c\x4a"

                ""_av, 5));
        }
        else {
            RED_CHECK_FILE_CONTENTS(fdx_path, "v3\n"_av);
        }
    }
}


RED_AUTO_TEST_CLIPRDR(TestCliprdrTextValidationBeforeTransfer, ClipDataTest const& d, d, {
    //                       always
    //           icap  fdx  storage verify
    ClipDataTest{true, false, false, true, ValidationResult::IsAccepted},
    ClipDataTest{true, false, false, true, ValidationResult::IsRejected},
})
{
    RED_TEST(d.with_validator);
    RED_TEST(d.verify_before_transfer);

    bytes_view temp_av;
    MsgComparator msg_comparator;

    auto cliprdr_params = d.default_channel_params();
    cliprdr_params.validator_params.block_invalid_text_up = true;
    // cliprdr_params.validator_params.block_invalid_text_down = true;
    cliprdr_params.validator_params.max_file_size_rejected = 30;

    auto channel_ctx = std::make_unique<ClipDataTest::ChannelCtx>(
        msg_comparator,
        nullptr,
        cliprdr_params,
        d, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

    initialize_channel(msg_comparator, *channel_ctx, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

    const bool is_accepted = d.validation_result == ValidationResult::IsAccepted;
    const bool is_rejected = d.validation_result == ValidationResult::IsRejected;
    const auto response_error = Msg::ToMod{
        8, first_last_flags, "\x05\x00\x02\x00\x00\x00\x00\x00"_av};


    // small data (3 bytes)
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(RDPECLIP::CF_UNICODETEXT);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_copy_bytes("a\0b\0c\0"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x01\x00\x02up"
            "\x00\x01\x00\x09""format_id\x00\x02""13"_av
        }),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"
            " format=CF_UNICODETEXT(13) size=6 partial_data=abc"_av
        }),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x0a\x00\x00\x00\x01"_av}),
        TEST_BUF(Msg::ToValidator{"a\0b\0c\0"_av}),
        TEST_BUF(Msg::ToValidator{"\x03\x00\x00\x00\x04\x00\x00\x00\x01"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(1)));
        },
        TEST_BUF_IF(is_accepted, Msg::Log6{"TEXT_VERIFICATION direction=UP copy_id=1 status=ok"_av}),
        TEST_BUF_IF(is_rejected, Msg::Log6{"TEXT_VERIFICATION direction=UP copy_id=1 status=fail"_av}),
        TEST_BUF_IF(is_accepted, Msg::ToMod{14, first_last_flags,
            "\x05\x00\x01\x00\x06\x00\x00\x00""a\x00""b\x00""c\x00"_av}),
        TEST_BUF_IF(is_rejected, response_error)
    );
    //@}


    // small data (3 bytes, 2 packets)
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(RDPECLIP::CF_UNICODETEXT);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, 6, [&](OutStream& out){
                out.out_copy_bytes("x\0"_av);
            });
            channel_ctx->process_client_message(temp_av, 14, first_flags);
        },
        TEST_BUF(Msg::ToValidator{
            "\x07\x00\x00\x00\x19\x00\x00\x00\x02\x00\x02up"
            "\x00\x01\x00\x09""format_id\x00\x02""13"_av
        }),
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"
            " format=CF_UNICODETEXT(13) size=6 partial_data=x"_av
        }),
        TEST_BUF(Msg::ToValidator{"\x01\x00\x00\x00\x06\x00\x00\x00\x02"_av}),
        TEST_BUF(Msg::ToValidator{"x\0"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            RED_TEST(channel_ctx->dlp_message(d.validation_result, FileValidatorId(2)));
        },
        TEST_BUF_IF(is_accepted, Msg::Log6{"TEXT_VERIFICATION direction=UP copy_id=2 status=ok"_av}),
        TEST_BUF_IF(is_rejected, Msg::Log6{"TEXT_VERIFICATION direction=UP copy_id=2 status=fail"_av}),
        TEST_BUF_IF(is_accepted, Msg::ToMod{14, first_flags,
            "\x05\x00\x01\x00\x06\x00\x00\x00x\x00"_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            channel_ctx->process_client_message("y\0z\0"_av, 14, last_flags);
        },
        TEST_BUF_IF(is_accepted, Msg::ToMod{14, last_flags, "y\x00z\x00"_av}),
        TEST_BUF_IF(is_rejected, response_error)
    );
    //@}


    // too big
    //@{
    msg_comparator.run(
        TEST_PROCESS {
            Buffer buf;
            temp_av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                out.out_uint32_le(RDPECLIP::CF_UNICODETEXT);
            });
            channel_ctx->process_server_message(temp_av);
        },
        TEST_BUF(Msg::ToFront{12, first_last_flags, temp_av})
    );

    msg_comparator.run(
        TEST_PROCESS {
            using namespace Cliprdr;
            Buffer buf;
            temp_av = buf.build_ok(RDPECLIP::CB_FORMAT_DATA_RESPONSE, [&](OutStream& out){
                out.out_copy_bytes("a\0b\0c\0d\0e\0f\0g\0h\0i\0j\0k\0l\0m\0n\0o\0p\0q\0"_av);
            });
            channel_ctx->process_client_message(temp_av);
        },
        TEST_BUF(Msg::Log6{
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"
            " format=CF_UNICODETEXT(13) size=34 partial_data=abcdefghijklmnopq"_av
        }),
        TEST_BUF(response_error)
    );
    //@}
}
