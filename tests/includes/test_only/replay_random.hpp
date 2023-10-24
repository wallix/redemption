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
   Author(s): Christophe Grosjean,

   Random Generators

*/

#pragma once

#include "core/error.hpp"
#include "utils/random.hpp"

#include <vector>
#include <cstring>

class ReplayRandom final : public Random
{
    // caller responsibility to provide enough data
    // or access to vector will throw an error
    std::vector<uint8_t> seed;
    uint32_t i = 0;

public:
    explicit ReplayRandom(std::vector<uint8_t> seed)
    : seed(std::move(seed))
    {
    }

    void random(writable_bytes_view buf) override
    {
        if (buf.size() > seed.size() - i) {
            throw Error(ERR_RANDOM_SOURCE_FAILED);
        }

        std::memcpy(buf.data(), seed.data() + i, buf.size());
        i += buf.size();
    }
};
