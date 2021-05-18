/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/file.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "transport/crypto_transport.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"

#include <snappy.h> // for SNAPPY_VERSION
#include <string_view>

using namespace std::string_view_literals;


static struct stat get_stat(char const* filename)
{
    class stat st;
    stat(filename, &st);
    return st;
}


namespace
{
    void init_keys(CryptoContext & cctx)
    {
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
    }

    ssize_t write_file(WorkingFile const& wf, bytes_view data)
    {
        int fd = open(wf, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU);
        auto res = write(fd, data.data(), data.size());
        close(fd);
        return res;
    }

    template<class T, std::size_t N>
    ssize_t write_file(WorkingFile const& wf, T const(&a)[N])
    {
        return write_file(wf, {a, N});
    }
} // namespace

RED_AUTO_TEST_CASE(TestEncryption1)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::cryptofile);

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    ocrypto::Result res2 = encrypter.write(cstr_array_view("toto"));
    memcpy(result + offset, res2.buf.data(), res2.buf.size());
    offset += res2.buf.size();
    RED_CHECK_EQUAL(res2.buf.size(), 0);
    RED_CHECK_EQUAL(res2.consumed, 4);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        RED_CHECK_EQUAL(res2.buf.size(), 28);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    uint8_t expected_result[68] =  { 'W', 'C', 'F', 'M', // Magic
                                       1, 0, 0, 0,       // Version
                                  // iv
                                  0xb8, 0x6c, 0xda, 0xa6, 0xf0, 0xf6, 0x30, 0x8d,
                                  0xa8, 0x16, 0xa6, 0x6e, 0xe0, 0xc3, 0xe5, 0xcc,
                                  0x98, 0x76, 0xdd, 0xf5, 0xd0, 0x26, 0x74, 0x5f,
                                  0x88, 0x4c, 0xc2, 0x50, 0xc0, 0xdf, 0xc9, 0x50,
                                  // Data
                                  0x10, 0x00, 0x00, 0x00,
                                  0x26, 0xf6, 0x39, 0x17, 0x14, 0x45, 0x7e, 0x3b,
                                  0xfa, 0xfc, 0x11, 0x8a, 0xc0, 0x92, 0xf7, 0x53,
                                  'M', 'F', 'C', 'W',    // EOF Magic
                                  0x04, 0x00, 0x00, 0x00 // Total Length of decrypted data
                                  };
    RED_CHECK(make_array_view(result, 68) == make_array_view(expected_result));

    auto expected_hash = cstr_array_view(
        "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
        "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
        "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
        "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29");
    RED_CHECK(make_array_view(qhash) == expected_hash);
    RED_CHECK(make_array_view(fhash) == expected_hash);

}

RED_AUTO_TEST_CASE_WF(TestEncryption2, wf)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[8192];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::cryptofile);

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.
    {
        ocrypto::Result res2 = encrypter.write(cstr_array_view("to"));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 2);
    }
    // This test is very similar to Encryption1, but we are performing 2 writes
    {
        ocrypto::Result res2 = encrypter.write(cstr_array_view("to"));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 2);
    }
    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 28);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    uint8_t expected_result[68] =  { 'W', 'C', 'F', 'M', // Magic
                                       1, 0, 0, 0,       // Version
                                  // iv
                                  0xb8, 0x6c, 0xda, 0xa6, 0xf0, 0xf6, 0x30, 0x8d,
                                  0xa8, 0x16, 0xa6, 0x6e, 0xe0, 0xc3, 0xe5, 0xcc,
                                  0x98, 0x76, 0xdd, 0xf5, 0xd0, 0x26, 0x74, 0x5f,
                                  0x88, 0x4c, 0xc2, 0x50, 0xc0, 0xdf, 0xc9, 0x50,
                                  // Data
                                  0x10, 0x00, 0x00, 0x00,
                                  0x26, 0xf6, 0x39, 0x17, 0x14, 0x45, 0x7e, 0x3b,
                                  0xfa, 0xfc, 0x11, 0x8a, 0xc0, 0x92, 0xf7, 0x53,
                                  'M', 'F', 'C', 'W',    // EOF Magic
                                  0x04, 0x00, 0x00, 0x00 // Total Length of decrypted data
                                  };
    RED_CHECK(make_array_view(result, offset) == make_array_view(expected_result));

    RED_TEST(write_file(wf, expected_result) == 68);

    auto expected_hash = cstr_array_view(
        "\x29\x5c\x52\xcd\xf6\x99\x92\xc3"
        "\xfe\x2f\x05\x90\x0b\x62\x92\xdd"
        "\x12\x31\x2d\x3e\x1d\x17\xd3\xfd"
        "\x8e\x9c\x3b\x52\xcd\x1d\xf7\x29");
    RED_CHECK(make_array_view(qhash) == make_array_view(expected_hash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_hash));

    char clear[12] = {};

    InCryptoTransport decrypter(cctx, InCryptoTransport::EncryptionMode::Auto);
    decrypter.open(wf, { derivator, sizeof(derivator)});
    RED_CHECK_EQUAL(Transport::Read::Ok, decrypter.atomic_read(clear, 4));
    RED_CHECK_EQUAL(decrypter.partial_read(clear+4, 1), 0);
    RED_CHECK(decrypter.is_encrypted());
    decrypter.close();

    RED_CHECK(make_array_view(clear, 4) == "toto"_av);
}

RED_AUTO_TEST_CASE(testSetEncryptionSchemeType)
{
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

        CryptoContext cctx;
        cctx.set_hmac_key(hmac_key);
        cctx.set_get_trace_key_cb(trace_20161025_fn);
        cctx.set_master_derivator(cstr_array_view(
            FIXTURES_PATH "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
            "-192304,wab-4-2-4.yourdomain,5560.mwrm"
        ));

        RED_CHECK(not cctx.old_encryption_scheme);
        RED_CHECK_EQUAL(
            get_encryption_scheme_type(cctx,
                FIXTURES_PATH "/verifier/recorded/"
                "cgrosjean@10.10.43.13,proxyuser@win2008,20161025"
                "-192304,wab-4-2-4.yourdomain,5560.mwrm",
                nullptr, nullptr),
            EncryptionSchemeTypeResult::OldScheme);
    }
    {
        // E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
        uint8_t hmac_key[] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };

        auto trace_fn = [](uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme) {
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
            static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH);
            memcpy(buffer, trace_key, sizeof(trace_key));
            return 0;
        };

        CryptoContext cctx;
        cctx.set_hmac_key(hmac_key);
        cctx.set_get_trace_key_cb(trace_fn);
        cctx.set_master_derivator(cstr_array_view(
            FIXTURES_PATH "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
        ));

        RED_CHECK_EQUAL(
            get_encryption_scheme_type(cctx,
            FIXTURES_PATH "/verifier/recorded/"
                "toto@10.10.43.13,Administrateur@QA@cible,"
                "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
                nullptr, nullptr),
            EncryptionSchemeTypeResult::NewScheme);
    }
    {
        CryptoContext cctx;
        RED_CHECK_EQUAL(
            get_encryption_scheme_type(cctx, FIXTURES_PATH "/sample.txt",  nullptr, nullptr),
            EncryptionSchemeTypeResult::NoEncrypted);
    }
    {
        CryptoContext cctx;
        RED_CHECK_EQUAL(
            get_encryption_scheme_type(cctx, FIXTURES_PATH "/blogiblounga", nullptr, nullptr),
            EncryptionSchemeTypeResult::Error);
        RED_CHECK_EQUAL(errno, ENOENT);
    }
}

// This sample was generated using udevrandom on Linux
static uint8_t randomSample[8192] = {
#include "fixtures/randomdata.hpp"
};

RED_AUTO_TEST_CASE(TestEncryptionLarge1)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384] = {};
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::cryptofile);

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    memcpy(result + offset, res.buf.data(), res.buf.size());
    offset += res.buf.size();
    RED_CHECK_EQUAL(res.buf.size(), 40);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
    }

    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 8612);
    }

    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, make_array_view(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH];
    unsigned char fhash[MD_HASH::DIGEST_LENGTH];
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 8);
        RED_CHECK_EQUAL(res2.consumed, 0);

    }
    RED_CHECK_EQUAL(offset, 8660);

    WorkingFile wf("tmp.enc");
    RED_TEST(write_file(wf, array_view{&result[0], offset}) == 8660);

    char clear[sizeof(randomSample)+sizeof(randomSample)] = {};

    InCryptoTransport decrypter(cctx, InCryptoTransport::EncryptionMode::Encrypted);
    decrypter.open(wf, {derivator, sizeof(derivator)});
    RED_CHECK_EQUAL(Transport::Read::Ok, decrypter.atomic_read(clear, sizeof(clear)));
    RED_CHECK_EQUAL(0, decrypter.partial_read(clear, 1));
    decrypter.close();


    RED_CHECK(make_array_view(clear, sizeof(randomSample)) == make_array_view(randomSample));
    RED_CHECK(make_array_view(clear).from_offset(sizeof(randomSample)) == make_array_view(randomSample));


    unsigned char fhash2[MD_HASH::DIGEST_LENGTH];

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(make_array_view(cctx.get_hmac_key()));
    hmac.update({result, offset});
    hmac.final(fhash2);

    InCryptoTransport::HASH fh;
    RED_TEST(InCryptoTransport::read_fhash(wf.c_str(), cctx.get_hmac_key(), fh));
    RED_CHECK(make_array_view(fh.hash) == make_array_view(fhash));
    RED_CHECK(make_array_view(fh.hash) == make_array_view(fhash2));


    unsigned char qhash2[MD_HASH::DIGEST_LENGTH];

    SslHMAC_Sha256_Delayed hmac2;
    hmac2.init(make_array_view(cctx.get_hmac_key()));
    hmac2.update({result, 4096});
    hmac2.final(qhash2);

    #if SNAPPY_VERSION < (1<<16|1<<8|4)
        auto expected_qhash = cstr_array_view(
            "\x88\x80\x2e\x37\x08\xca\x43\x30\xed\xd2\x72\x27\x2d\x05\x5d\xee"
            "\x01\x71\x4a\x12\xa5\xd9\x72\x84\xec\x0e\xd5\xaa\x47\x9e\xc3\xc2"
            );

        auto expected_fhash = cstr_array_view(
            "\x62\x96\xe9\xa2\x20\x4f\x39\x21\x06\x4d\x1a\xcf\xf8\x6e\x34\x9c"
            "\xd6\xae\x6c\x44\xd4\x55\x57\xd5\x29\x04\xde\x58\x7f\x1d\x0b\x35"
            );
    #else
        auto expected_qhash = cstr_array_view(
            "\xdf\xd9\xf0\xcc\x20\x77\x38\xd4\x55\x44\x9f\xf0\xce\x6f\xf6\xd1"
            "\x62\x16\x0e\xbf\x76\xa9\x26\x4d\xa9\xd3\x40\x22\x13\xbd\x10\x2a"
            );

        auto expected_fhash = cstr_array_view(
            "\xcb\xfe\x7b\x9a\xe6\x69\x80\x4a\xf8\xc8\x28\x68\xfd\xef\x18\x11"
            "\x22\x27\xce\xb1\xb6\x1c\xac\xe9\x1b\x04\x41\x23\xd6\xed\x75\x49"
            );
    #endif

    RED_CHECK(make_array_view(qhash) == make_array_view(expected_qhash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_fhash));
    RED_CHECK(make_array_view(qhash2) == make_array_view(expected_qhash));
    RED_CHECK(make_array_view(fhash2) == make_array_view(expected_fhash));
}

RED_AUTO_TEST_CASE(TestEncryptionLargeNoEncryptionChecksum)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::localfile_hashed);

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, make_array_view(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    unsigned char qhash[MD_HASH::DIGEST_LENGTH] {};
    unsigned char fhash[MD_HASH::DIGEST_LENGTH] {};
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }
    RED_CHECK_EQUAL(offset, sizeof(randomSample)*2);

    auto expected_qhash = cstr_array_view(
        "\x73\xe8\x21\x3a\x8f\xa3\x61\x0e\x0f\xfe\x14\x28\xff\xcd\x1d\x97"
        "\x7f\xc8\xe8\x90\x44\xfc\x4f\x75\xf7\x6c\xa3\x5b\x0d\x2e\x14\x80");
    auto expected_fhash = cstr_array_view(
        "\x07\xa7\xe7\x14\x9b\xf7\xeb\x34\x57\xdc\xce\x07\x5c\x62\x61\x34"
        "\x51\x42\x7d\xe0\x0f\xbe\xda\x53\x11\x08\x75\x31\x40\xc5\x50\xe8");

    RED_CHECK(make_array_view(qhash) == make_array_view(expected_qhash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_fhash));

    uint8_t qhash2[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash2[MD_HASH::DIGEST_LENGTH] {};

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(make_array_view(cctx.get_hmac_key()));
    hmac.update(make_array_view(randomSample));
    hmac.update(make_array_view(randomSample));
    hmac.final(fhash2);

    SslHMAC_Sha256_Delayed quick_hmac;
    quick_hmac.init(make_array_view(cctx.get_hmac_key()));
    quick_hmac.update({randomSample, 4096});
    quick_hmac.final(qhash2);

    RED_CHECK(make_array_view(fhash2) == make_array_view(expected_fhash));
    // "\x73\xe8\x21\x3a\x8f\xa3\x61\x0e\x0f\xfe\x14\x28\xff\xcd\x1d\x97\x7f\xc8\xe8\x90\x44\xfc\x4f\x75\xf7\x6c\xa3\x5b\x0d\x2e\x14\x80"
    RED_CHECK(make_array_view(qhash2) == make_array_view(expected_qhash));
}

RED_AUTO_TEST_CASE(TestEncryptionLargeNoEncryption)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::localfile);

    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a large block of pseudo random data
    // with that kind of data I expect poor compression results
    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }

    {
        ocrypto::Result res2 = encrypter.write(make_array_view(randomSample));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), sizeof(randomSample));
    }


    // I write the same block *again* now I should reach some compression
//    size_t towrite = 0;
//    encrypter.write(result+offset, sizeof(result)-offset, towrite, make_array_view(randomSample));
//    offset += towrite;
//    RED_CHECK_EQUAL(towrite, 8612);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    uint8_t expected_qhash[MD_HASH::DIGEST_LENGTH] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    uint8_t expected_fhash[MD_HASH::DIGEST_LENGTH] = {
        0x10, 0x01, 0x12, 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09,0x1A, 0x0B, 0x1C, 0x0D, 0x1E, 0x0F,
        0x10, 0x01, 0x12, 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09,0x1A, 0x0B, 0x1C, 0x0D, 0x0E, 0x0F
    };

    uint8_t qhash[MD_HASH::DIGEST_LENGTH];
    ::memcpy(qhash, expected_qhash, MD_HASH::DIGEST_LENGTH);
    uint8_t fhash[MD_HASH::DIGEST_LENGTH];
    ::memcpy(fhash, expected_fhash, MD_HASH::DIGEST_LENGTH);
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }
    RED_CHECK_EQUAL(offset, sizeof(randomSample)*2);

    // Check qhash and fhash are left unchanged if no checksum is enabled
    RED_CHECK(make_array_view(qhash) == make_array_view(expected_qhash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_fhash));
}

RED_AUTO_TEST_CASE(TestEncryptionSmallNoEncryptionChecksum)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t result[16384];
    size_t offset = 0;
    uint8_t derivator[] = { 'A', 'B', 'C', 'D' };

    cctx.set_trace_type(TraceType::localfile_hashed);
    ocrypto encrypter(cctx, rnd);
    // Opening an encrypted stream usually results in some header put in result buffer
    // Of course no such header will be needed in non encrypted files
    ocrypto::Result res = encrypter.open(make_array_view(derivator));
    RED_CHECK_EQUAL(res.buf.size(), 0);

    // writing data to compressed/encrypted buffer may result in data to write
    // ... or not as this writing may be differed.

    // Let's send a small block of data
    {
        uint8_t data[5] = {1, 2, 3, 4, 5};
        ocrypto::Result res2 = encrypter.write(make_array_view(data));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 5);
    }

    // Let's send a small block of data
    {
        uint8_t data[5] = {1, 2, 3, 4, 5};
        ocrypto::Result res2 = encrypter.write(make_array_view(data));
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        offset += res2.buf.size();
        RED_CHECK_EQUAL(res2.buf.size(), 5);
    }

    RED_CHECK_EQUAL(offset, 10);

    // close flushes all opened buffers and writes potential trailer
    // the full file hash is also returned which is made of two parts
    // a partial hash for the first 4K of the file
    // and a full hash for the whole file
    // obviously the two will be identical for short files
    // and differs for larger ones
    uint8_t qhash[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH] {};
    {
        ocrypto::Result res2 = encrypter.close(qhash, fhash);
        memcpy(result + offset, res2.buf.data(), res2.buf.size());
        RED_CHECK_EQUAL(res2.buf.size(), 0);
        RED_CHECK_EQUAL(res2.consumed, 0);
    }

    auto expected_qhash = cstr_array_view(
        "\x3b\x79\xd5\x76\x98\x66\x4f\xe1\xdd\xd4\x90\x5b\xa5\x56\x6a\xa3"
        "\x14\x45\x5e\xf3\x8c\x04\xc4\xc4\x49\x6b\x00\xd4\x5e\x82\x13\x68");
    auto expected_fhash = cstr_array_view(
        "\x3b\x79\xd5\x76\x98\x66\x4f\xe1\xdd\xd4\x90\x5b\xa5\x56\x6a\xa3\x14"
        "\x45\x5e\xf3\x8c\x04\xc4\xc4\x49\x6b\x00\xd4\x5e\x82\x13\x68");

    RED_CHECK(make_array_view(qhash) == make_array_view(expected_qhash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_fhash));

    uint8_t qhash2[MD_HASH::DIGEST_LENGTH] {};
    uint8_t fhash2[MD_HASH::DIGEST_LENGTH] {};

    SslHMAC_Sha256_Delayed hmac;
    hmac.init(make_array_view(cctx.get_hmac_key()));
    uint8_t data[5] = {1, 2, 3, 4, 5};
    hmac.update(make_array_view(data));
    hmac.update(make_array_view(data));
    hmac.final(fhash2);

    SslHMAC_Sha256_Delayed quick_hmac;
    quick_hmac.init(make_array_view(cctx.get_hmac_key()));
    quick_hmac.update(make_array_view(data));
    quick_hmac.update(make_array_view(data));
    quick_hmac.final(qhash2);

    RED_CHECK(make_array_view(fhash2) == make_array_view(expected_fhash));
    RED_CHECK(make_array_view(qhash2) == make_array_view(expected_qhash));
}


RED_AUTO_TEST_CASE(TestRdpLogonInfo)
{
    RED_TEST_DATAS(
        (TraceType::cryptofile,
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5"_av)    (TraceType::localfile_hashed,
        "\xc5\x28\xb4\x74\x84\x3d\x8b\x14\xcf\x5b\xf4\x3a\x9c\x04\x9a\xf3"
        "\x23\x9f\xac\x56\x4d\x86\xb4\x32\x90\x69\xb5\xe1\x45\xd0\x76\x9b"_av)
        (TraceType::localfile,
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"_av)
    ) >>= [&](TraceType trace_type, bytes_view hash){
        uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
        uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

        LCGRandom rnd;
        CryptoContext cctx;
        init_keys(cctx);

        WorkingDirectory wd;

        auto finalname = wd.add_file("encrypted.txt");
        auto hash_finalname = wd.add_file("hash_encrypted.txt");

        {
            cctx.set_trace_type(trace_type);

            OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
            ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
            ct.send("We write, ", 10);
            ct.send("and again, ", 11);
            ct.send("and so on.", 10);
            ct.close(qhash, fhash);
        }

        RED_CHECK(make_array_view(fhash) == make_array_view(qhash));
        RED_CHECK(make_array_view(qhash) == hash);

        RED_CHECK_WORKSPACE(wd);
    };
}


RED_AUTO_TEST_CASE_WD(TestOutCryptoTransportBigFile, wd)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    auto finalname = wd.add_file("encrypted.txt");
    auto hash_finalname = wd.add_file("hash_encrypted.txt");
    {
        cctx.set_trace_type(TraceType::cryptofile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        char buf[200000]{};
        ct.send(buf, sizeof(buf));
        ct.close(qhash, fhash);
    }

    RED_CHECK(
        make_array_view(qhash) ==
        "\x39\xcd\x15\x84\x07\x35\x55\xf3\x9b\x45\xc7\xb2\xdd\x06\xa1\x0f"
        "\xd0\x9d\x44\xdd\xcd\x40\x49\x74\x14\xec\x72\x59\xa9\x7b\x7f\x81"_av
    );

    RED_CHECK(
        make_array_view(fhash) ==
        "\xc2\x55\x50\xf3\xcd\x56\xf3\xb9\x26\x37\x06\x9a\x3b\xb1\x26\xd6"
        "\x84\xfd\x6c\xac\x15\xc1\x76\x92\x2f\x16\xc0\xe3\x19\xce\xd0\xe4"_av
    );
}

RED_AUTO_TEST_CASE_WD(TestOutCryptoTransportAutoClose, wd)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    auto finalname = wd.add_file("encrypted.txt");
    auto hash_finalname = wd.add_file("hash_encrypted.txt");

    cctx.set_trace_type(TraceType::cryptofile);

    OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
    ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
    ct.send("We write, and again, and so on.", 31);

    // if there is no explicit close we can't get hash values
    // but the file is correctly closed and ressources freed
}

RED_AUTO_TEST_CASE_WD(TestOutCryptoTransportMultipleFiles, wd)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    auto finalname1 = wd.add_file("encrypted001.txt");
    auto finalname2 = wd.add_file("encrypted002.txt");
    auto hash_finalname1 = wd.add_file("hash_encrypted001.txt");
    auto hash_finalname2 = wd.add_file("hash_encrypted002.txt");

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
    {
        cctx.set_trace_type(TraceType::cryptofile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});

        ct.open(finalname1, hash_finalname1, 0, FilePermissions(0777));
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);

        ct.open(finalname2, hash_finalname2, 0, FilePermissions(0777));
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);
    }
}


using Read = Transport::Read;

RED_AUTO_TEST_CASE(TestInCryptoTransportClearText)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    WorkingDirectory wd;

    auto finalname = wd.add_file("clear.txt");
    auto hash_finalname = wd.add_file("hash_clear.txt");

    {
        cctx.set_trace_type(TraceType::localfile_hashed);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        ct.send("We write, and again, and so on.", 31);
        ct.close(qhash, fhash);
    }

    RED_CHECK_WORKSPACE(wd);

    auto expected_hash =
        "\xc5\x28\xb4\x74\x84\x3d\x8b\x14\xcf\x5b\xf4\x3a\x9c\x04\x9a\xf3"
        "\x23\x9f\xac\x56\x4d\x86\xb4\x32\x90\x69\xb5\xe1\x45\xd0\x76\x9b"_av;

    RED_CHECK(make_array_view(qhash) == expected_hash);
    RED_CHECK(make_array_view(fhash) == expected_hash);

    {
        char buffer[40];
        InCryptoTransport ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK(ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[31], 1));
        RED_CHECK(ct.is_eof());
        ct.close();
        RED_CHECK(make_array_view(buffer, 31) == "We write, and again, and so on."_av);
        // close followed by open
        ct.open(finalname);
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK(ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[31], 1));
        RED_CHECK(ct.is_eof());
        ct.close();
        RED_CHECK(make_array_view(buffer, 31) == "We write, and again, and so on."_av);

        InCryptoTransport::HASH fh;
        InCryptoTransport::HASH qh;
        RED_TEST(InCryptoTransport::read_fhash(finalname, cctx.get_hmac_key(), fh));
        RED_TEST(InCryptoTransport::read_qhash(finalname, cctx.get_hmac_key(), qh));
        RED_CHECK(make_array_view(qh.hash) == expected_hash);
        RED_CHECK(make_array_view(fh.hash) == expected_hash);

        auto st = get_stat(finalname);
        RED_CHECK_FILE_CONTENTS(hash_finalname, str_concat(
            "v2\n\n\nclear.txt 31 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec),
            " c528b474843d8b14cf5bf43a9c049af3239fac564d86b4329069b5e145d0769b"
            " c528b474843d8b14cf5bf43a9c049af3239fac564d86b4329069b5e145d0769b\n"));
    }

    RED_CHECK_WORKSPACE(wd);
}


RED_AUTO_TEST_CASE(TestInCryptoTransportBigCrypted)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    WorkingDirectory wd;

    auto finalname = wd.add_file("encrypted.txt");
    auto hash_finalname = wd.add_file("hash_encrypted.txt");
    {
        cctx.set_trace_type(TraceType::cryptofile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        ct.send(make_array_view(randomSample));
        ct.close(qhash, fhash);
    }

    RED_CHECK_WORKSPACE(wd);

    auto expected_qhash =
        "\x04\x52\x16\x50\xdb\x48\xe6\x70\x36\x3c\x68\xa9\xcd\xdb\xeb\x60"
        "\xf9\x25\x83\xbc\x0d\x2e\x09\x3f\xf2\xc9\x37\x5d\xa6\x9d\x7a\xf0"_av;

    auto expected_fhash =
        "\xa8\x7c\x51\x79\xe2\xcc\x2c\xe3\x51\x64\x40\xc0\xb0\xbd\xa8\x99"
        "\xcc\x46\xac\x42\x3f\x22\x0f\x64\x50\xbb\xbb\x7c\x45\xb8\x1c\xc4"_av;

    RED_CHECK(make_array_view(qhash) == expected_qhash);
    RED_CHECK(make_array_view(fhash) == expected_fhash);

    {
        char buffer[sizeof(randomSample)];
        InCryptoTransport ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK(ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, sizeof(buffer)-10));
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[sizeof(buffer)-10], 10));
        RED_CHECK(ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[sizeof(buffer)], 1));
        RED_CHECK(ct.is_eof());
        ct.close();
        RED_CHECK(make_array_view(buffer) == make_array_view(randomSample));

        InCryptoTransport::HASH fh;
        InCryptoTransport::HASH qh;
        RED_TEST(InCryptoTransport::read_fhash(finalname, cctx.get_hmac_key(), fh));
        RED_TEST(InCryptoTransport::read_qhash(finalname, cctx.get_hmac_key(), qh));
        RED_CHECK(make_array_view(qh.hash) == expected_qhash);
        RED_CHECK(make_array_view(fh.hash) == expected_fhash);
    }
    {
        char hash_buf[512];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(hash_finalname, cstr_array_view("encrypted.txt"));
        RED_CHECK(ct.is_encrypted());
        auto len = ct.partial_read(hash_buf, sizeof(hash_buf));
        hash_buf[len] = '\0';
        auto st = get_stat(finalname);
        RED_CHECK(hash_buf == str_concat(
            "v2\n\n\nencrypted.txt 8260 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec),
            " 04521650db48e670363c68a9cddbeb60f92583bc0d2e093ff2c9375da69d7af0"
            " a87c5179e2cc2ce3516440c0b0bda899cc46ac423f220f6450bbbb7c45b81cc4\n"));
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestInCryptoTransportCrypted)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    WorkingDirectory wd;

    auto finalname = wd.add_file("encrypted.txt");
    auto hash_finalname = wd.add_file("hash_encrypted.txt");
    {
        cctx.set_trace_type(TraceType::cryptofile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    RED_CHECK_WORKSPACE(wd);

    auto expected_hash = cstr_array_view(
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5");

    RED_CHECK(make_array_view(qhash) == make_array_view(expected_hash));
    RED_CHECK(make_array_view(fhash) == make_array_view(expected_hash));

    {
        char buffer[40];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK(ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, 30));
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[30], 1));
        RED_CHECK(ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[30], 1));
        ct.close();
        RED_CHECK(make_array_view(buffer, 31) == "We write, and again, and so on."_av);

        InCryptoTransport::HASH fh;
        InCryptoTransport::HASH qh;
        RED_TEST(InCryptoTransport::read_fhash(finalname, cctx.get_hmac_key(), fh));
        RED_TEST(InCryptoTransport::read_qhash(finalname, cctx.get_hmac_key(), qh));
        RED_CHECK(make_array_view(qh.hash) == make_array_view(qhash));
        RED_CHECK(make_array_view(fh.hash) == make_array_view(fhash));
    }
    {
        char hash_buf[512];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(hash_finalname, cstr_array_view("encrypted.txt"));
        auto len = ct.partial_read(hash_buf, sizeof(hash_buf));
        hash_buf[len] = '\0';
        auto st = get_stat(finalname);
        RED_CHECK(hash_buf == str_concat(
            "v2\n\n\nencrypted.txt 100 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec),
            " 2acc1e2cbffe64030d50eae7845a9dce6ec4e84ac2435f6c0f7f16f87b0180f5"
            " 2acc1e2cbffe64030d50eae7845a9dce6ec4e84ac2435f6c0f7f16f87b0180f5\n"));
    }

    RED_CHECK_WORKSPACE(wd);
}

static char clearSample[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas posuere, libero nec auctor auctor, leo sapien consequat nulla, non condimentum diam mauris eget ante. Nam in commodo elit, vel pulvinar risus. Sed commodo nisl aliquet nisl varius, non mollis ex sodales. Duis eu pellentesque odio, ac tincidunt libero. Aenean quam risus, lacinia non interdum ac, interdum ut neque. Morbi eget justo posuere, commodo erat et, elementum nisl. Nunc in tincidunt libero. Sed gravida purus in dolor iaculis dictum. Vivamus vel dolor eu purus scelerisque convallis lacinia vitae lacus. Nulla facilisi. Aliquam urna leo, facilisis id ipsum ac, rhoncus vulputate nibh. Nulla vitae aliquet lectus, ac finibus nulla. Suspendisse ut risus porttitor, rhoncus massa id, feugiat felis.\n\n"

"Suspendisse molestie erat et nibh efficitur varius. Nulla nec eros pulvinar, condimentum odio a, fermentum lorem. Vivamus sit amet sapien pulvinar, vehicula purus pellentesque, lobortis nisl. Curabitur orci metus, ornare a lobortis vitae, aliquam at risus. Phasellus tristique lacus quis vestibulum lacinia. Aenean interdum erat non odio semper, vitae ultricies ipsum mollis. Integer dapibus ultrices eros, elementum posuere massa sollicitudin sit amet. Phasellus quis diam imperdiet, tempor tortor eget, ultrices purus. Suspendisse semper at justo nec pharetra. Sed est velit, egestas at justo vitae, varius mattis massa. Donec efficitur orci et elementum consectetur. In malesuada molestie quam mattis egestas. Nulla dictum massa a metus feugiat, sed maximus metus aliquam. Phasellus sem lacus, dictum vel vestibulum cursus, elementum id dui.\n\n"

"Nulla sed ullamcorper massa, dignissim porttitor nunc. Nulla facilisi. Aliquam tincidunt, nulla fringilla tincidunt tincidunt, nulla magna accumsan velit, nec maximus elit risus sed justo. Donec tincidunt eget est vel ultricies. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nunc semper, nulla a eleifend feugiat, ex enim mattis felis, ut dapibus mi massa ac libero. Sed sit amet dignissim nisl, eget fermentum ex. In hac habitasse platea dictumst. Pellentesque tempor nisi quis laoreet euismod. Proin viverra nisi justo, id finibus massa bibendum id. Donec sit amet congue ex. Suspendisse eu nisi ligula. Aliquam mattis orci pellentesque elit laoreet interdum. Integer tincidunt sem nec lacus sagittis varius.\n\n"

"Nunc at sem in odio interdum placerat. Integer pulvinar, velit vitae euismod volutpat, turpis elit lobortis nibh, in ornare lorem velit vel orci. Integer sem odio, aliquam id vulputate non, dapibus sed est. Vivamus porta sapien leo, vulputate lobortis lacus commodo non. Aliquam fermentum maximus convallis. Sed vestibulum, massa et vestibulum interdum, tortor leo placerat quam, eu blandit neque mi sit amet elit. Etiam consequat dapibus augue sed ultrices. Proin eget massa id massa cursus mollis. Aenean non quam iaculis, consectetur justo rutrum, ullamcorper velit. Donec blandit leo quis sapien vehicula egestas. Fusce dignissim erat in nisi blandit sollicitudin et vitae neque. Nulla vitae dolor dolor. Donec scelerisque velit at purus auctor feugiat. Ut urna ante, lobortis ut placerat nec, scelerisque sit amet dui. Sed scelerisque, lorem et gravida tristique, elit nisl pretium eros, et hendrerit neque neque quis turpis.\n\n"

"Vestibulum id arcu in neque accumsan posuere. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut at tincidunt elit. Sed risus quam, rhoncus vitae venenatis interdum, auctor ut lectus. Nulla sit amet arcu sit amet dolor gravida cursus. Nulla nibh lacus, egestas sed lacus at, interdum auctor risus. Maecenas at ultrices orci. Praesent lacinia erat vitae sem posuere cursus. Ut commodo lorem in venenatis feugiat. Suspendisse semper neque vel sapien congue finibus. Donec eget massa imperdiet, posuere elit vitae, consectetur est. Integer dictum finibus mauris. Proin tempor ex eget enim fermentum, scelerisque aliquet enim vestibulum. Fusce justo felis, feugiat ac lobortis ut, euismod non felis.\n\n";

RED_AUTO_TEST_CASE(TestInCryptoTransportBigClear)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    WorkingDirectory wd;

    auto finalname = wd.add_file("clear.txt");
    auto hash_finalname = wd.add_file("hash_clear.txt");
    {
        cctx.set_trace_type(TraceType::localfile_hashed);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        ct.send(clearSample, sizeof(clearSample));
        ct.close(qhash, fhash);
    }

    RED_CHECK_WORKSPACE(wd);

    {
        char buffer[sizeof(clearSample)];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK(not ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, sizeof(buffer)-10));
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(&buffer[sizeof(buffer)-10], 10));
        RED_CHECK(ct.is_eof());
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(&buffer[sizeof(buffer)], 1));
        RED_CHECK(ct.is_eof());
        ct.close();
        RED_CHECK(make_array_view(buffer) == make_array_view(clearSample));

        auto expected_qhash =
            "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04"
            "\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"_av;

        auto expected_fhash =
            "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04"
            "\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"_av;
        RED_CHECK(make_array_view(qhash) == expected_qhash);
        RED_CHECK(make_array_view(fhash) == expected_fhash);

        InCryptoTransport::HASH fh;
        InCryptoTransport::HASH qh;
        RED_TEST(InCryptoTransport::read_fhash(finalname, cctx.get_hmac_key(), fh));
        RED_TEST(InCryptoTransport::read_qhash(finalname, cctx.get_hmac_key(), qh));
        RED_CHECK(make_array_view(qh.hash) == expected_qhash);
        RED_CHECK(make_array_view(fh.hash) == expected_fhash);

        auto st = get_stat(finalname);
        RED_CHECK_FILE_CONTENTS(hash_finalname, str_concat(
            "v2\n\n\nclear.txt 4047 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec),
            " cdbbf7cc04848d8729af68cb696fb104082dc6f0c0c099a0d978323b1f203f5b"
            " cdbbf7cc04848d8729af68cb696fb104082dc6f0c0c099a0d978323b1f203f5b\n"));
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestInCryptoTransportBigClearPartialRead)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    WorkingDirectory wd;

    auto finalname = wd.add_file("clear.txt");
    auto hash_finalname = wd.add_file("hash_clear.txt");

    {
        cctx.set_trace_type(TraceType::localfile_hashed);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(finalname, hash_finalname, 0, FilePermissions(0777));
        ct.send(clearSample, sizeof(clearSample));
        ct.close(qhash, fhash);
    }

    RED_CHECK_WORKSPACE(wd);

    auto expected_qhash =
        "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04"
        "\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"_av;

    auto expected_fhash =
        "\xcd\xbb\xf7\xcc\x04\x84\x8d\x87\x29\xaf\x68\xcb\x69\x6f\xb1\x04"
        "\x08\x2d\xc6\xf0\xc0\xc0\x99\xa0\xd9\x78\x32\x3b\x1f\x20\x3f\x5b"_av;

    RED_CHECK(make_array_view(qhash) == expected_qhash);
    RED_CHECK(make_array_view(fhash) == expected_fhash);

    {
        char buffer[sizeof(clearSample)];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(finalname);
        RED_CHECK(not ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(20, ct.partial_read(buffer, 20));
        RED_CHECK_EQUAL(100, ct.partial_read(&buffer[20], 100));
        // At end of file partial_read should return what it can
        RED_CHECK_EQUAL(sizeof(buffer)-120, ct.partial_read(&buffer[120], sizeof(buffer)));


        ct.close();
        RED_CHECK(make_array_view(buffer) == make_array_view(clearSample));

        InCryptoTransport::HASH fh;
        InCryptoTransport::HASH qh;
        RED_TEST(InCryptoTransport::read_fhash(finalname, cctx.get_hmac_key(), fh));
        RED_TEST(InCryptoTransport::read_qhash(finalname, cctx.get_hmac_key(), qh));
        RED_CHECK(make_array_view(qh.hash) == expected_qhash);
        RED_CHECK(make_array_view(fh.hash) == expected_fhash);

        auto st = get_stat(finalname);
        RED_CHECK_FILE_CONTENTS(hash_finalname, str_concat(
            "v2\n\n\nclear.txt 4047 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec),
            " cdbbf7cc04848d8729af68cb696fb104082dc6f0c0c099a0d978323b1f203f5b"
            " cdbbf7cc04848d8729af68cb696fb104082dc6f0c0c099a0d978323b1f203f5b\n"));
    }

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE(TestInCryptoTransportBigRead)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    WorkingDirectory wd;

    const char * original_filename = FIXTURES_PATH "/dump_TLSw2008.txt";
    auto encrypted_file = wd.add_file("encrypted_file.enc");
    auto hash_encrypted_file = wd.add_file("hash_encrypted_file.enc");

    constexpr std::size_t original_filesize = 4166665;
    auto original_contents = RED_REQUIRE_GET_FILE_CONTENTS(original_filename);
    RED_CHECK_EQUAL(original_contents.size(), original_filesize);

    {
        cctx.set_trace_type(TraceType::localfile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(encrypted_file, hash_encrypted_file, 0, FilePermissions(0777));
        ct.send(original_contents);
    }

    char buffer[original_filesize];
    {
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(encrypted_file);
        RED_CHECK(not ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, original_filesize));
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(buffer, 1));
        ct.close();
    }
    {
        char hash_buf[512];
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(hash_encrypted_file, cstr_array_view("encrypted_file.enc"));
        RED_CHECK(not ct.is_encrypted());
        auto len = ct.partial_read(hash_buf, sizeof(hash_buf));
        hash_buf[len] = 0;
        ct.close();
        auto st = get_stat(encrypted_file);
        RED_CHECK(hash_buf == str_concat(
            "v2\n\n\nencrypted_file.enc 4166665 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec), '\n'));
    }
    RED_CHECK(make_array_view(buffer) == original_contents);

    RED_CHECK_WORKSPACE(wd);
}

RED_AUTO_TEST_CASE_WD(TestInCryptoTransportBigReadEncrypted, wd)
{
    LCGRandom rnd;
    CryptoContext cctx;
    init_keys(cctx);

    const char * original_filename = FIXTURES_PATH "/dump_TLSw2008.txt";
    auto encrypted_file = wd.add_file("encrypted_file.enc");
    auto hash_encrypted_file = wd.add_file("hash_encrypted_file.enc");

    constexpr std::size_t original_filesize = 4166665;
    auto original_contents = RED_REQUIRE_GET_FILE_CONTENTS(original_filename);
    RED_REQUIRE_EQUAL(original_contents.size(), original_filesize);

    uint8_t qhash[MD_HASH::DIGEST_LENGTH] = {};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH] = {};
    {
        cctx.set_trace_type(TraceType::cryptofile);

        OutCryptoTransport ct(cctx, rnd, [](const Error & /*error*/){});
        ct.open(encrypted_file, hash_encrypted_file, 0, FilePermissions(0777));
        ct.send(original_contents);
        ct.close(qhash, fhash);
    }

    char buffer[original_filesize];
    {
        InCryptoTransport  ct(cctx, InCryptoTransport::EncryptionMode::Auto);
        ct.open(encrypted_file);
        RED_CHECK(ct.is_encrypted());
        RED_CHECK(not ct.is_eof());
        RED_CHECK_EQUAL(Read::Ok, ct.atomic_read(buffer, original_filesize));
        RED_CHECK_EQUAL(Read::Eof, ct.atomic_read(buffer, 1));
        ct.close();

        InCryptoTransport::HASH fhash2;
        InCryptoTransport::HASH qhash2;
        RED_TEST(InCryptoTransport::read_fhash(encrypted_file, cctx.get_hmac_key(), fhash2));
        RED_TEST(InCryptoTransport::read_qhash(encrypted_file, cctx.get_hmac_key(), qhash2));

        RED_CHECK(make_array_view(qhash2.hash) == make_array_view(qhash));
        RED_CHECK(make_array_view(qhash) == make_array_view(qhash2.hash));
        RED_CHECK(make_array_view(fhash2.hash) == make_array_view(fhash));
        RED_CHECK(make_array_view(fhash) == make_array_view(fhash2.hash));

        char hash_buf[512];
        ct.open(hash_encrypted_file, cstr_array_view("encrypted_file.enc"));
        RED_CHECK(ct.is_encrypted());
        auto len = ct.partial_read(hash_buf, sizeof(hash_buf));
        hash_buf[len] = 0;
        ct.close();

        auto keys =
        #if SNAPPY_VERSION < (1<<16|1<<8|3)
            " 7cf2107dfde3165f62df78a4f52b0b4cd8c19d4944fd1fe35e333c89fc5fd437"
            " 91886e9e6df928de5de87658a40a21db4afc84f4bfb2f81cc83e42ed42b25960\n"_av
        #elif SNAPPY_VERSION < (1<<16|1<<8|4)
            " 95ac075e238b5a331242efce2852cff0d475ecdaf75d4b315488e298916820d6"
            " 27fa17cac1feda21f9b134d503e6292548a35925f0137474d896a3461b777b5a\n"_av
        #else
            " 95ac075e238b5a331242efce2852cff0d475ecdaf75d4b315488e298916820d6"
            " f5b6a73d68ac7405d988bbb60a88afd59b72a47bab2e03068573e7510451e801\n"_av
        #endif
        ;
        auto st = get_stat(encrypted_file);
        RED_CHECK(hash_buf == str_concat(
            "v2\n\n\nencrypted_file.enc 1143180 ",
            int_to_decimal_chars(st.st_mode), ' ',
            int_to_decimal_chars(st.st_uid), ' ',
            int_to_decimal_chars(st.st_gid), ' ',
            int_to_decimal_chars(st.st_dev), ' ',
            int_to_decimal_chars(st.st_ino), ' ',
            int_to_decimal_chars(st.st_mtim.tv_sec), ' ',
            int_to_decimal_chars(st.st_ctim.tv_sec), keys
        ));
    }

    RED_CHECK(make_array_view(buffer, original_filesize) == original_contents);
}
