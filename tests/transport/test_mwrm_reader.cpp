/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Jonathan Poelen
*/

#define RED_TEST_MODULE TestMwrmReader
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/genfstat.hpp"
#include "transport/mwrm_reader.hpp"
#include "transport/crypto_transport.hpp"
#include "test_only/transport/test_transport.hpp"

#include <fstream>


extern "C" {
    inline int hmac_fn(uint8_t * buffer)
    {
        // E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
        uint8_t hmac_key[] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        static_assert(sizeof(hmac_key) == MD_HASH::DIGEST_LENGTH, "");
        memcpy(buffer, hmac_key, sizeof(hmac_key));
        return 0;
    }

    inline int trace_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2 };
        static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH, "");
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

constexpr auto is_encrypted = InCryptoTransport::EncryptionMode::Encrypted;
constexpr auto is_not_encrypted = InCryptoTransport::EncryptionMode::NotEncrypted;


RED_AUTO_TEST_CASE(TestMwrmLineReader)
{
    char const * filename = "/tmp/test_app_verifier_s.txt";

    std::ofstream(filename) <<
        "abcd\n"
        "efghi\n"
        "jklmno\n"
    ;
    Fstat fstat;
    CryptoContext cctx;

    using Read = Transport::Read;

    {
        InCryptoTransport ifile(cctx, is_not_encrypted, fstat);
        ifile.open(filename);
        MwrmLineReader line_reader(ifile);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok);
        RED_CHECK_EQUAL(line_reader.get_buf().size(), 5);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok);
        RED_CHECK_EQUAL(line_reader.get_buf().size(), 6);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok);
        RED_CHECK_EQUAL(line_reader.get_buf().size(), 7);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Eof);
    }

    std::size_t const big_line_len = 3000;
    {
        std::string s(big_line_len, 'a');
        std::ofstream(filename) << s << '\n' << s << '\n';
    }

    {
        InCryptoTransport ifile(cctx, is_not_encrypted, fstat);
        ifile.open(filename);
        MwrmLineReader line_reader(ifile);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok);
        RED_CHECK_EQUAL(line_reader.get_buf().size(), big_line_len+1);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok);
        RED_CHECK_EQUAL(line_reader.get_buf().size(), big_line_len+1);
        RED_CHECK_EQUAL(line_reader.next_line(), Read::Eof);
    }

    std::ofstream(filename) << std::string(10000, 'a');

    {
        InCryptoTransport ifile(cctx, is_not_encrypted, fstat);
        ifile.open(filename);
        MwrmLineReader line_reader(ifile);
        RED_CHECK_EXCEPTION_ERROR_ID(
            RED_CHECK_EQUAL(line_reader.next_line(), Read::Ok),
            ERR_TRANSPORT_READ_FAILED);
    }

    remove(filename);
}

RED_AUTO_TEST_CASE(ReadClearHeaderV1)
{
    Fstat fstat;
    CryptoContext cctx;
    InCryptoTransport fd(cctx, is_not_encrypted, fstat);
    fd.open(FIXTURES_PATH "/verifier/recorded/v1_nochecksum_nocrypt.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers();
    RED_CHECK_EQUAL(reader.get_header().version, WrmVersion::v1);
    RED_CHECK(!reader.get_header().has_checksum);

    MetaLine meta_line;
    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Ok);
    RED_CHECK_EQUAL(meta_line.filename,
        "/var/wab/recorded/rdp/"
        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
        "wab-5-0-0.yourdomain,7681-000000.wrm");
    RED_CHECK_EQUAL(meta_line.size, 0);
    RED_CHECK_EQUAL(meta_line.mode, 0);
    RED_CHECK_EQUAL(meta_line.uid, 0);
    RED_CHECK_EQUAL(meta_line.gid, 0);
    RED_CHECK_EQUAL(meta_line.dev, 0);
    RED_CHECK_EQUAL(meta_line.ino, 0);
    RED_CHECK_EQUAL(meta_line.mtime, 0);
    RED_CHECK_EQUAL(meta_line.ctime, 0);
    RED_CHECK_EQUAL(meta_line.start_time, 1455815820);
    RED_CHECK_EQUAL(meta_line.stop_time, 1455816422);
    RED_CHECK_EQUAL(meta_line.with_hash, false);
}

RED_AUTO_TEST_CASE(ReadClearHeaderV2)
{
    Fstat fstat;
    CryptoContext cctx;
    InCryptoTransport fd(cctx, is_not_encrypted, fstat);
    fd.open(FIXTURES_PATH "/verifier/recorded/v2_nochecksum_nocrypt.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers();
    RED_CHECK_EQUAL(reader.get_header().version, WrmVersion::v2);
    RED_CHECK(!reader.get_header().has_checksum);

    MetaLine meta_line;
    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Ok);
    RED_CHECK_EQUAL(meta_line.filename,
        "/var/wab/recorded/rdp/"
        "toto@10.10.43.13,Administrateur@QA@cible,20160218-181658,"
        "wab-5-0-0.yourdomain,7681-000000.wrm");
    RED_CHECK_EQUAL(meta_line.size, 181826);
    RED_CHECK_EQUAL(meta_line.mode, 33056);
    RED_CHECK_EQUAL(meta_line.uid, 1001);
    RED_CHECK_EQUAL(meta_line.gid, 1001);
    RED_CHECK_EQUAL(meta_line.dev, 65030);
    RED_CHECK_EQUAL(meta_line.ino, 81);
    RED_CHECK_EQUAL(meta_line.mtime, 1455816421);
    RED_CHECK_EQUAL(meta_line.ctime, 1455816421);
    RED_CHECK_EQUAL(meta_line.start_time, 1455815820);
    RED_CHECK_EQUAL(meta_line.stop_time, 1455816422);
    RED_CHECK_EQUAL(meta_line.with_hash, false);
}

RED_AUTO_TEST_CASE(ReadClearHeaderV2Checksum)
{
    Fstat fstat;
    CryptoContext cctx;
    InCryptoTransport fd(cctx, is_not_encrypted, fstat);
    fd.open(FIXTURES_PATH "/sample_v2_checksum.mwrm");
    MwrmReader reader(fd);

    reader.read_meta_headers();
    RED_CHECK_EQUAL(reader.get_header().version, WrmVersion::v2);
    RED_CHECK(reader.get_header().has_checksum);

    MetaLine meta_line;
    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Ok);
    RED_CHECK_EQUAL(meta_line.filename, "./tests/fixtures/sample0.wrm");
    RED_CHECK_EQUAL(meta_line.size, 1);
    RED_CHECK_EQUAL(meta_line.mode, 2);
    RED_CHECK_EQUAL(meta_line.uid, 3);
    RED_CHECK_EQUAL(meta_line.gid, 4);
    RED_CHECK_EQUAL(meta_line.dev, 5);
    RED_CHECK_EQUAL(meta_line.ino, 6);
    RED_CHECK_EQUAL(meta_line.mtime, 7);
    RED_CHECK_EQUAL(meta_line.ctime, 8);
    RED_CHECK_EQUAL(meta_line.start_time, 1352304810);
    RED_CHECK_EQUAL(meta_line.stop_time, 1352304870);
    RED_CHECK_EQUAL(meta_line.with_hash, true);
    RED_CHECK_MEM_AC(
        meta_line.hash1,
        "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA"
        "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA");
    RED_CHECK_MEM_AC(
        meta_line.hash2,
        "\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB"
        "\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB\xBB");
}

inline int hmac_2016_fn(uint8_t * buffer)
{
    uint8_t hmac_key[32] = {
        0x56 , 0xdd , 0xb2 , 0x92 , 0x47 , 0xbe , 0x4b , 0x89 ,
        0x1f , 0x12 , 0x62 , 0x39 , 0x0f , 0x10 , 0xb9 , 0x8e ,
        0xac , 0xff , 0xbc , 0x8a , 0x8f , 0x71 , 0xfb , 0x21 ,
        0x07 , 0x7d , 0xef , 0x9c , 0xb3 , 0x5f , 0xf9 , 0x7b ,
    };
    memcpy(buffer, hmac_key, 32);
    return 0;
}

inline int trace_20161025_fn(uint8_t const * /*base*/, int /*len*/, uint8_t * buffer, unsigned /*oldscheme*/)
{
    uint8_t trace_key[32] = {
        0xa8, 0x6e, 0x1c, 0x63, 0xe1, 0xa6, 0xfd, 0xed,
        0x2f, 0x73, 0x17, 0xca, 0x97, 0xad, 0x48, 0x07,
        0x99, 0xf5, 0xcf, 0x84, 0xad, 0x9f, 0x4a, 0x16,
        0x66, 0x38, 0x09, 0xb7, 0x74, 0xe0, 0x58, 0x34,
    };
    memcpy(buffer, trace_key, 32);
    return 0;
}

RED_AUTO_TEST_CASE(ReadEncryptedHeaderV1Checksum)
{
    Fstat fstat;
    CryptoContext cctx;
    cctx.set_get_hmac_key_cb(hmac_2016_fn);
    cctx.set_get_trace_key_cb(trace_20161025_fn);
    cctx.old_encryption_scheme = 1;

    InCryptoTransport fd(cctx, is_encrypted, fstat);
    RED_REQUIRE_NO_THROW(fd.open(FIXTURES_PATH
        "/verifier/recorded/"
        "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
        "-192304,wab-4-2-4.yourdomain,5560.mwrm"));

    MwrmReader reader(fd);

    RED_REQUIRE_NO_THROW(reader.read_meta_headers());
    RED_CHECK_EQUAL(reader.get_header().version, WrmVersion::v1);
    RED_CHECK(!reader.get_header().has_checksum);

    MetaLine meta_line;
    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Ok);

    RED_CHECK_EQUAL(meta_line.filename,
        "/var/wab/recorded/rdp/"
        "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
        "-192304,wab-4-2-4.yourdomain,5560-000000.wrm");
    RED_CHECK_EQUAL(meta_line.size, 0);
    RED_CHECK_EQUAL(meta_line.mode, 0);
    RED_CHECK_EQUAL(meta_line.uid, 0);
    RED_CHECK_EQUAL(meta_line.gid, 0);
    RED_CHECK_EQUAL(meta_line.dev, 0);
    RED_CHECK_EQUAL(meta_line.ino, 0);
    RED_CHECK_EQUAL(meta_line.mtime, 0);
    RED_CHECK_EQUAL(meta_line.ctime, 0);
    RED_CHECK_EQUAL(meta_line.start_time, 1477416187);
    RED_CHECK_EQUAL(meta_line.stop_time, 1477416298);
    RED_CHECK_EQUAL(meta_line.with_hash, true);
    RED_CHECK_MEM_AC(meta_line.hash1,
      "\x32\xd7\xbb\xef\x41\xa7\xc1\x53\x47\xc9\xe8\xab\xc1\xec\xe0\x3b"
      "\xbd\x23\x0a\x71\xae\xba\x5c\xe2\xa0\xf5\x10\xd6\xe6\x94\x8e\x9a");
    RED_CHECK_MEM_AC(meta_line.hash2,
      "\xf4\xbd\x5d\xfb\xbc\xc3\x0d\x9d\x30\xfa\xdf\xed\x00\xec\x81\xad"
      "\xc2\x02\x19\xdd\xff\x79\x6f\xfa\xc1\xe7\x61\xfc\x4e\x07\x0b\x2c");

    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Eof);
}

RED_AUTO_TEST_CASE(ReadEncryptedHeaderV2Checksum)
{
    CryptoContext cctx;
    cctx.set_get_hmac_key_cb(hmac_fn);
    cctx.set_get_trace_key_cb(trace_fn);
    cctx.set_master_derivator(cstr_array_view(
        "toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
    ));

    Fstat fstat;
    InCryptoTransport fd(cctx, is_encrypted, fstat);
    fd.open(FIXTURES_PATH
        "/verifier/recorded/"
        "toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm");

    MwrmReader reader(fd);

    reader.read_meta_headers();
    RED_CHECK_EQUAL(reader.get_header().version, WrmVersion::v2);
    RED_CHECK(reader.get_header().has_checksum);

    MetaLine meta_line;
    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Ok);

    RED_CHECK_EQUAL(meta_line.filename,
        "/var/wab/recorded/rdp"
        "/toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335-000000.wrm");
    RED_CHECK_EQUAL(meta_line.size, 163032);
    RED_CHECK_EQUAL(meta_line.mode, 33056);
    RED_CHECK_EQUAL(meta_line.uid, 1001);
    RED_CHECK_EQUAL(meta_line.gid, 1001);
    RED_CHECK_EQUAL(meta_line.dev, 65030);
    RED_CHECK_EQUAL(meta_line.ino, 89);
    RED_CHECK_EQUAL(meta_line.mtime, 1455816632);
    RED_CHECK_EQUAL(meta_line.ctime, 1455816632);
    RED_CHECK_EQUAL(meta_line.start_time, 1455816611);
    RED_CHECK_EQUAL(meta_line.stop_time, 1455816633);
    RED_CHECK_EQUAL(meta_line.with_hash, true);
    RED_CHECK_MEM_AC(meta_line.hash1,
      "\x05\x6c\x10\xb7\xbd\x80\xa8\x72\x87\x33\x6d\xee\x6e\x43\x1d\x81"
      "\x56\x06\xa1\xf9\xf0\xe6\x37\x12\x07\x22\xe3\x0c\x2c\x8c\xd7\x77");
    RED_CHECK_MEM_AC(meta_line.hash2,
      "\xf3\xc5\x36\x2b\xc3\x47\xf8\xb4\x4a\x1d\x91\x63\xdd\x68\xed\x99"
      "\xc1\xed\x58\xc2\xd3\x28\xd1\xa9\x4a\x07\x7d\x76\x58\xca\x66\x7c");

    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Eof);
}

RED_AUTO_TEST_CASE(ReadHashV2WithoutHash)
{
    auto const data = cstr_array_view("v2\n\n\ncgrosjean@10.10.43.12,Administrateur@local@win2008,20170830-174010,wab-5-0-4.cgrtc,6916.mwrm 222 33056 1001 1001 65030 28 1504107644 1504107644\n");

    GeneratorTransport transport(data.data(), data.size());

    MwrmReader reader(transport);

    MetaLine meta_line;
    reader.set_header({WrmVersion::v2, false});
    reader.read_meta_hash_line(meta_line);
    RED_CHECK_EQUAL(meta_line.filename,
        "cgrosjean@10.10.43.12,Administrateur@local@win2008,20170830-174010,wab-5-0-4.cgrtc,6916.mwrm");
    RED_CHECK_EQUAL(meta_line.size, 222);
    RED_CHECK_EQUAL(meta_line.mode, 33056);
    RED_CHECK_EQUAL(meta_line.uid, 1001);
    RED_CHECK_EQUAL(meta_line.gid, 1001);
    RED_CHECK_EQUAL(meta_line.dev, 65030);
    RED_CHECK_EQUAL(meta_line.ino, 28);
    RED_CHECK_EQUAL(meta_line.mtime, 1504107644);
    RED_CHECK_EQUAL(meta_line.ctime, 1504107644);
    RED_CHECK_EQUAL(meta_line.with_hash, false);

    RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Eof);

    MwrmWriterBuf writer;
    writer.write_hash_file(meta_line);
    RED_CHECK_EQUAL(writer.c_str(), data.data());
}

RED_AUTO_TEST_CASE(ReadHashV1)
{
    {
        auto data = cstr_array_view("file_xyz 0000000000000000000000000000000000000000000000000000000000000000");

        GeneratorTransport transport(data.data(), data.size());

        MwrmReader reader(transport);

        MetaLine meta_line;
        reader.set_header({WrmVersion::v1, true});
        reader.read_meta_hash_line(meta_line);
        RED_CHECK_EQUAL(meta_line.filename, "file_xyz");
        RED_CHECK_EQUAL(meta_line.with_hash, true);
        RED_CHECK_MEM_AC(meta_line.hash1,
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30");
        RED_CHECK_MEM_AC(meta_line.hash2,
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30");

        RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Eof);
    }
    {
        auto data = cstr_array_view("file_xyz  0\n000000000000000000000000000000000000000000000000000000000000\n");

        GeneratorTransport transport(data.data(), data.size());

        MwrmReader reader(transport);

        MetaLine meta_line;
        reader.set_header({WrmVersion::v1, false});
        reader.read_meta_hash_line(meta_line);
        RED_CHECK_EQUAL(meta_line.filename, "file_xyz");
        RED_CHECK_EQUAL(meta_line.with_hash, true);
        RED_CHECK_MEM_AC(meta_line.hash1,
        " \x30\n\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30");
        RED_CHECK_MEM_AC(meta_line.hash2,
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
        "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\n");

        RED_CHECK_EQUAL(reader.read_meta_line(meta_line), Transport::Read::Eof);
    }
}
