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
#include "system/redemption_unit_tests.hpp"

//#define LOGNULL
#define LOGPRINT

#include "utils/sugar/iter.hpp"

#include "capture/wrm_capture.hpp"
#include "capture/capture.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "core/error.hpp"


RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr),
        FIXTURES_PATH "/sample", ".mwrm", 0);
    unsigned char buffer[10000];
    unsigned char * pbuffer = buffer;
    size_t total = 0;
    auto test = [&]{
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv_new(pbuffer, sizeof(buffer));
            total += sizeof(buffer);
        }
    };
   RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
    total += pbuffer - buffer;
    // total size if sum of sample sizes
    RED_CHECK_EQUAL(2200000, total);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2)
{
    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample_v2", ".mwrm", 0);
    unsigned char buffer[10000];
    unsigned char * pbuffer = buffer;
    size_t total = 0;
    auto test = [&]{
        for (size_t i = 0; i < 221 ; i++){
            pbuffer = buffer;
            wrm_trans.recv_new(pbuffer, sizeof(buffer));
            total += sizeof(buffer);
        }
    };
   RED_CHECK_EXCEPTION_ERROR_ID(test(), ERR_TRANSPORT_NO_MORE_DATA);
    total += pbuffer - buffer;
    // total size if sum of sample sizes
    RED_CHECK_EQUAL(2200000, total);                             // 1471394 + 444578 + 290245
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        FIXTURES_PATH "/sample0.wrm 1352304810 1352304870\n",
//        FIXTURES_PATH "/sample1.wrm 1352304870 1352304930\n",
//        FIXTURES_PATH "/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);
        RED_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(3, mwrm_trans.get_seqno());

       RED_CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);

    RED_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM2_RIO)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        FIXTURES_PATH "/sample0.wrm 1352304810 1352304870\n",
//        FIXTURES_PATH "/sample1.wrm 1352304870 1352304930\n",
//        FIXTURES_PATH "/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/sample", ".mwrm", 0);
    RED_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(3, mwrm_trans.get_seqno());

   RED_CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM3)
{
//        "800 600\n",
//        "0\n",
//        "\n",
//        "/var/rdpproxy/recorded/sample0.wrm 1352304810 1352304870\n",
//        "/var/rdpproxy/recorded/sample1.wrm 1352304870 1352304930\n",
//        "/var/rdpproxy/recorded/sample2.wrm 1352304930 1352304990\n",

    // This is what we are actually testing, chaining of several files content

    {
        InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/moved_sample", ".mwrm", 0);
        RED_CHECK_EQUAL(0, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(1, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(2, mwrm_trans.get_seqno());

        mwrm_trans.next();
        RED_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
        RED_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
        RED_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
        RED_CHECK_EQUAL(3, mwrm_trans.get_seqno());

       RED_CHECK_EXCEPTION_ERROR_ID(mwrm_trans.next(), ERR_TRANSPORT_NO_MORE_DATA);
    }

    // check we can do it two times
    InMetaSequenceTransport mwrm_trans(static_cast<CryptoContext*>(nullptr), FIXTURES_PATH "/moved_sample", ".mwrm", 0);

    RED_CHECK_EQUAL(0, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample0.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304810, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(1, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample1.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304870, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(2, mwrm_trans.get_seqno());

    mwrm_trans.next();
    RED_CHECK_EQUAL(FIXTURES_PATH "/sample2.wrm", mwrm_trans.path());
    RED_CHECK_EQUAL(1352304930, mwrm_trans.begin_chunk_time());
    RED_CHECK_EQUAL(1352304990, mwrm_trans.end_chunk_time());
    RED_CHECK_EQUAL(3, mwrm_trans.get_seqno());
}

RED_AUTO_TEST_CASE(TestCryptoInmetaSequenceTransport)
{
    OpenSSL_add_all_digests();

    // cleanup of possible previous test files
    {
        const char * file[] = {"/tmp/TESTOFS.mwrm", "TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0; i < sizeof(file)/sizeof(char*); ++i){
            ::unlink(file[i]);
        }
    }

    RED_CHECK(true);

    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

    RED_CHECK(true);

    {
        LCGRandom rnd(0);
        Fstat fstat;
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;
        wrmcapture_OutMetaSequenceTransport crypto_trans(true, true, cctx, rnd, fstat, "", "/tmp/", "TESTOFS", tv, 800, 600, groupid, nullptr);
        crypto_trans.send("AAAAX", 5);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        crypto_trans.next();
        crypto_trans.send("BBBBXCCCCX", 10);
        tv.tv_sec += 100;
        crypto_trans.timestamp(tv);
        RED_CHECK(true);
    }

    {
        InMetaSequenceTransport crypto_trans(&cctx, "TESTOFS", ".mwrm", 1);

        char buffer[1024] = {};
        char * bob = buffer;

        RED_CHECK_NO_THROW(crypto_trans.recv_new(bob, 15));

        RED_CHECK_EQUAL_RANGES(make_array_view(buffer, 15), cstr_array_view("AAAAXBBBBXCCCCX"));
    }

    const char * files[] = {
        "/tmp/TESTOFS.mwrm", // hash
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (const char * file : files){
        RED_CHECK_MESSAGE(!::unlink(file), "failed to unlink " << file);
    }
}

RED_AUTO_TEST_CASE(CryptoTestInMetaSequenceTransport2)
{
    CryptoContext cctx;
    cctx.set_master_key(cstr_array_view(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    ));
    cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

   RED_CHECK_EXCEPTION_ERROR_ID(InMetaSequenceTransport(&cctx, "TESTOFSXXX", ".mwrm", 1), ERR_TRANSPORT_OPEN_FAILED);
}
