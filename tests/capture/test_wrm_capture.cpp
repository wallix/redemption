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

   Unit test to capture interface to video recording to flv or mp4frecv
*/

#define RED_TEST_MODULE TestWrmCapture
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/file_to_graphic.hpp"
#include "capture/wrm_capture.hpp"
#include "core/app_path.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/transport.hpp"

#include "test_only/fake_stat.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/gdi/test_graphic.hpp"

#include <cstring>
#include <string>


template<class Writer>
void wrmcapture_write_meta_headers(Writer & writer, uint16_t width, uint16_t height, bool has_checksum)
{
    char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
    const int len = sprintf(
        header1,
        "v2\n"
        "%u %u\n"
        "%s\n"
        "\n\n",
        unsigned(width),
        unsigned(height),
        has_checksum  ? "checksum" : "nochecksum"
    );
    RED_CHECK_EQ(writer.write(header1, len), len);
}


RED_AUTO_TEST_CASE(TestWrmCapture)
{
    const struct CheckFiles {
        const char * filename;
        int size;
    } fileinfos[] = {
        {"./capture-000000.wrm", 1646},
        {"./capture-000001.wrm", 3508},
        {"./capture-000002.wrm", 3463},
        {"./capture-000003.wrm", -1},
        {"./capture.mwrm", 165},
        {"/tmp/capture-000000.wrm", 40},
        {"/tmp/capture-000001.wrm", 40},
        {"/tmp/capture-000002.wrm", 40},
        {"/tmp/capture-000003.wrm", -1},
        {"/tmp/capture.mwrm", 34},
    };
    for (auto const & d : fileinfos) {
        unlink(d.filename);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0)
    //     |GraphicToFile::Verbose::primary_orders)
    //     |GraphicToFile::Verbose::secondary_orders)
    //     |GraphicToFile::Verbose::bitmap_update)
        ;

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp/";

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, app_path(AppPath::Wrm));     // default value, actual one should come from movie_path
        strcat(path, "/");
        strcpy(basename, "capture");
        strcpy(extension, "");          // extension is currently ignored

        cctx.set_trace_type(TraceType::localfile);

        WrmParams wrm_params(
            BitsPerPixel{24},
            false,
            cctx,
            rnd,
            fstat,
            hash_path,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        TestGraphic gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(
          CaptureParams{now, basename, "", record_path, groupid, nullptr, SmartVideoCropping::disable, 0},
          wrm_params, gd_drawable);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable->draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable->draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable->draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable->draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable->draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        gd_drawable->draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable->draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
    }

    for (auto x : fileinfos) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
        ::unlink(x.filename);
    }
}

RED_AUTO_TEST_CASE(TestWrmCaptureLocalHashed)
{
    const char * filesinfo[] = {
        "./capture-000000.wrm",
        "./capture-000001.wrm",
        "./capture-000002.wrm",
        "./capture-000003.wrm",
        "./capture.mwrm",
        "/tmp/capture.mwrm",
    };
    for (auto x: filesinfo) {
        ::unlink(x);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;

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

        RED_CHECK(true);

        cctx.set_trace_type(TraceType::localfile_hashed);

        WrmParams wrm_params(
            BitsPerPixel{24},
            false,
            cctx,
            rnd,
            fstat,
            "/tmp/",
            std::chrono::seconds{1},
            std::chrono::seconds{3},
            WrmCompressionAlgorithm::no_compression,
            0 //0xFFFF VERBOSE
        );

        RED_CHECK(true);

        TestGraphic gd_drawable(scr.cx, scr.cy);

        WrmCaptureImpl wrm(
            CaptureParams{now, "capture", "", "./", 1000, nullptr, SmartVideoCropping::disable, 0},
            wrm_params/* authentifier */, gd_drawable);

        RED_CHECK(true);

        auto const color_cxt = gdi::ColorCtx::depth24();
        bool ignore_frame_in_timeval = false;

        gd_drawable->draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable->draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable->draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable->draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable->draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        gd_drawable->draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

        RED_CHECK(true);

        // ------------------------------ BREAKPOINT ------------------------------

        gd_drawable->draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
        now.tv_sec++;
        wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
        // The destruction of capture object will finalize the metafile content
        RED_CHECK(true);

    }
    RED_CHECK(true);

    // TODO: we may have several mwrm sizes as it contains varying length numbers
    // the right solution would be to inject predictable fstat in test environment
    struct CheckFiles {
        const char * filename;
        size_t size;
        size_t altsize;
    } fileinfo[] = {
        {"./capture-000000.wrm", 1646, 0},
        {"./capture-000001.wrm", 3508, 0},
        {"./capture-000002.wrm", 3463, 0},
        {"./capture-000003.wrm", static_cast<size_t>(-1), static_cast<size_t>(-1)},
        {"./capture.mwrm", 676, 553},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if (x.size != fsize){
            RED_CHECK_EQUAL(x.altsize, fsize);
        }
        ::unlink(x.filename);
    }
}


template<class Writer>
int wrmcapture_write_meta_file(
    Writer & writer, Fstat & fstat, const char * filename,
    time_t start_sec, time_t stop_sec
) {
    struct stat stat;
    int err = fstat.stat(filename, stat);
    if (err){
        return err;
    }

    MwrmWriterBuf mwrm_buf;
    MwrmWriterBuf::HashArray dummy_hash;
    mwrm_buf.write_line(filename, stat, start_sec, stop_sec, false, dummy_hash, dummy_hash);

    auto buf = mwrm_buf.buffer();
    ssize_t res = writer.write(cbyte_ptr(buf.data()), buf.size());
    if (res < static_cast<ssize_t>(buf.size())) {
        return res < 0 ? res : 1;
    }
    return 0;
}


RED_AUTO_TEST_CASE(TestWriteFilename)
{
    auto wrmcapture_write_filename = [](char const * filename) {
        struct stat st;
        FakeFstat().stat("", st);
        MwrmWriterBuf mwrm_buf;
        MwrmWriterBuf::HashArray dummy_hash;
        mwrm_buf.write_line(filename, st, 0, 0, false, dummy_hash, dummy_hash);
        auto buf = mwrm_buf.buffer();
        return std::string{cbyte_ptr(buf.data()), buf.size()};
    };

#define TEST_WRITE_FILENAME(origin_filename, wrote_filename)    \
    RED_CHECK_EQUAL(wrmcapture_write_filename(origin_filename), \
    wrote_filename " 0 0 0 0 0 0 0 0 0 0\n");                   \

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

RED_AUTO_TEST_CASE(TestOutmetaTransport)
{
    ::unlink("./xxx-000000.wrm");
    ::unlink("./xxx-000001.wrm");
    ::unlink("./xxx.mwrm");
    ::unlink("./hash-xxx-000000.wrm");
    ::unlink("./hash-xxx-000001.wrm");
    ::unlink("./hash-xxx.mwrm");

    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        LCGRandom rnd(0);
        FakeFstat fstat;

        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;

        cctx.set_trace_type(TraceType::localfile);

        OutMetaSequenceTransport wrm_trans(cctx, rnd, fstat, "./", "./hash-", "xxx", now, 800, 600, groupid, nullptr);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    const char * meta_path = "./xxx.mwrm";
    const char * filename = meta_path + 2;
    const char * meta_hash_path = "./hash-xxx.mwrm";

    MwrmWriterBuf meta_file_buf;
    MwrmWriterBuf::HashArray dummy_hash;
    struct stat st {};
    meta_file_buf.write_hash_file(filename, st, false, dummy_hash, dummy_hash);

    RED_CHECK_EQUAL(meta_file_buf.buffer().size(), filesize(meta_hash_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_hash_path));

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;

    wrmcapture_write_meta_headers(meta_len_writer, 800, 600, false);

    FakeFstat fstat;
    const char * file1 = "./xxx-000000.wrm";
    RED_CHECK(!wrmcapture_write_meta_file(meta_len_writer, fstat, file1, sec_start, sec_start+1));
    RED_CHECK_EQUAL(10, filesize(file1));
    RED_CHECK_EQUAL(0, ::unlink(file1));
    RED_CHECK_EQUAL(36, filesize("./hash-xxx-000000.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("./hash-xxx-000000.wrm"));

    const char * file2 = "./xxx-000001.wrm";
    RED_CHECK(!wrmcapture_write_meta_file(meta_len_writer, fstat, file2, sec_start, sec_start+1));
    RED_CHECK_EQUAL(5, filesize(file2));
    RED_CHECK_EQUAL(0, ::unlink(file2));
    RED_CHECK_EQUAL(36, filesize("./hash-xxx-000001.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("./hash-xxx-000001.wrm"));

    RED_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_path));
}


RED_AUTO_TEST_CASE(TestOutmetaTransportWithSum)
{
    ::unlink("/tmp/xxx-000000.wrm");
    ::unlink("/tmp/xxx-000001.wrm");
    ::unlink("/tmp/xxx.mwrm");

    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

        LCGRandom rnd(0);
        FakeFstat fstat;

        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;

        cctx.set_trace_type(TraceType::localfile_hashed);

        OutMetaSequenceTransport wrm_trans(cctx, rnd, fstat, "./", "/tmp/", "xxx", now, 800, 600, groupid, nullptr);
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
    wrmcapture_write_meta_headers(meta_len_writer, 800, 600, true);

    const unsigned hash_size = (1 + MD_HASH::DIGEST_LENGTH*2) * 2;

    FakeFstat fstat;

//    char file1[1024];
//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    const char * file1 = "./xxx-000000.wrm";
    wrmcapture_write_meta_file(meta_len_writer, fstat, file1, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    RED_CHECK_EQUAL(10, filesize(file1));
    RED_CHECK_EQUAL(0, ::unlink(file1));

//    char file2[1024];
//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    const char * file2 = "./xxx-000001.wrm";
    wrmcapture_write_meta_file(meta_len_writer, fstat, file2, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    RED_CHECK_EQUAL(5, filesize(file2));
    RED_CHECK_EQUAL(0, ::unlink(file2));

//    char meta_path[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    const char * meta_path = "./xxx.mwrm";
    RED_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    RED_CHECK_EQUAL(0, ::unlink(meta_path));

    RED_CHECK_EQUAL(166, filesize("/tmp/xxx-000000.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx-000000.wrm"));
    RED_CHECK_EQUAL(166, filesize("/tmp/xxx-000001.wrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx-000001.wrm"));
    RED_CHECK_EQUAL(160, filesize("/tmp/xxx.mwrm"));
    RED_CHECK_EQUAL(0, ::unlink("/tmp/xxx.mwrm"));
}

RED_AUTO_TEST_CASE(TestWrmCaptureKbdInput)
{
    const struct CheckFiles {
        const char * filename;
        int size;
    } fileinfos[] = {
        {"./capture_kbd_input-000000.wrm", 292},
        {"./capture_kbd_input.mwrm", 77},
        {"/tmp/capture_kbd_input-000000.wrm", 50},
        {"/tmp/capture_kbd_input.mwrm", 44},
    };
    for (auto const & d : fileinfos) {
        unlink(d.filename);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0)
    //     |GraphicToFile::Verbose::primary_orders)
    //     |GraphicToFile::Verbose::secondary_orders)
    //     |GraphicToFile::Verbose::bitmap_update)
        ;

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp/";

        char basename[1024];
        char extension[128];
        strcpy(basename, "capture_kbd_input");
        strcpy(extension, "");          // extension is currently ignored

        cctx.set_trace_type(TraceType::localfile);

        WrmParams wrm_params(
            BitsPerPixel{24},
            false,
            cctx,
            rnd,
            fstat,
            hash_path,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        TestGraphic gd_drawable(4, 1);

        WrmCaptureImpl wrm(
          CaptureParams{now, basename, "", record_path, groupid, nullptr, SmartVideoCropping::disable, 0},
          wrm_params, gd_drawable);

        bool ignore_frame_in_timeval = false;

        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);

        wrm.kbd_input(now, 'i');
        wrm.kbd_input(now, 'p');
        wrm.kbd_input(now, 'c');
        wrm.kbd_input(now, 'o');
        wrm.kbd_input(now, 'n');
        wrm.kbd_input(now, 'f');
        wrm.kbd_input(now, 'i');
        now.tv_sec++;
        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);

        wrm.kbd_input(now, 'g');
        wrm.kbd_input(now, '\r');

        wrm.session_update(now, cstr_array_view("FOREGROUND_WINDOW_CHANGED=WINDOW\x01CLASS\x01COMMAND_LINE"));

        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);
    }

    struct KbdInput : public gdi::KbdInputApi
    {
        KbdInput(std::string& output) : output(output) {

        }

        bool kbd_input(timeval const & /*now*/, uint32_t uchar) override
        {
            this->output += char(uchar);

            return true;
        }

        void enable_kbd_input_mask(bool /*enable*/) override {}

        std::string & output;
    };

    struct CaptureProbe : public gdi::CaptureProbeApi
    {
        CaptureProbe(std::string& output) : output(output) {

        }

        void session_update(timeval const & /*now*/, array_view_const_char message) override
        {
            output.append(message.data(), message.size());
        }

        void possible_active_window_change() override {}

        std::string & output;
    };

    int fd = ::open(fileinfos[0].filename, O_RDONLY);
    RED_REQUIRE_NE(fd, -1);
    InFileTransport in_wrm_trans(unique_fd{fd});

    FileToGraphic player(in_wrm_trans, {}, {}, false, false, to_verbose_flags(0));

    std::string output;
    KbdInput kbd_input(output);
    CaptureProbe capture_probe(output);
    player.add_consumer(nullptr, nullptr, &kbd_input, &capture_probe, nullptr, nullptr);

    while(player.next_order())
    {
        player.interpret_order();
    }

//    RED_CHECK_EQUAL(output, "FOREGROUND_WINDOW_CHANGED=WINDOW\x01CLASS\x01COMMAND_LINE");

    RED_CHECK_SMEM_AC(output, "ipconfig\rFOREGROUND_WINDOW_CHANGED=WINDOW\x01CLASS\x01COMMAND_LINE\x00");

    for (auto x : fileinfos) {
        auto fsize = filesize(x.filename);
        RED_CHECK_MESSAGE(
            x.size == fsize,
            "check " << x.size << " == filesize(\"" << x.filename
            << "\") failed [" << x.size << " != " << fsize << "]"
        );
       ::unlink(x.filename);
    }
}

RED_AUTO_TEST_CASE(TestWrmCaptureRemoteApp)
{
    const struct CheckFiles {
        const char * filename;
        int size;
    } fileinfos[] = {
        {"./capture_remoteapp-000000.wrm", 1670},
        {"./capture_remoteapp.mwrm", 81},
        {"/tmp/capture_remoteapp-000000.wrm", 50},
        {"/tmp/capture_remoteapp.mwrm", 44},
    };
    for (auto const & d : fileinfos) {
        unlink(d.filename);
    }

    {
        // Timestamps are applied only when flushing
        timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);

        LCGRandom rnd(0);
        FakeFstat fstat;
        CryptoContext cctx;

        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(0);

        WrmCompressionAlgorithm wrm_compression_algorithm = WrmCompressionAlgorithm::no_compression;
        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = std::chrono::seconds{1};
        std::chrono::seconds wrm_break_interval = std::chrono::seconds{3};

        const char * record_path = "./";
        const int groupid = 0; // www-data
        const char * hash_path = "/tmp/";

        char basename[1024];
        char extension[128];
        strcpy(basename, "capture_remoteapp");
        strcpy(extension, "");          // extension is currently ignored

        cctx.set_trace_type(TraceType::localfile);

        WrmParams wrm_params(
            BitsPerPixel{24},
            true,   // RemoteApp
            cctx,
            rnd,
            fstat,
            hash_path,
            wrm_frame_interval,
            wrm_break_interval,
            wrm_compression_algorithm,
            int(wrm_verbose)
        );

        auto const color_cxt = gdi::ColorCtx::depth24();

        TestGraphic gd_drawable(800, 600);

        WrmCaptureImpl wrm(
          CaptureParams{now, basename, "", record_path, groupid, nullptr, SmartVideoCropping::v1, 0},
          wrm_params, gd_drawable);

        bool ignore_frame_in_timeval = false;

        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);

        wrm.draw(RDPOpaqueRect(scr, encode_color24()(BLACK)), scr, color_cxt);

        Rect rect = Rect(50, 50, 320, 200);
        wrm.draw(RDPOpaqueRect(rect, encode_color24()(YELLOW)), rect, color_cxt);
        wrm.visibility_rects_event(rect);


        now.tv_sec++;
        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);

        wrm.draw(RDPOpaqueRect(scr, encode_color24()(BLACK)), scr, color_cxt);

        rect = Rect(125, 75, 370, 250);
        wrm.draw(RDPOpaqueRect(rect, encode_color24()(BLUE)), rect, color_cxt);
        wrm.visibility_rects_event(rect);

        wrm.send_timestamp_chunk(now, ignore_frame_in_timeval);
    }

    int fd = ::open(fileinfos[0].filename, O_RDONLY);
    RED_REQUIRE_NE(fd, -1);
    InFileTransport in_wrm_trans(unique_fd{fd});

    FileToGraphic player(in_wrm_trans, {}, {}, false, false, to_verbose_flags(0));

    while(player.next_order())
    {
        player.interpret_order();
    }

    RED_CHECK(player.info.remote_app);
    RED_CHECK_EQUAL(player.max_image_frame_rect, Rect(50, 50, 320, 200).disjunct(Rect(125, 75, 370, 250)));
    RED_CHECK_EQUAL(player.min_image_frame_dim, Dimension(370, 250));

    for (auto x : fileinfos) {
        RED_CHECK_FILE_SIZE_AND_CLEAN(x.filename, x.size);
    }
}
