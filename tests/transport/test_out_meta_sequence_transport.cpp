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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "transport/out_meta_sequence_transport.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "test_only/lcg_random.hpp"


using namespace std::chrono_literals;

static void gen_out_meta_seq(
    TraceType trace_type, WorkingDirectory& record_wd, WorkingDirectory& hash_wd)
{
    CryptoContext cctx;
    cctx.set_master_key(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ""_av
    );
    cctx.set_hmac_key("12345678901234567890123456789012"_av);
    LCGRandom rnd;

    RealTimePoint tp(1352304810s);
    const int groupid = 0;

    cctx.set_trace_type(trace_type);

    OutMetaSequenceTransport wrm_trans(
        cctx, rnd, record_wd.dirname(), hash_wd.dirname(), "xxx",
        tp, 800, 600, groupid, nullptr, FilePermissions(0777));
    wrm_trans.send("AAAAX", 5);
    wrm_trans.send("BBBBX", 5);
    wrm_trans.next();
    wrm_trans.send("CCCCX", 5);
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

static std::string file_contents_prefix(WorkingFileBase filename, std::size_t n){
    return prefix(RED_CHECK_GET_FILE_CONTENTS(filename), n);
}

RED_AUTO_TEST_CASE(TestOutmetaTransportLocal)
{
    WorkingDirectory record_wd{"record"};
    WorkingDirectory hash_wd{"hash"};

    gen_out_meta_seq(TraceType::localfile, record_wd, hash_wd);

    auto mwrm = record_wd.add_file("xxx.mwrm");
    auto file1 = record_wd.add_file("xxx-000000.wrm");
    auto file2 = record_wd.add_file("xxx-000001.wrm");
    auto st1 = get_stat(file1);
    auto st2 = get_stat(file2);
    auto mwrmst = get_stat(mwrm);

    RED_TEST_FILE_CONTENTS(file1, "AAAAXBBBBX"_av);
    RED_TEST_FILE_CONTENTS(file2, "CCCCX"_av);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n"
        "800 600\n"
        "nochecksum\n"
        "\n"
        "\n",
        file1, " 10 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec), " 1352304810 1352304811\n",
        file2, " 5 ",
        int_to_decimal_chars(st2.st_mode), ' ',
        int_to_decimal_chars(st2.st_uid), ' ',
        int_to_decimal_chars(st2.st_gid), ' ',
        int_to_decimal_chars(st2.st_dev), ' ',
        int_to_decimal_chars(st2.st_ino), ' ',
        int_to_decimal_chars(st2.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st2.st_ctim.tv_sec), " 1352304811 1352304811\n")});

    auto hash1 = hash_wd.add_file("xxx-000000.wrm");
    auto hash2 = hash_wd.add_file("xxx-000001.wrm");

    RED_TEST_FILE_CONTENTS(hash1, str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx-000000.wrm 10 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec), '\n'));
    RED_TEST_FILE_CONTENTS(hash2, str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx-000001.wrm 5 ",
        int_to_decimal_chars(st2.st_mode), ' ',
        int_to_decimal_chars(st2.st_uid), ' ',
        int_to_decimal_chars(st2.st_gid), ' ',
        int_to_decimal_chars(st2.st_dev), ' ',
        int_to_decimal_chars(st2.st_ino), ' ',
        int_to_decimal_chars(st2.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st2.st_ctim.tv_sec), '\n'));


    auto mhash = hash_wd.add_file("xxx.mwrm");
    RED_TEST_FILE_CONTENTS(mhash, array_view{str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx.mwrm ",
        int_to_decimal_chars(mwrmst.st_size), ' ',
        int_to_decimal_chars(mwrmst.st_mode), ' ',
        int_to_decimal_chars(mwrmst.st_uid), ' ',
        int_to_decimal_chars(mwrmst.st_gid), ' ',
        int_to_decimal_chars(mwrmst.st_dev), ' ',
        int_to_decimal_chars(mwrmst.st_ino), ' ',
        int_to_decimal_chars(mwrmst.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(mwrmst.st_ctim.tv_sec), '\n')});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
}

RED_AUTO_TEST_CASE(TestOutmetaTransportHashed)
{
    WorkingDirectory record_wd{"record"};
    WorkingDirectory hash_wd{"hash"};

    gen_out_meta_seq(TraceType::localfile_hashed, record_wd, hash_wd);

    auto mwrm = record_wd.add_file("xxx.mwrm");
    auto file1 = record_wd.add_file("xxx-000000.wrm");
    auto file2 = record_wd.add_file("xxx-000001.wrm");
    auto st1 = get_stat(file1);
    auto st2 = get_stat(file2);
    auto mwrmst = get_stat(mwrm);

    RED_TEST_FILE_CONTENTS(file1, "AAAAXBBBBX"_av);
    RED_TEST_FILE_CONTENTS(file2, "CCCCX"_av);
    RED_TEST_FILE_CONTENTS(mwrm, array_view{str_concat(
        "v2\n"
        "800 600\n"
        "checksum\n"
        "\n"
        "\n",
        file1, " 10 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec), " 1352304810 1352304811"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7\n",
        file2, " 5 ",
        int_to_decimal_chars(st2.st_mode), ' ',
        int_to_decimal_chars(st2.st_uid), ' ',
        int_to_decimal_chars(st2.st_gid), ' ',
        int_to_decimal_chars(st2.st_dev), ' ',
        int_to_decimal_chars(st2.st_ino), ' ',
        int_to_decimal_chars(st2.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st2.st_ctim.tv_sec), " 1352304811 1352304811"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36\n")});

    auto hash1 = hash_wd.add_file("xxx-000000.wrm");
    auto hash2 = hash_wd.add_file("xxx-000001.wrm");

    RED_TEST_FILE_CONTENTS(hash1, str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx-000000.wrm 10 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec),
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7\n"));
    RED_TEST_FILE_CONTENTS(hash2, str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx-000001.wrm 5 ",
        int_to_decimal_chars(st2.st_mode), ' ',
        int_to_decimal_chars(st2.st_uid), ' ',
        int_to_decimal_chars(st2.st_gid), ' ',
        int_to_decimal_chars(st2.st_dev), ' ',
        int_to_decimal_chars(st2.st_ino), ' ',
        int_to_decimal_chars(st2.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st2.st_ctim.tv_sec),
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36\n"));

    auto expected_prefix = str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx.mwrm ",
        int_to_decimal_chars(mwrmst.st_size), ' ',
        int_to_decimal_chars(mwrmst.st_mode), ' ',
        int_to_decimal_chars(mwrmst.st_uid), ' ',
        int_to_decimal_chars(mwrmst.st_gid), ' ',
        int_to_decimal_chars(mwrmst.st_dev), ' ',
        int_to_decimal_chars(mwrmst.st_ino), ' ',
        int_to_decimal_chars(mwrmst.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(mwrmst.st_ctim.tv_sec), ' ');
    auto hash_mwrm = RED_CHECK_GET_FILE_CONTENTS(hash_wd.add_file("xxx.mwrm"));

    RED_TEST(hash_mwrm.size() == expected_prefix.size() + 65*2);
    RED_TEST(prefix(hash_mwrm, expected_prefix.size()) == expected_prefix);

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
}

RED_AUTO_TEST_CASE(TestOutmetaTransportCrypted)
{
    WorkingDirectory record_wd{"record"};
    WorkingDirectory hash_wd{"hash"};

    gen_out_meta_seq(TraceType::cryptofile, record_wd, hash_wd);

    auto file1 = record_wd.add_file("xxx-000000.wrm");
    auto file2 = record_wd.add_file("xxx-000001.wrm");

    RED_TEST_FILE_CONTENTS(file1,
        "WCFM\x01\x00\x00\x00\x78\x58\xae\x52\xb0\xae\x6c\x17\x68\x5a\x13\xbd"
        "\xa0\x53\x7a\xf3\x58\x12\x7b\xce\x90\x8e\xa8\xad\x48\x40\x87\x31\x80"
        "\x1f\x45\xb6\x10\x00\x00\x00\x85\x9d\x59\xc0\x6c\x90\xfb\x04\xcd\x1f"
        "\x45\x75\x9f\x88\xad$MFCW\n\x00\x00\x00"_av_hex);
    RED_TEST_FILE_CONTENTS(file2,
        "WCFM\x01\x00\x00\x00\xf8\x4f\x14.0>\xfc\xa9\xe8\x01\x80\x41\x20\x93"
        "\xcf\x2a\xd8\x69\x9c\x4f\x10\x7e\xd1\x4c\xc8\x47\xcb\xff\x00\xbf\x0f"
        "\x19\x10\x00\x00\x00\x1c\xbaT/u\x17\x17\xea\xf9\xa2\xf9\x9a\x13\x72"
        "\xda\xb6MFCW\x05\x00\x00\x00"_av_hex);

    RED_TEST(file_contents_prefix(record_wd.add_file("xxx.mwrm"), 4) == "WCFM"_av);
    RED_TEST(file_contents_prefix(hash_wd.add_file("xxx-000000.wrm"), 4) == "WCFM"_av);
    RED_TEST(file_contents_prefix(hash_wd.add_file("xxx-000001.wrm"), 4) == "WCFM"_av);
    RED_TEST(file_contents_prefix(hash_wd.add_file("xxx.mwrm"), 4) == "WCFM"_av);

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
}
