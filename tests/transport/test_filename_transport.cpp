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
#define BOOST_TEST_MODULE TestXXX
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
// #define LOGNULL

#include "in_filename_transport.hpp"
#include "out_filename_transport.hpp"
#include "crypto_in_filename_transport.hpp"
#include "crypto_out_filename_transport.hpp"


BOOST_AUTO_TEST_CASE(TestFilename)
{
    const char * const filename = "/tmp/inoufiletest.txt";

    ::unlink(filename);

    {
        OutFilenameTransport in(filename);
        in.send("ABCDE", 5);
    }

    {
        InFilenameTransport out(filename);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        out.recv(p, 5);
        BOOST_CHECK_EQUAL(sp-s, 5);
        BOOST_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            out.recv(p, 1);
            BOOST_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}

BOOST_AUTO_TEST_CASE(TestFilenameCrypto)
{
    OpenSSL_add_all_digests();

    const char * const filename = "/tmp/inoufiletest_crypt.txt";

    ::unlink(filename);

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    memcpy(cctx.crypto_key,
           "\x00\x01\x02\x03\x04\x05\x06\x07"
           "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
           "\x10\x11\x12\x13\x14\x15\x16\x17"
           "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
           CRYPTO_KEY_LENGTH);

    {
        CryptoOutFilenameTransport in(&cctx, filename);
        in.send("ABCDE", 5);
    }

    {
        CryptoInFilenameTransport out(&cctx, filename);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        out.recv(p, 5);
        BOOST_CHECK_EQUAL(sp-s, 5);
        BOOST_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            out.recv(p, 1);
            BOOST_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}
