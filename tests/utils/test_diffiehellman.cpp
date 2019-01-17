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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean, Meng Tan
*/

#define RED_TEST_MODULE TestDiffieHellman
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/diffiehellman.hpp"

RED_AUTO_TEST_CASE(TestDiffieHellman)
{
    {
        uint64_t generator = 2534321354;
        uint64_t modulus = 1465213542;
        DiffieHellman dh(generator, modulus);

        uint64_t x = 431364;
        uint64_t y = 657313;
        uint64_t n = 1643213;

        uint64_t res = dh.xpowymodn(x, y, n);
        RED_CHECK_EQUAL(res, 154659u);
        uint8_t buffer[8] = {};
        dh.uint64_to_uint8p(res, buffer);
        uint64_t test = dh.uint8p_to_uint64(buffer);
        RED_CHECK_EQUAL(res, test);
    }
    {
        uint64_t gen = 32431233;
        uint64_t mod = 676513231;
        DiffieHellman dh_alice(gen, mod);
        DiffieHellman dh_bob(gen, mod);
        uint64_t pub_alice = dh_alice.createInterKey();
        uint64_t pub_bob = dh_bob.createInterKey();
        uint64_t key_alice = dh_alice.createEncryptionKey(pub_bob);
        uint64_t key_bob = dh_bob.createEncryptionKey(pub_alice);
        RED_CHECK_EQUAL(key_alice, key_bob);
    }
}
