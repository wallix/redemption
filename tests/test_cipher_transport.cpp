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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCipherTransport

#include <boost/test/auto_unit_test.hpp>
#include <stdlib.h>
#include <unistd.h>

#include "log.hpp"
#include "error.hpp"
#include "cipher_transport.hpp"


BOOST_AUTO_TEST_CASE(TestCryptOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    //const char * mystr = "1234567890";
    const unsigned char * data = (const unsigned char*)"123456";
    size_t len = 6;

    unsigned char pEncryptedStr[1024] = {};
    unsigned char pDecryptedStr[1024] = {};
    std::size_t outlen;
    CipherCryptData crypt_data(pEncryptedStr);
    CipherCrypt cipher_crypt(&crypt_data);
    cipher_crypt.init_encrypt();
    cipher_crypt.start(CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
    size_t p = 0;
    for (size_t n = 2; n <= len; p = n, n += 2){
        cipher_crypt.update(data + p, n-p);
    }
    if (p != len){
        cipher_crypt.update(data + p, len - p);
    }
    cipher_crypt.stop();

    outlen = crypt_data.size();
    BOOST_CHECK_EQUAL(outlen, 8);
    const char * expected = "\x17\xf9\x7b\xe7\xfb\x8b\xfa\xd6";
    BOOST_CHECK(0 == memcmp(expected, pEncryptedStr, sizeof(expected)-1)); 

    crypt_data.reset(pDecryptedStr);
    cipher_crypt.init_decrypt();
    cipher_crypt.start(CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), key, iv);
    for (std::size_t n = 2; n <= len; p = n, n += 2){
        cipher_crypt.update(data + p, n-p);
    }
    if (p != len){
        cipher_crypt.update(data + p, len - p);
    }
    cipher_crypt.stop();
    BOOST_CHECK(0 == memcmp(expected, pEncryptedStr, sizeof(expected)-1)); 
}

