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

    timeval now;
    now.tv_sec = 1352304810;
    now.tv_usec = 0;
    const int groupid = 0;

    cctx.set_trace_type(trace_type);

    OutMetaSequenceTransport wrm_trans(
        cctx, rnd, record_wd.dirname(), hash_wd.dirname(), "xxx",
        now, 800, 600, groupid, nullptr, -1);
    wrm_trans.send("AAAAX", 5);
    wrm_trans.send("BBBBX", 5);
    wrm_trans.next();
    wrm_trans.send("CCCCX", 5);
}

static std::string prefix(std::string data, std::size_t n){
    if (data.size() > n) {
        data.resize(n);
    }
    return data;
};

static std::string file_contents_prefix(WorkingFileBase filename, std::size_t n){
    return prefix(RED_CHECK_GET_FILE_CONTENTS(filename), n);
};

RED_AUTO_TEST_CASE(TestOutmetaTransportLocal)
{
    WorkingDirectory record_wd{"record"};
    WorkingDirectory hash_wd{"hash"};

    gen_out_meta_seq(TraceType::localfile, record_wd, hash_wd);

    auto file1 = record_wd.add_file("xxx-000000.wrm");
    auto file2 = record_wd.add_file("xxx-000001.wrm");

    RED_TEST_FILE_CONTENTS(file1, "AAAAXBBBBX"_av);
    RED_TEST_FILE_CONTENTS(file2, "CCCCX"_av);
    RED_TEST_FILE_CONTENTS(record_wd.add_file("xxx.mwrm"), array_view{str_concat(
        "v2\n"
        "800 600\n"
        "nochecksum\n"
        "\n"
        "\n",
        file1, " 10 0 0 0 0 0 0 0 1352304810 1352304811\n",
        file2, " 5 0 0 0 0 0 0 0 1352304811 1352304811\n")});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000000.wrm"),
        "v2\n"
        "\n"
        "\n"
        "xxx-000000.wrm 10 0 0 0 0 0 0 0\n"_av);
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000001.wrm"),
        "v2\n"
        "\n"
        "\n"
        "xxx-000001.wrm 5 0 0 0 0 0 0 0\n"_av);
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx.mwrm"), array_view{str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx.mwrm ", int_to_chars(file1.size() + file2.size() + 103), " 0 0 0 0 0 0 0\n")});

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
}

RED_AUTO_TEST_CASE(TestOutmetaTransportHashed)
{
    WorkingDirectory record_wd{"record"};
    WorkingDirectory hash_wd{"hash"};

    gen_out_meta_seq(TraceType::localfile_hashed, record_wd, hash_wd);

    auto file1 = record_wd.add_file("xxx-000000.wrm");
    auto file2 = record_wd.add_file("xxx-000001.wrm");

    RED_TEST_FILE_CONTENTS(file1, "AAAAXBBBBX"_av);
    RED_TEST_FILE_CONTENTS(file2, "CCCCX"_av);
    RED_TEST_FILE_CONTENTS(record_wd.add_file("xxx.mwrm"), array_view{str_concat(
        "v2\n"
        "800 600\n"
        "checksum\n"
        "\n"
        "\n",
        file1, " 10 0 0 0 0 0 0 0 1352304810 1352304811"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7\n",
        file2, " 5 0 0 0 0 0 0 0 1352304811 1352304811"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36\n")});
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000000.wrm"),
        "v2\n"
        "\n"
        "\n"
        "xxx-000000.wrm 10 0 0 0 0 0 0 0"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7"
        " d873d36d05d92a7e7b0d0e1dca7d994f090f204185d38a6e2a1c1723a76326b7\n"_av);
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000001.wrm"),
        "v2\n"
        "\n"
        "\n"
        "xxx-000001.wrm 5 0 0 0 0 0 0 0"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36"
        " 3e6965faf9da00b75a8a4031748f22ffe9d992751bf189ea603d6acb8d172c36\n"_av);

    auto expected_prefix = str_concat(
        "v2\n"
        "\n"
        "\n"
        "xxx.mwrm ", int_to_chars(file1.size() + file2.size() + 361), " 0 0 0 0 0 0 0");
    auto hash_mwrm = RED_CHECK_GET_FILE_CONTENTS(hash_wd.add_file("xxx.mwrm"));

    RED_TEST(hash_mwrm.size() == expected_prefix.size() + 131);
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
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000000.wrm"),
        "\x57\x43\x46\x4d\x01\x00\x00\x00\x38\xa4\xf1\x4b\x70\xc6\x35\xa4\x28"
        "\xfe\x8b\xed\x60\x43\xf8\x13\x18\x0e\x40\x5f\x50\x56\xa2\xe6\x08\x94"
        "\x0f\xd2\x40\xbf\xe1\xe0\x70\x00\x00\x00\x49\x19\x3c\x53\x4e\x5a\x7b"
        "\x51\x16\xf2\x89\x94\xb6\x03\x75\x58\xc8\xd5\x37\x81\x44\x4a\xd4\x4d"
        "\xc4\x88\x7c\x79\xa1\x60\xa8\x85\xb5\x07\xfb\x41\x51\x56\x86\x99\x10"
        "\x72\xe3\xc5\x79\xf8\xd1\x36\x4f\xfc\x3f\x79\xb9\x4a\xf9\xbe\x28\x67"
        "\x08\x30\x7d\x22\xa8\x3e\x31\xcf\x3e\x39\xc5\x57\xf4\x95\x04\x1d\xac"
        "\x78\xa1\x15\x72\xd9\xe2\x8a\x34\x7b\xd4\x71\xc8\xa3\x2f\xe4\x8d\xb1"
        "\x4d\x12\xe0\xdf\xbc\x3e\xc5\x2f\xeb\x07\x89\xd9\x1e\xa2\xd0\x42\x26"
        "\xa8\xc4\x1b\x4d\x46\x43\x57\xa7\x00\x00\x00"_av_hex);
    RED_TEST_FILE_CONTENTS(hash_wd.add_file("xxx-000001.wrm"),
        "\x57\x43\x46\x4d\x01\x00\x00\x00\xb8\x5b\x86\xac\xf0\x15\x30\x37\xa8"
        "\x65\x5f\x12\xe0\x42\x70\xcc\x98\x25\x00\x5f\xd0\x05\xa6\xba\x88\x5b"
        "\x2a\xa0\xc0\x1e\x3f\x3f\x70\x00\x00\x00\xd2\xb3\x84\x3e\xee\xf8\x75"
        "\xea\xb5\x7d\x72\xd1\xe7\xaa\x88\xf0\x76\xbd\x13\x9a\xf8\x6b\x92\x95"
        "\xe1\x15\x27\x34\xf2\xf6\xfc\xae\xf9\x05\x05\x21\x6c\xc2\xc7\x3a\x03"
        "\x60\x8b\x3d\xae\x0a\x23\x9d\x4c\xc9\x63\x01\xd7\x54\xa1\x64\xa2\xa5"
        "\x05\xf0\x11\xfb\xca\x79\xc0\x17\xf7\x2a\x50\x81\x11\x1d\x19\xa4\x54"
        "\xaf\xd2\x7c\xec\x5e\xfd\x27\xf8\x46\x19\x7e\xcb\x53\x79\x97\x29\x72"
        "\xf6\xe1\x3c\x8b\x5f\xe1\x90\xe0\x2e\x6d\x91\x4f\xde\x0b\x2d\x79\x46"
        "\x49\x5f\x56\x4d\x46\x43\x57\xa7\x00\x00\x00"_av_hex);
    RED_TEST(file_contents_prefix(hash_wd.add_file("xxx.mwrm"), 4) == "WCFM"_av);

    RED_CHECK_WORKSPACE(record_wd);
    RED_CHECK_WORKSPACE(hash_wd);
}
