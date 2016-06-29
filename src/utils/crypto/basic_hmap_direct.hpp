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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

namespace detail_ {

/**
 * the HMAC_direct transform looks like:
 * \a Digest(K XOR opad, \a Digest(K XOR ipad, text))
 * where K is an n byte key
 * ipad is the byte 0x36 repeated \a pad_length times
 * opad is the byte 0x5c repeated \a pad_length times
 * and text is the data being protected
 */
template<class Digest, std::size_t pad_length>
class basic_HMAC_direct
{
    uint8_t k_opad[pad_length];
    Digest context;

public:
    basic_HMAC_direct(const uint8_t * const key, size_t key_len)
    {
        if (key_len > pad_length) {
            unsigned char digest[Digest::DIGEST_LENGTH];
            Digest disgest;
            disgest.update(digest, Digest::DIGEST_LENGTH);
            disgest.final(digest, Digest::DIGEST_LENGTH);
            key_len = Digest::DIGEST_LENGTH;
        }
        uint8_t k_ipad[pad_length];
        for (size_t i = 0; i < key_len; i++) {
            k_ipad[i] = 0x36 ^ key[i];
            k_opad[i] = 0x5C ^ key[i];
        }
        context.update(k_ipad, pad_length);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        context.update(data, data_size);
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(Digest::DIGEST_LENGTH == out_data_size);
        context.final(out_data, Digest::DIGEST_LENGTH);

        Digest disgest;
        disgest.update(this->k_opad, pad_length);
        disgest.update(out_data, Digest::DIGEST_LENGTH);
        disgest.final(out_data, Digest::DIGEST_LENGTH);
    }
};

}
