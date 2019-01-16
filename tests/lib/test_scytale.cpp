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

#define RED_TEST_MODULE Testscytale
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "lib/scytale.hpp"
#include "transport/crypto_transport.hpp"


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

RED_AUTO_TEST_CASE(Testscytale)
{
    const char * finalname = "encrypted.txt";
    const char * hash_finalname = "hash_encrypted.txt";
    ::unlink(finalname);
    ::unlink(hash_finalname);

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

        RED_CHECK_EQ(scytale_writer_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_EQ(scytale_writer_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

        scytale_writer_delete(handle);
    }

    // Reader
    {
        auto handle = scytale_reader_new(finalname, &hmac_fn, &trace_fn, 0, 0);
        RED_REQUIRE(handle);
        RED_CHECK_EQ(scytale_reader_open(handle, finalname, finalname), 0);

        uint8_t buf[31];

        size_t total = 0;
        while (total < sizeof(buf)) {
            int res = scytale_reader_read(handle, &buf[total], 10);
            RED_REQUIRE_GT(res, 0);
            total += size_t(res);
        }
        RED_CHECK_MEM_C(bytes_view(buf, 31), "We write, and again, and so on.");
        RED_CHECK_EQ(scytale_reader_close(handle), 0);

        RED_CHECK_EQ(scytale_reader_error_message(handle), "No error");

        RED_CHECK_EQ(scytale_reader_qhash(handle, finalname), 0);
        RED_CHECK_EQ(scytale_reader_fhash(handle, finalname), 0);

        RED_CHECK_EQ(scytale_reader_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_EQ(scytale_reader_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        scytale_reader_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);
}

RED_AUTO_TEST_CASE(TestscytaleWriteUseRandom)
{
    const char * finalname = "encrypted.txt";
    const char * hash_finalname = "hash_encrypted.txt";

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

        memcpy(qhash, scytale_writer_qhashhex(handle), sizeof(qhash));
        memcpy(fhash, scytale_writer_fhashhex(handle), sizeof(fhash));
        RED_CHECK_NE(qhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_NE(fhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

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

        auto qhash2 = scytale_writer_qhashhex(handle);
        auto fhash2 = scytale_writer_fhashhex(handle);
        RED_CHECK_NE(qhash2, qhash);
        RED_CHECK_NE(fhash2, fhash);

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

        scytale_writer_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);
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
    RED_CHECK_NE(scytale_writer_error_message(handle_w), "No error");

    auto handle_r = scytale_reader_new("/", &hmac_fn, &trace_fn, 0, 0);
    RED_CHECK_EQ(scytale_reader_open(handle_r, "/", "/"), -1);
    RED_CHECK_NE(scytale_reader_error_message(handle_r), "No error");

    RED_CHECK_EQ(scytale_reader_qhash(handle_r, "/"), -1);
    RED_CHECK_EQ(scytale_reader_fhash(handle_r, "/"), -1);
    RED_CHECK_NE(scytale_reader_error_message(handle_r), "No error");

    scytale_writer_delete(handle_w);
    scytale_reader_delete(handle_r);

    RED_CHECK_EQ(scytale_writer_write(nullptr, bytes("We write, "), 10), -1);
    RED_CHECK_EQ(scytale_writer_close(nullptr), -1);
    RED_CHECK_NE(scytale_writer_error_message(nullptr), "No error");

    uint8_t buf[12];
    RED_CHECK_EQ(scytale_reader_read(nullptr, buf, 10), -1);
    RED_CHECK_EQ(scytale_reader_close(nullptr), -1);
    RED_CHECK_NE(scytale_reader_error_message(nullptr), "No error");
}

RED_AUTO_TEST_CASE(TestscytaleKeyDerivation2)
{
    // master derivator: "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
    RedCryptoKeyHandle * handle = scytale_key_new("563eb6e8158f0eed2e5fb6bc2893bc15270d7e7815fa804a723ef4fb315ff4b2");
    RED_CHECK_NE(handle, nullptr);
    const char * derivator = "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm";
    const char * result = scytale_key_derivate(handle, bytes(derivator), std::strlen(derivator));
    RED_CHECK_EQ(result, "C5CC4737881CD6ABA89843CE239201E8D63783325DC5E0391D90165265B2F648");

    // .log behave as .mwrm for historical reasons
    const char * derivator2 = "toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.log";
    const char * r2 = scytale_key_derivate(handle, bytes(derivator2), std::strlen(derivator2));
    RED_CHECK_EQ(r2, "C5CC4737881CD6ABA89843CE239201E8D63783325DC5E0391D90165265B2F648");

    scytale_key_delete(handle);
}

RED_AUTO_TEST_CASE(TestscytaleKeyDerivation)
{
    // master derivator: "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560.mwrm"
    RedCryptoKeyHandle * handle = scytale_key_new("a86e1c63e1a6fded2f7317ca97ad480799f5cf84ad9f4a16663809b774e05834");
    RED_CHECK_NE(handle, nullptr);
    const char * derivator = "cgrosjean@10.10.43.13,proxyuser@win2008,20161025-192304,wab-4-2-4.yourdomain,5560-000000.wrm";
    const char * result = scytale_key_derivate(handle, bytes(derivator), std::strlen(derivator));
    RED_CHECK_EQ(result, "CABD9CEE0BF786EC31532C954BD15F8B3426AC3C8B96FB4C77B57156EA5B6A89");
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
        RED_CHECK_EQ(line->filename, "/var/wab/recorded/rdp/toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,wab-5-0-0.yourdomain,7681-000000.wrm");
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
        RED_CHECK_EQ(line->with_hash, false);

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

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), 0);
        auto line = scytale_meta_reader_get_line(meta_handle);
        RED_CHECK_EQ(line->filename, "./tests/fixtures/sample0.wrm");
        RED_CHECK_EQ(line->size, 0);
        RED_CHECK_EQ(line->mode, 0);
        RED_CHECK_EQ(line->uid, 0);
        RED_CHECK_EQ(line->gid, 0);
        RED_CHECK_EQ(line->dev, 0);
        RED_CHECK_EQ(line->ino, 0);
        RED_CHECK_EQ(line->mtime, 0);
        RED_CHECK_EQ(line->ctime, 0);
        RED_CHECK_EQ(line->start_time, 1352304810);
        RED_CHECK_EQ(line->stop_time, 1352304870);
        RED_CHECK_EQ(line->with_hash, false);

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), 0);
        line = scytale_meta_reader_get_line(meta_handle);
        RED_CHECK_EQ(line->filename, "./tests/fixtures/sample1.wrm");
        RED_CHECK_EQ(line->size, 0);
        RED_CHECK_EQ(line->mode, 0);
        RED_CHECK_EQ(line->uid, 0);
        RED_CHECK_EQ(line->gid, 0);
        RED_CHECK_EQ(line->dev, 0);
        RED_CHECK_EQ(line->ino, 0);
        RED_CHECK_EQ(line->mtime, 0);
        RED_CHECK_EQ(line->ctime, 0);
        RED_CHECK_EQ(line->start_time, 1352304870);
        RED_CHECK_EQ(line->stop_time, 1352304930);
        RED_CHECK_EQ(line->with_hash, false);

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), 0);
        line = scytale_meta_reader_get_line(meta_handle);
        RED_CHECK_EQ(line->filename, "./tests/fixtures/sample2.wrm");
        RED_CHECK_EQ(line->size, 0);
        RED_CHECK_EQ(line->mode, 0);
        RED_CHECK_EQ(line->uid, 0);
        RED_CHECK_EQ(line->gid, 0);
        RED_CHECK_EQ(line->dev, 0);
        RED_CHECK_EQ(line->ino, 0);
        RED_CHECK_EQ(line->mtime, 0);
        RED_CHECK_EQ(line->ctime, 0);
        RED_CHECK_EQ(line->start_time, 1352304930);
        RED_CHECK_EQ(line->stop_time, 1352304990);
        RED_CHECK_EQ(line->with_hash, false);

        RED_CHECK_EQ(scytale_meta_reader_read_line(meta_handle), ERR_TRANSPORT_NO_MORE_DATA);
        RED_CHECK_EQ(scytale_meta_reader_read_line_eof(meta_handle), 1);

        scytale_meta_reader_delete(meta_handle);
        RED_CHECK_EQ(scytale_reader_close(handle), 0);
        scytale_reader_delete(handle);
    }
}
