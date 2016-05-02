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
#define BOOST_TEST_MODULE TestXXX
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
// #define LOGNULL

#include <stdio.h>
#include "system/ssl_calls.hpp"

BOOST_AUTO_TEST_CASE(TestAES)
{
    SslAES aes;

    uint8_t key24[] = "clef très très secrete";
    uint8_t iv[] = "vecteur d'initialisation pas secret du tout";
    uint8_t iv2[] = "vecteur d'initialisation pas secret du tout";

    uint8_t inbuf[1024]= "secret très confidentiel\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t outbuf[1024] = {};
    uint8_t decrypted[1024] = {};


    aes.set_key(key24, 24);

    aes.crypt_cbc(32, iv, inbuf, outbuf);

    aes.decrypt_cbc(32, iv2, outbuf, decrypted);

    BOOST_CHECK_EQUAL(memcmp(inbuf,
                             decrypted,
                             32),
                      0);

}

