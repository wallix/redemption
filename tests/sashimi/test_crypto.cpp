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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Unit test to buffer object
   Using lib boost test functions
*/

#define RED_TEST_MODULE TestCrypto
#include "system/redemption_unit_tests.hpp"

#include "utils/log.hpp"
#include "sashimi/libcrypto.hpp"

// test AES cipher

RED_AUTO_TEST_CASE(TestBlowfishCipher)
{
    ssh_blowfish_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "blowfish-cbc"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0x4e, 0xf9, 0x97, 0x45, 0x61, 0x98, 0xdd, 0x78, 0xe1, 0xc0, 0x30, 0xe7, 0x4c, 0x14, 0xd2, 0x61,  // N..Ea..x..0.L..a
        0x53, 0xe3, 0xb2, 0xa5, 0xa3, 0xbc, 0x29, 0x7f, 0x25, 0x83, 0x9e, 0x1d, 0xc0, 0x03, 0x82, 0x2e,  // S.....).%.......
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_blowfish_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}


RED_AUTO_TEST_CASE(TestAES128CtrCipher)
{
    ssh_aes128_ctr_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes128-ctr"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
       0x66, 0xE9, 0x4B, 0xD4, 0xEF, 0x8A, 0x2C, 0x3B, 0x88, 0x4c, 0xFA, 0x59, 0xCA, 0x34, 0x2B, 0x2E,
       0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61, 0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a,
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes128_ctr_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}

RED_AUTO_TEST_CASE(TestAES192CtrCipher)
{
    ssh_aes192_ctr_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes192-ctr"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0xaa, 0xe0, 0x69, 0x92, 0xac, 0xbf, 0x52, 0xa3, 0xe8, 0xf4, 0xa9, 0x6e, 0xc9, 0x30, 0x0b, 0xd7,  // ..i...R....n.0..
        0xcd, 0x33, 0xb2, 0x8a, 0xc7, 0x73, 0xf7, 0x4b, 0xa0, 0x0e, 0xd1, 0xf3, 0x12, 0x57, 0x24, 0x35,  // .3...s.K.....W$5
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes192_ctr_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}

RED_AUTO_TEST_CASE(TestAES256CtrCipher)
{
    ssh_aes256_ctr_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes256-ctr"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0xdc, 0x95, 0xc0, 0x78, 0xa2, 0x40, 0x89, 0x89, 0xad, 0x48, 0xa2, 0x14, 0x92, 0x84, 0x20, 0x87,  // ...x.@...H.... .
        0x53, 0x0f, 0x8a, 0xfb, 0xc7, 0x45, 0x36, 0xb9, 0xa9, 0x63, 0xb4, 0xf1, 0xc4, 0xcb, 0x73, 0x8b,  // S....E6..c....s.
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes256_ctr_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}


RED_AUTO_TEST_CASE(TestAES128CbcCipher)
{
    ssh_aes128_cbc_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes128-cbc"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b, 0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e,  // f.K...,;.L.Y.4+.
        0xf7, 0x95, 0xbd, 0x4a, 0x52, 0xe2, 0x9e, 0xd7, 0x13, 0xd3, 0x13, 0xfa, 0x20, 0xe9, 0x8d, 0xbc,  // ...JR....... ...
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes128_cbc_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}

RED_AUTO_TEST_CASE(TestAES192CbcCipher)
{
    ssh_aes192_cbc_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes192-cbc"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0xaa, 0xe0, 0x69, 0x92, 0xac, 0xbf, 0x52, 0xa3, 0xe8, 0xf4, 0xa9, 0x6e, 0xc9, 0x30, 0x0b, 0xd7,  // ..i...R....n.0..
        0x52, 0xf6, 0x74, 0xb7, 0xb9, 0x03, 0x0f, 0xda, 0xb1, 0x3d, 0x18, 0xdc, 0x21, 0x4e, 0xb3, 0x31,  // R.t......=..!N.1
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes192_cbc_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}

RED_AUTO_TEST_CASE(TestAES256CbcCipher)
{
    ssh_aes256_cbc_cipher_struct cipher;
    RED_CHECK(0 == strcmp(cipher.name, "aes256-cbc"));
    uint8_t encrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t encrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher.set_encrypt_key(encrypt_key, encrypt_IV);

    uint8_t in[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t out[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t expected_out[32] = {
        0xdc, 0x95, 0xc0, 0x78, 0xa2, 0x40, 0x89, 0x89, 0xad, 0x48, 0xa2, 0x14, 0x92, 0x84, 0x20, 0x87,  // ...x.@...H.... .
        0x08, 0xc3, 0x74, 0x84, 0x8c, 0x22, 0x82, 0x33, 0xc2, 0xb3, 0x4f, 0x33, 0x2b, 0xd2, 0xe9, 0xd3,  // ..t..".3..O3+...
    };
    cipher.cbc_encrypt(in, out, 16);
    cipher.cbc_encrypt(in+16, out+16, 16);
//    hexdump_d(out, 32);
    RED_CHECK_EQUAL(0, memcmp(expected_out, out, 32));

    uint8_t back_to_in[32] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    ssh_aes256_cbc_cipher_struct cipher2;
    uint8_t decrypt_key[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t decrypt_IV[32] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    cipher2.set_decrypt_key(decrypt_key, decrypt_IV);
    cipher2.cbc_decrypt(out, back_to_in, 16);
    cipher2.cbc_decrypt(out+16, back_to_in+16, 16);
//    hexdump_d(back_to_in, 32);
    RED_CHECK_EQUAL(0, memcmp(in, back_to_in, 32));
}


RED_AUTO_TEST_CASE(TestFindPreferredInAvailable)
{
    {
        SSHString res = find_matching("a", "a", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("a", &res[0], res.size()) == 0);
    }

    {
        SSHString res = find_matching("a", "b", ',');
        RED_CHECK_EQUAL(res.size(), 0);
    }

    {
        SSHString res = find_matching("b,a", "b", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("b", &res[0], res.size()) == 0);
    }

    {
        SSHString res = find_matching("a,b", "b", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("b", &res[0], res.size()) == 0);
    }

    {
        SSHString res = find_matching("a,b,c", "b", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("b", &res[0], res.size()) == 0);
    }

    {
        SSHString res = find_matching("a,b,c", "a,b", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("a", &res[0], res.size()) == 0);
    }
    {
        SSHString res = find_matching("a,b,c", "b,a", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("b", &res[0], res.size()) == 0);
    }
    {
        SSHString res = find_matching("a,b,c", "d,b,a", ',');
        RED_CHECK_EQUAL(res.size(), 1);
        RED_CHECK(memcmp("b", &res[0], res.size()) == 0);
    }
    {
        SSHString res = find_matching("ab,abc,abcd", "abcde,abc,abcd", ',');
        RED_CHECK_EQUAL(res.size(), 3);
        RED_CHECK(memcmp("abc", &res[0], res.size()) == 0);
    }

}


RED_AUTO_TEST_CASE(TestFindTextInList)
{
    RED_CHECK_EQUAL(0, find_key("titi", {{"abc", 1},{"bcde", 2}}, 0));
    RED_CHECK_EQUAL(1, find_key("abc", {{"abc", 1},{"bcde", 2}}, 0));
    RED_CHECK_EQUAL(2, find_key("bcde", {{"abc", 1},{"bcde", 2}}, 0));
    RED_CHECK_EQUAL(1, find_key("abc", {{"abc", 1},{"bcde", 2}, {"abc", 3}}, 0));
    RED_CHECK_EQUAL(3, find_key("abcd", {{"abc", 1},{"bcde", 2}, {"abcd", 3}}, 0));
}

//openssl ecparam -name secp256k1 -genkey -noout -out secp256k1-key.pem

// /usr/bin/openssl dsaparam -out xxx.passphrase -genkey 2048
// /usr/bin/openssl gendsa -out dsa.key xxx.passphrase

// /usr/bin/openssl dsaparam -genkey 4096

//-----BEGIN DSA PARAMETERS-----
//MIIELQKCAgEAxajp7mCZbVtjqYpSbS5ZKLwTEu8gH33wRzxnB9tFIxNvfnQ58Tp1
//tJK0B8Qz92Gy9YHheAX/wDvoURTnrPxovk3Me7OrhqmE4OnZ6550aZz1hueOFAOb
//whmnCd5qY2F2XVgoBjrEwwSMwteKVGOiCgr1hx3+O3441RJ1DL02izOCu+P4M/Mu
//YhPsQJIv2rwjbBBbaBomvgkqaGOQdWzLF6qyMaMGzmUYuU78qeqruqANBnA7QtQq
//Y6dK/goV8Ghyuqqza5dSoajG21mhKDzqkSTBZ8iZj4w+/LRzhEONVqwBgDvIk8Xg
//4jzmHEeWKj0+EuVpZpZu4dabZ/W+4kdMIU0Y7lBM1gv6JUoi9tjUPaj6/u+qs4wc
//+Fv4kiTSjiu58UpXks7zNmS7uxZESA39KRHxvwA4Mn9EHF8Fvy4lF09Xjvksh8+j
//TjkqvNYnWW5QAG0XrPzZJHmWncwCHqyOH5lRrjn3NhuYPNc2u5mgt94HXCFinUou
//kz1dgsKO+QROpkpZxW9YEM04O7H8sXtAfXTBWD6M6jS09FrDw6ee67bKPfLPEy4P
//9Q5jE8Tr2K35wM1V3mlkIgo/X/XibprNcke9UL2kXj7BFb9A/L5iIRBNLS/uSSLK
//GK12RwwBJ74lr6p1yyQk+GjevxOtfLwRjDLgWOYKRjVRley9SJ2JF8kCIQD379UE
//TV3xLIgasKKyeIIH0xQxVYBJi1js3Q5kUBr9pwKCAgEAp8336+W2F+3ZQEle0zww
//HEM/yy7ZeLYpbL6VZ5Ri8599Tq4LV/IKEjyF9q5HWYgqedd0DOUH2gIsYHIYWv/V
//eZC15ooDfXQ5NL8gAgpBF92H13VNfxg0cIGNB0aaQG6NWmhadyaBWrlvhlTe1Yhh
//CWCZ/k1IEQvQu2yIe0wEv1OZW0zotoBiUytku0c0qiGzRTI5hcB5R24jU6HnIQWh
//TjH1j+ZWCXTA04fp2bLH9KdykbTqCs7qf/eDL1Htz40hxTm1Zrr/sFGcPnx7pAgA
//6mfseqqn2id0yuLNQ8e8AUE/2B/5Dl3LNZI9mHKEmNygLaQLnVqOSQEJJrq72mb+
//Yp6ERbWV+M3VHVp48LGzFguMVwGB2/2Ol/WEZDn0nOwYgDjjy9B+X9l8EWhTutQ8
//5zq+K7sPqjiSnqvy9Ym/to1hBOb2xXDfyOGWPEVSXyBWwQbu6xTj4l1qBE3SfdOl
//3ckbqjM9tuMglEsmE2KHLHncuF9jBaCq/WEr0KnUwtD0WB1HBBg5q29YCUF51PFV
///GOI8OSsu33c5j+hFgfW7/KW68gP82QsmyvgQ8xSBdh3eA8KR+ZO9u47juCH3Sbg
//QCQU9CkNY6JBtmrRSF6MGhN5r/2Or17FrB0bABsLxcYU0pbHoYX21gY99eLYWHSl
//nocX4uBEBR/C6/c6KIi+dc4=
//-----END DSA PARAMETERS-----
//-----BEGIN DSA PRIVATE KEY-----
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
//-----END DSA PRIVATE KEY-----



struct SshPrivateKey {};

struct SshPrivateDSAKey {
    DSA *dsa;
    SshPrivateDSAKey(char * b64_key, size_t bsize)
    {
        struct BIO_deleter{
            BIO_deleter(){}
            void operator()(BIO * p) const noexcept {
               BIO_free(p);
            }
        };
        std::unique_ptr<BIO, BIO_deleter> mem(BIO_new_mem_buf(b64_key, bsize));
        this->dsa = PEM_read_bio_DSAPrivateKey(mem.get(), nullptr, nullptr, nullptr);
        if (this->dsa == nullptr) {
            LOG(LOG_INFO, "Parsing private key: %s", ERR_error_string(ERR_get_error(), nullptr));
            throw Error(ERR_SSH_PARSE_PRIVATE_DSA_KEY);
        }
    }

};

struct SshPrivateRSAKey {
    RSA *rsa;
    SshPrivateRSAKey(char * b64_key, size_t bsize)
    {
        struct BIO_deleter{
            BIO_deleter(){}
            void operator()(BIO * p) const noexcept {
               BIO_free(p);
            }
        };
        std::unique_ptr<BIO, BIO_deleter> mem(BIO_new_mem_buf(b64_key, bsize));
        this->rsa = PEM_read_bio_RSAPrivateKey(mem.get(), nullptr, nullptr, nullptr);
        if (this->rsa == nullptr) {
            LOG(LOG_INFO, "Parsing private key: %s", ERR_error_string(ERR_get_error(), nullptr));
            throw Error(ERR_SSH_PARSE_PRIVATE_RSA_KEY);
        }
    }
};




RED_AUTO_TEST_CASE(CreateDSAKeyFromPem)
{
       char b64_key[] = "-----BEGIN DSA PRIVATE KEY-----\n"
"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\n"
"-----END DSA PRIVATE KEY-----\n";
    OpenSSL_add_all_algorithms();
    // sizeof-2 because neither \n or trailing zero are necessary
    // but it also works if they are provided
    SshPrivateDSAKey privkey(b64_key, sizeof(b64_key)-2);

}
