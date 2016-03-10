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
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include "transport/out_meta_sequence_transport.hpp"
#include "fileutils.hpp"


BOOST_AUTO_TEST_CASE(TestOutmetaTransport)
{
    unsigned sec_start = 1352304810;
    {
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        OutMetaSequenceTransport wrm_trans(static_cast<CryptoContext*>(nullptr), "./", "./hash-", "xxx", now, 800, 600, groupid);
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
    Inifile ini;
    ini.configuration_holder().set_value("crypto", "key0",
                                         "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                         "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    ini.configuration_holder().set_value("crypto", "key1", "12345678901234567890123456789012");

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    {
        timeval now;
        now.tv_sec = sec_start;
        now.tv_usec = 0;
        const int groupid = 0;
        OutMetaSequenceTransportWithSum wrm_trans(&cctx, "./", "./", "xxx", now, 800, 600, groupid);
        wrm_trans.send("AAAAX", 5);
        wrm_trans.send("BBBBX", 5);
        wrm_trans.next();
        wrm_trans.send("CCCCX", 5);
    } // brackets necessary to force closing sequence

    struct {
        size_t len = 0;
        ssize_t write(char const * s, size_t len) {
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

