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
#include "core/session_reactor.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "utils/sugar/algostring.hpp"
#include "core/log_id.hpp"
#include "capture/fdx_capture.hpp"
#include "mod/file_validator_service.hpp"

#include "./test_channel.hpp"


namespace
{
    class TestResponseSender : public VirtualChannelDataSender
    {

    public:
        StaticOutStream<1600> streams[13];
        size_t total_in_stream = 0;


        explicit TestResponseSender() = default;

        void operator()(uint32_t /*total_length*/, uint32_t /*flags*/, bytes_view chunk_data)
                override
        {
            RED_REQUIRE(this->total_in_stream < std::size(this->streams));
            this->streams[this->total_in_stream].out_copy_bytes(chunk_data);
            this->total_in_stream++;
        }

        bytes_view back() const noexcept
        {
            return streams[this->total_in_stream-1].get_produced_bytes();
        }
    };


    struct Buffer
    {
        StaticOutStream<1600> out;

        template<class F>
        bytes_view build(uint16_t msgType, uint16_t msgFlags, F f) &
        {
            using namespace RDPECLIP;
            array_view_u8 av = out.out_skip_bytes(CliprdrHeader::size());
            f(this->out);
            OutStream stream_header(av);
            CliprdrHeader(msgType, msgFlags, out.get_offset() - av.size()).emit(stream_header);
            return out.get_produced_bytes();
        }
    };

    using namespace std::string_view_literals;

    inline constexpr auto fdx_basename = "sid,blabla.fdx"sv;
    inline constexpr auto sid = "my_session_id"sv;

    struct ReportMessageStorage : NullReportMessage
    {
        std::vector<std::string> messages;

        void log6(LogId id, const timeval /*time*/, KVList kv_list) override
        {
            std::string s = detail::log_id_string_map[int(id)].data();
            for (auto& kv : kv_list) {
                str_append(s, ' ', kv.key, '=', kv.value);
            }
            messages.emplace_back(std::move(s));
        }
    };

    struct DataTest
    {
        DataTest(char const* name)
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
            "sid,blabla", sid, -1, cctx, rnd, fstat,
            ReportError()};
    };

    auto add_file(DataTest& data_test, std::string_view suffix)
    {
        auto basename = str_concat(sid, suffix);
        auto path = data_test.fdx_record_path.add_file(basename);
        (void)data_test.fdx_hash_path.add_file(basename);
        return path;
    }
}

RED_AUTO_TEST_CASE(TestCliprdrChannelFilterDataFileWithoutLock)
{
    struct D
    {
        bool with_validator;
        bool with_fdx_capture;
        bool always_file_storage;
        bool verify_before_download;
    };

    Inifile ini;
    SesmanInterface sesman(ini);
    using namespace std::string_view_literals;

    RED_TEST_CONTEXT_DATA(D const& d,
          "with validator: " << d.with_validator <<
        "  with fdx: " << d.with_fdx_capture <<
        "  always_file_storage: " << d.always_file_storage <<
        "  verify_before_download: " << d.verify_before_download, {
        D{true, false, true, false},
        D{true, true, false, false},
        D{true, true, true, false},

        D{false, false, true, false},
        D{false, true, false, false},
        D{false, true, true, false},

        D{true, false, true, true},
        D{true, true, false, true},
        D{true, true, true, true},
    })
    {
        auto const err_count = RED_ERROR_COUNT();

        auto fdx_ctx = [&]{
            if (d.with_fdx_capture) {
                return std::make_unique<DataTest>(
                    &"abcdefgh"[d.with_validator * 2 + d.always_file_storage]
                );
            }
            return std::unique_ptr<DataTest>();
        }();

        {
            SessionReactor session_reactor;
            timeval time_test;
            time_test.tv_sec = 12345;
            time_test.tv_usec = 54321;
            session_reactor.set_current_time(time_test);

            ReportMessageStorage report_message;
            BufTransport validator_transport;
            FileValidatorService file_validator_service(validator_transport);

            BaseVirtualChannel::Params base_params(report_message, RDPVerbose::cliprdr /*| RDPVerbose::cliprdr_dump*/);

            ClipboardVirtualChannelParams clipboard_virtual_channel_params {};
            clipboard_virtual_channel_params.clipboard_down_authorized = true;
            clipboard_virtual_channel_params.clipboard_up_authorized   = true;
            clipboard_virtual_channel_params.clipboard_file_authorized = true;
            clipboard_virtual_channel_params.validator_params.down_target_name = "down";
            clipboard_virtual_channel_params.validator_params.up_target_name = "up";
            clipboard_virtual_channel_params.validator_params.log_if_accepted = true;
            clipboard_virtual_channel_params.validator_params.enable_clipboard_text_up = true;
            clipboard_virtual_channel_params.validator_params.verify_before_download
                = d.verify_before_download;

            TestResponseSender to_client_sender;
            TestResponseSender to_server_sender;

            ClipboardVirtualChannel clipboard_virtual_channel(
                &to_client_sender, &to_server_sender, session_reactor,
                base_params, clipboard_virtual_channel_params,
                d.with_validator ? &file_validator_service : nullptr,
                ClipboardVirtualChannel::FileStorage{
                    fdx_ctx ? &fdx_ctx->fdx : nullptr,
                    d.always_file_storage
                }
            );

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            auto process_server_message = [&](bytes_view av){
                auto flags
                = CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST
                | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                clipboard_virtual_channel.process_server_message(
                    av.size(), flags, av, out_asynchronous_task, sesman);
            };

            auto process_client_message = [&](bytes_view av){
                auto flags
                = CHANNELS::CHANNEL_FLAG_FIRST
                | CHANNELS::CHANNEL_FLAG_LAST
                | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
                clipboard_virtual_channel.process_client_message(av.size(), flags, av);
            };

            const auto use_long_format = Cliprdr::IsLongFormat(true);
            const auto file_group = Cliprdr::formats::file_group_descriptor_w.ascii_name;
            const auto file_group_id = 49262;

            RED_CHECK(report_message.messages.size() == 0);
            RED_CHECK(validator_transport.buf == ""_av);

            {
                using namespace RDPECLIP;
                Buffer buf;
                auto av = buf.build(CB_CLIP_CAPS, 0, [&](OutStream& out){
                    out.out_uint16_le(CB_CAPSTYPE_GENERAL);
                    out.out_clear_bytes(2);
                    GeneralCapabilitySet{CB_CAPS_VERSION_1, CB_USE_LONG_FORMAT_NAMES}.emit(out);
                });

                process_server_message(av);
                process_client_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 1);
            RED_REQUIRE(to_server_sender.total_in_stream == 1);
            RED_CHECK(to_client_sender.back() == to_server_sender.back());
            RED_CHECK(to_client_sender.back() ==
                "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00" //................ !
                "\x01\x00\x00\x00\x02\x00\x00\x00" //........
                ""_av);
            RED_CHECK(report_message.messages.size() == 0);
            RED_CHECK(validator_transport.buf == ""_av);

            {
                StaticOutStream<1600> out;
                Cliprdr::format_list_serialize_with_header(
                    out,
                    use_long_format,
                    std::array{Cliprdr::FormatNameRef{file_group_id, file_group}});

                process_client_message(out.get_produced_bytes());
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 1);
            RED_REQUIRE(to_server_sender.total_in_stream == 2);
            RED_CHECK(to_server_sender.back() ==
                "\x02\x00\x00\x00\x2e\x00\x00\x00\x6e\xc0\x00\x00\x46\x00\x69\x00" //........n...F.i. !
                "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" //l.e.G.r.o.u.p.D. !
                "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" //e.s.c.r.i.p.t.o. !
                "\x72\x00\x57\x00\x00\x00" //r.W... !
                ""_av);
            RED_CHECK(report_message.messages.size() == 0);
            RED_CHECK(validator_transport.buf == ""_av);

            // skip format list response

            {
                Buffer buf;
                auto av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                    out.out_uint32_le(file_group_id);
                });
                process_server_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 2);
            RED_REQUIRE(to_server_sender.total_in_stream == 2);
            RED_CHECK(to_client_sender.back() ==
                "\x04\x00\x00\x00\x04\x00\x00\x00\x6e\xc0\x00\x00"
                ""_av);
            RED_CHECK(report_message.messages.size() == 0);
            RED_CHECK(validator_transport.buf == ""_av);

            {
                using namespace Cliprdr;
                Buffer buf;
                auto av = buf.build(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, [&](OutStream& out){
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
                process_client_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 2);
            RED_REQUIRE(to_server_sender.total_in_stream == 3);
            RED_CHECK(to_server_sender.back() ==
                "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x40\x00\x00\x00" //....T........... !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x0c\x00\x00\x00\x61\x00\x62\x00\x63\x00\x00\x00\x00\x00\x00\x00" //....a.b.c....... !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //............ !
                ""_av);
            RED_REQUIRE(report_message.messages.size() == 1);
            RED_CHECK(report_message.messages.back() ==
                "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION format=FileGroupDescriptorW(49262) size=596"_av);
            RED_CHECK(validator_transport.buf == ""_av);

            {
                using namespace RDPECLIP;
                Buffer buf;
                auto av = buf.build(CB_FILECONTENTS_REQUEST, CB_RESPONSE_OK, [&](OutStream& out){
                    FileContentsRequestPDU(0, 0, FILECONTENTS_RANGE, 0, 0, 12, 0, true).emit(out);
                });
                process_server_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 3);
            RED_REQUIRE(to_server_sender.total_in_stream == 3);
            RED_CHECK(to_client_sender.back() ==
                "\x08\x00\x01\x00\x1c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
                "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00" //................ !
                "\x00\x00\x00\x00"_av);
            RED_CHECK(report_message.messages.size() == 1);

            // file content

            {
                using namespace Cliprdr;
                Buffer buf;
                auto av = buf.build(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, [&](OutStream& out){
                    out.out_uint32_le(0);
                    out.out_copy_bytes("data_abcdefg"_av);
                });
                process_client_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 3);
            RED_REQUIRE(to_server_sender.total_in_stream == 4);
            RED_CHECK(to_server_sender.back() ==
                "\t\x00\x01\x00\x10\x00\x00\x00\x00\x00\x00\x00""data_abcdefg"
                ""_av);
            RED_REQUIRE(report_message.messages.size() == 2);
            RED_CHECK(report_message.messages.back() ==
                "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION file_name=abc size=12"
                " sha256=d1b9c9db455c70b7c6a70225a00f859931e498f7f5e07f2c962e1078c0359f5e"_av);


            size_t expected_report_messages_size = 2;

            if (d.with_validator) {
                RED_CHECK(validator_transport.buf ==
                    "\x07\x00\x00\x00\x19\x00\x00\x00\x01\x00\x02up\x00\x01\x00\b"
                    "filename\x00\x03""abc\x01\x00\x00\x00\x10\x00\x00\x00"
                    "\x01""data_abcdefg\x03\x00\x00\x00\x04\x00\x00\x00\x01"
                    ""_av);

                validator_transport.buf.clear();
                StaticOutStream<256> out;
                auto status = "ok"_av;

                using namespace LocalFileValidatorProtocol;
                FileValidatorHeader(MsgType::Result, 0/*len*/).emit(out);
                FileValidatorResultHeader{ValidationResult::IsAccepted, FileValidatorId(1),
                    checked_int(status.size())}.emit(out);
                out.out_copy_bytes(status);
                auto av = out.get_produced_bytes().as_chars();

                validator_transport.buf.assign(av.data(), av.size());
                clipboard_virtual_channel.DLP_antivirus_check_channels_files();
                validator_transport.buf.clear();

                ++expected_report_messages_size;
            }

            RED_REQUIRE(report_message.messages.size() == expected_report_messages_size);

            if (d.with_validator) {
                RED_CHECK(report_message.messages.back() ==
                    "FILE_VERIFICATION direction=UP file_name=abc status=ok"_av);
            }

            if (fdx_ctx && d.always_file_storage) {
                RED_CHECK_FILE_CONTENTS(add_file(*fdx_ctx, ",000001.tfl"), "data_abcdefg"sv);
            }

            // check TEXT_VALIDATION

            {
                StaticOutStream<1600> out;
                Cliprdr::format_list_serialize_with_header(
                    out,
                    Cliprdr::IsLongFormat(use_long_format),
                    std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, nullptr}});

                process_client_message(out.get_produced_bytes());
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 3);
            RED_REQUIRE(to_server_sender.total_in_stream == 5);
            RED_CHECK(to_server_sender.back() ==
                "\x02\x00\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00\x00\x00"
                ""_av);
            RED_CHECK(report_message.messages.size() == expected_report_messages_size);
            RED_CHECK(validator_transport.buf == ""_av);

            // skip format list response

            {
                Buffer buf;
                auto av = buf.build(RDPECLIP::CB_FORMAT_DATA_REQUEST, 0, [&](OutStream& out){
                    out.out_uint32_le(RDPECLIP::CF_UNICODETEXT);
                });
                process_server_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 4);
            RED_REQUIRE(to_server_sender.total_in_stream == 5);
            RED_CHECK(to_client_sender.back() ==
                "\x04\x00\x00\x00\x04\x00\x00\x00\r\x00\x00\x00"
                ""_av);
            RED_CHECK(report_message.messages.size() == expected_report_messages_size);
            RED_CHECK(validator_transport.buf == ""_av);

            {
                using namespace Cliprdr;
                Buffer buf;
                auto av = buf.build(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, [&](OutStream& out){
                    out.out_copy_bytes("a\0b\0c\0"_av);
                });
                process_client_message(av);
            }
            RED_REQUIRE(to_client_sender.total_in_stream == 4);
            RED_REQUIRE(to_server_sender.total_in_stream == 6);
            RED_CHECK(to_server_sender.back() ==
                "\x05\x00\x01\x00\x06\x00\x00\x00""a\x00""b\x00""c\x00"_av);
            ++expected_report_messages_size;
            RED_REQUIRE(report_message.messages.size() == expected_report_messages_size);
            RED_CHECK(report_message.messages.back() ==
            "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX format=CF_UNICODETEXT(13) size=6 partial_data=abc"_av);
            if (d.with_validator) {
                RED_CHECK(validator_transport.buf ==
                    "\x07\x00\x00\x00\"\x00\x00\x00\x02\x00\x02up\x00\x01\x00\x13microsoft_locale_id\x00\x01"
                    "0\x01\x00\x00\x00\x07\x00\x00\x00\x02""abc\x03\x00\x00\x00\x04\x00\x00\x00\x02"_av);
                validator_transport.buf.clear();

                StaticOutStream<256> out;
                auto status = "ok"_av;

                using namespace LocalFileValidatorProtocol;
                FileValidatorHeader(MsgType::Result, 0/*len*/).emit(out);
                FileValidatorResultHeader{ValidationResult::IsAccepted, FileValidatorId(2),
                    checked_int(status.size())}.emit(out);
                out.out_copy_bytes(status);
                auto av = out.get_produced_bytes().as_chars();
                validator_transport.buf.assign(av.data(), av.size());

                clipboard_virtual_channel.DLP_antivirus_check_channels_files();

                ++expected_report_messages_size;
            }
            RED_REQUIRE(report_message.messages.size() == expected_report_messages_size);
            if (d.with_validator) {
                RED_CHECK(report_message.messages.back() ==
                    "TEXT_VERIFICATION direction=UP copy_id=2 status=ok"_av);
            }
        }

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

        if (err_count != RED_ERROR_COUNT()) {
            break;
        }
    }
}
