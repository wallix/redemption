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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/data_test_case.hpp"
#include "test_only/test_framework/file.hpp"

#include "utils/sugar/algostring.hpp"
#include "utils/sugar/finally.hpp"
#include "lib/scytale.hpp"
#include "transport/crypto_transport.hpp"
#include "capture/mwrm3.hpp"

#include <string_view>
#include <fstream>


using namespace std::string_view_literals;


namespace
{
    // 86410558C495CC4E492157874774088A33B02AB865CC384120FEC2C9B872C82C
    uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
        0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
        0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
        0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
        0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c,
    };

    int trace_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 611FD4CDE595B7FDA65038FCD886514F597E8E9081F6F4489C7741510F530EE8
        uint8_t trace_key[SslSha256::DIGEST_LENGTH] = {
            0x61, 0x1f, 0xd4, 0xcd, 0xe5, 0x95, 0xb7, 0xfd,
            0xa6, 0x50, 0x38, 0xfc, 0xd8, 0x86, 0x51, 0x4f,
            0x59, 0x7e, 0x8e, 0x90, 0x81, 0xf6, 0xf4, 0x48,
            0x9c, 0x77, 0x41, 0x51, 0x0f, 0x53, 0x0e, 0xe8,
        };
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }

    uint8_t const * bytes(char const * p)
    {
        return byte_ptr_cast(p);
    }
}

RED_AUTO_TEST_CASE_WD(Testscytale, wd)
{
    char const* derivator = "encrypted.txt";
    auto finalname = wd.add_file(derivator);
    auto hash_finalname = wd.add_file("hash_encrypted.txt");

    // Writer
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new_with_test_random(with_encryption, with_checksum, finalname, hmac_key, &trace_fn, false, false);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        RED_CHECK_EQ(scytale_writer_get_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);
        RED_CHECK_EQ(scytale_writer_get_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);

        RED_CHECK_EQ(scytale_writer_get_error_message(handle), "No error"sv);

        scytale_writer_delete(handle);
    }

    // Reader
    {
        auto handle = scytale_reader_new(finalname, hmac_key, &trace_fn, 0, 0);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_reader_open(handle, finalname, derivator), 0);

        uint8_t buf[31];

        size_t total = 0;
        while (total < sizeof(buf)) {
            long long res = scytale_reader_read(handle, &buf[total], 10);
            RED_REQUIRE_GT(res, 0);
            total += size_t(res);
        }
        RED_CHECK(writable_bytes_view(buf, 31) == "We write, and again, and so on."_av);
        RED_CHECK_EQ(scytale_reader_close(handle), 0);

        RED_CHECK_EQ(scytale_reader_get_error_message(handle), "No error"sv);

        RED_CHECK_EQ(scytale_reader_qhash(handle, finalname), 0);
        RED_CHECK_EQ(scytale_reader_fhash(handle, finalname), 0);

        RED_CHECK_EQ(scytale_reader_get_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);
        RED_CHECK_EQ(scytale_reader_get_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);

        scytale_reader_delete(handle);
    }
}

RED_AUTO_TEST_CASE_WD(TestscytaleWriteUseRandom, wd)
{
    auto finalname = wd.add_file("encrypted.txt");
    auto hash_finalname = wd.add_file("hash_encrypted.txt");

    HashHexArray qhash;
    HashHexArray fhash;

    // Writer with udev random
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname, hmac_key, &trace_fn, false, false);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        memcpy(qhash, scytale_writer_get_qhashhex(handle), sizeof(qhash));
        memcpy(fhash, scytale_writer_get_fhashhex(handle), sizeof(fhash));
        RED_CHECK_NE(qhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);
        RED_CHECK_NE(fhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5"sv);

        RED_CHECK_EQ(scytale_writer_get_error_message(handle), "No error"sv);

        scytale_writer_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);

    // Writer with udev random
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname,  hmac_key, &trace_fn, false, false);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        auto qhash2 = scytale_writer_get_qhashhex(handle);
        auto fhash2 = scytale_writer_get_fhashhex(handle);
        RED_CHECK_NE(qhash2, qhash);
        RED_CHECK_NE(fhash2, fhash);

        RED_CHECK_EQ(scytale_writer_get_error_message(handle), "No error"sv);

        scytale_writer_delete(handle);
    }
}

RED_AUTO_TEST_CASE(TestscytaleReaderOpenAutoDetectScheme)
{
    uint8_t hmac_key[32] = {
        0x56 , 0xdd , 0xb2 , 0x92 , 0x47 , 0xbe , 0x4b , 0x89 ,
        0x1f , 0x12 , 0x62 , 0x39 , 0x0f , 0x10 , 0xb9 , 0x8e ,
        0xac , 0xff , 0xbc , 0x8a , 0x8f , 0x71 , 0xfb , 0x21 ,
        0x07 , 0x7d , 0xef , 0x9c , 0xb3 , 0x5f , 0xf9 , 0x7b ,
    };

    auto trace_20161025_fn = [](uint8_t const * /*base*/, int /*len*/, uint8_t * buffer, unsigned /*oldscheme*/) {
        uint8_t trace_key[32] = {
            0xa8, 0x6e, 0x1c, 0x63, 0xe1, 0xa6, 0xfd, 0xed,
            0x2f, 0x73, 0x17, 0xca, 0x97, 0xad, 0x48, 0x07,
            0x99, 0xf5, 0xcf, 0x84, 0xad, 0x9f, 0x4a, 0x16,
            0x66, 0x38, 0x09, 0xb7, 0x74, 0xe0, 0x58, 0x34,
        };
        memcpy(buffer, trace_key, 32);
        return 0;
    };

    char const * derivator =
        FIXTURES_PATH "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
        "-192304,wab-4-2-4.yourdomain,5560.mwrm";
    char const * filename =
        FIXTURES_PATH "/verifier/recorded/"
        "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
        "-192304,wab-4-2-4.yourdomain,5560.mwrm";

    auto handle = scytale_reader_new(derivator, hmac_key, trace_20161025_fn, 0, 0);
    RED_CHECK_EQ(
        scytale_reader_open_with_auto_detect_encryption_scheme(handle, filename, filename),
        int(EncryptionSchemeTypeResult::OldScheme));

    char buf[20]{};
    RED_CHECK_EQ(scytale_reader_read(handle, byte_ptr_cast(buf), sizeof(buf)), 20);
    RED_CHECK(make_array_view(buf) == "800 600\n\n\n/var/wab/r"_av);

    scytale_reader_close(handle);
    scytale_reader_delete(handle);
}

RED_AUTO_TEST_CASE(TestscytaleError)
{
    auto handle_w = scytale_writer_new(1, 1, "/", hmac_key, &trace_fn, false, false);
    RED_CHECK_EQ(scytale_writer_open(handle_w, "/", "/", 0), -1);
    RED_CHECK_NE(scytale_writer_get_error_message(handle_w), "No error"sv);

    auto handle_r = scytale_reader_new("/", hmac_key, &trace_fn, 0, 0);
    RED_CHECK_EQ(scytale_reader_open(handle_r, "/", "/"), -1);
    RED_CHECK_NE(scytale_reader_get_error_message(handle_r), "No error"sv);

    RED_CHECK_EQ(scytale_reader_qhash(handle_r, "/"), -1);
    RED_CHECK_EQ(scytale_reader_fhash(handle_r, "/"), -1);
    RED_CHECK_NE(scytale_reader_get_error_message(handle_r), "No error"sv);

    scytale_writer_delete(handle_w);
    scytale_reader_delete(handle_r);

    RED_CHECK_EQ(scytale_writer_write(nullptr, bytes("We write, "), 10), -1);
    RED_CHECK_EQ(scytale_writer_close(nullptr), -1);
    RED_CHECK_NE(scytale_writer_get_error_message(nullptr), "No error"sv);

    uint8_t buf[12];
    RED_CHECK_EQ(scytale_reader_read(nullptr, buf, 10), -1);
    RED_CHECK_EQ(scytale_reader_close(nullptr), -1);
    RED_CHECK_NE(scytale_reader_get_error_message(nullptr), "No error"sv);
}

RED_AUTO_TEST_CASE(TestscytaleMeta)
{
    {
        auto filename = FIXTURES_PATH "/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,wab-5-0-0.yourdomain,7681.mwrm";
        auto handle = scytale_reader_new(filename, hmac_key, &trace_fn, 0, 0);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_reader_open(handle, filename, filename), 0);

        auto meta_handle = scytale_meta_reader_new(handle);
        RED_CHECK_NE(meta_handle, nullptr);

        RED_CHECK_EQ(scytale_meta_reader_read_header(meta_handle), 0);
        auto header = scytale_meta_reader_get_header(meta_handle);
        RED_CHECK_EQ(header->version, 2);
        RED_CHECK_EQ(header->has_checksum, 0);

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), 0);
        auto line = scytale_meta_reader_get_line(meta_handle);
        RED_CHECK_EQ(line->filename, "/var/wab/recorded/rdp/toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,wab-5-0-0.yourdomain,7681-000000.wrm"sv);
        RED_CHECK_EQ(line->size, 181826);
        RED_CHECK_EQ(line->mode, 33056);
        RED_CHECK_EQ(line->uid, 1001);
        RED_CHECK_EQ(line->gid, 1001);
        RED_CHECK_EQ(line->dev, 65030);
        RED_CHECK_EQ(line->ino, 81);
        RED_CHECK_EQ(line->mtime, 1455816421);
        RED_CHECK_EQ(line->ctime, 1455816421);
        RED_CHECK_EQ(line->start_time, 1455815820);
        RED_CHECK_EQ(line->stop_time, 1455816422);
        RED_CHECK(not line->with_hash);

        RED_CHECK_EQ(scytale_meta_reader_read_line_eof(meta_handle), 0);

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), ERR_TRANSPORT_NO_MORE_DATA);
        RED_CHECK_EQ(scytale_meta_reader_read_line_eof(meta_handle), 1);

        scytale_meta_reader_delete(meta_handle);
        RED_CHECK_EQ(scytale_reader_close(handle), 0);
        scytale_reader_delete(handle);
    }
    {
        auto filename = FIXTURES_PATH "/sample.mwrm";
        auto handle = scytale_reader_new(filename, hmac_key, &trace_fn, 0, 0);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_reader_open(handle, filename, filename), 0);

        auto meta_handle = scytale_meta_reader_new(handle);
        RED_CHECK_NE(meta_handle, nullptr);

        RED_CHECK_EQ(scytale_meta_reader_read_header(meta_handle), 0);
        auto header = scytale_meta_reader_get_header(meta_handle);
        RED_CHECK_EQ(header->version, 1);
        RED_CHECK_EQ(header->has_checksum, 0);

        struct Data
        {
            std::string_view filename;
            uint64_t start_time;
            uint64_t stop_time;
        };

        RED_TEST_CONTEXT_DATA(Data const& data, "filename: " << data.filename, {
            Data{"./tests/fixtures/sample0.wrm"sv, 1352304810, 1352304870},
            Data{"./tests/fixtures/sample1.wrm"sv, 1352304870, 1352304930},
            Data{"./tests/fixtures/sample2.wrm"sv, 1352304930, 1352304990},
        })
        {
            RED_TEST(scytale_meta_reader_read_line(meta_handle) == 0);
            auto line = scytale_meta_reader_get_line(meta_handle);
            RED_TEST(line->filename == data.filename);
            RED_TEST(line->size == 0);
            RED_TEST(line->mode == 0);
            RED_TEST(line->uid == 0);
            RED_TEST(line->gid == 0);
            RED_TEST(line->dev == 0);
            RED_TEST(line->ino == 0);
            RED_TEST(line->mtime == 0);
            RED_TEST(line->ctime == 0);
            RED_TEST(line->start_time == data.start_time);
            RED_TEST(line->stop_time == data.stop_time);
            RED_TEST(not line->with_hash);
        }

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), ERR_TRANSPORT_NO_MORE_DATA);
        RED_CHECK_EQ(scytale_meta_reader_read_line_eof(meta_handle), 1);

        scytale_meta_reader_delete(meta_handle);
        RED_CHECK_EQ(scytale_reader_close(handle), 0);
        scytale_reader_delete(handle);
    }
}


RED_AUTO_TEST_CASE(ScytaleTfl)
{
    char const* master_derivator = "abc";

    auto tfl2_hash_content_prefix = "v2\n\n\n0123456789abcdef,000002.tfl "_av;

    struct Data
    {
        std::string_view name;
        bool has_encryption;
        bool has_checksum;
        array_view_const_char tfl2_content;
        array_view_const_char tfl2_hash_content_prefix;
        array_view_const_char fdx_content;

        array_view_const_char decrypted_fdx_content = {};
    };

    RED_TEST_CONTEXT_DATA(Data const& data, "encryption: " << data.has_encryption << " checksum: " << data.has_checksum, {
        Data{"nochecksum_noencryption", false, false,

            "abcdefg"_av,

            tfl2_hash_content_prefix,

            "v3\n\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00"
            "\x00\x00\b\t\x00\x2C\x00""file2.txt0123456789abcdef/0123456789abcdef,000002.tfl"_av
        },

        Data{"checksum", false, true,

            "abcdefg"_av,

            tfl2_hash_content_prefix,

            "v3\n\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00"
            "\x00\x00\x0b\t\x00\x2C\x00""file2.txt0123456789abcdef/0123456789abcdef,000002.tfl\x82"
            "H?\xb0\xb6&\rt9\xc2MZ\"/\xff\xc9\xad""D\xf9RG\xdc\x7f\xf9\x02Jyh\xed"
            "\xf3\x90\x0c\x82H?\xb0\xb6&\rt9\xc2MZ\"/\xff\xc9\xad""D\xf9RG\xdc"
            "\x7f\xf9\x02Jyh\xed\xf3\x90\x0c"_av
        },

        Data{"encryption", true, true,

            "WCFM\x01\x00\x00\x00""8\xa4\xf1Kp\xc6""5\xa4(\xfe\x8b\xed`C\xf8\x13\x18"
            "\x0e@_PV\xa2\xe6\b\x94\x0f\xd2@\xbf\xe1\xe0\x10\x00\x00\x00\xb0""CA|H"
            "\xbb\xb3r\xbeZ\xbf\x1e\xfas\x0e\x91MFCW\x07\x00\x00\x00"_av,

            "WCFM\x01\x00\x00\x00\xf8O\x14.0>"_av,

            "WCFM\x01\x00\x00\x00\xb8l\xda\xa6\xf0\xf6""0\x8d\xa8\x16\xa6n\xe0\xc3"
            "\xe5\xcc\x98v\xdd\xf5\xd0&t_\x88L\xc2P\xc0\xdf\xc9Pp\x00\x00\x00\x9e\xe0"
            "Y\xab\xceV\xdb\xed\xfe\x0bH\xc5(\xbb\xee\xa5\xf1\xd1$\xd7\xc7\xdfv\xfd"
            "\x9d\xc6\xc3\xc4kA\x97\xcdsr\xd9Z\xb7R0|\x86m\xfc\x12\xd6\x1a\xea\xe0#X"
            "\x1d\xef\xba\xa6T[\x87@\xae\xe5\r\x82\xa9`\xf5""A\xa8\x9b\x04\xe1\x19\x04"
            "\x94\xea[?x\"qU\x93Iq\xf5\xe6\xc5?#\x8f\"\xe4\x10\xa0\xbc\xac\x90\xa7L"
            "\x03\x0b""8\xdc\xaa.k\x8e\xf5{<fg_MFCW\x8f\x00\x00\x00"_av,

            "v3\n\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00""D\x00\x00\x00\x00\x00\x00"
            "\x00\x0b\t\x00\x2C\x00""file2.txt0123456789abcdef/0123456789abcdef,000002.tfl0\xeb""e"
            "\x8e\xa2\x83\xc0""F\x9e""4\xffm\xd2\xd2[\xbb""7\x8a\xf9\x03\"\xdd{\x16"
            "\n\xfeP\xe3\x13\x88\xd4\xf7""0\xeb""e\x8e\xa2\x83\xc0""F\x9e""4\xffm"
            "\xd2\xd2[\xbb""7\x8a\xf9\x03\"\xdd{\x16\n\xfeP\xe3\x13\x88\xd4\xf7"_av
        },
    })
    { WorkingDirectory wd(data.name); RED_TEST_CONTEXT("wd: " << wd.dirname()) {

        auto count_error = RED_ERROR_COUNT();

        auto wd_hash = wd.create_subdirectory("hash");
        auto wd_record = wd.create_subdirectory("record");

        auto sid = "0123456789abcdef"sv;
        auto fdx_filebase = "sid,blabla"sv;
        auto fdx_filename = "sid,blabla.fdx"sv;

        auto wd_fdx_record = wd_record.create_subdirectory(sid);
        auto wd_fdx_hash = wd_hash.create_subdirectory(sid);

        auto* fdx = scytale_fdx_writer_new_with_test_random(
            data.has_encryption, data.has_checksum, master_derivator, hmac_key, trace_fn,
            wd_record.dirname(), wd_hash.dirname(), fdx_filebase.data(), 0, sid.data());

        auto* tfl = scytale_fdx_writer_open_tfl(fdx, "file1.txt", int(Mwrm3::Direction::Unknown));
        RED_REQUIRE(tfl);

        RED_TEST(3 == scytale_tfl_writer_write(tfl, bytes("abc"), 3));
        RED_TEST(4 == scytale_tfl_writer_write(tfl, bytes("defg"), 4));

        RED_TEST(0 == scytale_tfl_writer_cancel(tfl));

        tfl = scytale_fdx_writer_open_tfl(fdx, "file2.txt", int(Mwrm3::Direction::ClientToServer));
        RED_REQUIRE(tfl);

        RED_TEST(3 == scytale_tfl_writer_write(tfl, bytes("abc"), 3));
        RED_TEST(4 == scytale_tfl_writer_write(tfl, bytes("defg"), 4));

        RED_TEST(0 == scytale_tfl_writer_close(tfl, 0, nullptr, 0));

        auto fname = str_concat(sid, ",000002.tfl"_av);
        auto file2path = wd_fdx_record.add_file(fname);
        auto file2hash = wd_fdx_hash.add_file(fname);

        auto hres = data.tfl2_hash_content_prefix;
        std::string content = RED_REQUIRE_GET_FILE_CONTENTS(file2hash);
        RED_REQUIRE(content.size() >= hres.size());
        RED_TEST(content.substr(0, hres.size()) == hres);

        RED_CHECK_FILE_CONTENTS(file2path, data.tfl2_content);

        RED_TEST(0 == scytale_fdx_writer_close(fdx));
        RED_TEST(1 == scytale_fdx_writer_close(fdx)); // double close

        RED_TEST("No error"sv == scytale_fdx_writer_get_error_message(fdx));

        auto fdxpath = wd_record.add_file(fdx_filename);
        (void)wd_hash.add_file(fdx_filename);

        RED_TEST(fdxpath.string() == scytale_fdx_get_path(fdx));

        RED_TEST(0 == scytale_fdx_writer_delete(fdx));

        RED_CHECK_FILE_CONTENTS(fdxpath, data.fdx_content);

        if (data.has_encryption)
        {
            std::array<uint8_t, 1024> buffer;

            auto readall = [&](char const* filename, char const* derivator) {
                RED_TEST_CONTEXT("filename: " << filename << "\n    derivator: " << derivator)
                {
                    auto* reader = scytale_reader_new(master_derivator, hmac_key, trace_fn, 0, 0);
                    RED_REQUIRE(reader);
                    auto free_reader = finally([&]{ scytale_reader_delete(reader); });

                    RED_TEST(0 == scytale_reader_open(reader, filename, derivator));

                    auto len = scytale_reader_read(reader, buffer.data(), buffer.size());
                    RED_REQUIRE(scytale_reader_get_error_message(reader) == "No error"sv);

                    RED_TEST(0 == scytale_reader_close(reader));

                    return bytes_view(buffer.data(), std::size_t(len));
                }

                REDEMPTION_DIAGNOSTIC_PUSH
                REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunreachable-code-return")
                return bytes_view{}; /*NOLINT*/
                REDEMPTION_DIAGNOSTIC_POP
            };

            auto content = readall(file2hash.c_str(), fname.c_str());
            RED_TEST(content.first(std::min(content.size(), tfl2_hash_content_prefix.size())) == tfl2_hash_content_prefix);

            RED_TEST(readall(file2path.c_str(), fname.c_str()) == "abcdefg"_av);
            RED_TEST(readall(fdxpath.c_str(), fdx_filename.data()) == data.decrypted_fdx_content);
        }

        RED_CHECK_WORKSPACE(wd);

        if (count_error != RED_ERROR_COUNT()) {
            break;
        }
    } }
}

RED_AUTO_TEST_CASE_WD(ScytaleMWrm3Reader, wd)
{
    auto filename = wd.add_file("a.mwrm3");

    std::ofstream(filename.string()) <<
        "v3\n"
        "\x04\x00"
        "\x04\x00\x00\x00\x00\x00\x00\x00"
        "\x03\x00\x00\x00\x00\x00\x00\x00"
        "\x08"
        "\x05\x00"
        "\x18\x00"
        "file4"
        "my_session_id,000004.tfl"sv
    ;

    auto reader = scytale_reader_new("", nullptr, nullptr, 0, 0);
    RED_REQUIRE(reader);
    auto free_reader = finally([&]{ scytale_reader_delete(reader); });

    RED_REQUIRE(0 == scytale_reader_open(reader, filename.c_str(), ""));

    auto mwrm3 = scytale_mwrm3_reader_new(reader);
    RED_REQUIRE(mwrm3);
    auto free_mwrm3 = finally([&]{ scytale_mwrm3_reader_delete(mwrm3); });

    ScytaleMwrm3ReaderData const* data;

    struct scytale_bytes_view
    {
        uint8_t const* ptr;
        uint32_t size;

        bytes_view bytes() const
        {
            return {ptr, size};
        }
    };

    data = scytale_mwrm3_reader_read_next(mwrm3);
    RED_REQUIRE(scytale_mwrm3_reader_get_error_message(mwrm3) == ""sv);
    RED_REQUIRE(data);
    RED_TEST(data->type == ('v' | '3' << 8));
    RED_TEST(data->fmt == ""sv);

    data = scytale_mwrm3_reader_read_next(mwrm3);
    RED_REQUIRE(scytale_mwrm3_reader_get_error_message(mwrm3) == ""sv);
    RED_REQUIRE(data);
    RED_TEST(data->type == 4);
    {
        RED_REQUIRE(data->fmt == "uuuussBBB"sv);
        auto* raw_data = static_cast<char const*>(data->data);
        RED_REQUIRE(raw_data);
        struct D
        {
            uint64_t file_id;
            uint64_t file_size;
            uint64_t direction;
            uint64_t transfered_status;
            scytale_bytes_view original_filename;
            scytale_bytes_view tfl_filename;
            scytale_bytes_view qhash;
            scytale_bytes_view fhash;
            scytale_bytes_view sig;
        };
        D const& d = *static_cast<D const*>(data->data);
        RED_TEST(d.file_id == 4);
        RED_TEST(d.file_size == 3);
        RED_TEST(d.direction == int(Mwrm3::Direction::ClientToServer));
        RED_TEST(d.transfered_status == int(Mwrm3::TransferedStatus::Unknown));
        RED_TEST(d.original_filename.bytes() == "file4"_av);
        RED_TEST(d.tfl_filename.bytes() == "my_session_id,000004.tfl"_av);
        RED_TEST(d.qhash.bytes() == ""_av);
        RED_TEST(d.fhash.bytes() == ""_av);
        RED_TEST(d.sig.bytes() == ""_av);
    }

    RED_TEST(!scytale_mwrm3_reader_read_next(mwrm3));
}
