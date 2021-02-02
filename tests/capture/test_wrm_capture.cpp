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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "core/log_id.hpp"
#include "capture/file_to_graphic.hpp"
#include "capture/wrm_capture.hpp"
#include "transport/file_transport.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/algostring.hpp"

#include "test_only/lcg_random.hpp"
#include "test_only/gdi/test_graphic.hpp"

#include <cstring>
#include <string>

namespace
{
    struct WrmForTest
    {
        // Timestamps are applied only when flushing
        MonotonicTimePoint now {344535s + 23us};

        Rect scr {0, 0, 800, 600};

        LCGRandom rnd;
        CryptoContext cctx;

        TestGraphic gd_drawable {scr.cx, scr.cy};

        WrmCaptureImpl wrm;

        WrmForTest(
            TraceType trace_type,
            WorkingDirectory& record_wd,
            WorkingDirectory& hash_wd,
            WorkingDirectory& tmp_wd,
            char const* filebase = "capture",
            uint16_t cx = 0, uint16_t cy = 0,
            bool remote_app = false)
        : gd_drawable(cx ? cx : scr.cx, cy ? cy : scr.cy)
        , wrm(
            CaptureParams{
                now, DurationFromMonotonicTimeToRealTime{1000s - now.time_since_epoch()},
                filebase, tmp_wd.dirname(), record_wd.dirname(),
                /*groupid=*/0, nullptr, SmartVideoCropping::disable, 0},
            WrmParams{
                BitsPerPixel{24},
                remote_app,
                [&]() -> CryptoContext& {
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
                    cctx.set_trace_type(trace_type);
                    return cctx;
                }(),
                rnd,
                hash_wd.dirname(),
                /*wrm_frame_interval = */1s,
                /*wrm_break_interval = */3s,
                WrmCompressionAlgorithm::no_compression,
                RDPSerializerVerbose::none,
                -1u,
            },
            gd_drawable)
        {}
    };
}

static void gen_wrm1(
    TraceType trace_type,
    WorkingDirectory& record_wd,
    WorkingDirectory& hash_wd,
    WorkingDirectory& tmp_wd)
{
    WrmForTest wrm_test(trace_type, record_wd, hash_wd, tmp_wd);
    const auto scr = wrm_test.scr;
    auto& gd_drawable = wrm_test.gd_drawable;
    auto& wrm = wrm_test.wrm;
    auto now = wrm_test.now;

    auto const color_cxt = gdi::ColorCtx::depth24();
    bool ignore_frame_in_timeval = false;

    gd_drawable->draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    gd_drawable->draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), encode_color24()(BLUE)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    gd_drawable->draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), encode_color24()(WHITE)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    // ------------------------------ BREAKPOINT ------------------------------

    gd_drawable->draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), encode_color24()(RED)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    gd_drawable->draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), encode_color24()(BLACK)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    gd_drawable->draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), encode_color24()(PINK)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);

    // ------------------------------ BREAKPOINT ------------------------------

    gd_drawable->draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
    wrm.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), encode_color24()(WABGREEN)), scr, color_cxt);
    now += 1s;
    wrm.periodic_snapshot(now, 0, 0, ignore_frame_in_timeval);
    // The destruction of capture object will finalize the metafile content
}

static struct stat get_stat(char const* filename)
{
    class stat st;
    stat(filename, &st);
    return st;
}

static std::string prefix(std::string data, std::size_t n){
    if (data.size() > n) {
        data.resize(n);
    }
    return data;
}

RED_AUTO_TEST_CASE(TestWrmCapture)
{
    WorkingDirectory record_wd("record");
    WorkingDirectory hash_wd("hash");
    WorkingDirectory tmp_wd("tmp");

    gen_wrm1(TraceType::localfile, record_wd, hash_wd, tmp_wd);

    auto mwrm = record_wd.add_file("capture.mwrm");
    auto wrm1 = record_wd.add_file("capture-000000.wrm");
    auto wrm2 = record_wd.add_file("capture-000001.wrm");
    auto wrm3 = record_wd.add_file("capture-000002.wrm");
    auto st = get_stat(mwrm);
    auto st1 = get_stat(wrm1);
    auto st2 = get_stat(wrm2);
    auto st3 = get_stat(wrm3);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n800 600\nnochecksum\n\n\n",
        wrm1, " 1646 ",
        int_to_chars(st1.st_mode), ' ',
        int_to_chars(st1.st_uid), ' ',
        int_to_chars(st1.st_gid), ' ',
        int_to_chars(st1.st_dev), ' ',
        int_to_chars(st1.st_ino), ' ',
        int_to_chars(st1.st_mtim.tv_sec), ' ',
        int_to_chars(st1.st_ctim.tv_sec), " 1000 1004\n",
        wrm2, " 3508 ",
        int_to_chars(st2.st_mode), ' ',
        int_to_chars(st2.st_uid), ' ',
        int_to_chars(st2.st_gid), ' ',
        int_to_chars(st2.st_dev), ' ',
        int_to_chars(st2.st_ino), ' ',
        int_to_chars(st2.st_mtim.tv_sec), ' ',
        int_to_chars(st2.st_ctim.tv_sec), " 1004 1007\n",
        wrm3, " 3463 ",
        int_to_chars(st3.st_mode), ' ',
        int_to_chars(st3.st_uid), ' ',
        int_to_chars(st3.st_gid), ' ',
        int_to_chars(st3.st_dev), ' ',
        int_to_chars(st3.st_ino), ' ',
        int_to_chars(st3.st_mtim.tv_sec), ' ',
        int_to_chars(st3.st_ctim.tv_sec), " 1007 1008\n")});
    RED_TEST_FILE_SIZE(wrm1, 1646);
    RED_TEST_FILE_SIZE(wrm2, 3508);
    RED_TEST_FILE_SIZE(wrm3, 3463);

    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture.mwrm"), array_view{str_concat(
        "v2\n\n\ncapture.mwrm ",
        int_to_chars(st.st_size), ' ',
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), '\n')});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000000.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000000.wrm 1646 ",
        int_to_chars(st1.st_mode), ' ',
        int_to_chars(st1.st_uid), ' ',
        int_to_chars(st1.st_gid), ' ',
        int_to_chars(st1.st_dev), ' ',
        int_to_chars(st1.st_ino), ' ',
        int_to_chars(st1.st_mtim.tv_sec), ' ',
        int_to_chars(st1.st_ctim.tv_sec), '\n')});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000001.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000001.wrm 3508 ",
        int_to_chars(st2.st_mode), ' ',
        int_to_chars(st2.st_uid), ' ',
        int_to_chars(st2.st_gid), ' ',
        int_to_chars(st2.st_dev), ' ',
        int_to_chars(st2.st_ino), ' ',
        int_to_chars(st2.st_mtim.tv_sec), ' ',
        int_to_chars(st2.st_ctim.tv_sec), '\n')});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000002.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000002.wrm 3463 ",
        int_to_chars(st3.st_mode), ' ',
        int_to_chars(st3.st_uid), ' ',
        int_to_chars(st3.st_gid), ' ',
        int_to_chars(st3.st_dev), ' ',
        int_to_chars(st3.st_ino), ' ',
        int_to_chars(st3.st_mtim.tv_sec), ' ',
        int_to_chars(st3.st_ctim.tv_sec), '\n')});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(tmp_wd);
}

RED_AUTO_TEST_CASE(TestWrmCaptureLocalHashed)
{
    WorkingDirectory record_wd("record");
    WorkingDirectory hash_wd("hash");
    WorkingDirectory tmp_wd("tmp");

    gen_wrm1(TraceType::localfile_hashed, record_wd, hash_wd, tmp_wd);

    auto mwrm = record_wd.add_file("capture.mwrm");
    auto wrm1 = record_wd.add_file("capture-000000.wrm");
    auto wrm2 = record_wd.add_file("capture-000001.wrm");
    auto wrm3 = record_wd.add_file("capture-000002.wrm");
    auto st = get_stat(mwrm);
    auto st1 = get_stat(wrm1);
    auto st2 = get_stat(wrm2);
    auto st3 = get_stat(wrm3);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n800 600\nchecksum\n\n\n",
        wrm1, " 1646 ",
        int_to_chars(st1.st_mode), ' ',
        int_to_chars(st1.st_uid), ' ',
        int_to_chars(st1.st_gid), ' ',
        int_to_chars(st1.st_dev), ' ',
        int_to_chars(st1.st_ino), ' ',
        int_to_chars(st1.st_mtim.tv_sec), ' ',
        int_to_chars(st1.st_ctim.tv_sec), " 1000 1004"
        " 4ac40d3a6ed890ac5dbf3e4a2d2d418e2b3117dc3a05b7634938e4ab4c6203b0"
        " 4ac40d3a6ed890ac5dbf3e4a2d2d418e2b3117dc3a05b7634938e4ab4c6203b0\n",
        wrm2, " 3508 ",
        int_to_chars(st2.st_mode), ' ',
        int_to_chars(st2.st_uid), ' ',
        int_to_chars(st2.st_gid), ' ',
        int_to_chars(st2.st_dev), ' ',
        int_to_chars(st2.st_ino), ' ',
        int_to_chars(st2.st_mtim.tv_sec), ' ',
        int_to_chars(st2.st_ctim.tv_sec), " 1004 1007"
        " bfe254764f99bb1f349bfbc669492fd6d307b15a1b9223b0513718150b9a30da"
        " bfe254764f99bb1f349bfbc669492fd6d307b15a1b9223b0513718150b9a30da\n",
        wrm3, " 3463 ",
        int_to_chars(st3.st_mode), ' ',
        int_to_chars(st3.st_uid), ' ',
        int_to_chars(st3.st_gid), ' ',
        int_to_chars(st3.st_dev), ' ',
        int_to_chars(st3.st_ino), ' ',
        int_to_chars(st3.st_mtim.tv_sec), ' ',
        int_to_chars(st3.st_ctim.tv_sec), " 1007 1008"
        " 0a6285817cd490b2fafc626db08eaa85169976d0aba96e9f7f4f34895adc4b97"
        " 0a6285817cd490b2fafc626db08eaa85169976d0aba96e9f7f4f34895adc4b97\n")});
    RED_TEST_FILE_SIZE(wrm1, 1646);
    RED_TEST_FILE_SIZE(wrm2, 3508);
    RED_TEST_FILE_SIZE(wrm3, 3463);

    auto hash_content = RED_CHECK_GET_FILE_CONTENTS(hash_wd.add_file("capture.mwrm"));
    auto hash_expected = str_concat(
        "v2\n\n\ncapture.mwrm ",
        int_to_chars(st.st_size), ' ',
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), ' ');
    RED_TEST(hash_content.size() == hash_expected.size() + 65*2);
    RED_TEST(prefix(hash_content, hash_expected.size()) == hash_expected);
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000000.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000000.wrm 1646 ",
        int_to_chars(st1.st_mode), ' ',
        int_to_chars(st1.st_uid), ' ',
        int_to_chars(st1.st_gid), ' ',
        int_to_chars(st1.st_dev), ' ',
        int_to_chars(st1.st_ino), ' ',
        int_to_chars(st1.st_mtim.tv_sec), ' ',
        int_to_chars(st1.st_ctim.tv_sec),
        " 4ac40d3a6ed890ac5dbf3e4a2d2d418e2b3117dc3a05b7634938e4ab4c6203b0"
        " 4ac40d3a6ed890ac5dbf3e4a2d2d418e2b3117dc3a05b7634938e4ab4c6203b0\n")});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000001.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000001.wrm 3508 ",
        int_to_chars(st2.st_mode), ' ',
        int_to_chars(st2.st_uid), ' ',
        int_to_chars(st2.st_gid), ' ',
        int_to_chars(st2.st_dev), ' ',
        int_to_chars(st2.st_ino), ' ',
        int_to_chars(st2.st_mtim.tv_sec), ' ',
        int_to_chars(st2.st_ctim.tv_sec),
        " bfe254764f99bb1f349bfbc669492fd6d307b15a1b9223b0513718150b9a30da"
        " bfe254764f99bb1f349bfbc669492fd6d307b15a1b9223b0513718150b9a30da\n")});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture-000002.wrm"), array_view{str_concat(
        "v2\n\n\ncapture-000002.wrm 3463 ",
        int_to_chars(st3.st_mode), ' ',
        int_to_chars(st3.st_uid), ' ',
        int_to_chars(st3.st_gid), ' ',
        int_to_chars(st3.st_dev), ' ',
        int_to_chars(st3.st_ino), ' ',
        int_to_chars(st3.st_mtim.tv_sec), ' ',
        int_to_chars(st3.st_ctim.tv_sec),
        " 0a6285817cd490b2fafc626db08eaa85169976d0aba96e9f7f4f34895adc4b97"
        " 0a6285817cd490b2fafc626db08eaa85169976d0aba96e9f7f4f34895adc4b97\n")});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(tmp_wd);
}

RED_AUTO_TEST_CASE(TestWrmCaptureKbdInput)
{
    WorkingDirectory record_wd("record");
    WorkingDirectory hash_wd("hash");
    WorkingDirectory tmp_wd("tmp");

    {
        WrmForTest wrm_test(TraceType::localfile, record_wd, hash_wd, tmp_wd,
            "capture_kbd_input", 4, 1);
        auto& wrm = wrm_test.wrm;
        auto now = wrm_test.now;

        wrm.send_timestamp_chunk(now);

        wrm.kbd_input(now, 'i');
        wrm.kbd_input(now, 'p');
        wrm.kbd_input(now, 'c');
        wrm.kbd_input(now, 'o');
        wrm.kbd_input(now, 'n');
        wrm.kbd_input(now, 'f');
        wrm.kbd_input(now, 'i');
        now += 1s;
        wrm.send_timestamp_chunk(now);

        wrm.kbd_input(now, 'g');
        wrm.kbd_input(now, '\r');

        wrm.session_update(now, LogId::FOREGROUND_WINDOW_CHANGED, {
            KVLog("windows"_av, "WINDOW"_av),
            KVLog("class"_av, "CLASS"_av),
            KVLog("command_line"_av, "COMMAND_LINE"_av),
        });

        wrm.send_timestamp_chunk(now);
    }

    struct KbdInput : public gdi::KbdInputApi
    {
        KbdInput(std::string& output) : output(output) {

        }

        bool kbd_input(MonotonicTimePoint /*now*/, uint32_t uchar) override
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

        void session_update(MonotonicTimePoint /*now*/, LogId id, KVLogList kv_list) override
        {
            std::string buf;
            log_format_set_info(buf, id, kv_list);
            output += buf;
        }

        void possible_active_window_change() override {}

        std::string & output;
    };

    auto first_file = record_wd.add_file("capture_kbd_input-000000.wrm");

    int fd = ::open(first_file.c_str(), O_RDONLY);
    RED_REQUIRE_NE(fd, -1);
    InFileTransport in_wrm_trans(unique_fd{fd});

    FileToGraphic player(in_wrm_trans, {}, {}, false, FileToGraphic::Verbose(0));

    std::string output;
    KbdInput kbd_input(output);
    CaptureProbe capture_probe(output);
    player.add_consumer(nullptr, nullptr, &kbd_input, &capture_probe, nullptr, nullptr, nullptr);

    while(player.next_order())
    {
        player.interpret_order();
    }

    RED_CHECK(output == "ipconfig\rtype=\"FOREGROUND_WINDOW_CHANGED\" windows=\"WINDOW\" class=\"CLASS\" command_line=\"COMMAND_LINE\""_av);

    auto mwrm = record_wd.add_file("capture_kbd_input.mwrm");
    auto st = get_stat(first_file);
    auto mst = get_stat(mwrm);
    RED_TEST_FILE_SIZE(first_file, 303);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n"
        "4 1\n"
        "nochecksum\n"
        "\n"
        "\n",
        first_file, " 303 ",
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), " 1000 1002\n")});

    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture_kbd_input-000000.wrm"), array_view{str_concat(
        "v2\n"
        "\n"
        "\n"
        "capture_kbd_input-000000.wrm 303 ",
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), '\n')});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture_kbd_input.mwrm"), array_view{str_concat(
        "v2\n\n\ncapture_kbd_input.mwrm ",
        int_to_chars(mst.st_size), ' ',
        int_to_chars(mst.st_mode), ' ',
        int_to_chars(mst.st_uid), ' ',
        int_to_chars(mst.st_gid), ' ',
        int_to_chars(mst.st_dev), ' ',
        int_to_chars(mst.st_ino), ' ',
        int_to_chars(mst.st_mtim.tv_sec), ' ',
        int_to_chars(mst.st_ctim.tv_sec), '\n')});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(tmp_wd);
}

RED_AUTO_TEST_CASE(TestWrmCaptureRemoteApp)
{
    WorkingDirectory record_wd("record");
    WorkingDirectory hash_wd("hash");
    WorkingDirectory tmp_wd("tmp");

    {
        WrmForTest wrm_test(TraceType::localfile, record_wd, hash_wd, tmp_wd,
            "capture_remoteapp", 0, 0, true);
        auto& wrm = wrm_test.wrm;
        auto now = wrm_test.now;

        const auto scr = wrm_test.scr;
        auto const color_cxt = gdi::ColorCtx::depth24();

        wrm.send_timestamp_chunk(now);

        wrm.draw(RDPOpaqueRect(scr, encode_color24()(BLACK)), scr, color_cxt);

        Rect rect = Rect(50, 50, 320, 200);
        wrm.draw(RDPOpaqueRect(rect, encode_color24()(YELLOW)), rect, color_cxt);
        wrm.visibility_rects_event(rect);


        now += 1s;
        wrm.send_timestamp_chunk(now);

        wrm.draw(RDPOpaqueRect(scr, encode_color24()(BLACK)), scr, color_cxt);

        rect = Rect(125, 75, 370, 250);
        wrm.draw(RDPOpaqueRect(rect, encode_color24()(BLUE)), rect, color_cxt);
        wrm.visibility_rects_event(rect);

        wrm.send_timestamp_chunk(now);
    }

    auto first_file = record_wd.add_file("capture_remoteapp-000000.wrm");

    int fd = ::open(first_file.c_str(), O_RDONLY);
    RED_REQUIRE_NE(fd, -1);
    InFileTransport in_wrm_trans(unique_fd{fd});

    FileToGraphic player(in_wrm_trans, {}, {}, false, FileToGraphic::Verbose(0));

    while(player.next_order())
    {
        player.interpret_order();
    }

    RED_CHECK(player.get_wrm_info().remote_app);
    RED_CHECK_EQUAL(player.max_image_frame_rect, Rect(50, 50, 320, 200).disjunct(Rect(125, 75, 370, 250)));
    RED_CHECK_EQUAL(player.min_image_frame_dim, Dimension(370, 250));

    auto mwrm = record_wd.add_file("capture_remoteapp.mwrm");
    auto st = get_stat(first_file);
    auto mst = get_stat(mwrm);
    RED_TEST_FILE_SIZE(first_file, 1670);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n"
        "800 600\n"
        "nochecksum\n"
        "\n"
        "\n",
        first_file, " 1670 ",
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), " 1000 1002\n")});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture_remoteapp-000000.wrm"), array_view{str_concat(
        "v2\n"
        "\n"
        "\n"
        "capture_remoteapp-000000.wrm 1670 ",
        int_to_chars(st.st_mode), ' ',
        int_to_chars(st.st_uid), ' ',
        int_to_chars(st.st_gid), ' ',
        int_to_chars(st.st_dev), ' ',
        int_to_chars(st.st_ino), ' ',
        int_to_chars(st.st_mtim.tv_sec), ' ',
        int_to_chars(st.st_ctim.tv_sec), '\n')});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("capture_remoteapp.mwrm"), array_view{str_concat(
        "v2\n\n\ncapture_remoteapp.mwrm ",
        int_to_chars(mst.st_size), ' ',
        int_to_chars(mst.st_mode), ' ',
        int_to_chars(mst.st_uid), ' ',
        int_to_chars(mst.st_gid), ' ',
        int_to_chars(mst.st_dev), ' ',
        int_to_chars(mst.st_ino), ' ',
        int_to_chars(mst.st_mtim.tv_sec), ' ',
        int_to_chars(mst.st_ctim.tv_sec), '\n')});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(tmp_wd);
}
