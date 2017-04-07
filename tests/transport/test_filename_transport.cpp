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

#define UNIT_TEST_MODULE TestXXX
#include "system/redemption_unit_tests.hpp"


#define LOGPRINT
// #define LOGNULL

#include "transport/in_filename_transport.hpp"
#include "transport/out_filename_transport.hpp"

RED_AUTO_TEST_CASE(TestFilename)
{
    const char * const filename = "/tmp/inoufiletest.txt";

    ::unlink(filename);

    {
        OutFilenameTransport out(filename);
        out.send("ABCDE", 5);
    }

    {
        Inifile ini;
        ini.set<cfg::crypto::key0>(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        );
        ini.set<cfg::crypto::key1>("12345678901234567890123456789012");


        LCGRandom rnd(0);

        CryptoContext cctx(rnd, ini);

        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

        int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            RED_CHECK(false);
        }

        InFilenameTransport in(&cctx, fd, base, base_len);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        in.recv_atomic(*p, 5);
        RED_CHECK_EQUAL(sp-s, 5);
        RED_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            in.recv_atomic(*p, 1);
// Behavior changed, first return 0, then exception
//            RED_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestFilenameCrypto)
{
    OpenSSL_add_all_digests();

    const char * const filename = "/tmp/inoufiletest_crypt.txt";

    ::unlink(filename);

    Inifile ini;
    ini.set<cfg::crypto::key0>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::key1>("12345678901234567890123456789012");

    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    {
        CryptoOutFilenameTransport out(&cctx, filename);
        out.send("ABCDE", 5);
    }

    {
        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

        int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            RED_CHECK(false);
        }

        InFilenameTransport in(&cctx, fd, base, base_len);
        char s[5];
        char * sp = s;
        char ** p = &sp;
        in.recv(*p, 5);
        RED_CHECK_EQUAL(sp-s, 5);
        RED_CHECK_EQUAL(strncmp(s, "ABCDE", 5), 0);
        try {
            sp = s;
            p = &sp;
            in.recv_atomic(*p, 1);
// BEhavior changed. IS it OK ?
            RED_CHECK_EQUAL(sp-s, 0);
//            RED_CHECK(false);
        }
        catch (Error & e) {
        }
    }

    ::unlink(filename);
}
