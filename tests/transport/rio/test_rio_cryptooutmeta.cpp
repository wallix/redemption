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

#define LOGNULL
#include "log.hpp"

#include "rio/rio.h"
#include "rio/rio_impl.h"

BOOST_AUTO_TEST_CASE(TestCryptoOutMetaCleaning)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    CryptoContext crypto_ctx;
    memset(&crypto_ctx, 0, sizeof(crypto_ctx));
    memcpy(crypto_ctx.crypto_key,
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08",
        sizeof(crypto_ctx.crypto_key));

    RIO_ERROR status = RIO_ERROR_OK;
    SQ * seq  = NULL;
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;
    const int groupid = 0;
    RIO * rt = rio_new_cryptooutmeta(&status, &seq, &crypto_ctx, "", "/tmp/", "TESTOFS", ".mwrm", "800 600", "", "", &tv, groupid);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));

    rio_full_clear(rt);

    const char * file[] = {
        "TESTOFS.mwrm",
        "/tmp/TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) >= 0)
        {
            BOOST_CHECK(false);
            LOG(LOG_ERR, "File \"%s\" is always present!", file[i]);
        }
    }
}
