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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapture
#include "system/redemption_unit_tests.hpp"


// #define LOGNULL
#define LOGPRINT

#include "utils/log.hpp"

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/stream.hpp"

#include "transport/transport.hpp"
#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "capture/capture.hpp"
#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"

BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
    BOOST_CHECK(true);
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
        ini.set<cfg::globals::movie_path>("capture");

        LCGRandom rnd(0);
        Fstat fstat;
        CryptoContext cctx;

        // TODO remove this after unifying capture interface
        bool full_video = false;
        // TODO remove this after unifying capture interface
        bool no_timestamp = false;
        // TODO remove this after unifying capture interface

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(ini.get<cfg::debug::capture>())
 |(ini.get<cfg::debug::primary_orders>()?GraphicToFile::Verbose::primary_orders:GraphicToFile::Verbose::none)
 |(ini.get<cfg::debug::secondary_orders>()?GraphicToFile::Verbose::secondary_orders:GraphicToFile::Verbose::none)
 |(ini.get<cfg::debug::bitmap_update>()?GraphicToFile::Verbose::bitmap_update:GraphicToFile::Verbose::none);

        WrmCompressionAlgorithm wrm_compression_algorithm = ini.get<cfg::video::wrm_compression_algorithm>();
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = ini.get<cfg::video::frame_interval>();
        std::chrono::seconds wrm_break_interval = ini.get<cfg::video::break_interval>();
        TraceType wrm_trace_type = ini.get<cfg::globals::trace_type>();


        FlvParams flv_params = flv_params_from_ini(scr.cx, scr.cy, ini);
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = record_tmp_path;

        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        bool capture_png = bool(capture_flags & CaptureFlags::png);
        bool capture_pattern_checker = false;

        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        bool capture_flv = bool(capture_flags & CaptureFlags::flv);
        bool capture_flv_full = full_video;
        bool capture_meta = capture_ocr;
        bool capture_kbd = false;

        OcrParams ocr_params = {
                ini.get<cfg::ocr::version>(),
                ocr::locale::LocaleId(
                    static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
                ini.get<cfg::ocr::on_title_bar_only>(),
                ini.get<cfg::ocr::max_unrecog_char_rate>(),
                ini.get<cfg::ocr::interval>()
        };

//        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
                capture_wrm ?"wrm ":"",
                capture_png ?"png ":"",
                capture_kbd ? 1 : 0,
                capture_flv ?"flv ":"",
                capture_flv_full ?"flv_full ":"",
                capture_pattern_checker ?"pattern ":"",
                capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? 2 : 1) : 0,
                capture_meta?"meta ":""
            );
//        }

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, movie_path);
        strcpy(extension, "");          // extension is currently ignored

        if (!canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension))
        ) {
            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }

        PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, false,
                                nullptr, record_tmp_path, basename, groupid};

        MetaParams meta_params;
        KbdLogParams kbdlog_params;
        PatternCheckerParams patter_checker_params;
        SequencedVideoParams sequenced_video_params;
        FullVideoParams full_video_params;

        WrmParams wrm_params(
            24,
            wrm_trace_type,
            cctx,
            rnd,
            fstat,
            record_path,
            hash_path,
            basename,
            groupid,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        const char * pattern_kill = ini.get<cfg::context::pattern_kill>().c_str();
        const char * pattern_notify = ini.get<cfg::context::pattern_notify>().c_str();
        int debug_capture = ini.get<cfg::debug::capture>();
        bool flv_capture_chunk = ini.get<cfg::globals::capture_chunk>();
        bool meta_enable_session_log = false;
        const std::chrono::duration<long int> flv_break_interval = ini.get<cfg::video::flv_break_interval>();
        bool syslog_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog);
        bool rt_display = ini.get<cfg::video::rt_display>();
        bool disable_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm);
        bool session_log_enabled = false;
        bool keyboard_fully_masked = ini.get<cfg::session_log::keyboard_input_masking_level>()
             != ::KeyboardInputMaskingLevel::fully_masked;
        bool meta_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta);

        Capture capture(
                          capture_wrm, wrm_params
                        , capture_png, png_params
                        , capture_pattern_checker, patter_checker_params
                        , capture_ocr, ocr_params
                        , capture_flv, sequenced_video_params
                        , capture_flv_full, full_video_params
                        , capture_meta, meta_params
                        , capture_kbd, kbdlog_params
                        , basename
                        , now, scr.cx, scr.cy, 24, 24
                        , record_tmp_path
                        , record_path
                        , groupid
                        , flv_params
                        , no_timestamp, nullptr
                        , nullptr
                        , pattern_kill
                        , pattern_notify
                        , debug_capture
                        , flv_capture_chunk
                        , meta_enable_session_log
                        , flv_break_interval
                        , syslog_keyboard_log
                        , rt_display
                        , disable_keyboard_log
                        , session_log_enabled
                        , keyboard_fully_masked
                        , meta_keyboard_log
                        );

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    {
        FilenameGenerator png_seq(
//            FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION
          , "./" , "capture", ".png"
        );

        const char * filename;

        filename = png_seq.get(0);
        BOOST_CHECK_EQUAL(3098, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(1);
        BOOST_CHECK_EQUAL(3125, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(2);
        BOOST_CHECK_EQUAL(3140, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(3);
        BOOST_CHECK_EQUAL(3158, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(4);
        BOOST_CHECK_EQUAL(3172, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(5);
        BOOST_CHECK_EQUAL(3197, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(6);
        BOOST_CHECK_EQUAL(3223, ::filesize(filename));
        ::unlink(filename);
        filename = png_seq.get(7);
        BOOST_CHECK_EQUAL(false, file_exist(filename));
    }

    struct CheckFiles {
        const char * filename;
        size_t size;
        size_t altsize;
    } fileinfo[] = {
        {"./capture-000000.wrm", 1646, 0},
        {"./capture-000001.wrm", 3508, 0},
        {"./capture-000002.wrm", 3463, 0},
        {"./capture-000003.wrm", static_cast<size_t>(-1), static_cast<size_t>(-1)},
        {"./capture.mwrm", 288, 285},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if (x.altsize != fsize){
            BOOST_CHECK_EQUAL(x.size, fsize);
        }
        ::unlink(x.filename);
    }
}

BOOST_AUTO_TEST_CASE(TestBppToOtherBppCapture)
{
    Inifile ini;
    ini.set<cfg::video::rt_display>(1);

    // Timestamps are applied only when flushing
    timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 12, 10);

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
    ini.set<cfg::globals::movie_path>("capture");

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;

    // TODO remove this after unifying capture interface
    bool full_video = false;
    // TODO remove this after unifying capture interface
    bool no_timestamp = false;

    GraphicToFile::Verbose wrm_verbose = to_verbose_flags(ini.get<cfg::debug::capture>())
        | (ini.get<cfg::debug::primary_orders>() ?GraphicToFile::Verbose::primary_orders:GraphicToFile::Verbose::none)
        | (ini.get<cfg::debug::secondary_orders>() ?GraphicToFile::Verbose::secondary_orders:GraphicToFile::Verbose::none)
        | (ini.get<cfg::debug::bitmap_update>() ?GraphicToFile::Verbose::bitmap_update:GraphicToFile::Verbose::none);

    WrmCompressionAlgorithm wrm_compression_algorithm = ini.get<cfg::video::wrm_compression_algorithm>();
    std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = ini.get<cfg::video::frame_interval>();
    std::chrono::seconds wrm_break_interval = ini.get<cfg::video::break_interval>();
    TraceType wrm_trace_type = ini.get<cfg::globals::trace_type>();

    FlvParams flv_params = flv_params_from_ini(scr.cx, scr.cy, ini);
    const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
    const char * record_path = record_tmp_path;
    bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
    bool capture_png = bool(capture_flags & CaptureFlags::png);
    bool capture_pattern_checker = false;

    bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
    bool capture_flv = bool(capture_flags & CaptureFlags::flv);
    bool capture_flv_full = full_video;
    bool capture_meta = capture_ocr;
    bool capture_kbd = false;

    OcrParams ocr_params = {
            ini.get<cfg::ocr::version>(),
            ocr::locale::LocaleId(
                static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
            ini.get<cfg::ocr::on_title_bar_only>(),
            ini.get<cfg::ocr::max_unrecog_char_rate>(),
            ini.get<cfg::ocr::interval>()
    };

    if (ini.get<cfg::debug::capture>()) {
        LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
            capture_wrm ?"wrm ":"",
            capture_png ?"png ":"",
            capture_kbd ? 1 : 0,
            capture_flv ?"flv ":"",
            capture_flv_full ?"flv_full ":"",
            capture_pattern_checker ?"pattern ":"",
            capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? 2 : 1) : 0,
            capture_meta?"meta ":""
        );
    }

    const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
    const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
    const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

    char path[1024];
    char basename[1024];
    char extension[128];
    strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
    strcpy(basename, movie_path);
    strcpy(extension, "");          // extension is currently ignored

    if (!canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension))
    ) {
        LOG(LOG_ERR, "Buffer Overflowed: Path too long");
        throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
    }

    PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, false,
                        nullptr, record_tmp_path, basename, groupid};

    MetaParams meta_params;
    KbdLogParams kbdlog_params;
    PatternCheckerParams patter_checker_params;
    SequencedVideoParams sequenced_video_params;
    FullVideoParams full_video_params;

    WrmParams wrm_params(
        24,
        wrm_trace_type,
        cctx,
        rnd,
        fstat,
        record_path,
        hash_path,
        basename,
        groupid,
        wrm_frame_interval,
        wrm_break_interval,
        wrm_compression_algorithm,
        int(wrm_verbose)
    );


    const char * pattern_kill = ini.get<cfg::context::pattern_kill>().c_str();
    const char * pattern_notify = ini.get<cfg::context::pattern_notify>().c_str();
    int debug_capture = ini.get<cfg::debug::capture>();
    bool flv_capture_chunk = ini.get<cfg::globals::capture_chunk>();
    bool meta_enable_session_log = false;
    const std::chrono::duration<long int> flv_break_interval = ini.get<cfg::video::flv_break_interval>();
    bool syslog_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog);
    bool rt_display = ini.get<cfg::video::rt_display>();
    bool disable_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm);
    bool session_log_enabled = false;
    bool keyboard_fully_masked = ini.get<cfg::session_log::keyboard_input_masking_level>()
         != ::KeyboardInputMaskingLevel::fully_masked;
    bool meta_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta);

    // TODO remove this after unifying capture interface
    Capture capture(
                     capture_wrm, wrm_params
                   , capture_png, png_params
                   , capture_pattern_checker, patter_checker_params
                   , capture_ocr, ocr_params
                   , capture_flv, sequenced_video_params
                   , capture_flv_full, full_video_params
                   , capture_meta, meta_params
                   , capture_kbd, kbdlog_params
                   , basename
                   , now, scr.cx, scr.cy, 16, 16
                   , record_tmp_path
                   , record_path
                   , groupid
                   , flv_params
                   , no_timestamp, nullptr
                   , nullptr
                   , pattern_kill
                   , pattern_notify
                   , debug_capture
                   , flv_capture_chunk
                   , meta_enable_session_log
                   , flv_break_interval
                   , syslog_keyboard_log
                   , rt_display
                   , disable_keyboard_log
                   , session_log_enabled
                   , keyboard_fully_masked
                   , meta_keyboard_log
                   );
    auto const color_cxt = gdi::ColorCtx::depth16();
    Pointer pointer1(Pointer::POINTER_EDIT);
    capture.set_pointer(pointer1);

    bool ignore_frame_in_timeval = true;

    capture.draw(RDPOpaqueRect(scr, RDPColor(color_encode(BLUE, 16))), scr, color_cxt);
    now.tv_sec++;
    capture.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    const char * filename = "./capture-000000.png";

    auto s = get_file_contents<std::string>(filename);
    CHECK_SIG2(
        reinterpret_cast<const uint8_t*>(s.data()), s.size(),
        "\x39\xb2\x11\x9d\x25\x64\x8d\x7b\xce\x3e\xf1\xf0\xad\x29\x50\xea\xa3\x01\x5c\x27"
    );
    ::unlink(filename);
}



BOOST_AUTO_TEST_CASE(TestPattern)
{
    for (int i = 0; i < 2; ++i) {
        struct Auth : NullAuthentifier
        {
            std::string reason;
            std::string message;

            void report(const char * reason, const char * message) override {
                this->reason = reason;
                this->message = message;
            }
        } authentifier;
        PatternsChecker checker(authentifier, i ? ".de." : nullptr, i ? nullptr : ".de.");

        auto const reason = i ? "FINDPATTERN_KILL" : "FINDPATTERN_NOTIFY";

        checker(cstr_array_view("Gestionnaire"));

        BOOST_CHECK(authentifier.reason.empty());
        BOOST_CHECK(authentifier.message.empty());

        checker(cstr_array_view("Gestionnaire de serveur"));

        BOOST_CHECK_EQUAL(authentifier.reason,  reason);
        BOOST_CHECK_EQUAL(authentifier.message, "$ocr:.de.|Gestionnaire de serveur");

        checker(cstr_array_view("Gestionnaire de licences TS"));

        BOOST_CHECK_EQUAL(authentifier.reason,  reason);
        BOOST_CHECK_EQUAL(authentifier.message, "$ocr:.de.|Gestionnaire de licences TS");
    }
}


BOOST_AUTO_TEST_CASE(TestSessionMeta)
{
    char const out_data[] =
        "1970-01-01 01:16:40 - [Kbd]ABCDABCDABCDABCDABCDABCDABCDABCDABCD\n"
        "1970-01-01 01:16:49 - [Kbd]ABCD\n"
        "1970-01-01 01:16:50 + Blah1\n"
        "1970-01-01 01:16:51 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:54 + Blah3\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        send_kbd(); now.tv_sec += 1;
        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
    }
}


BOOST_AUTO_TEST_CASE(TestSessionMeta2)
{
    char const out_data[] =
        "1970-01-01 01:16:40 + Blah1\n"
        "1970-01-01 01:16:41 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:44 + Blah3\n"
        "1970-01-01 01:16:45 + (break)\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

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
        meta.send_line(now.tv_sec, cstr_array_view("(break)"));
    }
}


BOOST_AUTO_TEST_CASE(TestSessionMeta3)
{
    char const out_data[] =
        "1970-01-01 01:16:40 - [Kbd]ABCD\n"
        "1970-01-01 01:16:41 + Blah1\n"
        "1970-01-01 01:16:42 - BUTTON_CLICKED=Démarrer\n"
        "1970-01-01 01:16:43 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:46 + Blah3\n"
        "1970-01-01 01:16:47 + (break)\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        meta.session_update(now, {"BUTTON_CLICKED=Démarrer", 24}); now.tv_sec += 1;

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.send_line(now.tv_sec, cstr_array_view("(break)"));
    }
}


BOOST_AUTO_TEST_CASE(TestSessionMeta4)
{
    char const out_data[] =
        "1970-01-01 01:16:40 - [Kbd]ABCD\n"
        "1970-01-01 01:16:41 + Blah1[Kbd]ABCD\n"
        "1970-01-01 01:16:42 - BUTTON_CLICKED=Démarrer\n"
        "1970-01-01 01:16:42 - Blah1[Kbd]ABCD\n"
        "1970-01-01 01:16:44 + Blah2[Kbd]ABCDABCD\n"
        "1970-01-01 01:16:47 + Blah3\n"
        "1970-01-01 01:16:48 + (break)\n"
    ;
    CheckTransport trans(out_data, sizeof(out_data) - 1);

    timeval now;
    now.tv_sec  = 1000;
    now.tv_usec = 0;

    {
        SessionMeta meta(now, trans);

        auto send_kbd = [&]{
            meta.kbd_input(now, 'A');
            meta.kbd_input(now, 'B');
            meta.kbd_input(now, 'C');
            meta.kbd_input(now, 'D');
        };

        send_kbd(); now.tv_sec += 1;

        meta.title_changed(now.tv_sec, cstr_array_view("Blah1")); now.tv_sec += 1;

        send_kbd();

        meta.session_update(now, {"BUTTON_CLICKED=Démarrer", 24}); now.tv_sec += 1;

        send_kbd(); now.tv_sec += 1;

        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah2")); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        send_kbd(); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.title_changed(now.tv_sec, cstr_array_view("Blah3")); now.tv_sec += 1;
        meta.periodic_snapshot(now, 0, 0, 0);
        meta.send_line(now.tv_sec, cstr_array_view("(break)"));
    }
}

class DrawableToFile
{
protected:
    Transport & trans;
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    const Drawable & drawable;

private:
    std::unique_ptr<uint8_t[]> scaled_buffer;

public:
    DrawableToFile(Transport & trans, const Drawable & drawable, unsigned zoom)
    : trans(trans)
    , zoom_factor(std::min(zoom, 100u))
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {
        const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
        const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    ~DrawableToFile() = default;

    /// \param  percent  0 to 100 or 100 if greater
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->drawable.height() * percent) / 100;
        this->zoom_factor = percent;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    bool logical_frame_ended() const {
        return this->drawable.logical_frame_ended;
    }

    void flush() {
        if (this->zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

private:
    void dump24() const {
        ::transport_dump_png24(
            this->trans, this->drawable.data(),
            this->drawable.width(), this->drawable.height(),
            this->drawable.rowsize(), true);
    }

    void scale_dump24() const {
        scale_data(
            this->scaled_buffer.get(), this->drawable.data(),
            this->scaled_width, this->drawable.width(),
            this->scaled_height, this->drawable.height(),
            this->drawable.rowsize());
        ::transport_dump_png24(
            this->trans, this->scaled_buffer.get(),
            this->scaled_width, this->scaled_height,
            this->scaled_width * 3, false);
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

BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrm)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);

    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    DumpPng24FromRDPDrawableAdapter dump_png24(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, GREEN), screen_rect, color_ctx);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), BLUE), screen_rect, color_ctx);
    consumer.sync();

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), WHITE), screen_rect, color_ctx);
    now.tv_sec++;
    consumer.timestamp(now);

    now.tv_sec++;
    consumer.timestamp(now);

    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
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
/* 0000 */ "\x11\x32\x32\xff\xff"             // WHITE rect
           "\x11\x62\x32\x00\x00"             // RED rect

           "\xf0\x03\x10\x00\x00\x00\x01\x00"
/* 0000 */ "\xc0\x99\x05\x3c\x00\x00\x00\x00" // time 1007000000

           "\x00\x00\x13\x00\x00\x00\x01\x00"
/* 0000 */ "\x01\x1f\x05\x00\x05\x00\x0a\x00\x0a\x00\x00" // BLACK rect
   ;


BOOST_AUTO_TEST_CASE(Test6SecondsStrippedScreenToWrmReplay2)
{
    // Same as above, show timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect screen_rect(0, 0, 800, 600);
    StaticOutStream<65536> stream;
    CheckTransport trans(expected_stripped_wrm2, sizeof(expected_stripped_wrm2)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    DumpPng24FromRDPDrawableAdapter dump_png24(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();

    consumer.draw(RDPOpaqueRect(screen_rect, GREEN), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 50, 700, 30), BLUE), screen_rect, color_ctx);

    now.tv_sec++;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(0, 100, 700, 30), WHITE), screen_rect, color_ctx);
    consumer.draw(RDPOpaqueRect(Rect(0, 150, 700, 30), RED), screen_rect, color_ctx);
    now.tv_sec+=6;
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 10), BLACK), screen_rect, color_ctx);

    consumer.sync();
}

BOOST_AUTO_TEST_CASE(TestCaptureToWrmReplayToPng)
{
    // Same as above, show timestamps are applied only when flushing
    BOOST_CHECK_EQUAL(0, 0);
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
    if (fd == -1){
        LOG(LOG_INFO, "open failed with error : %s on %s", strerror(errno), path);
        BOOST_CHECK(false);
        return;
    }

    OutFileTransport trans(fd);
    BOOST_CHECK_EQUAL(0, 0);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    DumpPng24FromRDPDrawableAdapter dump_png24_api(drawable);
    GraphicToFile consumer(now, trans, screen_rect.cx, screen_rect.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png24_api, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();
    BOOST_CHECK_EQUAL(0, 0);
    RDPOpaqueRect cmd0(screen_rect, GREEN);
    consumer.draw(cmd0, screen_rect, color_ctx);
    RDPOpaqueRect cmd1(Rect(0, 50, 700, 30), BLUE);
    consumer.draw(cmd1, screen_rect, color_ctx);
    now.tv_sec++;
    BOOST_CHECK_EQUAL(0, 0);
    consumer.timestamp(now);
    consumer.sync();
    BOOST_CHECK_EQUAL(0, 0);

    RDPOpaqueRect cmd2(Rect(0, 100, 700, 30), WHITE);
    consumer.draw(cmd2, screen_rect, color_ctx);
    RDPOpaqueRect cmd3(Rect(0, 150, 700, 30), RED);
    consumer.draw(cmd3, screen_rect, color_ctx);
    now.tv_sec+=6;
    consumer.timestamp(now);
    consumer.sync();
    BOOST_CHECK_EQUAL(0, 0);
    trans.disconnect(); // close file before reading filesize
    BOOST_CHECK_EQUAL(1588, filesize(filename));

    char in_path[1024];
    len = strlen(filename);
    memcpy(in_path, filename, len);
    in_path[len] = 0;

    fd = ::open(in_path, O_RDONLY);
    if (fd == -1){
        LOG(LOG_INFO, "open '%s' failed with error : %s", path, strerror(errno));
        BOOST_CHECK(false);
        return;
    }
    InFileTransport in_wrm_trans(fd);

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testcap", ".png", groupid, nullptr);

    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl(), 100);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr);

    png_recorder.flush();
    out_png_trans.next();

    // Green Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Blue Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Timestamp
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // White Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    // Red Rect
    BOOST_CHECK_EQUAL(true, player.next_order());
    player.interpret_order();
    png_recorder.flush();
    out_png_trans.next();

    BOOST_CHECK_EQUAL(false, player.next_order());
    in_wrm_trans.disconnect();

    // clear PNG files
    size_t sz[6] = {1476, 2786, 2800, 2800, 2814, 2823};
    for (int i = 0; i < 6 ; i++){
        const char * filename = out_png_trans.seqgen()->get(i);
        BOOST_CHECK_EQUAL(sz[i], ::filesize(filename));
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

BOOST_AUTO_TEST_CASE(TestSaveCache)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1, 511);
    trans.disable_remaining_error();
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    DumpPng24FromRDPDrawableAdapter dump_png(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression);
    auto const color_ctx = gdi::ColorCtx::depth24();
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, BLUE), scr, color_ctx);

    uint8_t comp20x10RED[] = {
        0xc0, 0x04, 0x00, 0x00, 0xFF, // MIX 20 (0, 0, FF)
        0x00, 0x94                    // FILL 9 * 20
    };

    Bitmap bloc20x10(24, 24, nullptr, 20, 10, comp20x10RED, sizeof(comp20x10RED), true );
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

BOOST_AUTO_TEST_CASE(TestReloadSaveCache)
{
    GeneratorTransport in_wrm_trans(expected_Red_on_Blue_wrm, sizeof(expected_Red_on_Blue_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadSaveCache", ".png", groupid, nullptr);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();

    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(298, ::filesize(filename));
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

BOOST_AUTO_TEST_CASE(TestSaveOrderStates)
{
    // Timestamps are applied only when flushing
    struct timeval now;
    now.tv_usec = 0;
    now.tv_sec = 1000;

    Rect scr(0, 0, 100, 100);
    CheckTransport trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(2, 256, false),
                       BmpCache::CacheOption(2, 1024, false),
                       BmpCache::CacheOption(2, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    DumpPng24FromRDPDrawableAdapter dump_png(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression);
    auto const color_cxt = gdi::ColorCtx::depth24();
    consumer.timestamp(now);

    consumer.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(5), BLUE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr.shrink(10), RED), scr, color_cxt);

    consumer.sync();

    consumer.send_save_state_chunk();

    now.tv_sec++;
    consumer.timestamp(now);
    consumer.draw(RDPOpaqueRect(scr.shrink(20), GREEN), scr, color_cxt);
    consumer.sync();
}

BOOST_AUTO_TEST_CASE(TestReloadOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_reset_rect_wrm, sizeof(expected_reset_rect_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestReloadOrderStates", ".png", groupid, nullptr);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(341, ::filesize(filename));
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

BOOST_AUTO_TEST_CASE(TestContinuationOrderStates)
{
    GeneratorTransport in_wrm_trans(expected_continuation_wrm, sizeof(expected_continuation_wrm)-1);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "TestContinuationOrderStates", ".png", groupid, nullptr);
    const FilenameGenerator * seq = out_png_trans.seqgen();
    BOOST_CHECK(seq);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    const char * filename = seq->get(0);
    BOOST_CHECK_EQUAL(341, ::filesize(filename));
    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestImageChunk)
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
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // RED rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // BLUE RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // WHITE RECT

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
    CheckTransport trans(expected_stripped_wrm, sizeof(expected_stripped_wrm)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                        BmpCache::CacheOption(600, 256, false),
                        BmpCache::CacheOption(300, 1024, false),
                        BmpCache::CacheOption(262, 4096, false));
    PointerCache ptr_cache;
    GlyphCache gly_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    DumpPng24FromRDPDrawableAdapter dump_png_api(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png_api, WrmCompressionAlgorithm::no_compression);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.sync();
    consumer.send_image_chunk();
}

BOOST_AUTO_TEST_CASE(TestImagePNGMediumChunks)
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
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // RED rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // BLUE RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // WHITE RECT

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
    CheckTransport trans(expected, sizeof(expected)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    DumpPng24FromRDPDrawableAdapter dump_png_api(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png_api, WrmCompressionAlgorithm::no_compression);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.sync();

    OutChunkedBufferingTransport<100> png_trans(trans);
    BOOST_CHECK_NO_THROW(consumer.dump_png24(png_trans, true));
}

BOOST_AUTO_TEST_CASE(TestImagePNGSmallChunks)
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
    /* 0000 */ "\x19\x0a\x1c\x14\x0a\xff"             // RED rect
    /* 0000 */ "\x11\x5f\x05\x05\xF6\xf9\x00\xFF\x11" // BLUE RECT
    /* 0000 */ "\x3f\x05\xfb\xf7\x07\xff\xff"         // WHITE RECT

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
    CheckTransport trans(expected, sizeof(expected)-1, 511);
    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 256, false),
                       BmpCache::CacheOption(300, 1024, false),
                       BmpCache::CacheOption(262, 4096, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(scr.cx, scr.cy);
    DumpPng24FromRDPDrawableAdapter dump_png_api(drawable);
    GraphicToFile consumer(now, trans, scr.cx, scr.cy, 24, bmp_cache, gly_cache, ptr_cache, dump_png_api, WrmCompressionAlgorithm::no_compression);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), BLUE), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), WHITE), scr, color_cxt);
    consumer.sync();

    OutChunkedBufferingTransport<16> png_trans(trans);
    consumer.dump_png24(png_trans, true);
//    DumpPng24FromRDPDrawableAdapter(consumer).dump_png24(png_trans, true);

}

BOOST_AUTO_TEST_CASE(TestReadPNGFromTransport)
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
    ::transport_read_png24(
        in_png_trans, const_cast<uint8_t*>(d.data()),
        d.width(), d.height(), d.rowsize()
    );
    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, nullptr);
    DumpPng24FromRDPDrawableAdapter(d).dump_png24(png_trans, true);
//    d.dump_png24(png_trans, true);
    ::unlink(png_trans.seqgen()->get(0));
}



BOOST_AUTO_TEST_CASE(TestExtractPNGImagesFromWRM)
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
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, nullptr);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    out_png_trans.disconnect();
    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestExtractPNGImagesFromWRMTwoConsumers)
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
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));
    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, nullptr);
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl(), 100);

    PngCapture::OutFilenameSequenceTransport second_out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "second_testimg", ".png", groupid, nullptr);
    DrawableToFile second_png_recorder(second_out_png_trans, drawable1.impl(), 100);

    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }

    const char * filename;

    png_recorder.flush();
    filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);

    second_png_recorder.flush();
    filename = second_out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestExtractPNGImagesThenSomeOtherChunk)
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
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));
    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, nullptr);
    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable.impl(), 100);

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    while (player.next_order()){
        player.interpret_order();
    }
    png_recorder.flush();
    BOOST_CHECK_EQUAL(1004u, static_cast<unsigned>(player.record_now.tv_sec));

    const char * filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestKbdCapture)
{
    struct : auth_api {
        mutable std::string s;

        void log4(bool duplicate_with_pid, const char* type, const char* extra) override {
            (void)duplicate_with_pid;
            (void)type;
            BOOST_REQUIRE(extra);
            s += extra;
        }

        void report(const char*, const char*) override {}
        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
    } auth;

    timeval const time = {0, 0};
    SessionLogKbd kbd_capture(auth);

    {
        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data='a'", auth.s);
    }

    kbd_capture.enable_kbd_input_mask(true);
    auth.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        // prob is not enabled
        BOOST_CHECK_EQUAL(auth.s.size(), 0);
    }

    kbd_capture.enable_kbd_input_mask(false);
    auth.s.clear();

    {
        kbd_capture.kbd_input(time, 'a');

        BOOST_CHECK_EQUAL(auth.s.size(), 0);

        kbd_capture.enable_kbd_input_mask(true);

        BOOST_CHECK_EQUAL(auth.s.size(), 8);
        BOOST_CHECK_EQUAL("data='a'", auth.s);
        auth.s.clear();

        kbd_capture.kbd_input(time, 'a');
        kbd_capture.flush();

        BOOST_CHECK_EQUAL(auth.s.size(), 0);
    }
}


BOOST_AUTO_TEST_CASE(TestKbdCapturePatternNotify)
{
    struct : auth_api {
        mutable std::string s;

        void report(const char* reason, const char* message) override {
            s += reason;
            s += " -- ";
            s += message;
            s += "\n";
        }

        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
        void log4(bool, const char*, const char*) override {}
    } auth;

    PatternKbd kbd_capture(&auth, "$kbd:abcd", nullptr);

    char const str[] = "abcdaaaaaaaaaaaaaaaabcdeaabcdeaaaaaaaaaaaaabcde";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }

    BOOST_CHECK_EQUAL(4, pattern_count);
    BOOST_CHECK_EQUAL(
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
        "FINDPATTERN_KILL -- $kbd:abcd|abcd\n"
      , auth.s
    );
}


BOOST_AUTO_TEST_CASE(TestKbdCapturePatternKill)
{
    struct : auth_api {
        bool is_killed = 0;

        void report(const char* , const char* ) override {
            this->is_killed = 1;
        }

        void set_auth_channel_target(const char*) override {}
        void set_auth_error_message(const char*) override {}
        void log4(bool, const char*, const char*) override {}
    } auth;

    PatternKbd kbd_capture(&auth, "$kbd:ab/cd", nullptr);

    char const str[] = "abcdab/cdaa";
    unsigned pattern_count = 0;
    for (auto c : str) {
        if (!kbd_capture.kbd_input({0, 0}, c)) {
            ++pattern_count;
        }
    }
    BOOST_CHECK_EQUAL(1, pattern_count);
    BOOST_CHECK_EQUAL(auth.is_killed, true);
}




BOOST_AUTO_TEST_CASE(TestSample0WRM)
{
    const char * input_filename = FIXTURES_PATH "/sample0.wrm";

    int fd = ::open(input_filename, O_RDONLY);
    if (fd == -1){
        LOG(LOG_INFO, "open '%s' failed with error : %s", input_filename, strerror(errno));
        BOOST_CHECK(false);
        return;
    }

    InFileTransport in_wrm_trans(fd);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".png", groupid, nullptr);
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl(), 100);

//    png_recorder.update_config(ini);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr);

    PngCapture::OutFilenameSequenceTransport out_wrm_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".wrm", groupid, nullptr);

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
    DumpPng24FromRDPDrawableAdapter dump_png{drawable};
    GraphicToFile graphic_to_file(
        player.record_now,
        out_wrm_trans,
        player.screen_rect.cx,
        player.screen_rect.cy,
        24,
        bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression
    );
    WrmCaptureImpl::NativeCaptureLocal wrm_recorder(graphic_to_file, player.record_now, std::chrono::seconds{1}, std::chrono::seconds{20});

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    player.add_consumer(&graphic_to_file, &wrm_recorder, nullptr, nullptr, &wrm_recorder);

    bool requested_to_stop = false;

    BOOST_CHECK_EQUAL(1352304810u, static_cast<unsigned>(player.record_now.tv_sec));
    player.play(requested_to_stop);

    png_recorder.flush();
    BOOST_CHECK_EQUAL(1352304870u, static_cast<unsigned>(player.record_now.tv_sec));

    graphic_to_file.sync();
    const char * filename;

    out_png_trans.disconnect();
    out_wrm_trans.disconnect();

    filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(21280, ::filesize(filename));
    ::unlink(filename);

    filename = out_wrm_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(490454, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(1);
    BOOST_CHECK_EQUAL(1008253, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(2);
    BOOST_CHECK_EQUAL(195756, ::filesize(filename));
    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestReadPNGFromChunkedTransport)
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
    in_png_trans.recv_new(end, sz_buf); // skip first chunk header
    InStream stream(buf);

//    in_png_trans.recv(&stream.end, 107); // skip first chunk header

    uint16_t chunk_type = stream.in_uint16_le();
    uint32_t chunk_size = stream.in_uint32_le();
    uint16_t chunk_count = stream.in_uint16_le();
    (void)chunk_count;

    RDPDrawable d(20, 10);
    gdi::GraphicApi * gdi = &d;
    set_rows_from_image_chunk(in_png_trans, chunk_type, chunk_size, d.width(), {&gdi, 1});

    const int groupid = 0;
    PngCapture::OutFilenameSequenceTransport png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "testimg", ".png", groupid, nullptr);
    DumpPng24FromRDPDrawableAdapter(d).dump_png24(png_trans, true);
//    d.dump_png24(png_trans, true);
    ::unlink(png_trans.seqgen()->get(0));
}





class ochecksum_buf_null_buf
{
    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;
    size_t file_size = nosize;

    SslHMAC_Sha256_Delayed hmac;
    SslHMAC_Sha256_Delayed quick_hmac;
    unsigned char const (&hmac_key)[SHA256_DIGEST_LENGTH];

public:
    explicit ochecksum_buf_null_buf(unsigned char const (&hmac_key)[SHA256_DIGEST_LENGTH])
    : hmac_key(hmac_key)
    {}

    int open()
    {
        this->hmac.init(this->hmac_key, sizeof(this->hmac_key));
        this->quick_hmac.init(this->hmac_key, sizeof(this->hmac_key));
        this->file_size = 0;
        return 0;
    }

    ssize_t write(const void * data, size_t len)
    {
        this->hmac.update(static_cast<const uint8_t *>(data), len);
        if (this->file_size < quick_size) {
            auto const remaining = std::min(quick_size - this->file_size, len);
            this->quick_hmac.update(static_cast<const uint8_t *>(data), remaining);
            this->file_size += remaining;
        }
        return len;
    }

    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
    {
        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
        this->file_size = nosize;
        return 0;
    }
};



#include "utils/fileutils.hpp"

BOOST_AUTO_TEST_CASE(TestOutFilenameSequenceTransport)
{
    PngCapture::OutFilenameSequenceTransport fnt(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test_outfilenametransport", ".txt", getgid(), nullptr);
    fnt.send("We write, ", 10);
    fnt.send("and again, ", 11);
    fnt.send("and so on.", 10);

    fnt.next();
    fnt.send(" ", 1);
    fnt.send("A new file.", 11);

    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(0)), 31);
    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(1)), 12);

    fnt.disconnect();
    unlink(fnt.seqgen()->get(0));
    unlink(fnt.seqgen()->get(1));
}
