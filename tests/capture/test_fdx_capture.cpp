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
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "test_only/lcg_random.hpp"
#include "transport/mwrm_reader.hpp"


static struct stat get_stat(char const* filename)
{
    class stat st;
    stat(filename, &st);
    return st;
}


RED_AUTO_TEST_CASE(tfl_suffix_genarator)
{
    TflSuffixGenerator gen;

    // 6 digits
    RED_TEST(gen.next() == ",000001.tfl");
    RED_TEST(gen.next() == ",000002.tfl");
    RED_TEST(gen.name_at(99999) == ",099999.tfl");
    RED_TEST(gen.name_at(999999) == ",999999.tfl");
    // 7 digits
    RED_TEST(gen.name_at(1000000) == ",1000000.tfl");
}

RED_AUTO_TEST_CASE(fdx_name_generator)
{
    using namespace std::string_view_literals;

#define MY_RECORD_PATH "/my/record/path"
#define MY_HASH_PATH "/my/hash/path/"
#define MY_SID "SESSION_ID"

    FdxNameGenerator gen(MY_RECORD_PATH, MY_HASH_PATH, MY_SID);

    RED_TEST(gen.get_current_basename() == MY_SID);
    RED_TEST(gen.get_current_relative_path() == MY_SID "/" MY_SID);
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID);
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID);

    gen.next_tfl();

    RED_TEST(gen.get_current_id() == Mwrm3::FileId(1));
    RED_TEST(gen.get_current_basename() == MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_relative_path() == MY_SID "/" MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID ",000001.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID ",000001.tfl");

    gen.next_tfl();

    RED_TEST(gen.get_current_id() == Mwrm3::FileId(2));
    RED_TEST(gen.get_current_basename() == MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_relative_path() == MY_SID "/" MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_record_path() == MY_RECORD_PATH "/" MY_SID "/" MY_SID ",000002.tfl");
    RED_TEST(gen.get_current_hash_path() == MY_HASH_PATH "/" MY_SID "/" MY_SID ",000002.tfl");

#undef MY_RECORD_PATH
#undef MY_HASH_PATH
#undef MY_SID
}

RED_AUTO_TEST_CASE_WD(fdx_capture_empty_fdx, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");
    auto sid = "my_session_id"sv;
    auto fdx_basename = "sid,blabla.fdx"sv;

    CryptoContext cctx;
    LCGRandom rnd;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        "sid,blabla", sid, -1, FilePermissions(0660), cctx, rnd,
        [](const Error & /*error*/){});

    (void)record_path.create_subdirectory(sid);
    (void)hash_path.create_subdirectory(sid);

    OutCryptoTransport::HashArray qhash;
    OutCryptoTransport::HashArray fhash;
    fdx_capture.close(qhash, fhash);

    RED_CHECK_FILE_CONTENTS(record_path.add_file(fdx_basename), "v3\n"sv);
    (void)hash_path.add_file(fdx_basename);
}

RED_AUTO_TEST_CASE_WD(fdx_capture, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");

    auto sid = "my_session_id"sv;
    auto fdx_basename = "sid,blabla.fdx"sv;

    CryptoContext cctx;
    LCGRandom rnd;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        "sid,blabla", sid, -1, FilePermissions(0660), cctx, rnd,
        [](const Error & /*error*/){});

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

    auto fdx_filepath = record_path.add_file(fdx_basename);
    auto st = get_stat(fdx_filepath);
    std::string file_content = RED_REQUIRE_GET_FILE_CONTENTS(fdx_filepath);

    RED_TEST(bytes_view(file_content) ==
        "v3\n\x04\x00\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00,"
        "\x05\x00\x26\x00"
        "file4my_session_id/my_session_id,000004.tflabcdefghijabcdefghijabcdefghijab"
        "\x04\x00\x05\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00L"
        "\x05\x00\x26\x00"
        "file5my_session_id/my_session_id,000005.tflABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"
        "\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x0c"
        "\x05\x00\x26\x00"
        "file1my_session_id/my_session_id,000001.tfl01234567890123456789012345678901"
        "\x04\x00\x03\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x30\x05"
        "\x00\x26\x00"
        "file1my_session_id/my_session_id,000003.tfl\x04\x00\x06\x00\x00\x00\x00\x00\x00"
        "\x00\x05\x00\x00\x00\x00\x00\x00\x00\x54\x05\x00\x26\x00"
        "file6my_session_id/my_session_id,000006.tflABCDEFGHIJABCDEFGHIJABCDEFGHIJAB"_av
    );

    RED_CHECK_FILE_CONTENTS(hash_path.add_file(fdx_basename), str_concat(
        "v2\n\n\nsid,blabla.fdx "sv,
        int_to_decimal_chars(file_content.size()), ' ',
        int_to_decimal_chars(st.st_mode), ' ',
        int_to_decimal_chars(st.st_uid), ' ',
        int_to_decimal_chars(st.st_gid), ' ',
        int_to_decimal_chars(st.st_dev), ' ',
        int_to_decimal_chars(st.st_ino), ' ',
        int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st.st_ctim.tv_sec), '\n'));

    auto tfl1 = fdx_record_path.add_file(str_concat(sid, ",000001.tfl"));
    auto tfl3 = fdx_record_path.add_file(str_concat(sid, ",000003.tfl"));
    auto tfl4 = fdx_record_path.add_file(str_concat(sid, ",000004.tfl"));
    auto tfl5 = fdx_record_path.add_file(str_concat(sid, ",000005.tfl"));
    auto tfl6 = fdx_record_path.add_file(str_concat(sid, ",000006.tfl"));
    auto st1 = get_stat(tfl1);
    auto st3 = get_stat(tfl3);
    auto st4 = get_stat(tfl4);
    auto st5 = get_stat(tfl5);
    auto st6 = get_stat(tfl6);
    RED_CHECK_FILE_CONTENTS(tfl1, "ijkl"sv);
    RED_CHECK_FILE_CONTENTS(tfl3, "qr"sv);
    RED_CHECK_FILE_CONTENTS(tfl4, "stu"sv);
    RED_CHECK_FILE_CONTENTS(tfl5, "vwxyz"sv);
    RED_CHECK_FILE_CONTENTS(tfl6, "abcde"sv);

    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000001.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000001.tfl 4 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec), '\n'));
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000003.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000003.tfl 2 ",
        int_to_decimal_chars(st3.st_mode), ' ',
        int_to_decimal_chars(st3.st_uid), ' ',
        int_to_decimal_chars(st3.st_gid), ' ',
        int_to_decimal_chars(st3.st_dev), ' ',
        int_to_decimal_chars(st3.st_ino), ' ',
        int_to_decimal_chars(st3.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st3.st_ctim.tv_sec), '\n'));
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000004.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000004.tfl 3 ",
        int_to_decimal_chars(st4.st_mode), ' ',
        int_to_decimal_chars(st4.st_uid), ' ',
        int_to_decimal_chars(st4.st_gid), ' ',
        int_to_decimal_chars(st4.st_dev), ' ',
        int_to_decimal_chars(st4.st_ino), ' ',
        int_to_decimal_chars(st4.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st4.st_ctim.tv_sec), '\n'));
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000005.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000005.tfl 5 ",
        int_to_decimal_chars(st5.st_mode), ' ',
        int_to_decimal_chars(st5.st_uid), ' ',
        int_to_decimal_chars(st5.st_gid), ' ',
        int_to_decimal_chars(st5.st_dev), ' ',
        int_to_decimal_chars(st5.st_ino), ' ',
        int_to_decimal_chars(st5.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st5.st_ctim.tv_sec), '\n'));
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000006.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000006.tfl 5 ",
        int_to_decimal_chars(st6.st_mode), ' ',
        int_to_decimal_chars(st6.st_uid), ' ',
        int_to_decimal_chars(st6.st_gid), ' ',
        int_to_decimal_chars(st6.st_dev), ' ',
        int_to_decimal_chars(st6.st_ino), ' ',
        int_to_decimal_chars(st6.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st6.st_ctim.tv_sec), '\n'));
}

RED_AUTO_TEST_CASE_WD(fdx_capture_big_name, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");

    auto sid = "my_session_id"sv;
    auto fdx_basename = "sid,blabla.fdx"sv;

    CryptoContext cctx;
    LCGRandom rnd;

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        "sid,blabla", sid, -1, FilePermissions(0660), cctx, rnd,
        [](const Error & /*error*/){});

    auto sig1 = Mwrm3::Sha256Signature{"abcdefghijabcdefghijabcdefghijab"_av};

    // very long file name
    // 64 bytes
    std::string s = "0123456789ABCDEF""0123456789ABCDEF""0123456789ABCDEF""0123456789ABCDEF";
    // 1KiB
    s = str_concat(s, s, s, s, s, s, s, s, s, s, s, s, s, s, s, s);
    // 16KiB + 1KiB
    s = str_concat(s, s, s, s, s, s, s, s, s, s, s, s, s, s, s, s, s);
    RED_TEST(s.size() == 1024*17);

    {
        FdxCapture::TflFile tfl = fdx_capture.new_tfl(Mwrm3::Direction::ServerToClient);
        RED_TEST(tfl.file_id == Mwrm3::FileId(1));
        tfl.trans.send("ijk"sv);
        fdx_capture.close_tfl(tfl, s, Mwrm3::TransferedStatus::Completed, sig1);
    }

    OutCryptoTransport::HashArray qhash;
    OutCryptoTransport::HashArray fhash;
    fdx_capture.close(qhash, fhash);

    auto fdx_record_path = record_path.create_subdirectory(sid);
    auto fdx_hash_path = hash_path.create_subdirectory(sid);

    auto fdx_filepath = record_path.add_file(fdx_basename);
    auto st = get_stat(fdx_filepath);
    std::string file_content = RED_REQUIRE_GET_FILE_CONTENTS(fdx_filepath);

    RED_TEST(bytes_view(file_content) == str_concat(
        "v3\n\x04\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00"
        "\x00\x00\x00\x00\x00\x34\x00\x40\x26\x00"_av,
        chars_view(s).first(1024*16), "my_session_id/my_session_id,000001.tflabcdefghijabcdefghijabcdefghijab"
        ""_av
    ));

    RED_CHECK_FILE_CONTENTS(hash_path.add_file(fdx_basename), str_concat(
        "v2\n\n\nsid,blabla.fdx "sv,
        int_to_decimal_chars(file_content.size()), ' ',
        int_to_decimal_chars(st.st_mode), ' ',
        int_to_decimal_chars(st.st_uid), ' ',
        int_to_decimal_chars(st.st_gid), ' ',
        int_to_decimal_chars(st.st_dev), ' ',
        int_to_decimal_chars(st.st_ino), ' ',
        int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st.st_ctim.tv_sec), '\n'));

    auto tfl1 = fdx_record_path.add_file(str_concat(sid, ",000001.tfl"));
    auto st1 = get_stat(tfl1);
    RED_CHECK_FILE_CONTENTS(tfl1, "ijk"sv);
    RED_CHECK_FILE_CONTENTS(fdx_hash_path.add_file(str_concat(sid, ",000001.tfl")), str_concat(
        "v2\n\n\nmy_session_id,000001.tfl 3 ",
        int_to_decimal_chars(st1.st_mode), ' ',
        int_to_decimal_chars(st1.st_uid), ' ',
        int_to_decimal_chars(st1.st_gid), ' ',
        int_to_decimal_chars(st1.st_dev), ' ',
        int_to_decimal_chars(st1.st_ino), ' ',
        int_to_decimal_chars(st1.st_mtim.tv_sec), ' ',
        int_to_decimal_chars(st1.st_ctim.tv_sec), '\n'));
}

RED_AUTO_TEST_CASE_WD(fdx_capture_encrypted, wd)
{
    using namespace std::string_view_literals;

    auto record_path = wd.create_subdirectory("record");
    auto hash_path = wd.create_subdirectory("hash");

    auto sid = "my_session_id"sv;
    auto fdx_basename = "sid,blabla.fdx"sv;

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

    FdxCapture fdx_capture(
        record_path.dirname().string(),
        hash_path.dirname().string(),
        "sid,blabla", sid, -1, FilePermissions(0660), cctx, rnd,
        [](const Error & /*error*/){});

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
    (void)hash_path.add_file(fdx_basename);

    std::string file_content = RED_REQUIRE_GET_FILE_CONTENTS(
        record_path.add_file(fdx_basename));

    RED_REQUIRE(file_content.size() > 4);
    RED_REQUIRE(array_view(file_content).first(4) == "WCFM"sv);

    file_content = RED_REQUIRE_GET_FILE_CONTENTS(
        fdx_record_path.add_file(str_concat(sid, ",000001.tfl")));

    RED_REQUIRE(file_content.size() > 4);
    RED_REQUIRE(array_view(file_content).first(4) == "WCFM"sv);
}
