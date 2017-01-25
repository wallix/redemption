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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOutmetaTransport
#include "system/redemption_unit_tests.hpp"

#define LOGNULL


#include "capture/capture.hpp"
#include "utils/fileutils.hpp"


BOOST_AUTO_TEST_CASE(TestOutmetaTransport)
{
    unsigned sec_start = 1352304810;
    {
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;

    const char * meta_path = "./xxx.mwrm";
    const char * meta_hash_path = "./hash-xxx.mwrm";
    meta_len_writer.len = 5; // header
    struct stat stat;
    BOOST_CHECK(!::stat(meta_path, &stat));
    BOOST_CHECK(!detail::write_meta_file_impl<false>(meta_len_writer, meta_path + 2, stat, 0, 0, nullptr));
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_hash_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_hash_path));


    meta_len_writer.len = 0;

    detail::write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, false);

    const char * file1 = "./xxx-000000.wrm";
    BOOST_CHECK(!detail::write_meta_file(meta_len_writer, file1, sec_start, sec_start+1));
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

    const char * file2 = "./xxx-000001.wrm";
    BOOST_CHECK(!detail::write_meta_file(meta_len_writer, file2, sec_start, sec_start+1));
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}


BOOST_AUTO_TEST_CASE(TestOutmetaTransportWithSum)
{
    unsigned sec_start = 1352304810;
    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
            "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        OutMetaSequenceTransportWithSum wrm_trans(cctx, "./", "./", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const *, size_t len) {
            this->len += len;
            return len;
        }
    } meta_len_writer;
    detail::write_meta_headers(meta_len_writer, nullptr, 800, 600, nullptr, true);

    const unsigned hash_size = (1 + MD_HASH_LENGTH*2) * 2;

//    char file1[1024];
//    snprintf(file1, 1024, "./xxx-%06u-%06u.wrm", getpid(), 0);
    const char * file1 = "./xxx-000000.wrm";
    detail::write_meta_file(meta_len_writer, file1, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(10, filesize(file1));
    BOOST_CHECK_EQUAL(0, ::unlink(file1));

//    char file2[1024];
//    snprintf(file2, 1024, "./xxx-%06u-%06u.wrm", getpid(), 1);
    const char * file2 = "./xxx-000001.wrm";
    detail::write_meta_file(meta_len_writer, file2, sec_start, sec_start+1);
    meta_len_writer.len += hash_size;
    BOOST_CHECK_EQUAL(5, filesize(file2));
    BOOST_CHECK_EQUAL(0, ::unlink(file2));

//    char meta_path[1024];
//    snprintf(meta_path, 1024, "./xxx-%06u.mwrm", getpid());
    const char * meta_path = "./xxx.mwrm";
    BOOST_CHECK_EQUAL(meta_len_writer.len, filesize(meta_path));
    BOOST_CHECK_EQUAL(0, ::unlink(meta_path));
}

BOOST_AUTO_TEST_CASE(TestRequestFullCleaning)
{
    unlink("./xxx-000000.wrm");
    unlink("./xxx-000001.wrm");
    unlink("./xxx.mwrm");

    timeval now;
    now.tv_sec = 1352304810;
    now.tv_usec = 0;
    const int groupid = 0;
    OutMetaSequenceTransport wrm_trans("./", "./hash-", "xxx", now, 800, 600, groupid, nullptr,
                                       FilenameGenerator::PATH_FILE_COUNT_EXTENSION);
    wrm_trans.send("AAAAX", 5);
    wrm_trans.send("BBBBX", 5);
    wrm_trans.next();
    wrm_trans.send("CCCCX", 5);

    const FilenameGenerator * sqgen = wrm_trans.seqgen();

    BOOST_CHECK(-1 != filesize(sqgen->get(0)));
    BOOST_CHECK(-1 != filesize(sqgen->get(1)));
    BOOST_CHECK(-1 != filesize("./xxx.mwrm"));

    wrm_trans.request_full_cleaning();

    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(0)));
    BOOST_CHECK_EQUAL(-1, filesize(sqgen->get(1)));
    BOOST_CHECK_EQUAL(-1, filesize("./xxx.mwrm"));
}


template<size_t N>
long write(transbuf::ochecksum_buf_null_buf & buf, char const (&s)[N]) {
    return buf.write(s, N-1);
}

BOOST_AUTO_TEST_CASE(TestOSumBuf)
{
    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));
    transbuf::ochecksum_buf_null_buf buf(cctx.get_hmac_key());
    buf.open();
    BOOST_CHECK_EQUAL(write(buf, "ab"), 2);
    BOOST_CHECK_EQUAL(write(buf, "cde"), 3);

    detail::hash_type hash;
    buf.close(hash);

    char hash_str[detail::hash_string_len + 1];
    *detail::swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
    );
}

#include <string>

BOOST_AUTO_TEST_CASE(TestWriteFilename)
{
    using detail::write_filename;

    struct {
        std::string s;

        int write(char const * data, std::size_t len) {
            s.append(data, len);
            return len;
        }
    } writer;

#define TEST_WRITE_FILENAME(origin_filename, wrote_filename) \
    write_filename(writer, origin_filename);                 \
    BOOST_CHECK_EQUAL(writer.s, wrote_filename);             \
    writer.s.clear()

    TEST_WRITE_FILENAME("abcde.txt", "abcde.txt");

    TEST_WRITE_FILENAME(R"(\abcde.txt)", R"(\\abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc\de.txt)", R"(abc\\de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt\)", R"(abcde.txt\\)");
    TEST_WRITE_FILENAME(R"(abc\\de.txt)", R"(abc\\\\de.txt)");
    TEST_WRITE_FILENAME(R"(\\\\)", R"(\\\\\\\\)");

    TEST_WRITE_FILENAME(R"( abcde.txt)", R"(\ abcde.txt)");
    TEST_WRITE_FILENAME(R"(abc de.txt)", R"(abc\ de.txt)");
    TEST_WRITE_FILENAME(R"(abcde.txt )", R"(abcde.txt\ )");
    TEST_WRITE_FILENAME(R"(abc  de.txt)", R"(abc\ \ de.txt)");
    TEST_WRITE_FILENAME(R"(    )", R"(\ \ \ \ )");
}

BOOST_AUTO_TEST_CASE(TestWriteHash)
{
    detail::hash_type hash;
    std::iota(std::begin(hash), std::end(hash), 0);

    char hash_str[detail::hash_string_len + 1];
    *detail::swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
        " 202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
    );
}

#include "utils/fileutils.hpp"

BOOST_AUTO_TEST_CASE(TestOutFilenameSequenceTransport)
{
    OutFilenameSequenceTransport fnt(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test_outfilenametransport", ".txt", getgid());
    fnt.send("We write, ", 10);
    fnt.send("and again, ", 11);
    fnt.send("and so on.", 10);

    fnt.next();
    fnt.send(" ", 1);
    fnt.send("A new file.", 11);

    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(0)), 31);
    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(1)), 12);

    fnt.disconnect();
    unlink(fnt.seqgen()->get(0));
    unlink(fnt.seqgen()->get(1));
}
