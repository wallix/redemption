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
#define BOOST_TEST_MODULE TestInMetaSequenceTransport
#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include "transport/out_meta_sequence_transport.hpp"
#include "transport/sequence_generator.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "error.hpp"

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm");
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, (unsigned)e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

BOOST_AUTO_TEST_CASE(TestMetav2)
{
    transbuf::ifile_buf ifile;
    ifile.open("./tests/fixtures/sample_v2.mwrm");

    struct ReaderBuf
    {
        transbuf::ifile_buf & buf;

        ssize_t operator()(char * buf, size_t len) const {
            return this->buf.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderBuf> reader({ifile});
    auto meta_header = detail::read_meta_headers(reader);
    BOOST_REQUIRE_EQUAL(meta_header.version, 2);
    BOOST_CHECK_EQUAL(meta_header.has_checksum, false);

    detail::MetaLine meta_line;
    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 1);
    BOOST_CHECK_EQUAL(meta_line.mode, 2);
    BOOST_CHECK_EQUAL(meta_line.uid, 3);
    BOOST_CHECK_EQUAL(meta_line.gid, 4);
    BOOST_CHECK_EQUAL(meta_line.dev, 5);
    BOOST_CHECK_EQUAL(meta_line.ino, 6);
    BOOST_CHECK_EQUAL(meta_line.mtime, 7);
    BOOST_CHECK_EQUAL(meta_line.ctime, 8);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304810);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304870);

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 9);
    BOOST_CHECK_EQUAL(meta_line.mode, 8);
    BOOST_CHECK_EQUAL(meta_line.uid, 10);
    BOOST_CHECK_EQUAL(meta_line.gid, 11);
    BOOST_CHECK_EQUAL(meta_line.dev, 12);
    BOOST_CHECK_EQUAL(meta_line.ino, 13);
    BOOST_CHECK_EQUAL(meta_line.mtime, 14);
    BOOST_CHECK_EQUAL(meta_line.ctime, 15);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304870);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304930);

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 16);
    BOOST_CHECK_EQUAL(meta_line.mode, 17);
    BOOST_CHECK_EQUAL(meta_line.uid, 18);
    BOOST_CHECK_EQUAL(meta_line.gid, 19);
    BOOST_CHECK_EQUAL(meta_line.dev, 20);
    BOOST_CHECK_EQUAL(meta_line.ino, 21);
    BOOST_CHECK_EQUAL(meta_line.mtime, 22);
    BOOST_CHECK_EQUAL(meta_line.ctime, 23);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304930);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304990);

    BOOST_CHECK_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), ERR_TRANSPORT_NO_MORE_DATA);
}

BOOST_AUTO_TEST_CASE(TestMetav2sum)
{
    transbuf::ifile_buf ifile;
    ifile.open("./tests/fixtures/sample_v2_checksum.mwrm");

    struct ReaderBuf
    {
        transbuf::ifile_buf & buf;

        ssize_t operator()(char * buf, size_t len) const {
            return this->buf.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderBuf> reader({ifile});
    auto meta_header = detail::read_meta_headers(reader);
    BOOST_REQUIRE_EQUAL(meta_header.version, 2);
    BOOST_CHECK_EQUAL(meta_header.has_checksum, true);

    using std::begin;
    using std::end;

    struct is_char {
        unsigned char c;
        bool operator()(unsigned char c) const {
            return this->c == c;
        }
    };

    detail::MetaLine meta_line;
    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 1);
    BOOST_CHECK_EQUAL(meta_line.mode, 2);
    BOOST_CHECK_EQUAL(meta_line.uid, 3);
    BOOST_CHECK_EQUAL(meta_line.gid, 4);
    BOOST_CHECK_EQUAL(meta_line.dev, 5);
    BOOST_CHECK_EQUAL(meta_line.ino, 6);
    BOOST_CHECK_EQUAL(meta_line.mtime, 7);
    BOOST_CHECK_EQUAL(meta_line.ctime, 8);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304810);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304870);
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xaa}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xbb}));

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 9);
    BOOST_CHECK_EQUAL(meta_line.mode, 8);
    BOOST_CHECK_EQUAL(meta_line.uid, 10);
    BOOST_CHECK_EQUAL(meta_line.gid, 11);
    BOOST_CHECK_EQUAL(meta_line.dev, 12);
    BOOST_CHECK_EQUAL(meta_line.ino, 13);
    BOOST_CHECK_EQUAL(meta_line.mtime, 14);
    BOOST_CHECK_EQUAL(meta_line.ctime, 15);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304870);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304930);
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xcc}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xdd}));

    BOOST_REQUIRE_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), 0);
    BOOST_CHECK_EQUAL(meta_line.size, 16);
    BOOST_CHECK_EQUAL(meta_line.mode, 17);
    BOOST_CHECK_EQUAL(meta_line.uid, 18);
    BOOST_CHECK_EQUAL(meta_line.gid, 19);
    BOOST_CHECK_EQUAL(meta_line.dev, 20);
    BOOST_CHECK_EQUAL(meta_line.ino, 21);
    BOOST_CHECK_EQUAL(meta_line.mtime, 22);
    BOOST_CHECK_EQUAL(meta_line.ctime, 23);
    BOOST_CHECK_EQUAL(meta_line.start_time, 1352304930);
    BOOST_CHECK_EQUAL(meta_line.stop_time, 1352304990);
    BOOST_CHECK(std::all_of(begin(meta_line.hash1), end(meta_line.hash1), is_char{0xee}));
    BOOST_CHECK(std::all_of(begin(meta_line.hash2), end(meta_line.hash2), is_char{0xff}));

    BOOST_CHECK_EQUAL(detail::read_meta_file(reader, meta_header, meta_line), ERR_TRANSPORT_NO_MORE_DATA);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample_v2", ".mwrm");
    char buffer[10000];
    char * pbuffer = buffer;
    size_t total = 0;
    try {
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv(&pbuffer, sizeof(buffer));
            total += pbuffer - buffer;
        }
    } catch (const Error & e) {
        BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, (unsigned)e.id);
        total += pbuffer - buffer;
    };
    // total size if sum of sample sizes
    BOOST_CHECK_EQUAL(1471394 + 444578 + 290245, total);
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm");
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm");

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2_RIO)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "./tests/fixtures/sample0.wrm 1352304810 1352304870\n",
//        "./tests/fixtures/sample1.wrm 1352304870 1352304930\n",
//        "./tests/fixtures/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    try {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/sample", ".mwrm");
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };

    } catch(const Error & e) {
        BOOST_CHECK(false);
    };
}

BOOST_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM3)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "/var/rdpproxy/recorded/sample0.wrm 1352304810 1352304870\n",
//        "/var/rdpproxy/recorded/sample1.wrm 1352304870 1352304930\n",
//        "/var/rdpproxy/recorded/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/moved_sample", ".mwrm");
        BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
        BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

        try {
            mwrm_trans.next();
            BOOST_CHECK(false);
        }
        catch (const Error & e){
            BOOST_CHECK_EQUAL((unsigned)ERR_TRANSPORT_NO_MORE_DATA, e.id);
        };
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), "./tests/fixtures/moved_sample", ".mwrm");

    BOOST_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample0.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample1.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    BOOST_CHECK_EQUAL("./tests/fixtures/sample2.wrm", mwrm_trans.path());
    BOOST_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    BOOST_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    BOOST_CHECK_EQUAL(3, mwrm_trans.get_seqno());

}

BOOST_AUTO_TEST_CASE(TestCryptoInmetaSequenceTransport)
{
    OpenSSL_add_all_digests();

    // cleanup of possible previous test files
    {
        const char * file[] = {"/tmp/TESTOFS.mwrm", "TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
            ::unlink(file[i]);
        }
    }

    BOOST_CHECK(true);

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));


    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini, 1);
    cctx.get_crypto_key();

    BOOST_CHECK(true);

    {
        struct timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;
        CryptoOutMetaSequenceTransport crypto_trans(&cctx, "", "/tmp/", "TESTOFS", tv, 800, 600, groupid,
                                                    nullptr, 0, FilenameGenerator::PATH_FILE_COUNT_EXTENSION);
        crypto_trans.send("AAAAX", 5);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        crypto_trans.next();
        crypto_trans.send("BBBBXCCCCX", 10);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
    }

    {
        CryptoInMetaSequenceTransport crypto_trans(&cctx, "TESTOFS", ".mwrm");

        char buffer[1024] = {};
        char * bob = buffer;
        char ** pbuffer = &bob;
        crypto_trans.recv(pbuffer, 15);
        BOOST_CHECK_EQUAL(15, *pbuffer - buffer);

        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
            LOG(LOG_ERR, "expected \"AAAAXBBBBXCCCCX\" got \"%s\"", buffer);
            BOOST_CHECK(false);
        }

        BOOST_CHECK(true);
    }

    const char * file[] = {
        "/tmp/TESTOFS.mwrm", // hash
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
        if (::unlink(file[i])){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(CryptoTestInMetaSequenceTransport2)
{

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini, 1);
    cctx.get_crypto_key();

    try {
        CryptoInMetaSequenceTransport(&cctx, "TESTOFSXXX", ".mwrm");
        BOOST_CHECK(false); // check open fails if file does not exist
    } catch (Error & e) {
        if (e.id != ERR_TRANSPORT_OPEN_FAILED) {
            BOOST_CHECK(false); // check open fails if file does not exist
        }
    }
}
