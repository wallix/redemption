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

#include "log.hpp"
#define LOGNULL

#include <stdlib.h>
#include <unistd.h>

#include "log.hpp"
#include "error.hpp"
#include "../transport/cipher_transport.hpp"


BOOST_AUTO_TEST_CASE(TestCryptOpenSSL)
{
    unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {1,2,3,4,5,6,7,8};

    const unsigned char * data = (const unsigned char*)"123456";
    size_t len = 6;
    unsigned char pEncryptedStr[1024] = {};

    unsigned char * crypt_data_pbuf = pEncryptedStr;

    EVP_CIPHER_CTX cipher_crypt_ctx;
    EVP_CIPHER_CTX_init(&cipher_crypt_ctx);
    EVP_EncryptInit_ex(&cipher_crypt_ctx, CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), 0, key, iv);

    int tmplen = 0;
    size_t n = 2;
    while (n <= len){
        if (EVP_EncryptUpdate(&cipher_crypt_ctx, crypt_data_pbuf, &tmplen, data + n - 2, n-(n-2))){
            crypt_data_pbuf += tmplen;
        }
        n += 2;
    }
    if ((n - 2) != len){
        if (EVP_EncryptUpdate(&cipher_crypt_ctx, crypt_data_pbuf, &tmplen, data + n - 2, len - (n-2)))
        {
            crypt_data_pbuf += tmplen;
        }
    }
    if (EVP_EncryptFinal_ex(&cipher_crypt_ctx, crypt_data_pbuf, &tmplen)){
        crypt_data_pbuf += tmplen;
    }
    BOOST_CHECK_EQUAL(crypt_data_pbuf - pEncryptedStr, 8);

    const char * expected = "\x17\xf9\x7b\xe7\xfb\x8b\xfa\xd6";
    BOOST_CHECK(0 == memcmp(expected, pEncryptedStr, sizeof(expected)-1)); 

    const unsigned char * crypted_data = (const unsigned char*)"\x17\xf9\x7b\xe7\xfb\x8b\xfa\xd6";
    size_t crypted_len = 8;

    unsigned char pDecryptedStr[1024] = {};

    unsigned char * decrypt_data_pbuf = pDecryptedStr;
    EVP_CIPHER_CTX_init(&cipher_crypt_ctx);
    EVP_DecryptInit_ex(&cipher_crypt_ctx, CipherMode::to_evp_cipher(CipherMode::BLOWFISH_CBC), 0, key, iv);
    unsigned p = 0;
    for (size_t n = 2; n <= crypted_len; p = n, n += 2){
        if (EVP_DecryptUpdate(&cipher_crypt_ctx, decrypt_data_pbuf, &tmplen, crypted_data + p, n-p)){
            decrypt_data_pbuf += tmplen;
        }
    }
    if (p != len){
        if (EVP_DecryptUpdate(&cipher_crypt_ctx, decrypt_data_pbuf, &tmplen, crypted_data + p, crypted_len - p)){
            decrypt_data_pbuf += tmplen;
        }
    }
    if (EVP_DecryptFinal_ex(&cipher_crypt_ctx, decrypt_data_pbuf, &tmplen)){
        decrypt_data_pbuf += tmplen;
    }
    pDecryptedStr[decrypt_data_pbuf - pDecryptedStr] = 0;
    const char * expected_clear = "123456";

    BOOST_CHECK_EQUAL(decrypt_data_pbuf - pDecryptedStr, 6);
    BOOST_CHECK(0 == memcmp(expected_clear, pDecryptedStr, sizeof(expected_clear)-1)); 
}

