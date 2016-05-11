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
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSslAES
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
// #define LOGNULL

#include <stdio.h>

//#include "system/ssl_calls.hpp"
#include "system/ssl_aes.hpp"

//#include <iostream>

BOOST_AUTO_TEST_CASE(TestAES)
{

    {
        SslAES aes;

        // very secret key
        uint8_t key24[] = {0, 1, 2, 3, 4, 5, 6, 7,
                           8, 9, 10, 11, 12, 13, 14, 15,
                           16, 17, 18, 19, 20, 21, 22, 23};
        // init vector not secret
        uint8_t iv[] = "vecteur d'initialisation pas secret du tout";
        // init vector not secret
        uint8_t iv2[] = "vecteur d'initialisation pas secret du tout";

        uint8_t inbuf[1024]= "secret très confidentiel\x00\x00\x00\x00\x00\x00\x00\x00";
        uint8_t outbuf[1024] = {};
        uint8_t decrypted[1024] = {};


        aes.set_key(key24, 16);

        aes.crypt_cbc(32, iv, inbuf, outbuf);

        aes.decrypt_cbc(32, iv2, outbuf, decrypted);


        BOOST_CHECK_EQUAL(memcmp(inbuf, decrypted, 32),
                        0);
    }

}

BOOST_AUTO_TEST_CASE(TestAES_EVP)
{
    EVP_CIPHER_CTX ectx; // [en|de]cryption context
    ::memset(&ectx, 0, sizeof(ectx));
    ::EVP_CIPHER_CTX_init(&ectx);
    uint8_t key[32] = {};
    uint8_t iv[32] = {};
    const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
    if (::EVP_EncryptInit_ex(&ectx, cipher, nullptr, key, iv) != 1) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(TestAES_direct)
{
    {
        /*SslAES_direct aes;

        uint8_t key24[] = "clef très très secrete\0v";
        uint8_t key24_2[] = "clef très très secrete\0v";
        uint8_t iv[] = "vecteur d'initialisation pas secret du tout";
        uint8_t iv2[] = "vecteur d'initialisation pas secret du tout";

        uint8_t inbuf[1024]= "secret très confidentiel\x00\x00\x00\x00\x00\x00\x00\x00";
        uint8_t outbuf[1024] = {};
        uint8_t decrypted[1024] = {};


        aes.set_key(key24, key24_2, 24);

        aes.crypt_cbc(32, iv, inbuf, outbuf);

        aes.decrypt_cbc(32, iv2, outbuf, decrypted);


        BOOST_CHECK_EQUAL(memcmp(inbuf,
                                decrypted,
                                32),
                        0);*/
    }
}
