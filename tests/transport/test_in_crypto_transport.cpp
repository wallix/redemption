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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestInCryptoTransport
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "utils/log.hpp"
#include "transport/out_crypto_transport.hpp"
#include "transport/in_crypto_transport.hpp"
#include <string.h>

BOOST_AUTO_TEST_CASE(TestInCryptoTransportClearText)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransport");
    OpenSSL_add_all_digests();

    LCGRandom rnd(0);
    CryptoContext cctx;
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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./clear.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(false, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    auto expected_hash = cstr_array_view(
        "\xc5\x28\xb4\x74\x84\x3d\x8b\x14\xcf\x5b\xf4\x3a\x9c\x04\x9a\xf3"
        "\x23\x9f\xac\x56\x4d\x86\xb4\x32\x90\x69\xb5\xe1\x45\xd0\x76\x9b");

    CHECK_MEM_AA(qhash, expected_hash);
    CHECK_MEM_AA(fhash, expected_hash);

    BOOST_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    BOOST_CHECK_NO_THROW([&]{
        char buffer[40];
        InCryptoTransport  ct(cctx, 0);
        ct.open(finalname);
        BOOST_CHECK_EQUAL(false, ct.is_eof());
        BOOST_CHECK_EQUAL(true, ct.atomic_read(buffer, 30));
        BOOST_CHECK_EQUAL(false, ct.is_eof());
        BOOST_CHECK_EQUAL(true, ct.atomic_read(&buffer[30], 1));
        BOOST_CHECK_EQUAL(true, ct.is_eof());
        BOOST_CHECK_EQUAL(false, ct.atomic_read(&buffer[30], 1));
        ct.close();
        CHECK_MEM_AC(make_array_view(buffer, 31), "We write, and again, and so on.");
    });

    BOOST_CHECK(::unlink(finalname) == 0);

}

BOOST_AUTO_TEST_CASE(TestInCryptoTransportCrypted)
{
    LOG(LOG_INFO, "Running test TestInCryptoTransportCrypted");
    OpenSSL_add_all_digests();

    LCGRandom rnd(0);
    CryptoContext cctx;
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

    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    const char * finalname = "./encrypted.txt";
    char tmpname[256];
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, S_IRUSR|S_IRGRP);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    auto expected_hash = cstr_array_view(
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5");

    CHECK_MEM_AA(qhash, expected_hash);
    CHECK_MEM_AA(fhash, expected_hash);

    BOOST_CHECK(::unlink(tmpname) == -1); // already removed while renaming

    BOOST_CHECK_NO_THROW([&]{
        char buffer[40];
        InCryptoTransport  ct(cctx, 0);
        ct.open(finalname);
        BOOST_CHECK_EQUAL(ct.is_encrypted(), true);
        BOOST_CHECK_EQUAL(false, ct.is_eof());
        BOOST_CHECK_EQUAL(true, ct.atomic_read(buffer, 30));
        BOOST_CHECK_EQUAL(false, ct.is_eof());
        BOOST_CHECK_EQUAL(true, ct.atomic_read(&buffer[30], 1));
        BOOST_CHECK_EQUAL(true, ct.is_eof());
        BOOST_CHECK_EQUAL(false, ct.atomic_read(&buffer[30], 1));
        ct.close();
        CHECK_MEM_AC(make_array_view(buffer, 31), "We write, and again, and so on.");
    });

    BOOST_CHECK(::unlink(finalname) == 0); // finalname exists
}

