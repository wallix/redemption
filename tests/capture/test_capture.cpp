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
#define BOOST_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
// #define LOGPRINT

#include "capture/capture.hpp"
#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "utils/fileutils.hpp"



BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
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
        ini.set<cfg::video::hash_path>("/tmp");
        ini.set<cfg::globals::movie_path>("capture");

        LCGRandom rnd(0);
        CryptoContext cctx;

        // TODO remove this after unifying capture interface
        bool full_video = false;
        // TODO remove this after unifying capture interface
        bool no_timestamp = false;
        // TODO remove this after unifying capture interface
        auth_api * authentifier = nullptr;
        // TODO remove this after unifying capture interface

        WrmParams wrm_params = {};
        PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, true, false};

        FlvParams flv_params = flv_params_from_ini(scr.cx, scr.cy, ini);
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = authentifier ? ini.get<cfg::video::record_path>().c_str() : record_tmp_path;

        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
        bool capture_png = bool(capture_flags & CaptureFlags::png)
                        && (!authentifier || png_params.png_limit > 0);
        bool capture_pattern_checker = authentifier
            && (::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
                || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str()));

        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
        bool capture_flv = bool(capture_flags & CaptureFlags::flv);
        bool capture_flv_full = full_video;
        bool capture_meta = capture_ocr;
        bool capture_kbd = authentifier
          ? !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
          || ini.get<cfg::session_log::enable_session_log>()
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())
          : false
        ;

        OcrParams ocr_params = {};

        Capture capture(  capture_wrm, wrm_params
                        , capture_png, png_params
                        , capture_pattern_checker
                        , capture_ocr, ocr_params
                        , capture_flv
                        , capture_flv_full
                        , capture_meta
                        , capture_kbd
                        , now, scr.cx, scr.cy, 24, 24
                        , record_tmp_path
                        , record_path
                        , flv_params
                        , no_timestamp, authentifier
                        , ini, cctx, rnd, nullptr);
        auto const color_cxt = gdi::GraphicColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);
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

    {
        FilenameGenerator wrm_seq(
//            FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION
          , "./" , "capture", ".wrm"
        );

        struct {
            size_t len = 0;
            ssize_t write(char const *, size_t len) {
                this->len += len;
                return len;
            }
        } meta_len_writer;
        detail::write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, false);

        const char * filename;

        filename = wrm_seq.get(0);
        BOOST_CHECK_EQUAL(1646, ::filesize(filename));
        detail::write_meta_file(meta_len_writer, filename, 1000, 1004);
        ::unlink(filename);
        filename = wrm_seq.get(1);
        BOOST_CHECK_EQUAL(3508, ::filesize(filename));
        detail::write_meta_file(meta_len_writer, filename, 1003, 1007);
        ::unlink(filename);
        filename = wrm_seq.get(2);
        BOOST_CHECK_EQUAL(3463, ::filesize(filename));
        detail::write_meta_file(meta_len_writer, filename, 1006, 1008);
        ::unlink(filename);
        filename = wrm_seq.get(3);
        BOOST_CHECK_EQUAL(false, file_exist(filename));

        FilenameGenerator mwrm_seq(
//            FilenameGenerator::PATH_FILE_PID_EXTENSION
            FilenameGenerator::PATH_FILE_EXTENSION
          , "./", "capture", ".mwrm"
        );
        filename = mwrm_seq.get(0);
        BOOST_CHECK_EQUAL(meta_len_writer.len, ::filesize(filename));
        ::unlink(filename);
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
    CryptoContext cctx;

    // TODO remove this after unifying capture interface
    bool full_video = false;
    // TODO remove this after unifying capture interface
    bool no_timestamp = false;
    // TODO remove this after unifying capture interface
    auth_api * authentifier = nullptr;

    WrmParams wrm_params = {};
    PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, true, false };
    FlvParams flv_params = flv_params_from_ini(scr.cx, scr.cy, ini);
    const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
    const char * record_path = authentifier ? ini.get<cfg::video::record_path>().c_str() : record_tmp_path;
    bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
    bool capture_png = bool(capture_flags & CaptureFlags::png)
                    && (!authentifier || png_params.png_limit > 0);
    bool capture_pattern_checker = authentifier
        && (::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
            || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str()));

    bool capture_ocr = bool(capture_flags & CaptureFlags::ocr) || capture_pattern_checker;
    bool capture_flv = bool(capture_flags & CaptureFlags::flv);
    bool capture_flv_full = full_video;
    bool capture_meta = capture_ocr;
    bool capture_kbd = authentifier
      ? !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
      || ini.get<cfg::session_log::enable_session_log>()
      || ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
      || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())
      : false
    ;

    OcrParams ocr_params = {};

    Capture capture( capture_wrm, wrm_params
                   , capture_png, png_params
                   , capture_pattern_checker
                   , capture_ocr, ocr_params
                   , capture_flv
                   , capture_flv_full
                   , capture_meta
                   , capture_kbd

                   , now, scr.cx, scr.cy, 16, 16
                   , record_tmp_path
                   , record_path
                   , flv_params
                   , no_timestamp, authentifier
                   , ini, cctx, rnd, nullptr);
    auto const color_cxt = gdi::GraphicColorCtx::depth24();
    Pointer pointer1(Pointer::POINTER_EDIT);
    capture.set_pointer(pointer1);

    bool ignore_frame_in_timeval = true;

    capture.draw(RDPOpaqueRect(scr, color_encode(BLUE, 16)), scr, color_cxt);
    now.tv_sec++;
    capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

    const char * filename = "./capture-000000.png";

    auto s = get_file_contents<std::string>(filename);
    char message[1024];
    if (!check_sig(reinterpret_cast<const uint8_t*>(s.data()), s.size(), message,
        "\x39\xb2\x11\x9d\x25\x64\x8d\x7b\xce\x3e\xf1\xf0\xad\x29\x50\xea\xa3\x01\x5c\x27"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }
    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestSimpleBreakpoint)
{
    Rect scr(0, 0, 800, 600);
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "test", ".wrm", groupid);

    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;

    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 768, false),
                       BmpCache::CacheOption(300, 3072, false),
                       BmpCache::CacheOption(262, 12288, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(800, 600, 24);
    DumpPng24FromRDPDrawableAdapter dump_png{drawable};
    GraphicToFile graphic_to_file(
        now, trans, 800, 600, 24,
        bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression
    );
    WrmCaptureImpl::NativeCaptureLocal consumer(graphic_to_file, now, std::chrono::seconds{1}, std::chrono::seconds{5});
    auto const color_cxt = gdi::GraphicColorCtx::depth24();

    drawable.show_mouse_cursor(false);

    bool ignore_frame_in_timeval = false;

    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    graphic_to_file.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);
    now.tv_sec += 6;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);
    trans.disconnect();

    const char * filename;

    filename = trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(1560, ::filesize(filename));
    ::unlink(filename);
    filename = trans.seqgen()->get(1);
    BOOST_CHECK_EQUAL(3365, ::filesize(filename));
    ::unlink(filename);
}

class VideoSequencer : public gdi::CaptureApi
{
    timeval start_break;
    std::chrono::microseconds break_interval;

protected:
    VideoCapture & action;

public:
    VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, VideoCapture & action)
    : start_break(now)
    , break_interval(break_interval)
    , action(action)
    {}

    std::chrono::microseconds get_interval() const {
        return this->break_interval;
    }

    void reset_now(const timeval& now) {
        this->start_break = now;
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        assert(this->break_interval.count());
        auto const interval = difftimeval(now, this->start_break);
        if (interval >= uint64_t(this->break_interval.count())) {
            this->action.next_video();
            this->start_break = now;
        }
        return this->break_interval;
    }
};

BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCapture)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        Drawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, flvgen);

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 250; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename;
    filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(40677, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(1));
    BOOST_CHECK_EQUAL(40011, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(2));
    BOOST_CHECK_EQUAL(41172, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(3));
    BOOST_CHECK_EQUAL(40610, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(4));
    BOOST_CHECK_EQUAL(40173, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(5));
    BOOST_CHECK_EQUAL(13539, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCaptureMP4)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture", ".mp4", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        Drawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "mp4", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, flvgen);

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 100; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename;
    filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(15505, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(1));
    BOOST_CHECK_EQUAL(14928, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(2));
    BOOST_CHECK_EQUAL(262, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCaptureOneChunk)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture_one_chunk", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        Drawable drawable(width, height);

//        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, {Level::high, width, height, 25, 15, 100000, "flv", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, flvgen);

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            r.y += vy;
            drawable.opaquerect(r.intersect(screen), color1);
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(1629235, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestFrameMarker)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "framemarked_opaquerect_videocapture", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        Drawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, flvgen);

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);
        auto const color3 = drawable.u32bgr_to_color(RED);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 25);
        Rect r1(10, 10 + 25, 50, 25);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            drawable.opaquerect(r1.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            r1.y += vy;
            r1.x += vx;
            drawable.opaquerect(r1.intersect(screen), color3);

            flvgen.preparing_video_frame();

            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(734469, filesize(filename));
    ::unlink(filename);
}
