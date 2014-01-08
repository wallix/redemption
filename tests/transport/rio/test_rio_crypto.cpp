/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of RIO Crypto module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRIOCrypto
#include <boost/test/auto_unit_test.hpp>

#include "rio/rio_impl.h"
#include "infiletransport.hpp"
#include "testtransport.hpp"

BOOST_AUTO_TEST_CASE(TestRIOCrypto)
{

    OpenSSL_add_all_digests();

    RIO       *rio;
    RIO_ERROR error;
    ssize_t   res;

    const char *data       = "TEST TEST TEST";
    size_t      datalen    = strlen(data);
    char        buffer[128];

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    memcpy(cctx.crypto_key,
       "\x00\x01\x02\x03\x04\x05\x06\x07"
       "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
       "\x10\x11\x12\x13\x14\x15\x16\x17"
       "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
       CRYPTO_KEY_LENGTH);

//    // Open write-only RIO Crypto object.
    rio = rio_new_crypto(&error, &cctx, "./RIOCrypto.dat", O_WRONLY);

    BOOST_CHECK((rio != NULL) && (error == RIO_ERROR_OK));

    if (rio != NULL) {
        // Write data to file using RIO Crypto object.
        res = rio_send(rio, data, datalen);

        BOOST_CHECK_EQUAL(res, datalen);


        // Close write-only RIO Crypto object.
        rio_delete(rio);


        // Open read-only RIO Crypto object.
        rio = rio_new_crypto(&error, &cctx, "./RIOCrypto.dat", O_RDONLY);

        BOOST_CHECK((rio != NULL) && (error == RIO_ERROR_OK));

        if (rio != NULL) {
            // Read data from file using RIO Crypto object.
            res = rio_recv(rio, buffer, sizeof(buffer));

            BOOST_CHECK_EQUAL(res, datalen);

            buffer[strlen(data)] = '\0';

            BOOST_CHECK(!strcmp(data, buffer));


            // Close write-only RIO Crypto object.
            rio_delete(rio);
        }

        // Delete test file.
        unlink("./RIOCrypto.dat");
    }


    // Open read-only RIO Crypto object.
    rio = rio_new_crypto(&error, &cctx, "./tests/fixtures/RIOCrypto.dat", O_RDONLY);

    BOOST_CHECK(rio != NULL);
    BOOST_CHECK(error == RIO_ERROR_OK);

    if (rio != NULL) {
        // Read data from file using RIO Crypto object.
        res = rio_recv(rio, buffer, datalen);

        BOOST_CHECK_EQUAL(res, datalen);

        buffer[datalen] = '\0';

        BOOST_CHECK(!strcmp(data, buffer));


        // Close write-only RIO Crypto object.
        rio_delete(rio);
    }
}   // BOOST_AUTO_TEST_CASE(TestRIOCrypto)

/*
BOOST_AUTO_TEST_CASE(TestRIOCrypto2)
{
    unsigned int i;

    // Generate the random key used bye "webcryptofile" library.
    for (i = 0; i < 32; i++)
        crypto_key[i] = i;

    RIO       *rio;
    RIO_ERROR error;
    ssize_t   res;

    const char *data       = "TEST TEST TEST";
    size_t      datalen    = strlen(data);
    char        buffer[15];

    // Open write-only RIO Crypto object.
    rio = rio_new_crypto(&error, "./RIOCrypto2.dat", O_WRONLY);

    BOOST_CHECK((rio != NULL) && (error == RIO_ERROR_OK));

    if (rio != NULL) {
        // Read data from file using write-only RIO Crypto object!!!
        res = rio_recv(rio, buffer, datalen);

        BOOST_CHECK_EQUAL(res, -RIO_ERROR_SEND_ONLY);


        // Write data to file using errored/closed RIO Crypto object!!!
        res = rio_send(rio, data, datalen);

        BOOST_CHECK_EQUAL(res, -RIO_ERROR_SEND_ONLY);
    }


    // Open read-only RIO Crypto object.
    rio = rio_new_crypto(&error, "./RIOCrypto2.dat", O_RDONLY);

    BOOST_CHECK((rio != NULL) && (error == RIO_ERROR_OK));

    if (rio != NULL) {
        // Write data to file using read-only RIO Crypto object!!!
        res = rio_send(rio, data, datalen);

        BOOST_CHECK_EQUAL(res, -RIO_ERROR_RECV_ONLY);


        // Read data from file using errored/closed RIO Crypto object!!!
        res = rio_recv(rio, buffer, datalen);

        BOOST_CHECK_EQUAL(res, -RIO_ERROR_RECV_ONLY);
    }

    // Delete test file.
    unlink("./RIOCrypto2.dat");
}   // BOOST_AUTO_TEST_CASE(TestRIOCrypto2)
*/

