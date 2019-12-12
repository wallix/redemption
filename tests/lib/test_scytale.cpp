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

#include <string_view>
#include <fstream>


using namespace std::string_view_literals;


extern "C" {
    inline
    int hmac_fn(uint8_t * buffer)
    {
        // 86410558C495CC4E492157874774088A33B02AB865CC384120FEC2C9B872C82C
        uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
            0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
            0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
            0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
            0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c,
        };
        memcpy(buffer, hmac_key, sizeof(hmac_key));
        return 0;
    }

    inline
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
}

inline uint8_t const * bytes(char const * p)
{
    return byte_ptr_cast(p);
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

        auto * handle = scytale_writer_new_with_test_random(with_encryption, with_checksum, finalname, &hmac_fn, &trace_fn, false, false);
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
        auto handle = scytale_reader_new(finalname, &hmac_fn, &trace_fn, 0, 0);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_reader_open(handle, finalname, derivator), 0);

        uint8_t buf[31];

        size_t total = 0;
        while (total < sizeof(buf)) {
            int res = scytale_reader_read(handle, &buf[total], 10);
            RED_REQUIRE_GT(res, 0);
            total += size_t(res);
        }
        RED_CHECK_MEM(writable_bytes_view(buf, 31), "We write, and again, and so on."_av);
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

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname, &hmac_fn, &trace_fn, false, false);
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

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname,  &hmac_fn, &trace_fn, false, false);
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
    auto hmac_2016_fn = [](uint8_t * buffer) {
        uint8_t hmac_key[32] = {
            0x56 , 0xdd , 0xb2 , 0x92 , 0x47 , 0xbe , 0x4b , 0x89 ,
            0x1f , 0x12 , 0x62 , 0x39 , 0x0f , 0x10 , 0xb9 , 0x8e ,
            0xac , 0xff , 0xbc , 0x8a , 0x8f , 0x71 , 0xfb , 0x21 ,
            0x07 , 0x7d , 0xef , 0x9c , 0xb3 , 0x5f , 0xf9 , 0x7b ,
        };
        memcpy(buffer, hmac_key, 32);
        return 0;
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

    auto handle = scytale_reader_new(derivator, hmac_2016_fn, trace_20161025_fn, 0, 0);
    RED_CHECK_EQ(
        scytale_reader_open_with_auto_detect_encryption_scheme(handle, filename, filename),
        int(EncryptionSchemeTypeResult::OldScheme));

    char buf[20]{};
    RED_CHECK_EQ(scytale_reader_read(handle, byte_ptr_cast(buf), sizeof(buf)), 20);
    RED_CHECK_EQUAL_RANGES(buf, cstr_array_view("800 600\n\n\n/var/wab/r"));

    scytale_reader_close(handle);
    scytale_reader_delete(handle);
}

RED_AUTO_TEST_CASE(TestscytaleError)
{
    auto handle_w = scytale_writer_new(1, 1, "/", &hmac_fn, &trace_fn, false, false);
    RED_CHECK_EQ(scytale_writer_open(handle_w, "/", "/", 0), -1);
    RED_CHECK_NE(scytale_writer_get_error_message(handle_w), "No error"sv);

    auto handle_r = scytale_reader_new("/", &hmac_fn, &trace_fn, 0, 0);
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

RED_AUTO_TEST_CASE(TestscytaleKeyDerivation2)
{
    // master derivator: "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
    ScytaleKeyHandle * handle = scytale_key_new("563eb6e8158f0eed2e5fb6bc2893bc15270d7e7815fa804a723ef4fb315ff4b2");
    RED_CHECK_NE(handle, nullptr);
    bytes_view derivator = "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"_av;
    const char * result = scytale_key_derivate(handle, derivator.as_u8p(), derivator.size());
    RED_CHECK_EQ(result, "C5CC4737881CD6ABA89843CE239201E8D63783325DC5E0391D90165265B2F648"sv);

    // .log behave as .mwrm for historical reasons
    bytes_view derivator2 = "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.log"_av;
    const char * r2 = scytale_key_derivate(handle, derivator2.as_u8p(), derivator2.size());
    RED_CHECK_EQ(r2, "C5CC4737881CD6ABA89843CE239201E8D63783325DC5E0391D90165265B2F648"sv);

    scytale_key_delete(handle);
}

RED_AUTO_TEST_CASE(TestscytaleKeyDerivation)
{
    // master derivator: "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm"
    ScytaleKeyHandle * handle = scytale_key_new("a86e1c63e1a6fded2f7317ca97ad480799f5cf84ad9f4a16663809b774e05834");
    RED_CHECK_NE(handle, nullptr);
    bytes_view derivator = "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560-000000.wrm"_av;
    const char * result = scytale_key_derivate(handle, derivator.as_u8p(), derivator.size());
    RED_CHECK_EQ(result, "CABD9CEE0BF786EC31532C954BD15F8B3426AC3C8B96FB4C77B57156EA5B6A89"sv);
    scytale_key_delete(handle);
}

RED_AUTO_TEST_CASE(TestscytaleMeta)
{
    {
        auto filename = FIXTURES_PATH "/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,wab-5-0-0.yourdomain,7681.mwrm";
        auto handle = scytale_reader_new(filename, &hmac_fn, &trace_fn, 0, 0);
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
        auto handle = scytale_reader_new(filename, &hmac_fn, &trace_fn, 0, 0);
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

            "v3\n"
            "\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\t\x00\x1b\x00\x01"
            "file2.txt0123456789abcdef,000002.tfl"
            "\x05\x00\x02\x00\x00\x00\x00\x00\x00\x00\07\x00\x00\x00\x00\x00\x00\x00\x00"_av
        },

        Data{"checksum", false, true,

            "abcdefg"_av,

            tfl2_hash_content_prefix,

            "v3\n"
            "\x04\x00\x2\x00\x00\x00\x00\x00\x00\x00\t\x00\x1b\x00\x01"
            "file2.txt0123456789abcdef,000002.tfl"
            "\x05\x00\x02\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00"
            "\x00\x00\x00\x03\x82H?\xb0\xb6&\rt9\xc2MZ\"/\xff\xc9\xad""D\xf9RG\xdc\x7f\xf9\x02Jyh"
            "\xed\xf3\x90\x0c\x82H?\xb0\xb6&\rt9\xc2MZ\"/\xffɭD\xf9RG\xdc\x7f\xf9\x02"
            "Jyh\xed\xf3\x90\x0c"_av
        },

        Data{"encryption", true, true,

            "WCFM\x01\x00\x00\x00xX\xaeR\xb0\xael\x17hZ\x13\xbd\xa0Sz\xf3X\x12{\xce\x90"
            "\x8e\xa8\xadH@\x87""1\x80\x1f""E\xb6\x10\x00\x00\x00\xbb\xa3\xde~\xaa\xe7"
            "b\x96s8\"\x97=\xf6\xf8JMFCW\x07\x00\x00\x00"_av,

            "WCFM\x01\x00\x00\x00\xf8O\x14.0>"_av,

            "WCFM\x01\x00\x00\x00""8\xa4\xf1Kp\xc6""5\xa4(\xfe\x8b\xed`C\xf8\x13\x18\x0e@_PV"
            "\xa2\xe6\b\x94\x0f\xd2@\xbf\xe1\xe0p\x00\x00\x00\x8b\xda>\xb9\x05\xfarX\xf6\x1d"
            "(\x91\xdb\xfe\xfb\xd9\xcbi\t\x9e;\x05\xb2\x9f{\xfc\xb1\xea\xae\x9f\xc7\xda\xe3"
            "\xa7NQ0\x07""8\xfev\x98)\xccJ \xd6""AM\x07xa\x84Gx\x12""5T\xd0\x9cw{\x9e]q9q"
            "\xac\xa8\xa0#R1\x1d""1Ie\xdbyK2\xd0\x1fh\xe1\xe0:\xa0\x8c\xbf,w\x01\xcd\xba\xa4"
            "\x00h\xb1R{2 \xe6""EfQ\x02w\x00\b\xcaMFCW\x89\x00\x00\x00"_av,

            "v3\n"
            "\x04\x00\x2\x00\x00\x00\x00\x00\x00\x00\t\x00\x1b\x00\x01"
            "file2.txt0123456789abcdef,000002.tfl"
            "\x05\x00\x02\x00\x00\x00\x00\x00\x00\x00""D\x00\x00\x00\x00\x00\x00\x00\x03"
            "\x85\xee\x12\xe1!(\x99\xf2\x18\xc5\xc0H%~5\xf1\x8f\xfd\x9e\x0b\xe4\xc3\xd0"
            "\xdd\xb7\x18\x7f\x8e\xc1\xec\xa1`\x85\xee\x12\xe1!(\x99\xf2\x18\xc5\xc0H%~"
            "5\xf1\x8f\xfd\x9e\x0b\xe4\xc3\xd0\xdd\xb7\x18\x7f\x8e\xc1\xec\xa1`"_av
        },
    })
    { WorkingDirectory wd(data.name); RED_TEST_CONTEXT("wd: " << wd.dirname()) {

        using Direction = ScytaleOpenTflDirection;

        auto count_error = RED_ERROR_COUNT;

        auto wd_hash = wd.create_subdirectory("hash");
        auto wd_record = wd.create_subdirectory("record");

        auto sid = "0123456789abcdef"sv;

        auto wd_fdx_record = wd_record.create_subdirectory(sid);
        auto wd_fdx_hash = wd_hash.create_subdirectory(sid);

        auto* fdx = scytale_fdx_writer_new_with_test_random(
            data.has_encryption, data.has_checksum, master_derivator, hmac_fn, trace_fn,
            wd_record.dirname(), wd_hash.dirname(), 0, sid.data());

        auto* tfl = scytale_fdx_writer_open_tfl(fdx, "file1.txt", int(Direction::unknown));
        RED_REQUIRE(tfl);

        RED_TEST(3 == scytale_tfl_writer_write(tfl, bytes("abc"), 3));
        RED_TEST(4 == scytale_tfl_writer_write(tfl, bytes("defg"), 4));

        RED_TEST(0 == scytale_tfl_writer_cancel(tfl));

        tfl = scytale_fdx_writer_open_tfl(fdx, "file2.txt", int(Direction::client_to_server));
        RED_REQUIRE(tfl);

        RED_TEST(3 == scytale_tfl_writer_write(tfl, bytes("abc"), 3));
        RED_TEST(4 == scytale_tfl_writer_write(tfl, bytes("defg"), 4));

        RED_TEST(0 == scytale_tfl_writer_close(tfl, nullptr, 0));

        auto fname = str_concat(sid, ",000002.tfl"_av);
        auto file2path = wd_fdx_record.add_file(fname);
        auto file2hash = wd_fdx_hash.add_file(fname);

        auto hres = data.tfl2_hash_content_prefix;
        std::string content = RED_REQUIRE_GET_FILE_CONTENTS(file2hash);
        RED_REQUIRE(content.size() >= hres.size());
        RED_TEST(content.substr(0, hres.size()) == hres);

        RED_CHECK_MEM_FILE_CONTENTS(file2path, data.tfl2_content);

        RED_TEST(0 == scytale_fdx_writer_close(fdx));
        RED_TEST(1 == scytale_fdx_writer_close(fdx)); // double close

        RED_TEST("No error"sv == scytale_fdx_writer_get_error_message(fdx));

        RED_TEST(0 == scytale_fdx_writer_delete(fdx));

        auto fdx_filename = str_concat(sid, ".fdx");
        auto fdxpath = wd_fdx_record.add_file(fdx_filename);
        (void)wd_fdx_hash.add_file(fdx_filename);

        RED_CHECK_MEM_FILE_CONTENTS(fdxpath, data.fdx_content);

        if (data.has_encryption)
        {
            std::array<uint8_t, 1024> buffer;

            auto readall = [&](char const* filename, char const* derivator){
                RED_TEST_CONTEXT("filename: " << filename << "\n    derivator: " << derivator)
                {
                    auto* reader = scytale_reader_new(master_derivator, hmac_fn, trace_fn, 0, 0);
                    RED_REQUIRE(reader);
                    auto free_reader = finally([&]{ scytale_reader_delete(reader); });

                    RED_TEST(0 == scytale_reader_open(reader, filename, derivator));

                    auto len = scytale_reader_read(reader, buffer.data(), buffer.size());
                    RED_REQUIRE(scytale_reader_get_error_message(reader) == "No error"sv);

                    RED_TEST(0 == scytale_reader_close(reader));

                    return bytes_view(buffer.data(), std::size_t(len));
                }
                return bytes_view{};
            };

            auto content = readall(file2hash.c_str(), fname.c_str());
            RED_TEST(content.first(std::min(content.size(), tfl2_hash_content_prefix.size())) == tfl2_hash_content_prefix);

            RED_TEST(readall(file2path.c_str(), fname.c_str()) == "abcdefg"_av);
            RED_TEST(readall(fdxpath.c_str(), fdx_filename.c_str()) == data.decrypted_fdx_content);
        }

        RED_CHECK_WORKSPACE(wd);

        if (count_error != RED_ERROR_COUNT) {
            break;
        }
    } }
}

RED_AUTO_TEST_CASE_WD(ScytaleMWrm3Reader, wd)
{
    auto filename = wd.add_file("a.mwrm3");

    std::ofstream(filename.string()) <<
        "v3\n"
        "\x04\x00\x02\x00\x00\x00\x00\x00\x00\x00\t\x00\x1b\x00\x01"
        "file2.txt0123456789abcdef,000002.tfl"
        "\x05\x00\x02\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00"sv
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
        RED_REQUIRE(data->fmt == "uuss"sv);
        auto* raw_data = static_cast<char const*>(data->data);
        RED_REQUIRE(!!raw_data);
        struct D
        {
            uint64_t file_id;
            uint64_t direction;
            scytale_bytes_view original_filename;
            scytale_bytes_view filename;
        };
        D const& d = *reinterpret_cast<D const*>(raw_data);
        RED_TEST(d.file_id == 2);
        RED_TEST(d.direction == int(ScytaleOpenTflDirection::client_to_server));
        RED_TEST(d.original_filename.bytes() == "file2.txt"_av);
        RED_TEST(d.filename.bytes() == "0123456789abcdef,000002.tfl"_av);
    }

    data = scytale_mwrm3_reader_read_next(mwrm3);
    RED_REQUIRE(scytale_mwrm3_reader_get_error_message(mwrm3) == ""sv);
    RED_REQUIRE(data);
    RED_TEST(data->type == 5);
    {
        RED_REQUIRE(data->fmt == "uuBBB"sv);
        auto* raw_data = static_cast<char const*>(data->data);
        RED_REQUIRE(!!raw_data);
        struct D
        {
            uint64_t file_id;
            uint64_t file_size;
            scytale_bytes_view qhash;
            scytale_bytes_view fhash;
            scytale_bytes_view sig;
        };
        D const& d = *reinterpret_cast<D const*>(raw_data);
        RED_TEST(d.file_id == 2);
        RED_TEST(d.file_size == 7);
        RED_TEST(d.qhash.bytes() == ""_av);
        RED_TEST(d.fhash.bytes() == ""_av);
        RED_TEST(d.sig.bytes() == ""_av);
    }

    RED_TEST(!scytale_mwrm3_reader_read_next(mwrm3));
}
