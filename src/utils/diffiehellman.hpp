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

   DiffieHellman module
*/

#pragma once

#include "utils/genrandom.hpp"

#include <cstdint>

class DiffieHellman {
    UdevRandom randgen;
    Random * rand;
    uint64_t max;
    uint64_t gen;
    uint64_t mod;
    uint64_t pub;
    uint64_t priv;
    uint64_t key;

public:
    enum {
        DH_MAX_BITS = 31
    };

    bool error;

    DiffieHellman(uint64_t generator, uint64_t modulus)
        : randgen()
        , rand(&this->randgen)
        , max((uint64_t(1) << DH_MAX_BITS) - 1)
        , gen(generator)
        , mod(modulus)
        , pub(0)
        , priv(0)
        , key(0)
        , error((generator >= max) || (modulus >= max))
    {
    }

    ~DiffieHellman() = default;

    //void set_random(Random * newrand) {
    //    this->rand = newrand;
    //}
    //void unset_random() {
    //    this->rand = &this->randgen;
    //}

    uint64_t createInterKey() {
        uint8_t privgen[8] = {};
        this->rand->random(privgen, 8);
        this->priv = this->uint8p_to_uint64(privgen) % this->max;
        this->pub = this->xpowymodn(this->gen, this->priv, this->mod);
        return this->pub;
    }
    uint64_t createEncryptionKey(uint64_t interKey) {
        if (interKey >= this->max) {
            this->error = true;
        }
        this->key = this->xpowymodn(interKey, this->priv, this->mod);
        return this->key;
    }

    static uint64_t xpowymodn(uint64_t x, uint64_t y, uint64_t n) {
        uint64_t res = 1;
        const uint64_t oneshift63 = uint64_t(1) << 63;
        for (int i = 0; i < 64; i++) {
            res = (res*res) % n;
            if ((y & oneshift63) != 0) {
                res = (res*x) % n;
            }
            y = y << 1;
        }
        return res;
    }
    static void uint64_to_uint8p(uint64_t number, uint8_t* buffer) {
        for (int i = 0; i < 8; i++) {
            buffer[i] = uint8_t(0xff & (number >> (8*(7-i))));
        }
    }
    static uint64_t uint8p_to_uint64(uint8_t const* buffer) {
        uint64_t res = 0;
        for (int i = 0; i < 8; i++) {
            res <<= 8;
            res += buffer[i];
        }
        return res;
    }
};


