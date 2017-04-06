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
#define BOOST_TEST_MODULE TestOutCryptoTransport
#include "system/redemption_unit_tests.hpp"
#include "check_mem.hpp"

#define LOGPRINT
#include "utils/log.hpp"
#include "transport/out_crypto_transport.hpp"
#include <string.h>

BOOST_AUTO_TEST_CASE(TestOutCryptoTransport)
{
    LOG(LOG_INFO, "Running test TestOutCryptoTransport");
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
        ct.open(finalname, 0);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }

    uint8_t expected_hash[MD_HASH::DIGEST_LENGTH+1] = "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5";

    CHECK_MEM_AC(
        qhash,
        "\x2a\xcc\x1e\x2c\xbf\xfe\x64\x03\x0d\x50\xea\xe7\x84\x5a\x9d\xce"
        "\x6e\xc4\xe8\x4a\xc2\x43\x5f\x6c\x0f\x7f\x16\xf8\x7b\x01\x80\xf5"
    );

    BOOST_CHECK(::unlink(tmpname) == -1); // already removed while renaming
    BOOST_CHECK(::unlink(finalname) == 0); // finalname exists
}

BOOST_AUTO_TEST_CASE(TestOutCryptoTransportAutoClose)
{
    LOG(LOG_INFO, "Running test TestOutCryptoTransportAutoClose");
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
    char tmpname[128];
    const char * finalname = "./encrypted.txt";
    try
    {
        OutCryptoTransport ct(true, true, cctx, rnd);
        ct.open(finalname, 0);
        ::strcpy(tmpname, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
    }
    catch (Error e){
        LOG(LOG_INFO, "exception raised %d", e.id);
    };
    // if there is no explicit close we can't get hash values
    // but the file is correctly closed and ressources freed
    BOOST_CHECK(::unlink(tmpname) == -1); // already removed while renaming
    BOOST_CHECK(::unlink(finalname) == 0); // finalname exists
}

BOOST_AUTO_TEST_CASE(TestOutCryptoTransportMultipleFiles)
{
    LOG(LOG_INFO, "Running test TestOutCryptoTransportMultipleFiles");
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
    char tmpname1[128];
    char tmpname2[128];
    const char * finalname1 = "./encrypted001.txt";
    const char * finalname2 = "./encrypted002.txt";
    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
    {
        OutCryptoTransport ct(true, true, cctx, rnd);

        ct.open(finalname1, 0);
        ::strcpy(tmpname1, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);

        ct.open(finalname2, 0);
        ::strcpy(tmpname2, ct.get_tmp());
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }
    BOOST_CHECK(::unlink(tmpname1) == -1); // already removed while renaming
    BOOST_CHECK(::unlink(finalname1) == 0); // finalname exists
    BOOST_CHECK(::unlink(tmpname2) == -1); // already removed while renaming
    BOOST_CHECK(::unlink(finalname2) == 0); // finalname exists
}

