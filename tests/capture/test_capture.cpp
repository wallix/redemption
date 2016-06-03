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

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
// #define LOGPRINT

#include "capture.hpp"
#include "check_sig.hpp"
#include "get_file_contents.hpp"

BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
    try  {

    Inifile ini;
    ini.set<cfg::video::rt_display>(1);
    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
    {
        LCGRandom rnd(0);

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
        ini.set<cfg::globals::trace_type>(TraceType::localfile);

        ini.set<cfg::video::record_tmp_path>("./");
        ini.set<cfg::video::record_path>("./");
        ini.set<cfg::video::hash_path>("/tmp");
        ini.set<cfg::globals::movie_path>("capture");

        CryptoContext cctx(rnd, ini);

        TODO("remove this after unifying capture interface");
        bool full_video = false;
        TODO("remove this after unifying capture interface");
        bool clear_png = false;
        TODO("remove this after unifying capture interface");
        bool no_timestamp = false;
        TODO("remove this after unifying capture interface");
        auth_api * authentifier = nullptr;
        Capture capture(
            now, scr.cx, scr.cy, 24, 24
            , clear_png, no_timestamp, authentifier
            , ini, rnd, cctx, full_video);
        bool ignore_frame_in_timeval = false;

        capture.draw(RDPOpaqueRect(scr, GREEN), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr);
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
        BOOST_CHECK_EQUAL(3484, ::filesize(filename));
        detail::write_meta_file(meta_len_writer, filename, 1006, tvtime().tv_sec);
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

    if (ini.get<cfg::globals::trace_type>() == TraceType::cryptofile) {
        FilenameGenerator mwrm_seq(
//            FilenameGenerator::PATH_FILE_PID_EXTENSION
            FilenameGenerator::PATH_FILE_EXTENSION
          , "/tmp/", "capture", ".mwrm"
        );
        const char * filename = mwrm_seq.get(0);
        BOOST_CHECK_EQUAL(32, ::filesize(filename));
        ::unlink(filename);
    }

    } catch (const Error & e) {
        BOOST_CHECK_EQUAL(e.id,  1);
    };
}

BOOST_AUTO_TEST_CASE(TestBppToOtherBppCapture)
{
    try {

    LCGRandom rnd(0);
    Inifile ini;
    ini.set<cfg::video::rt_display>(1);
    {
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
        ini.set<cfg::globals::trace_type>(TraceType::localfile);

        ini.set<cfg::video::record_tmp_path>("./");
        ini.set<cfg::video::record_path>("./");
        ini.set<cfg::video::hash_path>("/tmp");
        ini.set<cfg::globals::movie_path>("capture");

        CryptoContext cctx(rnd, ini);

        TODO("remove this after unifying capture interface");
        bool full_video = false;
        TODO("remove this after unifying capture interface");
        bool clear_png = false;
        TODO("remove this after unifying capture interface");
        bool no_timestamp = false;
        TODO("remove this after unifying capture interface");
        auth_api * authentifier = nullptr;

        Capture capture(now, scr.cx, scr.cy, 16, 16
                        , clear_png, no_timestamp, authentifier
                        , ini, rnd, cctx, full_video);

        Pointer pointer1(Pointer::POINTER_EDIT);
        capture.set_pointer(pointer1);

        bool ignore_frame_in_timeval = true;

        capture.draw(RDPOpaqueRect(scr, color_encode(BLUE, 16)), scr);
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

    } catch (...) {
        BOOST_CHECK(false);
    };
}
