/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCryptoOutMeta
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <outmetatransport.hpp>

BOOST_AUTO_TEST_CASE(TestCryptoOutMetaTransport)
{

    OpenSSL_add_all_digests();

    // cleanup of possible previous test files
    {
        const char * file[] = {"/tmp/TESTOFS.mwrm", "TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
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

    {
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;
        CryptoOutmetaTransport crypro_trans(&crypto_ctx, "", "/tmp/", "TESTOFS", tv, 800, 600, groupid,
                                            0, 0, FilenameGenerator::PATH_FILE_COUNT_EXTENSION);

        crypro_trans.send("AAAAX", 5);
        tv.tv_sec += 100;
        crypro_trans.timestamp(tv);
        crypro_trans.next();
        crypro_trans.send("BBBBXCCCCX", 10);
        tv.tv_sec += 100;
        crypro_trans.timestamp(tv);
        crypro_trans.next();
    }

    const char * file[] = {
        "TESTOFS.mwrm",
        "/tmp/TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]))
        {
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}
