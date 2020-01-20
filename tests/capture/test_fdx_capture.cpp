/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "capture/fdx_capture.hpp"
#include "utils/sugar/algostring.hpp"
#include "test_only/fake_stat.hpp"
#include "test_only/lcg_random.hpp"
#include "transport/mwrm_reader.hpp"


RED_AUTO_TEST_CASE(tfl_suffix_genarator)
{
    TflSuffixGenerator gen;

    RED_TEST(gen.next() == ",000001.tfl");
    RED_TEST(gen.next() == ",000002.tfl");
    RED_TEST(gen.name_at(99999) == ",099999.tfl");
    RED_TEST(gen.name_at(999999) == ",999999.tfl");
    RED_TEST(gen.name_at(1000000) == ",1000000.tfl");
}

RED_AUTO_TEST_CASE(fdx_name_generator)
{
    using namespace std::string_view_literals;

#define MY_RECORD_PATH "/my/record/path"
#define MY_HASH_PATH "/my/hash/path/"
#define MY_SID "SESSION_ID"

    FdxNameGenerator gen(MY_RECORD_PATH, MY_HASH_PATH, MY_SID);

    RED_TEST(gen.get_current_filename() == MY_SID ".fdx");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID ".fdx");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID ".fdx");

    gen.next_tfl();

    RED_TEST(gen.get_current_id() == Mwrm3::FileId(1));
    RED_TEST(gen.get_current_filename() == MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID ",000001.tfl");

    gen.next_tfl();

    RED_TEST(gen.get_current_id() == Mwrm3::FileId(2));
    RED_TEST(gen.get_current_filename() == MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID ",000002.tfl");

#undef MY_RECORD_PATH
#undef MY_HASH_PATH
#undef MY_SID
}

RED_AUTO_TEST_CASE_WD(fdx_capture_not_fdx, wd)
{
    (void)wd;

    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");
    auto sid = "my_session_id"sv;

    CryptoContext cctx;
    LCGRandom rnd;
    Fstat fstat;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        sid, -1, cctx, rnd, fstat,
        ReportError());

    auto fdx_record_path = record_path.create_subdirectory(sid);
    auto fdx_hash_path = hash_path.create_subdirectory(sid);

    (void)fdx_record_path.add_file(str_concat(sid, ".fdx"));
    (void)fdx_hash_path.add_file(str_concat(sid, ".fdx"));
}

RED_AUTO_TEST_CASE_WD(fdx_capture, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");

    auto sid = "my_session_id"sv;

    CryptoContext cctx;
    LCGRandom rnd;
    struct : Fstat
    {
        int stat(const char * filename, struct stat & stat) override
        {
            int err = Fstat::stat(filename, stat);
            stat.st_mode = 1;
            stat.st_uid = 2;
            stat.st_gid = 3;
            stat.st_dev = 4;
            stat.st_ino = 5;
            stat.st_mtim.tv_sec = 12345678;
            stat.st_ctim.tv_sec = 12345678;
            return err;
        }
    } fstat;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        sid, -1, cctx, rnd, fstat,
        ReportError());

    auto sig1 = Mwrm3::Sha256Signature{"abcdefghijabcdefghijabcdefghijab"_av};
    auto sig2 = Mwrm3::Sha256Signature{"ABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"_av};
    auto sig3 = Mwrm3::Sha256Signature{"01234567890123456789012345678901"_av};

    {
        FdxCapture::TflFile tfl1 = fdx_capture.new_tfl(Mwrm3::Direction::ClientToServer);
        FdxCapture::TflFile tfl2 = fdx_capture.new_tfl(Mwrm3::Direction::ClientToServer);
        FdxCapture::TflFile tfl3 = fdx_capture.new_tfl(Mwrm3::Direction::ServerToClient);
        FdxCapture::TflFile tfl4 = fdx_capture.new_tfl(Mwrm3::Direction::ClientToServer);
        FdxCapture::TflFile tfl5 = fdx_capture.new_tfl(Mwrm3::Direction::ClientToServer);
        RED_TEST(tfl1.file_id == Mwrm3::FileId(1));
        RED_TEST(tfl2.file_id == Mwrm3::FileId(2));
        RED_TEST(tfl3.file_id == Mwrm3::FileId(3));
        RED_TEST(tfl4.file_id == Mwrm3::FileId(4));
        RED_TEST(tfl5.file_id == Mwrm3::FileId(5));
        tfl1.trans.send("ijkl"sv);
        tfl2.trans.send("mnop"sv);
        tfl3.trans.send("qr"sv);
        tfl4.trans.send("stu"sv);
        tfl5.trans.send("vwxyz"sv);
        fdx_capture.close_tfl(tfl4, "file4", Mwrm3::TransferedStatus::Completed, sig1);
        fdx_capture.cancel_tfl(tfl2);
        fdx_capture.close_tfl(tfl5, "file5", Mwrm3::TransferedStatus::Broken, sig2);
        fdx_capture.close_tfl(tfl1, "file1", Mwrm3::TransferedStatus::Unknown, sig3);
        fdx_capture.close_tfl(tfl3, "file1" /*same name than tfl1*/,
            Mwrm3::TransferedStatus::Completed, Mwrm3::Sha256Signature{""_av});
    }

    {
        FdxCapture::TflFile tfl = fdx_capture.new_tfl(Mwrm3::Direction::ServerToClient);
        RED_TEST(tfl.file_id == Mwrm3::FileId(6));
        tfl.trans.send("abcde"sv);
        fdx_capture.close_tfl(tfl, "file6", Mwrm3::TransferedStatus::Broken, sig2);
    }

    {
        FdxCapture::TflFile tfl = fdx_capture.new_tfl(Mwrm3::Direction::ServerToClient);
        RED_TEST(tfl.file_id == Mwrm3::FileId(7));
        tfl.trans.send("fgh"sv);
        fdx_capture.cancel_tfl(tfl);
    }

    OutCryptoTransport::HashArray qhash;
    OutCryptoTransport::HashArray fhash;
    fdx_capture.close(qhash, fhash);

    auto fdx_record_path = record_path.create_subdirectory(sid);
    auto fdx_hash_path = hash_path.create_subdirectory(sid);

    auto fdx_filepath = fdx_record_path.add_file(str_concat(sid, ".fdx"));
    std::string file_content = RED_REQUIRE_GET_FILE_CONTENTS(fdx_filepath);

    RED_TEST(bytes_view(file_content) ==
        "v3\n\x04\x00\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00,"
        "\x05\x00\x18\x00""file4my_session_id,000004.tflabcdefghijabcdefghijabcdefghijab"
        "\x04\x00\x05\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00L"
        "\x05\x00\x18\x00""file5my_session_id,000005.tflABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"
        "\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x0c"
        "\x05\x00\x18\x00""file1my_session_id,000001.tfl01234567890123456789012345678901"
        "\x04\x00\x03\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00""0\x05"
        "\x00\x18\x00""file1my_session_id,000003.tfl\x04\x00\x06\x00\x00\x00\x00\x00\x00"
        "\x00\x05\x00\x00\x00\x00\x00\x00\x00T\x05\x00\x18\x00""file6my_session_id,"
        "000006.tflABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"_av);

    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ".fdx")), str_concat(
        "v2\n\n\nmy_session_id.fdx "sv,
        std::to_string(file_content.size()),
        " 1 2 3 4 5 12345678 12345678\n"sv));

    RED_CHECK_FILE_CONTENTS(fdx_record_path.add_file(str_concat(sid, ",000001.tfl")), "ijkl"sv);
    RED_CHECK_FILE_CONTENTS(fdx_record_path.add_file(str_concat(sid, ",000003.tfl")), "qr"sv);
    RED_CHECK_FILE_CONTENTS(fdx_record_path.add_file(str_concat(sid, ",000004.tfl")), "stu"sv);
    RED_CHECK_FILE_CONTENTS(fdx_record_path.add_file(str_concat(sid, ",000005.tfl")), "vwxyz"sv);
    RED_CHECK_FILE_CONTENTS(fdx_record_path.add_file(str_concat(sid, ",000006.tfl")), "abcde"sv);

    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000001.tfl")),
        "v2\n\n\nmy_session_id,000001.tfl 4 1 2 3 4 5 12345678 12345678\n"sv);
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000003.tfl")),
        "v2\n\n\nmy_session_id,000003.tfl 2 1 2 3 4 5 12345678 12345678\n"sv);
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000004.tfl")),
        "v2\n\n\nmy_session_id,000004.tfl 3 1 2 3 4 5 12345678 12345678\n"sv);
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000005.tfl")),
        "v2\n\n\nmy_session_id,000005.tfl 5 1 2 3 4 5 12345678 12345678\n"sv);
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000006.tfl")),
        "v2\n\n\nmy_session_id,000006.tfl 5 1 2 3 4 5 12345678 12345678\n"sv);
}

RED_AUTO_TEST_CASE_WD(fdx_capture_encrypted, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");

    auto sid = "my_session_id"sv;

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
    cctx.set_trace_type(TraceType::cryptofile);

    LCGRandom rnd;
    FakeFstat fstat;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        sid, -1, cctx, rnd, fstat,
        ReportError());

    auto sig1 = Mwrm3::Sha256Signature{"abcdefghijabcdefghijabcdefghijab"_av};

    {
        FdxCapture::TflFile tfl1 = fdx_capture.new_tfl(Mwrm3::Direction::ClientToServer);
        tfl1.trans.send("ijkl"sv);
        fdx_capture.close_tfl(tfl1, "file1", Mwrm3::TransferedStatus::Completed, sig1);
    }

    OutCryptoTransport::HashArray qhash {};
    OutCryptoTransport::HashArray fhash {};
    fdx_capture.close(qhash, fhash);

    OutCryptoTransport::HashArray zero_hash {};
    RED_TEST(make_array_view(qhash) != make_array_view(zero_hash));
    RED_TEST(make_array_view(fhash) != make_array_view(zero_hash));

    auto fdx_record_path = record_path.create_subdirectory(sid);
    auto fdx_hash_path = hash_path.create_subdirectory(sid);

    (void)fdx_hash_path.add_file(str_concat(sid, ",000001.tfl"));
    (void)fdx_hash_path.add_file(str_concat(sid, ".fdx"));

    std::string file_content = RED_REQUIRE_GET_FILE_CONTENTS(
        fdx_record_path.add_file(str_concat(sid, ".fdx")));

    RED_REQUIRE(file_content.size() > 4);
    RED_REQUIRE(array_view(file_content).first(4) == "WCFM"sv);

    file_content = RED_REQUIRE_GET_FILE_CONTENTS(
        fdx_record_path.add_file(str_concat(sid, ",000001.tfl")));

    RED_REQUIRE(file_content.size() > 4);
    RED_REQUIRE(array_view(file_content).first(4) == "WCFM"sv);
}
