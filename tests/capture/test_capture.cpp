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

#define RED_TEST_MODULE TestCapture
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/capture.hpp"
#include "capture/capture.cpp" // Yeaaahh...
#include "capture/file_to_graphic.hpp"
#include "capture/params_from_ini.hpp"
#include "configs/config.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/fake_stat.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/transport/test_transport.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/transport.hpp"
#include "utils/drawable.hpp"
#include "utils/fileutils.hpp"
#include "utils/png.hpp"
#include "utils/stream.hpp"

const auto file_not_exists = std::not_fn<bool(char const*)>(file_exist);

RED_AUTO_TEST_CASE(TestSplittedCapture)
{
    const struct CheckFiles {
        const char * filename;
        ssize_t size;
    } fileinfo[] = {
        {"./test_capture-000000.wrm", 1646},
        {"./test_capture-000001.wrm", 3508},
        {"./test_capture-000002.wrm", 3463},
        {"./test_capture-000003.wrm", -1},
        {"./test_capture.mwrm", 180},
        // hash
        {"/tmp/test_capture-000000.wrm", 45},
        {"/tmp/test_capture-000001.wrm", 45},
        {"/tmp/test_capture-000002.wrm", 45},
        {"/tmp/test_capture-000003.wrm", -1},
        {"/tmp/test_capture.mwrm", 39},
    };

    for (auto & f : fileinfo) {
        ::unlink(f.filename);
    }

    Inifile ini;
    ini.set<cfg::video::rt_display>(1);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        ini.set<cfg::video::frame_interval>(std::chrono::seconds{1});
        ini.set<cfg::video::break_interval>(std::chrono::seconds{3});

        ini.set<cfg::video::png_limit>(10); // one snapshot by second
        ini.set<cfg::video::png_interval>(std::chrono::seconds{1});

        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
        CaptureFlags capture_flags = CaptureFlags::wrm | CaptureFlags::png;

        ini.set<cfg::globals::trace_type>(TraceType::localfile);

        ini.set<cfg::video::record_tmp_path>("./");
        ini.set<cfg::video::record_path>("./");
        ini.set<cfg::video::hash_path>("/tmp/");
        ini.set<cfg::globals::movie_path>("test_capture");

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        // TODO remove this after unifying capture interface
        bool full_video = false;

        VideoParams video_params = video_params_from_ini(scr.cx, scr.cy,
            std::chrono::seconds::zero(), ini);
        video_params.no_timestamp = false;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = record_tmp_path;

        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        bool capture_png = bool(capture_flags & CaptureFlags::png);
        bool capture_pattern_checker = false;

        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        bool capture_video = bool(capture_flags & CaptureFlags::video);
        bool capture_video_full = full_video;
        bool capture_meta = capture_ocr;
        bool capture_kbd = false;

        OcrParams ocr_params = {
            ini.get<cfg::ocr::version>(),
            ocr::locale::LocaleId(
                static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
            ini.get<cfg::ocr::on_title_bar_only>(),
            ini.get<cfg::ocr::max_unrecog_char_rate>(),
            ini.get<cfg::ocr::interval>(),
            ini.get<cfg::debug::ocr>()
        };

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, app_path(AppPath::Wrm)); // default value, actual one should come from movie_path
        strcat(path, "/");
        strcpy(basename, movie_path);
        strcpy(extension, "");          // extension is currently ignored

        RED_CHECK(canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)));

        PngParams png_params = {
            0, 0, std::chrono::milliseconds{60}, 100, 0, false,
            false, static_cast<bool>(ini.get<cfg::video::rt_display>())};

        DrawableParams const drawable_params{scr.cx, scr.cy, nullptr};

        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
        kbd_log_params.session_log_enabled = false;

        PatternParams const pattern_params = pattern_params_from_ini(ini);

        SequencedVideoParams sequenced_video_params;
        FullVideoParams full_video_params;

        cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

        WrmParams const wrm_params = wrm_params_from_ini(BitsPerPixel{24}, false, cctx, rnd, fstat, hash_path, ini);

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
                          capture_params
                        , drawable_params
                        , capture_wrm, wrm_params
                        , capture_png, png_params
                        , capture_pattern_checker, pattern_params
                        , capture_ocr, ocr_params
                        , capture_video, sequenced_video_params
                        , capture_video_full, full_video_params
                        , capture_meta, meta_params
                        , capture_kbd, kbd_log_params
                        , video_params
                        , nullptr
                        , Rect()
                        );

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    {
        FilenameGenerator png_seq(
//            FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION
          , "./" , "test_capture", ".png"
        );

        const char * filename;

        filename = png_seq.get(0);
        RED_CHECK_EQUAL(3098, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(1);
        RED_CHECK_EQUAL(3125, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(2);
        RED_CHECK_EQUAL(3140, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(3);
        RED_CHECK_EQUAL(3158, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(4);
        RED_CHECK_EQUAL(3172, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(5);
        RED_CHECK_EQUAL(3197, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(6);
        RED_CHECK_EQUAL(3223, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(7);
        RED_CHECK_PREDICATE(file_not_exists, (filename));
    }

    for (auto x: fileinfo) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
        ::unlink(x.filename);
    }
}

RED_AUTO_TEST_CASE(TestBppToOtherBppCapture)
{
    Inifile ini;
    ini.set<cfg::video::rt_display>(1);

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

//    Rect scr(0, 0, 12, 10);


      Rect scr(0, 0, 100, 100);

    ini.set<cfg::video::frame_interval>(std::chrono::seconds{1});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{3});

    ini.set<cfg::video::png_limit>(10); // one snapshot by second
    ini.set<cfg::video::png_interval>(std::chrono::seconds{1});

    ini.set<cfg::video::capture_flags>(CaptureFlags::png);
    CaptureFlags capture_flags = CaptureFlags::png;

    ini.set<cfg::globals::trace_type>(TraceType::localfile);

    ini.set<cfg::video::record_tmp_path>("./");
    ini.set<cfg::video::record_path>("./");
    ini.set<cfg::video::hash_path>("/tmp");
    ini.set<cfg::globals::movie_path>("test_capture");

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;

    // TODO remove this after unifying capture interface
    bool full_video = false;
    // TODO remove this after unifying capture interface
    bool no_timestamp = false;

    VideoParams video_params = video_params_from_ini(scr.cx, scr.cy,
        std::chrono::seconds::zero(), ini);
    video_params.no_timestamp = no_timestamp;
    const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
    const char * record_path = record_tmp_path;
    bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
    bool capture_png = bool(capture_flags & CaptureFlags::png);
    bool capture_pattern_checker = false;

    bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
    bool capture_video = bool(capture_flags & CaptureFlags::video);
    bool capture_video_full = full_video;
    bool capture_meta = capture_ocr;
    bool capture_kbd = false;

    OcrParams const ocr_params = ocr_params_from_ini(ini);

    const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
    const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
    const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

    char path[1024];
    char basename[1024];
    char extension[128];
    strcpy(path, app_path(AppPath::Wrm));     // default value, actual one should come from movie_path
    strcat(path, "/");
    strcpy(basename, movie_path);
    strcpy(extension, "");          // extension is currently ignored

    RED_CHECK(canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)));

    PngParams png_params = {
        0, 0, std::chrono::milliseconds{60}, 100, 0, false,
        false, static_cast<bool>(ini.get<cfg::video::rt_display>())};

    DrawableParams const drawable_params{scr.cx, scr.cy, nullptr};

    MetaParams meta_params{
        MetaParams::EnableSessionLog::No,
        MetaParams::HideNonPrintable::No,
        MetaParams::LogClipboardActivities::Yes,
        MetaParams::LogFileSystemActivities::Yes,
        MetaParams::LogOnlyRelevantClipboardActivities::Yes
    };

    KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
    kbd_log_params.session_log_enabled = false;

    PatternParams const pattern_params = pattern_params_from_ini(ini);

    SequencedVideoParams sequenced_video_params;
    FullVideoParams full_video_params;

    cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

    WrmParams const wrm_params = wrm_params_from_ini(BitsPerPixel{24}, false, cctx, rnd, fstat, hash_path, ini);

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
                     capture_params
                   , drawable_params
                   , capture_wrm, wrm_params
                   , capture_png, png_params
                   , capture_pattern_checker, pattern_params
                   , capture_ocr, ocr_params
                   , capture_video, sequenced_video_params
                   , capture_video_full, full_video_params
                   , capture_meta, meta_params
                   , capture_kbd, kbd_log_params
                   , video_params
                   , nullptr
                   , Rect()
                   );
    auto const color_cxt = gdi::ColorCtx::depth16();
    capture.set_pointer(edit_pointer());

    bool ignore_frame_in_timeval = true;

    capture.draw(RDPOpaqueRect(scr, encode_color16()(BLUE)), scr, color_cxt);
    now.tv_sec++;
    capture.periodic_snapshot(now, 0, 5, ignore_frame_in_timeval);

    const char * filename = "./test_capture-000000.png";

    RED_CHECK_SIG(
        get_file_contents(filename), "\xbd\x6a\x84\x08\x3e\xe7\x19\xab\xb0\x67\xeb\x72\x94\x1f\xea\x26\xc4\x69\xe1\x37");
    ::unlink(filename);
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
        PatternsChecker checker(
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


RED_AUTO_TEST_CASE(TestSessionMeta)
{
    BufTransport trans;

    {
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

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
        meta.periodic_snapshot(now, 0, 0, 0);
        send_kbd();
        meta.title_changed(now.tv_sec, cstr_array_view("Blah1"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2"));
        now.tv_sec += 1;
        send_kbd();
        send_kbd();
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, '\"');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
            now.tv_sec += 1;
        };
        send_kbd();
        meta.periodic_snapshot(now, 0, 0, 0);
        send_kbd();
        meta.title_changed(now.tv_sec, cstr_array_view("Bl\"ah1"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah\\2"));
        meta.session_update(now, cstr_array_view("INPUT_LANGUAGE=fr\x01xy\\z"));
        now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.next_video(now.tv_sec);
    }

    RED_CHECK_EQ(
        trans.buf,
        "1970-01-01 01:16:41 + type=\"TITLE_BAR\" data=\"Blah1\"\n"
        "1970-01-01 01:16:42 - type=\"BUTTON_CLICKED\" windows=\"\" button=\"Démarrer\"\n"
        "1970-01-01 01:16:43 + type=\"TITLE_BAR\" data=\"Blah2\"\n"
        "1970-01-01 01:16:46 + type=\"TITLE_BAR\" data=\"Blah3\"\n"
        "1970-01-01 01:16:47 + (break)\n"
        "1970-01-01 01:16:47 - type=\"KBD_INPUT\" data=\"ABCDABCDABCD\"\n"
    );
}


RED_AUTO_TEST_CASE(TestSessionMeta4)
{
    BufTransport trans;

    {
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

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

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);

        meta.kbd_input(now, 'A'); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.kbd_input(now, 'B');

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.kbd_input(now, 'C'); now.tv_sec += 1;

        meta.possible_active_window_change();

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        meta.kbd_input(now, 'D'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'E'); now.tv_sec += 1;
        meta.kbd_input(now, 'F'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'G'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\t'); now.tv_sec += 1;
        meta.kbd_input(now, 'H'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, false, meta_params);
        SessionLogAgent log_agent(meta, meta_params);

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
        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        timeval now;
        now.tv_sec  = 1000;
        now.tv_usec = 0;
        SessionMeta meta(now, trans, true, meta_params);

        meta.kbd_input(now, 'A'); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.kbd_input(now, 'B');

        meta.session_update(now, {"BUTTON_CLICKED=\x01" "Démarrer", 25}); now.tv_sec += 1;

        meta.kbd_input(now, 'C'); now.tv_sec += 1;

        meta.possible_active_window_change();

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        meta.kbd_input(now, 'D'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'E'); now.tv_sec += 1;
        meta.kbd_input(now, 'F'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, 'G'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\r'); now.tv_sec += 1;
        meta.kbd_input(now, '\t'); now.tv_sec += 1;
        meta.kbd_input(now, 'H'); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
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


class DrawableToFile
{
    Transport & trans;
    const Drawable & drawable;

public:
    DrawableToFile(Transport & trans, const Drawable & drawable)
    : trans(trans)
    , drawable(drawable)
    {
    }

    ~DrawableToFile() = default;

    bool logical_frame_ended() const {
        return this->drawable.logical_frame_ended;
    }

    void flush() {
        ::dump_png24(this->trans, this->drawable, true);
    }
};

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
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1);

    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, encode_color24()(GREEN)), screen_rect, color_ctx);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), encode_color24()(BLUE)), screen_rect, color_ctx);
    consumer.sync();

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), encode_color24()(WHITE)), screen_rect, color_ctx);
    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), encode_color24()(RED));
    consumer.draw(cmd3, screen_rect, color_ctx);
    now.tv_sec++;
    consumer.timestamp(now);

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
    // Same as above, show timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm2, sizeof(expected_stripped_wrm2)-1);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, encode_color24()(GREEN)), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), encode_color24()(BLUE)), screen_rect, color_ctx);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), encode_color24()(WHITE)), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 150, 700, 30), encode_color24()(RED)), screen_rect, color_ctx);
    now.tv_sec+=6;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 10), encode_color24()(BLACK)), screen_rect, color_ctx);

    consumer.sync();
}

RED_AUTO_TEST_CASE(TestCaptureToWrmReplayToPng)
{
    // Same as above, show timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;

    const char * filename = "./testcap.wrm";
    size_t len = strlen(filename);
    char path[1024];
    memcpy(path, filename, len);
    path[len] = 0;
    int fd = ::creat(path, 0777);
    RED_REQUIRE_NE(fd, -1);

    OutFileTransport trans(unique_fd{fd});
    RED_CHECK_EQUAL(0, 0);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();
    RED_CHECK_EQUAL(0, 0);
    RDPOpaqueRect cmd0(screen_rect, encode_color24()(GREEN));
    consumer.draw(cmd0, screen_rect, color_ctx);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), encode_color24()(BLUE));
    consumer.draw(cmd1, screen_rect, color_ctx);
    now.tv_sec++;
    RED_CHECK_EQUAL(0, 0);
    consumer.timestamp(now);
    consumer.sync();
    RED_CHECK_EQUAL(0, 0);

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), encode_color24()(WHITE));
    consumer.draw(cmd2, screen_rect, color_ctx);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), encode_color24()(RED));
    consumer.draw(cmd3, screen_rect, color_ctx);
    now.tv_sec+=6;
    consumer.timestamp(now);
    consumer.sync();
    RED_CHECK_EQUAL(0, 0);
    trans.disconnect(); // close file before reading filesize
    RED_CHECK_EQUAL(1588, filesize(filename));

    char in_path[1024];
    len = strlen(filename);
    memcpy(in_path, filename, len);
    in_path[len] = 0;

    fd = ::open(in_path, O_RDONLY);
    RED_REQUIRE_NE(fd, -1);
    InFileTransport in_wrm_trans(unique_fd{fd});

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testcap", ".png", groupid, ReportError{});

    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl());
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr, nullptr);

    png_recorder.flush();
    out_png_trans.next();

    // Green Rect
    RED_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Blue Rect
    RED_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Timestamp
    RED_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // White Rect
    RED_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Red Rect
    RED_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    RED_CHECK_EQUAL(false, player.next_order());
    in_wrm_trans.disconnect();

    // clear PNG files
    size_t sz[6] = {1476, 2786, 2800, 2800, 2814, 2823};
    for (int i = 0; i < 6 ; i++){
        const char * filename = out_png_trans.seqgen()->get(i);
        RED_CHECK_EQUAL(sz[i], ::filesize(filename));
        ::unlink(filename);
    }
   ::unlink("./testcap.wrm");
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
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1);
    trans.disable_remaining_error();
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();
    consumer.timestamp(now);

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

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.save_bmp_caches();

    consumer.sync();
}

RED_AUTO_TEST_CASE(TestReloadSaveCache)
{
    GeneratorTransport in_wrm_trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadSaveCache", ".png", groupid, ReportError{});
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl());

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();

    const char * filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(298, ::filesize(filename));
    ::unlink(filename);
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
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
    auto const color_cxt = gdi::ColorCtx::depth24();
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(5), encode_color24()(BLUE)), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(10), encode_color24()(RED)), scr, color_cxt);

    consumer.sync();

    consumer.send_save_state_chunk();

    now.tv_sec++;
    consumer.timestamp(now);
    consumer.draw(RDPOpaqueRect(scr.shrink(20), encode_color24()(GREEN)), scr, color_cxt);
    consumer.sync();
}

RED_AUTO_TEST_CASE(TestReloadOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadOrderStates", ".png", groupid, ReportError{});
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl());

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(341, ::filesize(filename));
    ::unlink(filename);
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

RED_AUTO_TEST_CASE(TestContinuationOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_continuation_wrm, sizeof(expected_continuation_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestContinuationOrderStates", ".png", groupid, ReportError{});
    const FilenameGenerator * seq = out_png_trans.seqgen();
    RED_CHECK(seq);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl());

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = seq->get(0);
    RED_CHECK_EQUAL(341, ::filesize(filename));
    ::unlink(filename);
}

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

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                        BmpCache::CacheOption(600, 256, false),
                        BmpCache::CacheOption(300, 1024, false),
                        BmpCache::CacheOption(262, 4096, false));
    PointerCache ptr_cache;
    GlyphCache gly_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
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

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(expected, sizeof(expected)-1);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
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

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 20, 10);
    CheckTransport trans(expected, sizeof(expected)-1);
    BmpCache bmp_cache(BmpCache::Recorder, BitsPerPixel{24}, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    GraphicToFile consumer(now, trans, BitsPerPixel{24}, false, bmp_cache, gly_cache, ptr_cache, drawable, WrmCompressionAlgorithm::no_compression);
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
    const char source_png[] =
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
    ;

    RDPDrawable d(20, 10);
    GeneratorTransport in_png_trans(source_png, sizeof(source_png)-1);
    read_png24(in_png_trans, gdi::get_mutable_image_view(d));
    const int groupid = 0;
    OutFilenameSequenceTransport png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, ReportError{});
    dump_png24(png_trans, d, true);
    ::unlink(png_trans.seqgen()->get(0));
}



RED_AUTO_TEST_CASE(TestExtractPNGImagesFromWRM)
{
   const char source_wrm[] =
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

    GeneratorTransport in_wrm_trans(source_wrm, sizeof(source_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, ReportError{});
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl());

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    out_png_trans.disconnect();
    const char * filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}


RED_AUTO_TEST_CASE(TestExtractPNGImagesFromWRMTwoConsumers)
{
   const char source_wrm[] =
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
        ;

    GeneratorTransport in_wrm_trans(source_wrm, sizeof(source_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));
    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, ReportError{});
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl());

    OutFilenameSequenceTransport second_out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "second_testimg", ".png", groupid, ReportError{});
    DrawableToFile second_png_recorder(second_out_png_trans, drawable1.impl());

    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }

    const char * filename;

    png_recorder.flush();
    filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);

    second_png_recorder.flush();
    filename = second_out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}


RED_AUTO_TEST_CASE(TestExtractPNGImagesThenSomeOtherChunk)
{
   const char source_wrm[] =
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

    GeneratorTransport in_wrm_trans(source_wrm, sizeof(source_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));
    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, ReportError{});
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl());

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    RED_CHECK_EQUAL(1004u, static_cast<unsigned>(player.record_now.tv_sec));

    const char * filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestKbdCapture)
{
    struct : NullReportMessage {
        std::string s;

        void log6(const std::string &info, const ArcsightLogInfo & , const timeval ) override {
            s += info;
        }
    } report_message;

    timeval const time = {0, 0};
    SessionLogKbd kbd_capture(report_message);

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

        void log6(const std::string &info, const ArcsightLogInfo & , const timeval ) override {
            s += info;
        }
    } report_message;

    timeval const now = {0, 0};
    SessionLogKbd kbd_capture(report_message);

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

    PatternKbd kbd_capture(&report_message, "$kbd:abcd", nullptr);

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
        bool is_killed = 0;

        void report(const char* , const char* ) override {
            this->is_killed = 1;
        }
    } report_message;

    PatternKbd kbd_capture(&report_message, "$kbd:ab/cd", nullptr);

    char const str[] = "abcdab/cdaa";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }
    RED_CHECK_EQUAL(1, pattern_count);
    RED_CHECK_EQUAL(report_message.is_killed, true);
}




RED_AUTO_TEST_CASE(TestSample0WRM)
{
    const char * input_filename = FIXTURES_PATH "/sample0.wrm";

    int fd = ::open(input_filename, O_RDONLY);
    RED_REQUIRE_NE(fd, -1);

    InFileTransport in_wrm_trans(unique_fd{fd});
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".png", groupid, ReportError{});
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl());

//    png_recorder.update_config(ini);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr, nullptr);

    OutFilenameSequenceTransport out_wrm_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".wrm", groupid, ReportError{});

    const struct ToCacheOption {
        ToCacheOption(){}
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

    png_recorder.flush();
    RED_CHECK_EQUAL(1352304870u, static_cast<unsigned>(player.record_now.tv_sec));

    graphic_to_file.sync();
    const char * filename;

    out_png_trans.disconnect();
    out_wrm_trans.disconnect();

    filename = out_png_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(21280, ::filesize(filename));
    ::unlink(filename);

    filename = out_wrm_trans.seqgen()->get(0);
    RED_CHECK_EQUAL(490454, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(1);
    RED_CHECK_EQUAL(1008253, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(2);
    RED_CHECK_EQUAL(195756, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestReadPNGFromChunkedTransport)
{
    const char source_png[] =
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

    GeneratorTransport in_png_trans(source_png, sizeof(source_png)-1);
    constexpr std::size_t sz_buf = 8;
    uint8_t buf[sz_buf];
    auto end = buf;
    in_png_trans.recv_boom(end, sz_buf); // skip first chunk header
    InStream stream(buf);

//    in_png_trans.recv(&stream.end, 107); // skip first chunk header

    uint16_t chunk_type = stream.in_uint16_le();
    uint32_t chunk_size = stream.in_uint32_le();
    uint16_t chunk_count = stream.in_uint16_le();
    (void)chunk_count;

    RDPDrawable d(20, 10);
    gdi::GraphicApi * gdi = &d;
    set_rows_from_image_chunk(in_png_trans, WrmChunkType(chunk_type), chunk_size, d.width(), {&gdi, 1});

    const int groupid = 0;
    OutFilenameSequenceTransport png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, ReportError{});
    dump_png24(png_trans, d, true);
    ::unlink(png_trans.seqgen()->get(0));
}


RED_AUTO_TEST_CASE(TestPatternSearcher)
{
    PatternSearcher searcher(utils::MatchFinder::KBD_INPUT, "$kbd:e");
    bool check = false;
    auto report = [&](auto&, auto&){ check = true; };
    searcher.test_uchar(byte_ptr_cast("e"), 1, report); RED_CHECK(check); check = false;
    searcher.test_uchar(byte_ptr_cast("a"), 1, report); RED_CHECK(!check);
    // #15241: Pattern detection crash
    searcher.test_uchar(byte_ptr_cast("e"), 1, report); RED_CHECK(check);
}


RED_AUTO_TEST_CASE(TestOutFilenameSequenceTransport)
{
    OutFilenameSequenceTransport fnt(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test_outfilenametransport", ".txt", getgid(), ReportError{});
    fnt.send("We write, ", 10);
    fnt.send("and again, ", 11);
    fnt.send("and so on.", 10);

    fnt.next();
    fnt.send(" ", 1);
    fnt.send("A new file.", 11);

    RED_CHECK_EQUAL(filesize(fnt.seqgen()->get(0)), 31);
    RED_CHECK_EQUAL(filesize(fnt.seqgen()->get(1)), 12);

    fnt.disconnect();
    unlink(fnt.seqgen()->get(0));
    unlink(fnt.seqgen()->get(1));
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
        static_assert(sizeof(hmac_key) == MD_HASH::DIGEST_LENGTH, "");
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
        static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH, "");
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

#ifndef REDEMPTION_NO_FFMPEG
#include "lib/do_recorder.hpp"

RED_AUTO_TEST_CASE(TestMetaCapture)
{
    const struct CheckFiles {
        const char * filename;
        ssize_t size;
    } fileinfo1[] = {
        {"./test_capture-000000.wrm", 226},
        {"./test_capture-000001.wrm", 174},
        {"./test_capture.mwrm", 198},
    };

    const struct CheckFiles fileinfo2[] = {
        {"/tmp/test_capture-000000.mp4", 3565},
        {"/tmp/test_capture-000000.png", 244},
        {"/tmp/test_capture-000000.wrm", 80},
        {"/tmp/test_capture-000001.wrm", 80},
        {"/tmp/test_capture.mwrm", 74},
        {"/tmp/test_capture.pgs", 37},
    };

    for (auto & f : fileinfo1) {
        ::unlink(f.filename);
    }
    for (auto & f : fileinfo2) {
        ::unlink(f.filename);
    }

    Inifile ini;
    ini.set<cfg::video::rt_display>(1);

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    ini.set<cfg::video::frame_interval>(std::chrono::seconds{1});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{3});

    ini.set<cfg::video::png_limit>(10); // one snapshot by second
    ini.set<cfg::video::png_interval>(std::chrono::seconds{1});

    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm);
    CaptureFlags capture_flags = CaptureFlags::wrm;

    ini.set<cfg::globals::trace_type>(TraceType::localfile);

    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);

    ini.set<cfg::video::record_tmp_path>("./");
    ini.set<cfg::video::record_path>("./");
    ini.set<cfg::video::hash_path>("/tmp");
    ini.set<cfg::globals::movie_path>("test_capture");

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;

    // TODO remove this after unifying capture interface
    bool full_video = false;
    // TODO remove this after unifying capture interface
    bool no_timestamp = false;

    Rect scr(0, 0, 100, 100);

    VideoParams video_params = video_params_from_ini(scr.cx, scr.cy,
        std::chrono::seconds::zero(), ini);
    video_params.no_timestamp = no_timestamp;
    const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
    const char * record_path = record_tmp_path;
    bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
    bool capture_png = bool(capture_flags & CaptureFlags::png);
    bool capture_pattern_checker = false;

    bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
    bool capture_video = bool(capture_flags & CaptureFlags::video);
    bool capture_video_full = full_video;
    bool capture_meta = capture_ocr;
    bool capture_kbd = true;

    OcrParams const ocr_params = ocr_params_from_ini(ini);

    const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
    const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
    const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

    char path[1024];
    char basename[1024];
    char extension[128];
    strcpy(path, app_path(AppPath::Wrm));     // default value, actual one should come from movie_path
    strcat(path, "/");
    strcpy(basename, movie_path);
    strcpy(extension, "");          // extension is currently ignored

    RED_CHECK(canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)));

    PngParams png_params = {
        0, 0, std::chrono::milliseconds{60}, 100, 0, false,
        false, static_cast<bool>(ini.get<cfg::video::rt_display>())};

    DrawableParams const drawable_params{scr.cx, scr.cy, nullptr};

    MetaParams meta_params{
        MetaParams::EnableSessionLog::No,
        MetaParams::HideNonPrintable::No,
        MetaParams::LogClipboardActivities::Yes,
        MetaParams::LogFileSystemActivities::Yes,
        MetaParams::LogOnlyRelevantClipboardActivities::Yes
    };

    KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
    kbd_log_params.wrm_keyboard_log = true;
    kbd_log_params.session_log_enabled = false;

    PatternParams const pattern_params = pattern_params_from_ini(ini);

    SequencedVideoParams sequenced_video_params;
    FullVideoParams full_video_params;

    cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

    WrmParams const wrm_params = wrm_params_from_ini(BitsPerPixel{24}, false, cctx, rnd, fstat, hash_path, ini);

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

    {
        Capture capture(
                         capture_params
                       , drawable_params
                       , capture_wrm, wrm_params
                       , capture_png, png_params
                       , capture_pattern_checker, pattern_params
                       , capture_ocr, ocr_params
                       , capture_video, sequenced_video_params
                       , capture_video_full, full_video_params
                       , capture_meta, meta_params
                       , capture_kbd, kbd_log_params
                       , video_params
                       , nullptr
                       , Rect()
                       );
    //    auto const color_cxt = gdi::ColorCtx::depth16();
    //    capture.set_pointer(edit_pointer());

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

        //bool ignore_frame_in_timeval = true;

    //    capture.draw(RDPOpaqueRect(scr, encode_color16()(BLUE)), scr, color_cxt);


//        capture.periodic_snapshot(now, 0, 5, ignore_frame_in_timeval);

    //    const char * filename = "./test_capture-000000.png";

    //    auto s = get_file_contents<std::string>(filename);
    //    RED_CHECK_SIG2(
    //        byte_ptr_cast(s.data()), s.size(),
    //        "\xbd\x6a\x84\x08\x3e\xe7\x19\xab\xb0\x67\xeb\x72\x94\x1f\xea\x26\xc4\x69\xe1\x37"
    //    );
    //    ::unlink(filename);
    }

    {
        char const * argv[] {
            "recorder.py",
            "redrec",
            "-i",
                "./test_capture.mwrm",
            "--mwrm-path", FIXTURES_PATH,
            "-o",
                "/tmp/test_capture",
            "--chunk",
            "--video-codec", "mp4",
            "--json-pgs",
        };
        int argc = sizeof(argv)/sizeof(char*);

        LOG__REDEMPTION__OSTREAM__BUFFERED cout_buf;
        int res = do_main(argc, argv, hmac_fn, trace_fn);
        EVP_cleanup();
        RED_CHECK_EQUAL(cout_buf.str(), "Output file is \"/tmp/test_capture.mwrm\".\n\n");
        RED_CHECK_EQUAL(0, res);

        RED_CHECK_FILE_CONTENTS("/tmp/test_capture.meta",
            "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"KBD_INPUT\" data=\"Wallix\"\n");

        for (auto & f : fileinfo2) {
            ::unlink(f.filename);
        }
    }

    {
        char const * argv[] {
            "recorder.py",
            "redrec",
            "-i",
                "./test_capture.mwrm",
            "--config-file",
                FIXTURES_PATH "/disable_kbd_inpit_in_meta.ini",
            "--mwrm-path", FIXTURES_PATH,
            "-o",
                "/tmp/test_capture",
            "--chunk",
            "--video-codec", "mp4",
            "--json-pgs",
        };
        int argc = sizeof(argv)/sizeof(char*);

        LOG__REDEMPTION__OSTREAM__BUFFERED cout_buf;
        int res = do_main(argc, argv, hmac_fn, trace_fn);
        EVP_cleanup();
        RED_CHECK_EQUAL(cout_buf.str(), "Output file is \"/tmp/test_capture.mwrm\".\n\n");
        RED_CHECK_EQUAL(0, res);

        RED_CHECK_FILE_CONTENTS("/tmp/test_capture.meta",
            "1970-01-01 01:16:50 - type=\"NEW_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:51 - type=\"COMPLETED_PROCESS\" command_line=\"def\"\n"
            "1970-01-01 01:16:53 - type=\"NEW_PROCESS\" command_line=\"abc\"\n"
            "1970-01-01 01:16:55 - type=\"COMPLETED_PROCESS\" command_line=\"abc\"\n");

        for (auto & f : fileinfo2) {
            ::unlink(f.filename);
        }
    }

    for (auto & f : fileinfo1) {
        ::unlink(f.filename);
    }
}
#endif

RED_AUTO_TEST_CASE(TestResizingCapture)
{
    const struct CheckFiles {
        const char * filename;
        ssize_t size;
    } fileinfo[] = {
        {"./resizing-capture-0-000000.wrm", 1651},
        {"./resizing-capture-0-000001.wrm", 3428},
        {"./resizing-capture-0-000002.wrm", 4384},
        {"./resizing-capture-0-000003.wrm", 4388},
        {"./resizing-capture-0-000004.wrm", -1},
        {"./resizing-capture-0.mwrm", 256},
        // hash
        {"/tmp/resizing-capture-0-000000.wrm", 51},
        {"/tmp/resizing-capture-0-000001.wrm", 51},
        {"/tmp/resizing-capture-0-000002.wrm", 51},
        {"/tmp/resizing-capture-0-000003.wrm", 51},
        {"/tmp/resizing-capture-0-000004.wrm", -1},
        {"/tmp/resizing-capture-0.mwrm", 45},
    };

    for (auto & f : fileinfo) {
        ::unlink(f.filename);
    }

    Inifile ini;
    ini.set<cfg::video::rt_display>(1);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        ini.set<cfg::video::frame_interval>(std::chrono::seconds{1});
        ini.set<cfg::video::break_interval>(std::chrono::seconds{3});

        ini.set<cfg::video::png_limit>(10); // one snapshot by second
        ini.set<cfg::video::png_interval>(std::chrono::seconds{1});

        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
        CaptureFlags capture_flags = CaptureFlags::wrm | CaptureFlags::png;

        ini.set<cfg::globals::trace_type>(TraceType::localfile);

        ini.set<cfg::video::record_tmp_path>("./");
        ini.set<cfg::video::record_path>("./");
        ini.set<cfg::video::hash_path>("/tmp/");
        ini.set<cfg::globals::movie_path>("resizing-capture-0");

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        // TODO remove this after unifying capture interface
        bool full_video = false;

        VideoParams video_params = video_params_from_ini(scr.cx, scr.cy,
            std::chrono::seconds::zero(), ini);
        video_params.no_timestamp = false;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = record_tmp_path;

        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        bool capture_png = bool(capture_flags & CaptureFlags::png);
        bool capture_pattern_checker = false;

        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        bool capture_video = bool(capture_flags & CaptureFlags::video);
        bool capture_video_full = full_video;
        bool capture_meta = capture_ocr;
        bool capture_kbd = false;

        OcrParams ocr_params = {
            ini.get<cfg::ocr::version>(),
            ocr::locale::LocaleId(
                static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
            ini.get<cfg::ocr::on_title_bar_only>(),
            ini.get<cfg::ocr::max_unrecog_char_rate>(),
            ini.get<cfg::ocr::interval>(),
            ini.get<cfg::debug::ocr>()
        };

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, app_path(AppPath::Wrm)); // default value, actual one should come from movie_path
        strcat(path, "/");
        strcpy(basename, movie_path);
        strcpy(extension, "");          // extension is currently ignored

        RED_CHECK(canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)));

        PngParams png_params = {
            0, 0, std::chrono::milliseconds{60}, 100, 0, false,
            false, static_cast<bool>(ini.get<cfg::video::rt_display>())};

        DrawableParams const drawable_params{scr.cx, scr.cy, nullptr};

        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
        kbd_log_params.session_log_enabled = false;

        PatternParams const pattern_params = pattern_params_from_ini(ini);

        SequencedVideoParams sequenced_video_params;
        FullVideoParams full_video_params;

        cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

        WrmParams const wrm_params = wrm_params_from_ini(BitsPerPixel{24}, false, cctx, rnd, fstat, hash_path, ini);

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
                          capture_params
                        , drawable_params
                        , capture_wrm, wrm_params
                        , capture_png, png_params
                        , capture_pattern_checker, pattern_params
                        , capture_ocr, ocr_params
                        , capture_video, sequenced_video_params
                        , capture_video_full, full_video_params
                        , capture_meta, meta_params
                        , capture_kbd, kbd_log_params
                        , video_params
                        , nullptr
                        , Rect()
                        );

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 1200, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 1200, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 1200, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        scr.cx = 1024;
        scr.cy = 768;

        capture.resize(scr.cx, scr.cy);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(4, 200, 1200, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 1200, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(6, 300, 1200, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(7, 350, 1200, 30), encode_color24()(YELLOW)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // The destruction of capture object will finalize the metafile content
    }

    bool remove_files = !getenv("TestResizingCapture");

    {
        FilenameGenerator png_seq(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION
          , "./" , "resizing-capture-0", ".png"
        );

        const char * filename;

        filename = png_seq.get(0);
        RED_CHECK_EQUAL(3098, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(1);
        RED_CHECK_EQUAL(3117, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(2);
        RED_CHECK_EQUAL(3128, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(3);
        RED_CHECK_EQUAL(3140, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(4);
        RED_CHECK_EQUAL(4079, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(5);
        RED_CHECK_EQUAL(4103, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(6);
        RED_CHECK_EQUAL(4121, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(7);
        RED_CHECK_EQUAL(4136, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(8);
        RED_CHECK_PREDICATE(file_not_exists, (filename));
    }

    for (auto x: fileinfo) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
        if (remove_files) { ::unlink(x.filename); }
    }
}

RED_AUTO_TEST_CASE(TestResizingCapture1)
{
    const struct CheckFiles {
        const char * filename;
        ssize_t size;
    } fileinfo[] = {
        {"./resizing-capture-1-000000.wrm", 1646},
        {"./resizing-capture-1-000001.wrm", 3439},
        {"./resizing-capture-1-000002.wrm", 2630},
        {"./resizing-capture-1-000003.wrm", 2630},
        {"./resizing-capture-1-000004.wrm", -1},
        {"./resizing-capture-1.mwrm", 256},
        // hash
        {"/tmp/resizing-capture-1-000000.wrm", 51},
        {"/tmp/resizing-capture-1-000001.wrm", 51},
        {"/tmp/resizing-capture-1-000002.wrm", 51},
        {"/tmp/resizing-capture-1-000003.wrm", 51},
        {"/tmp/resizing-capture-1-000004.wrm", -1},
        {"/tmp/resizing-capture-1.mwrm", 45},
    };

    for (auto & f : fileinfo) {
        ::unlink(f.filename);
    }

    Inifile ini;
    ini.set<cfg::video::rt_display>(1);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        ini.set<cfg::video::frame_interval>(std::chrono::seconds{1});
        ini.set<cfg::video::break_interval>(std::chrono::seconds{3});

        ini.set<cfg::video::png_limit>(10); // one snapshot by second
        ini.set<cfg::video::png_interval>(std::chrono::seconds{1});

        ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
        CaptureFlags capture_flags = CaptureFlags::wrm | CaptureFlags::png;

        ini.set<cfg::globals::trace_type>(TraceType::localfile);

        ini.set<cfg::video::record_tmp_path>("./");
        ini.set<cfg::video::record_path>("./");
        ini.set<cfg::video::hash_path>("/tmp/");
        ini.set<cfg::globals::movie_path>("resizing-capture-1");

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        // TODO remove this after unifying capture interface
        bool full_video = false;

        VideoParams video_params = video_params_from_ini(scr.cx, scr.cy,
            std::chrono::seconds::zero(), ini);
        video_params.no_timestamp = false;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = record_tmp_path;

        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        bool capture_png = bool(capture_flags & CaptureFlags::png);
        bool capture_pattern_checker = false;

        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        bool capture_video = bool(capture_flags & CaptureFlags::video);
        bool capture_video_full = full_video;
        bool capture_meta = capture_ocr;
        bool capture_kbd = false;

        OcrParams ocr_params = {
            ini.get<cfg::ocr::version>(),
            ocr::locale::LocaleId(
                static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
            ini.get<cfg::ocr::on_title_bar_only>(),
            ini.get<cfg::ocr::max_unrecog_char_rate>(),
            ini.get<cfg::ocr::interval>(),
            ini.get<cfg::debug::ocr>()
        };

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, app_path(AppPath::Wrm)); // default value, actual one should come from movie_path
        strcat(path, "/");
        strcpy(basename, movie_path);
        strcpy(extension, "");          // extension is currently ignored

        RED_CHECK(canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)));

        PngParams png_params = {
            0, 0, std::chrono::milliseconds{60}, 100, 0, false,
            false, static_cast<bool>(ini.get<cfg::video::rt_display>())};

        DrawableParams const drawable_params{scr.cx, scr.cy, nullptr};

        MetaParams meta_params{
            MetaParams::EnableSessionLog::No,
            MetaParams::HideNonPrintable::No,
            MetaParams::LogClipboardActivities::Yes,
            MetaParams::LogFileSystemActivities::Yes,
            MetaParams::LogOnlyRelevantClipboardActivities::Yes
        };

        KbdLogParams kbd_log_params = kbd_log_params_from_ini(ini);
        kbd_log_params.session_log_enabled = false;

        PatternParams const pattern_params = pattern_params_from_ini(ini);

        SequencedVideoParams sequenced_video_params;
        FullVideoParams full_video_params;

        cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

        WrmParams const wrm_params = wrm_params_from_ini(BitsPerPixel{24}, false, cctx, rnd, fstat, hash_path, ini);

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
                          capture_params
                        , drawable_params
                        , capture_wrm, wrm_params
                        , capture_png, png_params
                        , capture_pattern_checker, pattern_params
                        , capture_ocr, ocr_params
                        , capture_video, sequenced_video_params
                        , capture_video_full, full_video_params
                        , capture_meta, meta_params
                        , capture_kbd, kbd_log_params
                        , video_params
                        , nullptr
                        , Rect()
                        );

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.resize(640, 480);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(7, 350, 700, 30), encode_color24()(YELLOW)), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    bool remove_files = !getenv("TestResizingCapture1");

    {
        FilenameGenerator png_seq(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION
          , "./" , "resizing-capture-1", ".png"
        );

        const char * filename;

        filename = png_seq.get(0);
        RED_CHECK_EQUAL(3098, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(1);
        RED_CHECK_EQUAL(3125, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(2);
        RED_CHECK_EQUAL(3140, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(3);
        RED_CHECK_EQUAL(3158, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(4);
        RED_CHECK_EQUAL(2301, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(5);
        RED_CHECK_EQUAL(2316, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(6);
        RED_CHECK_EQUAL(2330, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(7);
        RED_CHECK_EQUAL(2341, ::filesize(filename));
        if (remove_files) { ::unlink(filename); }
        filename = png_seq.get(8);
        RED_CHECK_PREDICATE(file_not_exists, (filename));
    }

    for (auto x: fileinfo) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
        if (remove_files) { ::unlink(x.filename); }
    }
}
