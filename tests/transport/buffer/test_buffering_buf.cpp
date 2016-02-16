/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2013
 * Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBufferingBuf
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include <fstream>
#include <sstream>

#include "transport/buffer/buffering_buf.hpp"
#include "transport/in_meta_sequence_transport.hpp"


BOOST_AUTO_TEST_CASE(TestOBufferingBuf)
{
    const char * filename = "./test_buffering_buf.test";
    {
        typedef transbuf::obuffering_buf<transbuf::ofile_buf, 10> obuffering_file;

        obuffering_file file;
        BOOST_CHECK(file.open(filename, 0644) > 0);
        BOOST_CHECK_EQUAL(file.write("ab", 2), 2);
        BOOST_CHECK_EQUAL(file.write("cde", 3), 3);
        BOOST_CHECK_EQUAL(file.write("fghi", 4), 4);
        BOOST_CHECK_EQUAL(file.write("jklmn", 5), 5);
        BOOST_CHECK_EQUAL(file.write("opqrst", 6), 6);
        BOOST_CHECK_EQUAL(file.write("0123456789", 10), 10);
        BOOST_CHECK_EQUAL(file.write("0123456789\n", 11), 11);
        BOOST_CHECK_EQUAL(file.flush(), 0);
        BOOST_CHECK_EQUAL(file.close(), 0);
    }

    std::filebuf file;
    std::ostringstream os;
    file.open(filename, std::ios::in);
    os << &file;
    BOOST_CHECK_EQUAL("abcdefghijklmnopqrst01234567890123456789\n", os.str());
}

// Note: buffering_buf is not used and the test is not specific to "buffering"
// this is a generic test that could apply to any buffer object.
// Not very usefull (testing all possible implementations of buffer in another
// that way test would be more interesting)

//BOOST_AUTO_TEST_CASE(TestIBufferingBuf)
//{
//    const char * filename = "./test_buffering_buf.test";
//    char buf[11+10+6+5+4+3+2 + 1] = {0};
//    {
//        typedef transbuf::ibuffering_buf<transbuf::ifile_buf, 10> ibuffering_file;

//        Inifile ini;
//        ini.set<cfg::crypto::key0>(cstr_array_view(
//            "\x00\x01\x02\x03\x04\x05\x06\x07"
//            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
//            "\x10\x11\x12\x13\x14\x15\x16\x17"
//            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
//        ));
//        ini.set<cfg::crypto::key1>(cstr_array_view("12345678901234567890123456789012"));

//        LCGRandom rnd(0);

//        CryptoContext cctx(rnd, ini, 1);

//        ibuffering_file file(&cctx);
//        BOOST_CHECK(file.open(filename, 0644) > 0);
//        BOOST_CHECK_EQUAL(file.read(buf, 2), 2);
//        BOOST_CHECK_EQUAL(file.read(buf+2, 3), 3);
//        BOOST_CHECK_EQUAL(file.read(buf+2+3, 4), 4);
//        BOOST_CHECK_EQUAL(file.read(buf+2+3+4, 5), 5);
//        BOOST_CHECK_EQUAL(file.read(buf+2+3+4+5, 6), 6);
//        BOOST_CHECK_EQUAL(file.read(buf+2+3+4+5+6, 10), 10);
//        BOOST_CHECK_EQUAL(file.read(buf+2+3+4+5+6+10, 11), 11);
//        BOOST_CHECK_EQUAL(file.close(), 0);
//    }

//    BOOST_CHECK_EQUAL("abcdefghijklmnopqrst01234567890123456789\n", std::string(buf));

//    unlink(filename);
//}
