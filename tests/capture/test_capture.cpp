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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "capture/capture.hpp"
#include "capture/capture.cpp" // Yeaaahh...
#include "capture/file_to_graphic.hpp"
#include "transport/file_transport.hpp"
#include "utils/drawable.hpp"
#include "utils/fileutils.hpp"
#include "utils/png.hpp"
#include "utils/stream.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/fake_stat.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/ostream_buffered.hpp"
#include "test_only/transport/test_transport.hpp"

namespace
{
    template<class F>
    void test_capture_context(
        char const* basename, CaptureFlags capture_flags, uint16_t cx, uint16_t cy,
        WorkingDirectory& record_wd, WorkingDirectory& hash_wd,
        F f, KbdLogParams kbd_log_params = KbdLogParams())
    {
        // Timestamps are applied only when flushing
        timeval now{1000, 0};

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;
        cctx.set_trace_type(TraceType::localfile);

        const char * record_tmp_path = record_wd.dirname();
        const char * record_path = record_tmp_path;
        const char * hash_path = hash_wd.dirname();
        const int groupid = 0;

        const bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        const bool capture_png = bool(capture_flags & CaptureFlags::png);

        const bool capture_pattern_checker = false;
        const bool capture_ocr = false;
        const bool capture_video = false;
        const bool capture_video_full = false;
        const bool capture_meta = false;
        const bool capture_kbd = kbd_log_params.wrm_keyboard_log;


        MetaParams meta_params;
        VideoParams video_params;
        PatternParams pattern_params {}; // reading with capture_kbd = true
        FullVideoParams full_video_params;
        SequencedVideoParams sequenced_video_params;
        OcrParams const ocr_params {
            OcrVersion::v1, ocr::locale::LocaleId::latin,
            false, 0, std::chrono::seconds::zero(), 0};

        PngParams const png_params = {
            0, 0, std::chrono::milliseconds{60}, 100, 0, false, false, true};

        DrawableParams const drawable_params{cx, cy, nullptr};

        WrmParams const wrm_params{
            BitsPerPixel{24},
            false,
            cctx,
            rnd,
            fstat,
            hash_path,
            std::chrono::seconds{1},
            std::chrono::seconds{3},
            WrmCompressionAlgorithm::no_compression,
            0
        };

        CaptureParams capture_params{
            now,
            basename,
            record_tmp_path,
            record_path,
            groupid,
            nullptr,
            SmartVideoCropping::disable,
            0
        };

        Capture capture(
            capture_params, drawable_params,
            capture_wrm, wrm_params,
            capture_png, png_params,
            capture_pattern_checker, pattern_params,
            capture_ocr, ocr_params,
            capture_video, sequenced_video_params,
            capture_video_full, full_video_params,
            capture_meta, meta_params,
            capture_kbd, kbd_log_params,
            video_params, nullptr, Rect()
        );

        f(capture, Rect{0, 0, cx, cy});
    }

    void capture_draw_color1(timeval& now, Capture& capture, Rect scr, uint16_t cy)
    {
        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, cy, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, cy, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(3, 150, cy, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
    }

    void capture_draw_color2(timeval& now, Capture& capture, Rect scr, uint16_t cy)
    {
        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(Rect(4, 200, cy, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, cy, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(6, 300, cy, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
    }


    const auto file_not_exists = std::not_fn<bool(char const*)>(file_exist);
} // namespace


RED_AUTO_TEST_CASE(TestSplittedCapture)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory record_wd("record");

    test_capture_context("test_capture", CaptureFlags::wrm | CaptureFlags::png,
        800, 600, record_wd, hash_wd, [](Capture& capture, Rect scr)
    {
        // Timestamps are applied only when flushing
        timeval now{1000, 0};

        capture_draw_color1(now, capture, scr, 700);
        capture_draw_color2(now, capture, scr, 700);
    });

    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000000.wrm"), 1646);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000001.wrm"), 3508);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000002.wrm"), 3463);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture.mwrm"), 174 + record_wd.dirname().size() * 3);

    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000000.png"), 3102);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000001.png"), 3127);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000002.png"), 3145);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000003.png"), 3162);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000004.png"), 3175);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000005.png"), 3201);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000006.png"), 3225);

    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture-000000.wrm"), 45);
    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture-000001.wrm"), 45);
    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture-000002.wrm"), 45);
    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture.mwrm"), 39);

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(record_wd);
}

RED_AUTO_TEST_CASE(TestBppToOtherBppCapture)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory record_wd("record");

    test_capture_context("test_capture", CaptureFlags::png,
        100, 100, record_wd, hash_wd, [](Capture& capture, Rect scr)
    {
        // Timestamps are applied only when flushing
        timeval now{1000, 0};

        auto const color_cxt = gdi::ColorCtx::depth16();
        capture.set_pointer(0, edit_pointer(), gdi::GraphicApi::SetPointerMode::Insert);

        bool ignore_frame_in_timeval = true;

        capture.draw(RDPOpaqueRect(scr, encode_color16()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 5, ignore_frame_in_timeval);
    });

    RED_CHECK_MEM_FILE_CONTENTS(record_wd.add_file("test_capture-000000.png"),
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR !
        "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........ !
        "\x03\x00\x00\x01\x6d\x49\x44\x41\x54\x78\x9c\xed\xd6\xc1\x0e\x82" //....mIDATx...... !
        "\x30\x10\x00\xd1\x5d\xe2\xff\xff\xf2\x7a\x30\x41\x02\x2d\x74\x82" //0...]....z0A.-t. !
        "\x08\x87\x79\x17\x23\xa5\x96\x8c\x58\x8c\x90\xae\x90\x9f\x97\xaa" //..y.#...X....... !
        "\x8a\x88\xcc\x5c\x8e\x7d\x0e\x2e\x8f\xcf\x47\x7a\xc7\x57\x9f\xb0" //...\.}....Gz.W.. !
        "\xd5\x3c\xb3\xb9\xfa\x03\xa7\x7f\xcf\x5e\x85\x58\xbe\x5d\x0d\xed" //.<.......^.X.].. !
        "\x8c\x6e\xcf\x3c\x9c\xd5\x5c\xfd\xf6\xe9\x55\xb5\x0d\x32\xc5\x78" //.n.<..\...U..2.x !
        "\xd4\x6b\x9c\x5c\xfd\x6f\xd3\x33\xf3\xb5\x33\xd6\xfb\xba\xaa\x6a" //.k.\.o.3..3....j !
        "\xfc\xe7\x46\xaf\xe9\x27\xd3\xcf\x6b\xae\xd8\x8d\x15\x9d\xad\x6a" //..F..'..k......j !
        "\xa4\x54\x6f\xb1\x71\xf7\xde\xec\xd1\xd9\xc8\xa6\x91\x69\xb7\x5f" //.To.q........i._ !
        "\xfa\x43\x64\x74\xee\xf9\xde\x83\xa3\x19\xee\xcc\xd3\x10\xfd\xe2" //.Cdt............ !
        "\x6e\x9c\x3e\xf2\x0c\x91\x74\xb5\x5c\x6e\x43\x55\xe5\x56\xbe\x23" //n.>...t.\nCU.V.# !
        "\x57\x3b\x97\xb1\x76\x64\x84\x9b\xfc\xa8\xe3\xff\x59\x9a\x19\x0b" //W;..vd......Y... !
        "\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01\xc6\x02\x8c\x05\x18" //0.`,.X...c...... !
        "\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01\xc6\x02\x8c\x05" //.0.`,.X...c..... !
        "\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01\xc6\x02\x8c" //..0.`,.X...c.... !
        "\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01\xc6\x02" //...0.`,.X...c... !
        "\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01\xc6" //....0.`,.X...c.. !
        "\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63\x01" //.....0.`,.X...c. !
        "\xc6\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00\x63" //......0.`,.X...c !
        "\x01\xc6\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1\x00" //.......0.`,.X... !
        "\x63\x01\xc6\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80\xb1" //c.......0.`,.X.. !
        "\x00\x63\x01\xc6\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58\x80" //.c.......0.`,.X. !
        "\xb1\x00\x63\x01\xc6\x02\x8c\x05\x18\x0b\x30\x16\x60\x2c\xc0\x58" //..c.......0.`,.X !
        "\xc0\x1b\x24\xa9\xa3\x8c\xa4\x69\x8f\x78\x00\x00\x00\x00\x49\x45" //..$....i.x....IE !
        "\x4e\x44\xae\x42\x60\x82" //ND.B`. !
        ""_av);

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(record_wd);
}

RED_AUTO_TEST_CASE(TestResizingCapture)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory record_wd("record");

    test_capture_context("resizing-capture-0", CaptureFlags::wrm | CaptureFlags::png,
        800, 600, record_wd, hash_wd, [](Capture& capture, Rect scr)
    {
        // Timestamps are applied only when flushing
        timeval now{1000, 0};

        capture_draw_color1(now, capture, scr, 1200);

        scr.cx = 1024;
        scr.cy = 768;

        capture.resize(scr.cx, scr.cy);

        capture_draw_color2(now, capture, scr, 1200);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(Rect(7, 350, 1200, 30), encode_color24()(YELLOW)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
    });

    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000000.wrm"), 1651);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000001.wrm"), 3428);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000002.wrm"), 4384);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000003.wrm"), 4388);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0.mwrm"), 248 + record_wd.dirname().size() * 4);

    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000000.png"), 3102 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000001.png"), 3121 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000002.png"), 3131 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000003.png"), 3143 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000004.png"), 4079 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000005.png"), 4103 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000006.png"), 4122 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-0-000007.png"), 4137 +- 100_v);

    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-0-000000.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-0-000001.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-0-000002.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-0-000003.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-0.mwrm"), 45);

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(record_wd);
}

RED_AUTO_TEST_CASE(TestResizingCapture1)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory record_wd("record");

    test_capture_context("resizing-capture-1", CaptureFlags::wrm | CaptureFlags::png,
        800, 600, record_wd, hash_wd, [](Capture& capture, Rect scr)
    {
        timeval now{1000, 0};

        capture_draw_color1(now, capture, scr, 700);

        capture.resize(640, 480);

        capture_draw_color2(now, capture, scr, 700);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(Rect(7, 350, 700, 30), encode_color24()(YELLOW)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
    });

    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000000.wrm"), 1646);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000001.wrm"), 3439);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000002.wrm"), 2630);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000003.wrm"), 2630);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1.mwrm"), 248 + record_wd.dirname().size() * 4);

    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000000.png"), 3102 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000001.png"), 3127 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000002.png"), 3145 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000003.png"), 3162 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000004.png"), 2304 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000005.png"), 2320 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000006.png"), 2334 +- 100_v);
    RED_TEST_FILE_SIZE(record_wd.add_file("resizing-capture-1-000007.png"), 2345 +- 100_v);

    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-1-000000.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-1-000001.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-1-000002.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-1-000003.wrm"), 51);
    RED_TEST_FILE_SIZE(hash_wd.add_file("resizing-capture-1.mwrm"), 45);

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(record_wd);
}

RED_AUTO_TEST_CASE(TestPattern)
{
    for (int i = 0; i < 2; ++i) {
        struct : NullReportMessage {
            std::string reason;
            std::string message;

            void report(const char * reason, const char * message) override
            {
                this->reason = reason;
                this->message = message;
            }
        } report_message;
        Capture::PatternsChecker checker(
            report_message, PatternParams{i ? nullptr : ".de.", i ? ".de." : nullptr, 0});

        auto const reason = i ? "FINDPATTERN_KILL" : "FINDPATTERN_NOTIFY";

        checker(cstr_array_view("Gestionnaire"));

        RED_CHECK(report_message.reason.empty());
        RED_CHECK(report_message.message.empty());

        checker(cstr_array_view("Gestionnaire de serveur"));

        RED_CHECK_EQUAL(report_message.reason,  reason);
        RED_CHECK_EQUAL(report_message.message, "$ocr:.de.|Gestionnaire de serveur");

        checker(cstr_array_view("Gestionnaire de licences TS"));

        RED_CHECK_EQUAL(report_message.reason,  reason);
        RED_CHECK_EQUAL(report_message.message, "$ocr:.de.|Gestionnaire de licences TS");
    }
}


namespace
{
    MetaParams make_meta_params()
    {
        return MetaParams{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };
    }

    Capture::SessionMeta make_session_meta(timeval now, Transport& trans, bool key_markers_hidden_state = false)
    {
        return Capture::SessionMeta(now, trans, key_markers_hidden_state, make_meta_params());
    }
} // namespace


RED_AUTO_TEST_CASE(TestSessionMeta)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
            now.tv_sec += 1;
        };

        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        send_kbd();
        meta.periodic_snapshot(now, 0, 0, false);
        send_kbd();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah1"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2"));
        now.tv_sec += 1;
        send_kbd();
        send_kbd();
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:50 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:51 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:54 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:55 - type=\"KBD_INPUT\" data=\"ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMetaQuoted)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, '\"');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
            now.tv_sec += 1;
        };
        send_kbd();
        meta.periodic_snapshot(now, 0, 0, false);
        send_kbd();
        meta.title_changed(now.tv_sec, cstr_array_view("Bl\"ah1"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah\\2"));
        meta.session_update(now, cstr_array_view("INPUT_LANGUAGE=fr\x01xy\\z"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:42 + type=\"TITLE_BAR\" data=\"Bl\\\"ah1\"\n"
        "1970-01-01 01:16:43 + type=\"TITLE_BAR\" data=\"Blah\\\\2\"\n"
        "1970-01-01 01:16:43 - type=\"INPUT_LANGUAGE\" identifier=\"fr\" display_name=\"xy\\\\z\"\n"
        "1970-01-01 01:16:44 - type=\"KBD_INPUT\" data=\"A\\\"CDA\\\"CD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMeta2)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.next_video(now.tv_sec);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:40 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:44 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:45 + (break)\n"
        "1970-01-01 01:16:45 - type=\"KBD_INPUT\" data=\"ABCDABCD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMeta3)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.session_update(now, {"CHECKBOX_CLICKED=User Properties\x01" "User cannot change password\x01" "1", 62}); now.tv_sec += 1;

        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.next_video(now.tv_sec);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:42 - type=\"BUTTON_CLICKED\" windows=\"\" button=\"Démarrer\"\n"
        "1970-01-01 01:16:43 - type=\"CHECKBOX_CLICKED\" windows=\"User Properties\" checkbox=\"User cannot change password\" state=\"checked\"\n"
        "1970-01-01 01:16:44 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:47 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:48 + (break)\n"
        "1970-01-01 01:16:48 - type=\"KBD_INPUT\" data=\"ABCDABCDABCD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMeta4)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        send_kbd();

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        send_kbd(); now.tv_sec += 1;

        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.next_video(now.tv_sec);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:42 - type=\"BUTTON_CLICKED\" windows=\"\" button=\"Démarrer\"\n"
        "1970-01-01 01:16:44 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:47 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:48 + (break)\n"
        "1970-01-01 01:16:48 - type=\"KBD_INPUT\" data=\"ABCDABCDABCDABCDABCD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMeta5)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);

        meta.kbd_input(now, 'A'); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.kbd_input(now, 'B');

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.kbd_input(now, 'C'); now.tv_sec += 1;

        meta.possible_active_window_change();

        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        meta.kbd_input(now, 'D'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'E'); now.tv_sec += 1;
        meta.kbd_input(now, 'F'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'G'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\t'); now.tv_sec += 1;
        meta.kbd_input(now, 'H'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.next_video(now.tv_sec);
        meta.kbd_input(now, 'I'); now.tv_sec += 1;
        meta.kbd_input(now, 'J'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'K'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah4")); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'a'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'L'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah5")); now.tv_sec += 1;
        meta.kbd_input(now, 'M'); now.tv_sec += 1;
        meta.kbd_input(now, 'N'); now.tv_sec += 1;
        meta.kbd_input(now, 'O'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'P'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah6")); now.tv_sec += 1;
        meta.kbd_input(now, 'Q'); now.tv_sec += 1;
        meta.kbd_input(now, 'R'); now.tv_sec += 1;
        meta.kbd_input(now, 0x2191); now.tv_sec += 1; // UP
        meta.kbd_input(now, 'S'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'T'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, 'U'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'V'); now.tv_sec += 1;
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:42 - type=\"BUTTON_CLICKED\" windows=\"\" button=\"Démarrer\"\n"
        "1970-01-01 01:16:43 - type=\"KBD_INPUT\" data=\"ABC\"\n"
        "1970-01-01 01:16:44 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:45 - type=\"KBD_INPUT\" data=\"D/<enter>\"\n"
        "1970-01-01 01:16:48 - type=\"KBD_INPUT\" data=\"EF/<enter>\"\n"
        "1970-01-01 01:16:52 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:52 - type=\"KBD_INPUT\" data=\"/<enter>G/<enter>\"\n"
        "1970-01-01 01:16:57 + (break)\n"
        "1970-01-01 01:17:00 - type=\"KBD_INPUT\" data=\"/<enter>/<tab>HIK\"\n"
        "1970-01-01 01:17:01 + type=\"TITLE_BAR\" data=\"Blah4\"\n"
        "1970-01-01 01:17:06 - type=\"KBD_INPUT\" data=\"/<backspace>/<backspace>L\"\n"
        "1970-01-01 01:17:07 + type=\"TITLE_BAR\" data=\"Blah5\"\n"
        "1970-01-01 01:17:13 - type=\"KBD_INPUT\" data=\"MP\"\n"
        "1970-01-01 01:17:14 + type=\"TITLE_BAR\" data=\"Blah6\"\n"
        "1970-01-01 01:17:27 - type=\"KBD_INPUT\" data=\"QR/<up>/<backspace>T//U//V\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionSessionLog)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans);
        Capture::SessionLogAgent log_agent(meta, make_meta_params());

        log_agent.session_update(now, cstr_array_view("NEW_PROCESS=abc")); now.tv_sec += 1;
        log_agent.session_update(now, cstr_array_view("BUTTON_CLICKED=de\01fg")); now.tv_sec += 1;
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:40 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
        "1970-01-01 01:16:41 - type=\"BUTTON_CLICKED\" windows=\"de\" button=\"fg\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMetaHiddenKey)
{
    BufTransport trans;

    {
        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        Capture::SessionMeta meta = make_session_meta(now, trans, true);

        meta.kbd_input(now, 'A'); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.kbd_input(now, 'B');

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.kbd_input(now, 'C'); now.tv_sec += 1;

        meta.possible_active_window_change();

        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        meta.kbd_input(now, 'D'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'E'); now.tv_sec += 1;
        meta.kbd_input(now, 'F'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'G'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\t'); now.tv_sec += 1;
        meta.kbd_input(now, 'H'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, false);
        meta.next_video(now.tv_sec);
        meta.kbd_input(now, 'I'); now.tv_sec += 1;
        meta.kbd_input(now, 'J'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'K'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah4")); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'a'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'L'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah5")); now.tv_sec += 1;
        meta.kbd_input(now, 'M'); now.tv_sec += 1;
        meta.kbd_input(now, 'N'); now.tv_sec += 1;
        meta.kbd_input(now, 'O'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'P'); now.tv_sec += 1;
        meta.possible_active_window_change();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah6")); now.tv_sec += 1;
        meta.kbd_input(now, 'Q'); now.tv_sec += 1;
        meta.kbd_input(now, 'R'); now.tv_sec += 1;
        meta.kbd_input(now, 0x2191); now.tv_sec += 1; // UP
        meta.kbd_input(now, 'S'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'T'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, 'U'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, '/'); now.tv_sec += 1;
        meta.kbd_input(now, 0x08); now.tv_sec += 1;
        meta.kbd_input(now, 'V'); now.tv_sec += 1;

        meta.session_update(now, cstr_array_view("BUTTON_CLICKED=\"Connexion Bureau à distance\"\x01" "&Connexion")); now.tv_sec += 1;
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:42 - type=\"BUTTON_CLICKED\" windows=\"\" button=\"Démarrer\"\n"
        "1970-01-01 01:16:43 - type=\"KBD_INPUT\" data=\"ABC\"\n"
        "1970-01-01 01:16:44 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:45 - type=\"KBD_INPUT\" data=\"D\"\n"
        "1970-01-01 01:16:48 - type=\"KBD_INPUT\" data=\"EF\"\n"
        "1970-01-01 01:16:52 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:52 - type=\"KBD_INPUT\" data=\"G\"\n"
        "1970-01-01 01:16:57 + (break)\n"
        "1970-01-01 01:17:00 - type=\"KBD_INPUT\" data=\"HIK\"\n"
        "1970-01-01 01:17:01 + type=\"TITLE_BAR\" data=\"Blah4\"\n"
        "1970-01-01 01:17:06 - type=\"KBD_INPUT\" data=\"L\"\n"
        "1970-01-01 01:17:07 + type=\"TITLE_BAR\" data=\"Blah5\"\n"
        "1970-01-01 01:17:13 - type=\"KBD_INPUT\" data=\"MP\"\n"
        "1970-01-01 01:17:14 + type=\"TITLE_BAR\" data=\"Blah6\"\n"
        "1970-01-01 01:17:28 - type=\"BUTTON_CLICKED\" windows=\"\\\"Connexion Bureau à distance\\\"\" button=\"&Connexion\"\n"
        "1970-01-01 01:17:28 - type=\"KBD_INPUT\" data=\"QRT/U/V\"\n"
    );
}

namespace
{
    struct TestGraphicToFile
    {
        timeval now{1000, 0};

        BmpCache bmp_cache;
        GlyphCache gly_cache;
        PointerCache ptr_cache;
        RDPDrawable drawable;
        GraphicToFile consumer;

        TestGraphicToFile(Transport& trans, Rect scr, bool small_cache)
        : bmp_cache(
            BmpCache::Recorder, BitsPerPixel{24}, 3, false,
            BmpCache::CacheOption(small_cache ? 2 : 600, 256, false),
            BmpCache::CacheOption(small_cache ? 2 : 300, 1024, false),
            BmpCache::CacheOption(small_cache ? 2 : 262, 4096, false)
        )
        , drawable(scr.cx, scr.cy)
        , consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression)
        {}

        void next_second(int n = 1)
        {
            now.tv_sec += n;
            consumer.timestamp(now);
        }
    };
} // namespace


const char expected_stripped_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x20\x03\x58\x02\x18\x00" // WRM version = 3, width = 800, height=600, bpp=24
           "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// initial screen content PNG image
/* 0000 */ "\x00\x10\xcc\x05\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00\x15\x14\x15" //... ...X........
/* 0020 */ "\x27\x00\x00\x05\x8b\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //'....IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x57\x03" //..............W.
/* 05b0 */ "\xfc\x93\x00\x01\x4b\x66\x2c\x0e\x00\x00\x00\x00\x49\x45\x4e\x44" //....Kf,.....IEND
/* 05c0 */ "\xae\x42\x60\x82"                                                 //.B`.

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xca\x9a\x3B\x00\x00\x00\x00" // 0x3B9ACA00 = 1000000000

           "\x00\x00\x10\x00\x00\x00\x01\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x09\x0a\x2c\x20\x03\x58\x02\xff"         // Green Rect(0, 0, 800, 600)

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x3BAA0C40 = 1001000000

/* 0000 */ "\x00\x00\x12\x00\x00\x00\x01\x00" // 0000: ORDERS  0012:chunk_len=18 0002: 1 orders
           "\x01\x6e\x32\x00\xbc\x02\x1e\x00\x00\xff"  // Blue  Rect(0, 50, 700, 80)

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x00\xd3\xd7\x3b\x00\x00\x00\x00" // time = 1004000000

/* 0000 */ "\x00\x00\x0d\x00\x00\x00\x01\x00"
           "\x11\x32\x32\xff\xff"

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00"
           "\x80\x57\xf6\x3b\x00\x00\x00\x00"

/* 0000 */ "\x00\x00\x0d\x00\x00\x00\x01\x00"
           "\x11\x62\x32\x00\x00"
    ;

RED_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrm)
{
    Rect screen_rect(0, 0, 800, 600);
    CheckTransport trans(cstr_array_view(expected_stripped_wrm));
    TestGraphicToFile tgtf(trans, screen_rect, false);
    GraphicToFile& consumer = tgtf.consumer;

    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, encode_color24()(GREEN)), screen_rect, color_ctx);
    tgtf.next_second();

    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), encode_color24()(BLUE)), screen_rect, color_ctx);
    consumer.sync();
    tgtf.next_second();
    tgtf.next_second();
    tgtf.next_second();

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), encode_color24()(WHITE)), screen_rect, color_ctx);
    tgtf.next_second();
    tgtf.next_second();

    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), encode_color24()(RED));
    consumer.draw(cmd3, screen_rect, color_ctx);
    tgtf.next_second();

    consumer.sync();
}

const char expected_stripped_wrm2[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x20\x03\x58\x02\x18\x00" // WRM version = 3, width = 800, height=600, bpp=24
           "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// initial screen content PNG image
/* 0000 */ "\x00\x10\xcc\x05\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00\x15\x14\x15" //... ...X........
/* 0020 */ "\x27\x00\x00\x05\x8b\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //'....IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0200 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0210 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0220 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0230 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0240 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0250 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0260 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0270 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0280 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0290 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 02f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0300 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0310 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0320 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0330 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0340 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0350 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0360 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0370 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0380 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0390 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 03f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0400 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0410 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0420 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0430 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0440 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0450 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0460 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0470 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0480 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0490 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 04f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0500 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0510 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0520 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0530 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0540 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0550 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0560 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0570 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0580 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0590 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 05a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x57\x03" //..............W.
/* 05b0 */ "\xfc\x93\x00\x01\x4b\x66\x2c\x0e\x00\x00\x00\x00\x49\x45\x4e\x44" //....Kf,.....IEND
/* 05c0 */ "\xae\x42\x60\x82"                                                 //.B`.

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xca\x9a\x3B\x00\x00\x00\x00" // 0x3B9ACA00 = 1000000000

           "\x00\x00\x1A\x00\x00\x00\x02\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x09\x0a\x2c\x20\x03\x58\x02\xff"         // Green Rect(0, 0, 800, 600)
           "\x01\x6e\x32\x00\xbc\x02\x1e\x00\x00\xff"  // Blue  Rect(0, 50, 700, 80)

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x40\x0c\xaa\x3b\x00\x00\x00\x00" // time = 1001000000

           "\x00\x00\x12\x00\x00\x00\x02\x00"
/* 0000 */ "\x11\x32\x32\xff\xff"             // encode_color24()(WHITE) rect
           "\x11\x62\x32\x00\x00"             // encode_color24()(RED) rect

           "\xf0\x03\x10\x00\x00\x00\x01\x00"
/* 0000 */ "\xc0\x99\x05\x3c\x00\x00\x00\x00" // time 1007000000

           "\x00\x00\x13\x00\x00\x00\x01\x00"
/* 0000 */ "\x01\x1f\x05\x00\x05\x00\x0a\x00\x0a\x00\x00" // encode_color24()(BLACK) rect
   ;


RED_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrmReplay2)
{
    Rect screen_rect(0, 0, 800, 600);
    CheckTransport trans(cstr_array_view(expected_stripped_wrm2));
    TestGraphicToFile tgtf(trans, screen_rect, false);
    GraphicToFile& consumer = tgtf.consumer;

    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, encode_color24()(GREEN)), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), encode_color24()(BLUE)), screen_rect, color_ctx);
    tgtf.next_second();

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), encode_color24()(WHITE)), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 150, 700, 30), encode_color24()(RED)), screen_rect, color_ctx);
    tgtf.next_second(6);

    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 10), encode_color24()(BLACK)), screen_rect, color_ctx);

    consumer.sync();
}


RED_AUTO_TEST_CASE(TestCaptureToWrmReplayToPng)
{
    Rect screen_rect(0, 0, 800, 600);

    BufTransport trans;
    TestGraphicToFile tgtf(trans, screen_rect, false);
    GraphicToFile& consumer = tgtf.consumer;

    auto const color_ctx = gdi::ColorCtx::depth24();

    RDPOpaqueRect cmd0(screen_rect, encode_color24()(GREEN));
    consumer.draw(cmd0, screen_rect, color_ctx);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), encode_color24()(BLUE));
    consumer.draw(cmd1, screen_rect, color_ctx);
    tgtf.next_second();
    consumer.sync();

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), encode_color24()(WHITE));
    consumer.draw(cmd2, screen_rect, color_ctx);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), encode_color24()(RED));
    consumer.draw(cmd3, screen_rect, color_ctx);
    tgtf.next_second(6);
    consumer.sync();

    RED_TEST_PASSPOINT();
    RED_TEST(trans.size() == 1588);

    GeneratorTransport in_wrm_trans(trans.data());

    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr, nullptr);

    BufTransport buftrans;
    dump_png24(buftrans, drawable1, true);
    RED_TEST(1476 == buftrans.size());

    for (std::size_t sz : {2786, 2800, 2800, 2814, 2823})
    {
        // Green Rect
        // Blue Rect
        // Timestamp
        // White Rect
        // Red Rect
        RED_TEST(player.next_order());
        player.interpret_order();

        buftrans.buf.clear();
        dump_png24(buftrans, drawable1, true);
        RED_TEST(sz == buftrans.size());
    }

    RED_TEST(!player.next_order());
    in_wrm_trans.disconnect();
}


const char expected_Red_on_Blue_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x64\x00\x64\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

/* 0000 */ "\x00\x10\x75\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x00\x34\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //....4IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x7e\x0c\x75\x94\x00\x01\xa8\x50\xf2" //.......~.u....P.
/* 0060 */ "\x39\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"             //9....IEND.B`.

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000

/* 0000 */ "\x00\x00\x2d\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
/* 0000 */ "\x19\x0a\x4c\x64\x64\xff"         // Blue rect  // order 0A=opaque rect
// -----------------------------------------------------
/* 0020 */ "\x03\x09\x00\x00\x04\x02"         // Secondary drawing order header. Order = 02: Compressed bitmap
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00" // 0x01=cacheId 0x00=pad 0x14=width(20) 0x0A=height(10) 0x18=24 bits
                                                  // 0x0007=bitmapLength 0x0000=cacheIndex
           "\xc0\x04\x00\x00\xff\x00\x94"         // compressed bitamp data (7 bytes)
// -----------------------------------------------------

           "\x59\x0d\x3d\x01\x00\x5a\x14\x0a\xcc" // order=0d : MEMBLT

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           "\x00\x00\x1e\x00\x00\x00\x01\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
// -----------------------------------------------------
/* 0000 */ "\x03\x09\x00\x00\x04\x02"
           "\x01\x00\x14\x0a\x18\x07\x00\x00\x00"
           "\xc0\x04\x00\x00\xff\x00\x94"
// -----------------------------------------------------
           ;

RED_AUTO_TEST_CASE(TestSaveCache)
{
    Rect scr(0, 0, 100, 100);
    CheckTransport trans(cstr_array_view(expected_Red_on_Blue_wrm));
    trans.disable_remaining_error();
    TestGraphicToFile tgtf(trans, scr, true);
    GraphicToFile& consumer = tgtf.consumer;

    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(scr, encode_color24()(BLUE)), scr, color_ctx);

    uint8_t comp20x10RED[] = {
        0xc0, 0x04, 0x00, 0x00, 0xFF, // MIX 20 (0, 0, FF)
        0x00, 0x94                    // FILL 9 * 20
    };

    Bitmap bloc20x10(BitsPerPixel{24}, BitsPerPixel{24}, nullptr, 20, 10, comp20x10RED, sizeof(comp20x10RED), true );
    consumer.draw(
        RDPMemBlt(0, Rect(0, scr.cy - 10, bloc20x10.cx(), bloc20x10.cy()), 0xCC, 0, 0, 0),
        scr,
        bloc20x10);
    consumer.sync();

    tgtf.next_second();

    consumer.save_bmp_caches();
    consumer.sync();
}

const char expected_reset_rect_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
           "\x03\x00\x64\x00\x64\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes
           //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

/* 0000 */ "\x00\x10\x75\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x00\x34\x49\x44\x41\x54\x78\x9c\xed\xc1\x01\x0d\x00" //....4IDATx......
/* 0030 */ "\x00\x00\xc2\xa0\xf7\x4f\x6d\x0e\x37\xa0\x00\x00\x00\x00\x00\x00" //.....Om.7.......
/* 0040 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x7e\x0c\x75\x94\x00\x01\xa8\x50\xf2" //.......~.u....P.
/* 0060 */ "\x39\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"             //9....IEND.B`.

/* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
/* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000

/* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
           "\x19\x0a\x1c\x64\x64\xff\x11"     // Red Rect
           "\x5f\x05\x05\xf6\xf6\x00\xff"     // Blue Rect
           "\x11\x5f\x05\x05\xf6\xf6\xff\x00" // Red Rect

           // save orders cache
/* 0000 */ "\x02\x10"
/* 0000 */ "\x0F\x02"                         //.data length
/* 0000 */ "\x00\x00\x01\x00"

/* 0000 */ "\x0a\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x0a\x00\x50" //...............P
/* 0040 */ "\x00\x50\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.P..............
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00" //................
/* 00a0 */ "\x01\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0180 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0190 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 01c0 */ "\x00\x00\x00\x00\x00\x00"                                         //......

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiDstBlt

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiOpaqueRect

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiOpaqueRect
/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/* 0000 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // MultiScrBlt

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           "\x00\x00\x10\x00\x00\x00\x01\x00"
           "\x11\x3f\x0a\x0a\xec\xec\x00\xff" // Green Rect
           ;

RED_AUTO_TEST_CASE(TestSaveOrderStates)
{
    Rect scr(0, 0, 100, 100);
    CheckTransport trans(cstr_array_view(expected_reset_rect_wrm));
    TestGraphicToFile tgtf(trans, scr, true);
    GraphicToFile& consumer = tgtf.consumer;

    auto const color_cxt = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(5), encode_color24()(BLUE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(10), encode_color24()(RED)), scr, color_cxt);

    consumer.sync();

    consumer.send_save_state_chunk();

    tgtf.next_second();
    consumer.draw(RDPOpaqueRect(scr.shrink(20), encode_color24()(GREEN)), scr, color_cxt);

    consumer.sync();
}

const char expected_continuation_wrm[] =
/* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
           "\x01\x00\x64\x00\x64\x00\x18\x00" // WRM version 1, width = 20, height=10, bpp=24
           "\x02\x00\x00\x01\x02\x00\x00\x04\x02\x00\x00\x10"  // caches sizes

           "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
           "\x40\x0C\xAA\x3B\x00\x00\x00\x00" // 0x000000003BAA0C40 = 1001000000

           // save images
/* 0000 */ "\x00\x10\x49\x01\x00\x00\x01\x00"

/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x64\x00\x00\x00\x64\x08\x02\x00\x00\x00\xff\x80\x02" //...d...d........
/* 0020 */ "\x03\x00\x00\x01\x08\x49\x44\x41\x54\x78\x9c\xed\xdd\x31\x0e\x83" //.....IDATx...1..
/* 0030 */ "\x30\x10\x00\x41\x3b\xe2\xff\x5f\x86\x32\xc8\x05\xb0\x55\x14\x34" //0..A;.._.2...U.4
/* 0040 */ "\xd3\xb9\xb3\x56\xa7\x73\x07\x73\x1f\x3c\xf5\xf9\xf5\x05\xfe\x89" //...V.s.s.<......
/* 0050 */ "\x58\x81\x58\x81\x58\xc1\xb6\x9c\xe7\xb0\xf1\xbf\xf6\x31\xcf\x47" //X.X.X........1.G
/* 0060 */ "\x93\x15\x88\x15\x88\x15\x88\x15\xac\x0b\x7e\xb1\x6c\xb8\xd7\xbb" //..........~.l...
/* 0070 */ "\x7e\xdf\x4c\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" //~.LV V V V V V V
/* 0080 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0090 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00a0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00b0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00c0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00d0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00e0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 00f0 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0100 */ "\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56\x20\x56" // V V V V V V V V
/* 0110 */ "\x70\xf3\x3d\x78\xff\xff\x38\x33\x59\x81\x58\x81\x58\x81\x58\xc1" //p.=x..83Y.X.X.X.
/* 0120 */ "\xb4\xc0\x9f\x33\x59\x81\x58\x81\x58\x81\x58\xc1\x01\x8e\xa9\x07" //...3Y.X.X.X.....
/* 0130 */ "\xcb\xdb\x96\x4d\x96\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60" //...M.....IEND.B`
/* 0140 */ "\x82"

           // save orders cache
           "\x02\x10\xA0\x01\x00\x00\x01\x00"
/* 0000 */ "\x0a\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/* 0000 */ "\x0a\x00\x0a\x00\x50\x00\x50\x00\xff\x00\x00\x00\x00\x00\x00\x00" //....P.P.........
/* 0010 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0030 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0040 */ "\x01\x00\x01\x00\x00\x00\x00\x00\x01\x00\x01\x00\x00\x00\x00\x00" //................
/* 0050 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0060 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0070 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0080 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0090 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00b0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00c0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0110 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

           "\x00\x00\x10\x00\x00\x00\x01\x00"
           "\x11\x3f\x0a\x0a\xec\xec\x00\xff" // Green Rect
           ;

RED_AUTO_TEST_CASE(TestImageChunk)
{
    const char expected_stripped_wrm[] =
    /* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
               "\x03\x00\x14\x00\x0A\x00\x18\x00" // WRM version = 3, width = 20, height=10, bpp=24
               "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
               //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// Initial black PNG image
/* 0000 */ "\x00\x10\x50\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00\x3b\x37\xe9" //.............;7.
/* 0020 */ "\xb1\x00\x00\x00\x0f\x49\x44\x41\x54\x28\x91\x63\x60\x18\x05\xa3" //.....IDAT(.c`...
/* 0030 */ "\x80\x96\x00\x00\x02\x62\x00\x01\xfc\x4c\x5e\xbd\x00\x00\x00\x00" //.....b...L^.....
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`.

    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
    /* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // encode_color24()(RED) rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // encode_color24()(BLUE) RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // encode_color24()(WHITE) RECT

    /* 0000 */ "\x00\x10\x74\x00\x00\x00\x01\x00" // 0x1000: IMAGE_CHUNK 0048: chunk_len=86 0001: 1 order
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
        "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00"             //.............
        "\x3b\x37\xe9\xb1"                                                 //;7..
        "\x00\x00\x00"
/* 0000 */ "\x33\x49\x44\x41\x54\x28\x91\x63\x64\x60\xf8\xcf\x80\x1b\xfc\xff" //3IDAT(.cd`......
/* 0010 */ "\xcf\xc0\xc8\x88\x53\x96\x09\x8f\x4e\x82\x60\x88\x6a\x66\x41\xe3" //....S...N.`.jfA.
/* 0020 */ "\xff\x67\x40\x0b\x9f\xff\xc8\x22\x8c\xa8\xa1\x3b\x70\xce\x66\x1c" //.g@...."...;p.f.
/* 0030 */ "\xb0\x78\x06\x00\x69\xde\x0a\x12\x3d\x77\xd0\x9e\x00\x00\x00\x00" //.x..i...=w......
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`
    ;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(cstr_array_view(expected_stripped_wrm));
    TestGraphicToFile tgtf(trans, scr, false);
    GraphicToFile& consumer = tgtf.consumer;
    RDPDrawable& drawable = tgtf.drawable;

    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.sync();
    consumer.send_image_chunk();
}

RED_AUTO_TEST_CASE(TestImagePNGMediumChunks)
{
    // Same test as above but forcing use of small png chunks
    // Easier to do than write tests with huge pngs to force PNG chunking.

    const char expected[] =
    /* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
               "\x03\x00\x14\x00\x0A\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
               "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
               //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// Initial black PNG image
/* 0000 */ "\x00\x10\x50\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00\x3b\x37\xe9" //.............;7.
/* 0020 */ "\xb1\x00\x00\x00\x0f\x49\x44\x41\x54\x28\x91\x63\x60\x18\x05\xa3" //.....IDAT(.c`...
/* 0030 */ "\x80\x96\x00\x00\x02\x62\x00\x01\xfc\x4c\x5e\xbd\x00\x00\x00\x00" //.....b...L^.....
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`.

    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
    /* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // encode_color24()(RED) rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // encode_color24()(BLUE) RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // encode_color24()(WHITE) RECT

    /* 0000 */ "\x01\x10\x64\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order

        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
        "\x00\x00\x00\x14\x00\x00\x00\x0a"
        "\x08\x02\x00\x00\x00"             //.............
        "\x3b\x37\xe9\xb1"                                                 //;7..
        "\x00\x00\x00\x32\x49\x44\x41\x54"                                 //...2IDAT
        "\x28\x91\x63\xfc\xcf\x80\x17"
        "\xfc\xff\xcf\xc0\xc8\x88\x4b\x92"
        "\x09" //(.c..........K..
        "\xbf\x5e\xfc\x60\x88\x6a\x66\x41\xe3\x33\x32\xa0\x84\xe0\x7f"
        "\x54" //.^.`.jfA.32....T
        "\x91\xff\x0c\x28\x81\x37\x70\xce\x66\x1c\xb0\x78\x06\x00\x69\xdc" //...(.7p.f..x..i.
        "\x0a\x12"                                                         //..
        "\x86"
        "\x00\x10\x17\x00\x00\x00\x01\x00"  // 0x1000: FINAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x4a\x0c\x44"                                                 //.J.D
        "\x00\x00\x00\x00\x49\x45\x4e\x44"                             //....IEND
        "\xae\x42\x60\x82"                                             //.B`.
        ;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(cstr_array_view(expected));
    TestGraphicToFile tgtf(trans, scr, false);
    GraphicToFile& consumer = tgtf.consumer;
    RDPDrawable& drawable = tgtf.drawable;

    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.sync();

    OutChunkedBufferingTransport<100> png_trans(trans);
    RED_CHECK_NO_THROW(consumer.dump_png24(png_trans, true));
}

RED_AUTO_TEST_CASE(TestImagePNGSmallChunks)
{
    // Same test as above but forcing use of small png chunks
    // Easier to do than write tests with huge pngs to force PNG chunking.

    const char expected[] =
    /* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=28 0001: 1 order
               "\x03\x00\x14\x00\x0A\x00\x18\x00" // WRM version = 3, width = 20, height=10, bpp=24
               "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"
               //"\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // For WRM version >3

// Initial black PNG image
/* 0000 */ "\x00\x10\x50\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00\x3b\x37\xe9" //.............;7.
/* 0020 */ "\xb1\x00\x00\x00\x0f\x49\x44\x41\x54\x28\x91\x63\x60\x18\x05\xa3" //.....IDAT(.c`...
/* 0030 */ "\x80\x96\x00\x00\x02\x62\x00\x01\xfc\x4c\x5e\xbd\x00\x00\x00\x00" //.....b...L^.....
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`.

    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
    /* 0000 */ "\x00\x00\x1e\x00\x00\x00\x03\x00" // 0000: ORDERS  001A:chunk_len=26 0002: 2 orders
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // encode_color24()(RED) rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // encode_color24()(BLUE) RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // encode_color24()(WHITE) RECT

    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x14\x00\x00\x00\x0a"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x08\x02\x00\x00\x00\x3b\x37\xe9"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xb1\x00\x00\x00\x32\x49\x44\x41"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x28\x91\x63\xfc\xcf\x80\x17"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xfc\xff\xcf\xc0\xc8\x88\x4b\x92"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x09\xbf\x5e\xfc\x60\x88\x6a\x66"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x41\xe3\x33\x32\xa0\x84\xe0\x7f"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x91\xff\x0c\x28\x81\x37\x70"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xce\x66\x1c\xb0\x78\x06\x00\x69"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xdc\x0a\x12\x86\x4a\x0c\x44\x00"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x49\x45\x4e\x44\xae"
    /* 0000 */ "\x00\x10\x0b\x00\x00\x00\x01\x00" // 0x1000: FINAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x42\x60\x82"
        ;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(cstr_array_view(expected));
    TestGraphicToFile tgtf(trans, scr, false);
    GraphicToFile& consumer = tgtf.consumer;
    RDPDrawable& drawable = tgtf.drawable;

    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    consumer.sync();

    OutChunkedBufferingTransport<16> png_trans(trans);
    consumer.dump_png24(png_trans, true);
    // drawable.dump_png24(png_trans, true); true);
}

RED_AUTO_TEST_CASE(TestReadPNGFromTransport)
{
    auto source_png =
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
        "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00"             //.............
        "\x3b\x37\xe9\xb1"                                                 //;7..
        "\x00\x00\x00\x32\x49\x44\x41\x54"                                 //...2IDAT
        "\x28\x91\x63\xfc\xcf\x80\x17\xfc\xff\xcf\xc0\xc8\x88\x4b\x92\x09" //(.c..........K..
        "\xbf\x5e\xfc\x60\x88\x6a\x66\x41\xe3\x33\x32\xa0\x84\xe0\x7f\x54" //.^.`.jfA.32....T
        "\x91\xff\x0c\x28\x81\x37\x70\xce\x66\x1c\xb0\x78\x06\x00\x69\xdc" //...(.7p.f..x..i.
        "\x0a\x12"                                                         //..
        "\x86\x4a\x0c\x44"                                                 //.J.D
        "\x00\x00\x00\x00\x49\x45\x4e\x44"                                 //....IEND
        "\xae\x42\x60\x82"                                                 //.B`.
        ""_av
    ;

    RDPDrawable d(20, 10);
    GeneratorTransport in_png_trans(source_png);
    read_png24(in_png_trans, gdi::get_mutable_image_view(d));
    BufTransport png_trans;
    dump_png24(png_trans, d, true);
}

const char source_wrm_png[] =
    /* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
               "\x03\x00\x14\x00\x0A\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24 PAD: 2 bytes
               "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"

// Initial black PNG image
/* 0000 */ "\x00\x10\x50\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00\x3b\x37\xe9" //.............;7.
/* 0020 */ "\xb1\x00\x00\x00\x0f\x49\x44\x41\x54\x28\x91\x63\x60\x18\x05\xa3" //.....IDAT(.c`...
/* 0030 */ "\x80\x96\x00\x00\x02\x62\x00\x01\xfc\x4c\x5e\xbd\x00\x00\x00\x00" //.....b...L^.....
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`.

    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x14\x00\x00\x00\x0a"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x08\x02\x00\x00\x00\x3b\x37\xe9"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xb1\x00\x00\x00\x32\x49\x44\x41"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x28\x91\x63\xfc\xcf\x80\x17"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xfc\xff\xcf\xc0\xc8\x88\x4b\x92"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x09\xbf\x5e\xfc\x60\x88\x6a\x66"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x41\xe3\x33\x32\xa0\x84\xe0\x7f"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x91\xff\x0c\x28\x81\x37\x70"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xce\x66\x1c\xb0\x78\x06\x00\x69"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xdc\x0a\x12\x86\x4a\x0c\x44\x00"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x49\x45\x4e\x44\xae"
    /* 0000 */ "\x00\x10\x0b\x00\x00\x00\x01\x00" // 0x1000: FINAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x42\x60\x82"
        ;

   const char source_wrm_png_then_other_chunk[] =
    /* 0000 */ "\xEE\x03\x1C\x00\x00\x00\x01\x00" // 03EE: META 0010: chunk_len=16 0001: 1 order
               "\x03\x00\x14\x00\x0A\x00\x18\x00" // WRM version 3, width = 20, height=10, bpp=24
               "\x58\x02\x00\x01\x2c\x01\x00\x04\x06\x01\x00\x10"

// Initial black PNG image
/* 0000 */ "\x00\x10\x50\x00\x00\x00\x01\x00"
/* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" //.PNG........IHDR
/* 0010 */ "\x00\x00\x00\x14\x00\x00\x00\x0a\x08\x02\x00\x00\x00\x3b\x37\xe9" //.............;7.
/* 0020 */ "\xb1\x00\x00\x00\x0f\x49\x44\x41\x54\x28\x91\x63\x60\x18\x05\xa3" //.....IDAT(.c`...
/* 0030 */ "\x80\x96\x00\x00\x02\x62\x00\x01\xfc\x4c\x5e\xbd\x00\x00\x00\x00" //.....b...L^.....
/* 0040 */ "\x49\x45\x4e\x44\xae\x42\x60\x82"                                 //IEND.B`.

    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xCA\x9A\x3B\x00\x00\x00\x00" // 0x000000003B9ACA00 = 1000000000
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x14\x00\x00\x00\x0a"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x08\x02\x00\x00\x00\x3b\x37\xe9"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xb1\x00\x00\x00\x32\x49\x44\x41"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x28\x91\x63\xfc\xcf\x80\x17"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xfc\xff\xcf\xc0\xc8\x88\x4b\x92"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x09\xbf\x5e\xfc\x60\x88\x6a\x66"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x41\xe3\x33\x32\xa0\x84\xe0\x7f"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x91\xff\x0c\x28\x81\x37\x70"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xce\x66\x1c\xb0\x78\x06\x00\x69"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xdc\x0a\x12\x86\x4a\x0c\x44\x00"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x49\x45\x4e\x44\xae"
    /* 0000 */ "\x00\x10\x0b\x00\x00\x00\x01\x00" // 0x1000: FINAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x42\x60\x82"
    /* 0000 */ "\xf0\x03\x10\x00\x00\x00\x01\x00" // 03F0: TIMESTAMP 0010: chunk_len=16 0001: 1 order
    /* 0000 */ "\x00\xD3\xD7\x3B\x00\x00\x00\x00" // 0x000000003bd7d300 = 1004000000
       ;

RED_AUTO_TEST_CASE(TestReload)
{
    struct Test
    {
        char const* name;
        array_view_const_char data;
        int file_len;
        time_t time;
    };

    RED_TEST_CONTEXT_DATA(auto const& test, test.name, {
        Test{"TestReloadSaveCache", cstr_array_view(expected_Red_on_Blue_wrm), 298, 1001},
        Test{"TestReloadOrderStates", cstr_array_view(expected_reset_rect_wrm), 341, 1001},
        Test{"TestContinuationOrderStates", cstr_array_view(expected_continuation_wrm), 341, 1001},
        Test{"testimg", cstr_array_view(source_wrm_png), 107, 1000},
        Test{"testimg_then_other_chunk", cstr_array_view(source_wrm_png_then_other_chunk), 107, 1004}
    })
    {
        BufTransport trans;

        {
            GeneratorTransport in_wrm_trans(test.data);
            timeval begin_capture;
            begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
            timeval end_capture;
            end_capture.tv_sec = 0; end_capture.tv_usec = 0;
            FileToGraphic player(
                in_wrm_trans, begin_capture, end_capture,
                false, false, to_verbose_flags(0));
            RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
            player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
            while (player.next_order()){
                player.interpret_order();
            }
            ::dump_png24(trans, drawable, true);
            RED_CHECK_EQUAL(test.time, static_cast<unsigned>(player.record_now.tv_sec));
        }

        RED_TEST(trans.size() == test.file_len);
    }
}

RED_AUTO_TEST_CASE(TestKbdCapture)
{
    struct : NullReportMessage {
        std::string s;

        void log6(const std::string &info, const ArcsightLogInfo &  /*unused*/, const timeval  /*unused*/) override {
            s += info;
        }
    } report_message;

    timeval const time = {0, 0};
    Capture::SessionLogKbd kbd_capture(report_message);

    {
        kbd_capture.kbd_input(time, 'a');
        // flush report buffer then empty buffer
        kbd_capture.flush();

        RED_CHECK_EQUAL(report_message.s.size(), 25);
        RED_CHECK_EQUAL("type=\"KBD_INPUT\" data=\"a\"", report_message.s);
    }

    kbd_capture.enable_kbd_input_mask(true);
    report_message.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        // prob is not enabled
        RED_CHECK_EQUAL(report_message.s.size(), 0);
    }

    kbd_capture.enable_kbd_input_mask(false);
    report_message.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');

        RED_CHECK_EQUAL(report_message.s.size(), 0);

        kbd_capture.enable_kbd_input_mask(true);

        RED_CHECK_EQUAL(report_message.s.size(), 25);
        RED_CHECK_EQUAL("type=\"KBD_INPUT\" data=\"a\"", report_message.s);
        report_message.s.clear();

        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        RED_CHECK_EQUAL(report_message.s.size(), 0);
    }
}

RED_AUTO_TEST_CASE(TestKbdCapture2)
{
    struct : NullReportMessage {
        std::string s;

        void log6(const std::string &info, const ArcsightLogInfo &  /*unused*/, const timeval  /*unused*/) override {
            s += info;
        }
    } report_message;

    timeval const now = {0, 0};
    Capture::SessionLogKbd kbd_capture(report_message);

    {
        kbd_capture.kbd_input(now, 't');

        kbd_capture.session_update(now, cstr_array_view("INPUT_LANGUAGE=fr\x01xy\\z"));

        RED_CHECK_EQUAL(report_message.s.size(), 0);

        kbd_capture.kbd_input(now, 'o');

        kbd_capture.kbd_input(now, 't');

        kbd_capture.kbd_input(now, 'o');

        kbd_capture.possible_active_window_change();

        RED_CHECK_EQUAL("type=\"KBD_INPUT\" data=\"toto\"", report_message.s);
    }
}

RED_AUTO_TEST_CASE(TestKbdCapturePatternNotify)
{
    struct : NullReportMessage {
        std::string s;

        void report(const char* reason, const char* message) override
        {
            str_append(s, reason, " -- ", message, "\n");
        }
    } report_message;

    Capture::PatternKbd kbd_capture(&report_message, "$kbd:abcd", nullptr);

    char const str[] = "abcdaaaaaaaaaaaaaaaabcdeaabcdeaaaaaaaaaaaaabcde";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }

    RED_CHECK_EQUAL(4, pattern_count);
    RED_CHECK_EQUAL(
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
      , report_message.s
    );
}


RED_AUTO_TEST_CASE(TestKbdCapturePatternKill)
{
    struct : NullReportMessage {
        bool is_killed = false;

        void report(const char*  /*reason*/, const char*  /*message*/) override {
            this->is_killed = true;
        }
    } report_message;

    Capture::PatternKbd kbd_capture(&report_message, "$kbd:ab/cd", nullptr);

    char const str[] = "abcdab/cdaa";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }
    RED_CHECK_EQUAL(1, pattern_count);
    RED_CHECK(report_message.is_killed);
}




RED_AUTO_TEST_CASE(TestSample0WRM)
{
    int fd = ::open(FIXTURES_PATH "/sample0.wrm", O_RDONLY);
    RED_REQUIRE_NE(fd, -1);

    InFileTransport in_wrm_trans(unique_fd{fd});
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);

    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr, nullptr);

    BufSequenceTransport out_wrm_trans;

    const struct ToCacheOption {
        ToCacheOption()= default;
        BmpCache::CacheOption operator()(const BmpCache::cache_ & cache) const {
            return BmpCache::CacheOption(cache.entries(), cache.bmp_size(), cache.persistent());
        }
    } to_cache_option;

    BmpCache bmp_cache(
        BmpCache::Recorder,
        player.bmp_cache->bpp,
        player.bmp_cache->number_of_cache,
        player.bmp_cache->use_waiting_list,
        to_cache_option(player.bmp_cache->get_cache(0)),
        to_cache_option(player.bmp_cache->get_cache(1)),
        to_cache_option(player.bmp_cache->get_cache(2)),
        to_cache_option(player.bmp_cache->get_cache(3)),
        to_cache_option(player.bmp_cache->get_cache(4))
    );
    GlyphCache gly_cache;
    PointerCache ptr_cache;

    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    GraphicToFile graphic_to_file(
        player.record_now, out_wrm_trans, BitsPerPixel{24}, false,
        bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression
    );
    WrmCaptureImpl::NativeCaptureLocal wrm_recorder(graphic_to_file, player.record_now, std::chrono::seconds{1}, std::chrono::seconds{20});

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    player.add_consumer(&graphic_to_file, &wrm_recorder, nullptr, nullptr, &wrm_recorder, nullptr);

    bool requested_to_stop = false;

    RED_CHECK_EQUAL(1352304810u, static_cast<unsigned>(player.record_now.tv_sec));
    player.play(requested_to_stop);

    BufTransport out_png_trans;
    ::dump_png24(out_png_trans, drawable, true);
    RED_TEST(out_png_trans.size() == 21280);

    RED_CHECK_EQUAL(1352304870u, static_cast<unsigned>(player.record_now.tv_sec));

    graphic_to_file.sync();

    RED_TEST(out_wrm_trans.size() == 3);
    RED_TEST(out_wrm_trans[0].size() == 490454);
    RED_TEST(out_wrm_trans[1].size() == 1008253);
    RED_TEST(out_wrm_trans[2].size() == 195756);
}

RED_AUTO_TEST_CASE(TestReadPNGFromChunkedTransport)
{
    auto source_png =
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x14\x00\x00\x00\x0a"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x08\x02\x00\x00\x00\x3b\x37\xe9"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xb1\x00\x00\x00\x32\x49\x44\x41"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x28\x91\x63\xfc\xcf\x80\x17"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xfc\xff\xcf\xc0\xc8\x88\x4b\x92"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x09\xbf\x5e\xfc\x60\x88\x6a\x66"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x41\xe3\x33\x32\xa0\x84\xe0\x7f"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x54\x91\xff\x0c\x28\x81\x37\x70"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xce\x66\x1c\xb0\x78\x06\x00\x69"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\xdc\x0a\x12\x86\x4a\x0c\x44\x00"
    /* 0000 */ "\x01\x10\x10\x00\x00\x00\x01\x00" // 0x1000: PARTIAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x00\x00\x00\x49\x45\x4e\x44\xae"
    /* 0000 */ "\x00\x10\x0b\x00\x00\x00\x01\x00" // 0x1000: FINAL_IMAGE_CHUNK 0048: chunk_len=100 0001: 1 order
        "\x42\x60\x82"
        ""_av
    ;

    InStream stream(source_png);

    uint16_t chunk_type = stream.in_uint16_le();
    uint32_t chunk_size = stream.in_uint32_le();
    uint16_t chunk_count = stream.in_uint16_le();
    (void)chunk_count;

    GeneratorTransport in_png_trans(source_png.from_at(8));

    RDPDrawable d(20, 10);
    gdi::GraphicApi * gdi = &d;
    set_rows_from_image_chunk(in_png_trans, WrmChunkType(chunk_type), chunk_size, d.width(), {&gdi, 1});

    BufTransport png_trans;
    ::dump_png24(png_trans, d, true);
    RED_TEST(png_trans.size() == 107);
}


RED_AUTO_TEST_CASE(TestPatternSearcher)
{
    PatternSearcher searcher(utils::MatchFinder::KBD_INPUT, "$kbd:e");
    bool check = false;
    auto report = [&](auto&, auto&){ check = true; };
    searcher.test_uchar(ZStrUtf8Char('e'), report); RED_CHECK(check); check = false;
    searcher.test_uchar(ZStrUtf8Char('a'), report); RED_CHECK(!check);
    // #15241: Pattern detection crash
    searcher.test_uchar(ZStrUtf8Char('e'), report); RED_CHECK(check);
}

extern "C" {
    inline int hmac_fn(uint8_t * buffer)
    {
        // E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
        uint8_t hmac_key[] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        static_assert(sizeof(hmac_key) == MD_HASH::DIGEST_LENGTH );
        memcpy(buffer, hmac_key, sizeof(hmac_key));
        return 0;
    }

    inline int trace_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2 };
        static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH );
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

#ifndef REDEMPTION_NO_FFMPEG
#include "lib/do_recorder.hpp"

RED_AUTO_TEST_CASE(TestMetaCapture)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory record_wd("record");


    KbdLogParams kbd_log_params {};
    kbd_log_params.wrm_keyboard_log = true;
    test_capture_context("test_capture", CaptureFlags::wrm,
        100, 100, record_wd, hash_wd, [](Capture& capture, Rect /*scr*/)
    {
        timeval now{1000, 0};
        now.tv_sec += 10;

        bool ignore_frame_in_timeval = true;
        capture.periodic_snapshot(now, 0, 5, ignore_frame_in_timeval);

        capture.session_update(now, cstr_array_view("NEW_PROCESS=def")); now.tv_sec++;

        capture.session_update(now, cstr_array_view("COMPLETED_PROCESS=def")); now.tv_sec++;

        capture.kbd_input(now, 'W');
        capture.kbd_input(now, 'a');
        capture.kbd_input(now, 'l'); now.tv_sec++;

        capture.session_update(now, cstr_array_view("NEW_PROCESS=abc")); now.tv_sec++;

        capture.kbd_input(now, 'l');
        capture.kbd_input(now, 'i');
        capture.kbd_input(now, 'x'); now.tv_sec++;

        capture.session_update(now, cstr_array_view("COMPLETED_PROCESS=abc")); now.tv_sec++;
    }, kbd_log_params);

    auto mwrm_file = record_wd.add_file("test_capture.mwrm");

    RED_TEST_FILE_SIZE(mwrm_file, 124 + record_wd.dirname().size() * 2);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000000.wrm"), 166);
    RED_TEST_FILE_SIZE(record_wd.add_file("test_capture-000001.wrm"), 907);

    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture-000000.wrm"), 45);
    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture-000001.wrm"), 45);
    RED_TEST_FILE_SIZE(hash_wd.add_file("test_capture.mwrm"), 39);

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(record_wd);

#define TEST_DO_MAIN(argv, res_result, hmac_fn, trace_fn, output, output_error) { \
    int argc = sizeof(argv)/sizeof(char*);                                        \
    tu::ostream_buffered cout_buf;                                                \
    tu::ostream_buffered cerr_buf(std::cerr);                                     \
    int res = do_main(argc, argv, hmac_fn, trace_fn);                             \
    EVP_cleanup();                                                                \
    RED_CHECK_SMEM(cout_buf.str(), output);                                       \
    RED_CHECK_SMEM(cerr_buf.str(), output_error);                                 \
    RED_TEST(res_result == res);                                                  \
}

    {
        WorkingDirectory output_wd("output1");
        auto output_prefix1 = output_wd.dirname().string() + "test_capture.mwrm";

        char const * argv[] {
            "recorder.py",
            "redrec",
            "-i", mwrm_file,
            "--mwrm-path", FIXTURES_PATH,
            "-o", output_prefix1.c_str(),
            "--chunk",
            "--video-codec", "mp4",
            "--json-pgs",
        };

        TEST_DO_MAIN(argv, 0, hmac_fn, trace_fn,
            str_concat("Output file is \"", output_prefix1, "\".\n\n"), ""_av);

        RED_CHECK_FILE_CONTENTS(output_wd.add_file("test_capture.meta"),
            "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"KBD_INPUT\" data=\"Wallix\"\n"_av);

        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture-000000.mp4"), 3565 +- 200_v);
        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture-000000.png"), 244);
        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture.pgs"), 37);

        RED_CHECK_WORKSPACE(output_wd);
    }

    {
        WorkingDirectory output_wd("output2");
        auto output_prefix2 = output_wd.dirname().string() + "test_capture.mwrm";

        char const * argv[] {
            "recorder.py",
            "redrec",
            "-i", mwrm_file,
            "--config-file", FIXTURES_PATH "/disable_kbd_inpit_in_meta.ini",
            "--mwrm-path", FIXTURES_PATH,
            "-o", output_prefix2.c_str(),
            "--chunk",
            "--video-codec", "mp4",
            "--json-pgs",
        };

        TEST_DO_MAIN(argv, 0, hmac_fn, trace_fn,
            str_concat("Output file is \"", output_prefix2, "\".\n\n"), ""_av);

        RED_CHECK_FILE_CONTENTS(output_wd.add_file("test_capture.meta"),
            "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n"_av);


        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture-000000.mp4"), 3565 +- 200_v);
        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture-000000.png"), 244);
        RED_TEST_FILE_SIZE(output_wd.add_file("test_capture.pgs"), 37);

        RED_CHECK_WORKSPACE(output_wd);
    }
}
#endif
