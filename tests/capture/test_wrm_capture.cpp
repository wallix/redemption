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

   Unit test to capture interface to video recording to flv or mp4
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT

#include "utils/log.hpp"
#include "utils/genrandom.hpp"
#include "utils/sugar/iter.hpp"
#include <snappy-c.h>

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/stream.hpp"

#include "transport/transport.hpp"
#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"

#include "capture/wrm_capture.hpp"


BOOST_AUTO_TEST_CASE(TestSimpleBreakpoint)
{
    Rect scr(0, 0, 800, 600);
    const int groupid = 0;
    wrmcapture_OutFilenameSequenceTransport trans(wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION, "./", "test", ".wrm", groupid, nullptr);

    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;

    BmpCache bmp_cache(BmpCache::Recorder, 24, 3, false,
                       BmpCache::CacheOption(600, 768, false),
                       BmpCache::CacheOption(300, 3072, false),
                       BmpCache::CacheOption(262, 12288, false));
    GlyphCache gly_cache;
    PointerCache ptr_cache;
    RDPDrawable drawable(800, 600);
    DumpPng24FromRDPDrawableAdapter dump_png{drawable};
    GraphicToFile graphic_to_file(now, trans, 800, 600, 24,
        bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression
    );
    WrmCaptureImpl::NativeCaptureLocal consumer(graphic_to_file, now, std::chrono::seconds{1}, std::chrono::seconds{5});
    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable.show_mouse_cursor(false);

    bool ignore_frame_in_timeval = false;

    drawable.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    graphic_to_file.draw(RDPOpaqueRect(scr, RED), scr, color_cxt);
    consumer.periodic_snapshot(now, 10, 10, ignore_frame_in_timeval);
    now.tv_sec += 6;
    consumer.periodic_snapshot(now, 10, 10, ignore_frame_in_timeval);
    trans.disconnect();

    const char * filename0 = "./test-000000.wrm";
    BOOST_CHECK_EQUAL(1560, ::filesize(filename0));
//    ::unlink(filename);
    const char * filename1 = "./test-000001.wrm";
    BOOST_CHECK_EQUAL(3365, ::filesize(filename1));
//    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestWrmCapture)
{
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0)
//         |GraphicToFile::Verbose::primary_orders)
//         |GraphicToFile::Verbose::secondary_orders)
//         |GraphicToFile::Verbose::bitmap_update)
        ;

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};
        TraceType wrm_trace_type = TraceType::localfile;

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp";

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, "capture");
        strcpy(extension, "");          // extension is currently ignored

        WrmParams wrm_params(
            24,
            wrm_trace_type,
            cctx,
            rnd,
            record_path,
            hash_path,
            basename,
            groupid,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    {
        struct CheckFiles {
            const char * filename;
            size_t size;
        } fileinfo[] = {
            {"./capture-000000.wrm", 1646},
            {"./capture-000001.wrm", 3508},
            {"./capture-000002.wrm", 3463},
            {"./capture-000003.wrm", static_cast<size_t>(-1)},
            {"./capture.mwrm", 285},
        };
        for (auto x: fileinfo) {
            size_t fsize = filesize(x.filename);
            BOOST_CHECK_EQUAL(x.size, fsize);
            ::unlink(x.filename);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestWrmCaptureLocalHashed)
{
    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
            "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
            "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
            "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
        ));
        cctx.set_hmac_key(cstr_array_view(
             "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
             "\x49\x21\x57\x87\x47\x74\x08\x8a"
             "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
             "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
        ));


        WrmParams wrm_params(
            24,
            TraceType::localfile_hashed,
            cctx,
            rnd,
            "./",
            "/tmp",
            "capture",
            1000, // ini.get<cfg::video::capture_groupid>()
            std::chrono::seconds{1},
            std::chrono::seconds{3},
            WrmCompressionAlgorithm::no_compression,
            0xFFFF
        );

        RDPDrawable gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(now, wrm_params, nullptr /* authentifier */, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, GREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    struct CheckFiles {
        const char * filename;
        size_t size;
    } fileinfo[] = {
        {"./capture-000000.wrm", 1646},
        {"./capture-000001.wrm", 3508},
        {"./capture-000002.wrm", 3463},
        {"./capture-000003.wrm", static_cast<size_t>(-1)},
        {"./capture.mwrm", 673},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        BOOST_CHECK_EQUAL(x.size, fsize);
        ::unlink(x.filename);
    }
}

BOOST_AUTO_TEST_CASE(TestOSumBuf)
{
    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
    wrmcapture_ochecksum_buf_null_buf buf(cctx.get_hmac_key());
    buf.open();
    BOOST_CHECK_EQUAL(buf.write("ab", 2), 2);
    BOOST_CHECK_EQUAL(buf.write("cde", 3), 3);

    wrmcapture_hash_type hash;
    buf.close(hash);

    char hash_str[wrmcapture_hash_string_len + 1];
    *wrmcapture_swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
    );
}

#include <string>

BOOST_AUTO_TEST_CASE(TestWriteFilename)
{
    struct {
        std::string s;

        int write(char const * data, std::size_t len) {
            s.append(data, len);
            return len;
        }
    } writer;

#define TEST_WRITE_FILENAME(origin_filename, wrote_filename) \
    wrmcapture_write_filename(writer, origin_filename);                 \
    BOOST_CHECK_EQUAL(writer.s, wrote_filename);             \
    writer.s.clear()

    TEST_WRITE_FILENAME("abcde.txt", "abcde.txt");

    TEST_WRITE_FILENAME(R"(\abcde.txt)", R"(\\abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc\de.txt)", R"(abc\\de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt\)", R"(abcde.txt\\)");
    TEST_WRITE_FILENAME(R"(abc\\de.txt)", R"(abc\\\\de.txt)");
    TEST_WRITE_FILENAME(R"(\\\\)", R"(\\\\\\\\)");

    TEST_WRITE_FILENAME(R"( abcde.txt)", R"(\ abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc de.txt)", R"(abc\ de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt )", R"(abcde.txt\ )");
    TEST_WRITE_FILENAME(R"(abc  de.txt)", R"(abc\ \ de.txt)");
    TEST_WRITE_FILENAME(R"(    )", R"(\ \ \ \ )");
}

BOOST_AUTO_TEST_CASE(TestWriteHash)
{
    wrmcapture_hash_type hash;
    std::iota(std::begin(hash), std::end(hash), 0);

    char hash_str[wrmcapture_hash_string_len + 1];
    *wrmcapture_swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
        " 202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
    );
}

BOOST_AUTO_TEST_CASE(TestOutmetaTransport)
{
    unsigned sec_start = 1352304810;
    {
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;

    const char * meta_path = "./xxx.mwrm";
    const char * meta_hash_path = "./hash-xxx.mwrm";
    meta_len_writer.len = 5; // header
    struct stat stat;
    BOOST_CHECK(!::stat(meta_path, &stat));
    BOOST_CHECK(!wrmcapture_write_meta_file_impl_false(meta_len_writer, meta_path + 2, stat, nullptr));
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_hash_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_hash_path));


    meta_len_writer.len = 0;

    wrmcapture_write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, false);

    const char * file1 = "./xxx-000000.wrm";
    BOOST_CHECK(!wrmcapture_write_meta_file(meta_len_writer, file1, sec_start, sec_start+1));
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

    const char * file2 = "./xxx-000001.wrm";
    BOOST_CHECK(!wrmcapture_write_meta_file(meta_len_writer, file2, sec_start, sec_start+1));
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}


BOOST_AUTO_TEST_CASE(TestOutmetaTransportWithSum)
{
    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransportWithSum wrm_trans(cctx, "./", "./", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;
    wrmcapture_write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, true);

    const unsigned hash_size = (1 + MD_HASH_LENGTH*2) * 2;

//    char file1[1024];
//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    const char * file1 = "./xxx-000000.wrm";
    wrmcapture_write_meta_file(meta_len_writer, file1, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

//    char file2[1024];
//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    const char * file2 = "./xxx-000001.wrm";
    wrmcapture_write_meta_file(meta_len_writer, file2, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

//    char meta_path[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    const char * meta_path = "./xxx.mwrm";
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}

BOOST_AUTO_TEST_CASE(TestRequestFullCleaning)
{
    unlink("./xxx-000000.wrm");
    unlink("./xxx-000001.wrm");
    unlink("./xxx.mwrm");

    timeval now;
    now.tv_sec = 1352304810;
    now.tv_usec = 0;
    const int groupid = 0;
    wrmcapture_OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid, nullptr,
                                       wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION);
    wrm_trans.send("AAAAX", 5);
    wrm_trans.send("BBBBX", 5);
    wrm_trans.next();
    wrm_trans.send("CCCCX", 5);

    const wrmcapture_FilenameGenerator * sqgen = wrm_trans.seqgen();

    BOOST_CHECK(-1 != filesize(sqgen->get(0)));
    BOOST_CHECK(-1 != filesize(sqgen->get(1)));
    BOOST_CHECK(-1 != filesize("./xxx.mwrm"));

    wrm_trans.request_full_cleaning();

    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(0)));
    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(1)));
    BOOST_CHECK_EQUAL(-1, filesize("./xxx.mwrm"));
}

//void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse);

//void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse)
//{
//    Rect screen(0, 0, drawable.width(), drawable.height());
//    auto const color_cxt = gdi::ColorCtx::depth24();
//    drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

//    uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
//    Rect r(10, 10, 50, 50);
//    int vx = 5;
//    int vy = 4;
//    for (size_t x = 0; x < duration; x++) {
//        drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
//        r.y += vy;
//        r.x += vx;
//        drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
//        usec += 40000LL;
//        now.tv_sec  = usec / 1000000LL;
//        now.tv_usec = (usec % 1000000LL);
//        //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
//        int cursor_x = mouse?r.x + 10:0;
//        int cursor_y = mouse?r.y + 10:0;
//        drawable.set_mouse_cursor_pos(cursor_x, cursor_y);
//        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
//        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
//        if ((r.x + r.cx >= drawable.width())  || (r.x < 0)) { vx = -vx; }
//        if ((r.y + r.cy >= drawable.height()) || (r.y < 0)) { vy = -vy; }
//    }
//}

//BOOST_AUTO_TEST_CASE(TestSequencedVideoCapture)
//{
//    {
//        struct notified_on_video_change : public NotifyNextVideo
//        {
//            void notify_next_video(const timeval& now, reason reason) 
//            {
//                LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
//                    static_cast<unsigned>(now.tv_sec),
//                    static_cast<unsigned>(now.tv_usec),
//                    static_cast<unsigned>(reason));
//            }
//        } next_video_notifier;

//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_videocapture", 
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture-000000.png", 3099},
//        {"./opaquerect_videocapture-000000.flv", 40677},
//        {"./opaquerect_videocapture-000001.png", 3104},
//        {"./opaquerect_videocapture-000001.flv", 40011},
//        {"./opaquerect_videocapture-000002.png", 3107},
//        {"./opaquerect_videocapture-000002.flv", 41172},
//        {"./opaquerect_videocapture-000003.png", 3099},
//        {"./opaquerect_videocapture-000003.flv", 40610},
//        {"./opaquerect_videocapture-000004.png", 3098},
//        {"./opaquerect_videocapture-000004.flv", 40173},
//        {"./opaquerect_videocapture-000005.png", 3098},
//        {"./opaquerect_videocapture-000005.flv", 13539},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestSequencedVideoCaptureMP4)
//{
//    {
//        struct notified_on_video_change : public NotifyNextVideo
//        {
//            void notify_next_video(const timeval& now, reason reason) 
//            {
//                LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
//                    static_cast<unsigned>(now.tv_sec),
//                    static_cast<unsigned>(now.tv_usec),
//                    static_cast<unsigned>(reason));
//            }
//        } next_video_notifier;

//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        SequencedVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_videocapture", 
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//        size_t alternativesize;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture-000000.png", 3099, 3099},
//        {"./opaquerect_videocapture-000000.mp4", 12999, 12985},
//        {"./opaquerect_videocapture-000001.png", 3104, 3104},
//        {"./opaquerect_videocapture-000001.mp4", 11726, 11712},
//        {"./opaquerect_videocapture-000002.png", 3107, 3107},
//        {"./opaquerect_videocapture-000002.mp4", 10798, 10784},
//        {"./opaquerect_videocapture-000003.png", 3099, 3099},
//        {"./opaquerect_videocapture-000003.mp4", 11329, 11315},
//        {"./opaquerect_videocapture-000004.png", 3098, 3098},
//        {"./opaquerect_videocapture-000004.mp4", 12331, 12317},
//        {"./opaquerect_videocapture-000005.png", 3098, 3098},
//        {"./opaquerect_videocapture-000005.mp4", 262, 0},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        if (fsize != x.size && fsize != x.alternativesize){
//            BOOST_CHECK_EQUAL(x.size, fsize);
//        }
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestVideoCaptureOneChunkFLV)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason) 
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_videocapture_one_chunk_xxx", 
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000 * 1000000l}, next_video_notifier);
//        simple_movie(now, 1000, drawable, video_capture, false, true);
//    }

//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_videocapture_one_chunk_xxx-000000.png", 3099},
//        {"./opaquerect_videocapture_one_chunk_xxx-000000.flv", 645722},
//        {"./opaquerect_videocapture_one_chunk_xxx-000001.png", static_cast<long unsigned>(-1)},
//        {"./opaquerect_videocapture_one_chunk_xxx-000001.flv", static_cast<long unsigned>(-1)},
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        FullVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_fullvideocapture_timestamp1", 
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp1.flv";
//    BOOST_CHECK_EQUAL(164693, filesize(filename));
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv2)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        FullVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_fullvideocapture_timestamp_mouse0", 
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, false);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp_mouse0.flv";
//    BOOST_CHECK_EQUAL(158699, filesize(filename));
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(TestFullVideoCaptureX264)
//{
//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        FullVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_fullvideocapture_timestamp2",
//            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    const char * filename = "./opaquerect_fullvideocapture_timestamp2.mp4";
//    size_t fsize = filesize(filename);
//     // size actually depends on the codec version and at least two slightltly different ones exists for h264
//    if (fsize != 54190 && fsize != 54176){
//        BOOST_CHECK_EQUAL(54190, filesize(filename));
//    }
//    ::unlink(filename);

//}

//BOOST_AUTO_TEST_CASE(SequencedVideoCaptureFLV)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason) 
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
//        SequencedVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_seqvideocapture",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000000}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    
//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//    } fileinfo[] = {
//        {"./opaquerect_seqvideocapture-000000.png", 3099},
//        {"./opaquerect_seqvideocapture-000000.flv", 29439},
//        {"./opaquerect_seqvideocapture-000001.png", 3099},
//        {"./opaquerect_seqvideocapture-000001.flv", 30726},
//        {"./opaquerect_seqvideocapture-000002.png", 3104},
//        {"./opaquerect_seqvideocapture-000002.flv", 29119},
//        {"./opaquerect_seqvideocapture-000003.png", 3101},
//        {"./opaquerect_seqvideocapture-000003.flv", 29108},
//        {"./opaquerect_seqvideocapture-000004.png", 3107},
//        {"./opaquerect_seqvideocapture-000004.flv", 29088},
//        {"./opaquerect_seqvideocapture-000005.png", 3101},
//        {"./opaquerect_seqvideocapture-000005.flv", 30560},
//        {"./opaquerect_seqvideocapture-000006.png", 3099},
//        {"./opaquerect_seqvideocapture-000006.flv", 29076},
//        {"./opaquerect_seqvideocapture-000007.png", 3101},
//        {"./opaquerect_seqvideocapture-000007.flv", 30125},
//        {"./opaquerect_seqvideocapture-000008.png", 3098},
//        {"./opaquerect_seqvideocapture-000008.flv", 28966},
//        {"./opaquerect_seqvideocapture-000009.png", 3098},
//        {"./opaquerect_seqvideocapture-000009.flv", 29309},
//        {"./opaquerect_seqvideocapture-000010.png", 3098},
//        {"./opaquerect_seqvideocapture-000010.flv", 13539}
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        BOOST_CHECK_EQUAL(x.size, fsize);
//        ::unlink(x.filename);
//    }
//}


//BOOST_AUTO_TEST_CASE(SequencedVideoCaptureX264)
//{
//    struct notified_on_video_change : public NotifyNextVideo
//    {
//        void notify_next_video(const timeval& now, reason reason) 
//        {
//            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
//                static_cast<unsigned>(now.tv_sec),
//                static_cast<unsigned>(now.tv_usec),
//                static_cast<unsigned>(reason));
//        }
//    } next_video_notifier;

//    {
//        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
//        RDPDrawable drawable(800, 600);
//        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
//        SequencedVideoCaptureImpl video_capture(now, 
//            "./", "opaquerect_seqvideocapture_timestamp2",
//            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
//            std::chrono::microseconds{1000000}, next_video_notifier);
//        simple_movie(now, 250, drawable, video_capture, false, true);
//    }
//    struct CheckFiles {
//        const char * filename;
//        size_t size;
//        // size actually depends on the codec version and at least two slightltly different ones exists for h264
//        size_t alternativesize;
//    } fileinfo[] = {
//        {"./opaquerect_seqvideocapture_timestamp2-000000.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000000.mp4", 7323, 7309},
//        {"./opaquerect_seqvideocapture_timestamp2-000001.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000001.mp4", 6889, 6875},
//        {"./opaquerect_seqvideocapture_timestamp2-000002.png", 3104, 3104},
//        {"./opaquerect_seqvideocapture_timestamp2-000002.mp4", 6629, 6615},
//        {"./opaquerect_seqvideocapture_timestamp2-000003.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000003.mp4", 6385, 6371},
//        {"./opaquerect_seqvideocapture_timestamp2-000004.png", 3107, 3107},
//        {"./opaquerect_seqvideocapture_timestamp2-000004.mp4", 6013, 5999},
//        {"./opaquerect_seqvideocapture_timestamp2-000005.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000005.mp4", 6036, 6022},
//        {"./opaquerect_seqvideocapture_timestamp2-000006.png", 3099, 3099},
//        {"./opaquerect_seqvideocapture_timestamp2-000006.mp4", 6133, 6119},
//        {"./opaquerect_seqvideocapture_timestamp2-000007.png", 3101, 3101},
//        {"./opaquerect_seqvideocapture_timestamp2-000007.mp4", 6410, 6396},
//        {"./opaquerect_seqvideocapture_timestamp2-000008.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000008.mp4", 6631, 6617},
//        {"./opaquerect_seqvideocapture_timestamp2-000009.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000009.mp4", 6876, 6862},
//        {"./opaquerect_seqvideocapture_timestamp2-000010.png", 3098, 3098},
//        {"./opaquerect_seqvideocapture_timestamp2-000010.mp4", 262, 262}
//    };
//    for (auto x: fileinfo) {
//        size_t fsize = filesize(x.filename);
//        if ((x.size != fsize) && (x.alternativesize != fsize)){
//            BOOST_CHECK_EQUAL(x.size, filesize(x.filename));
//        }
//        ::unlink(x.filename);
//    }
//}

