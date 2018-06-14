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
Copyright (C) Wallix 2018
Author(s): David Fort
*/

#pragma once


#include "utils/genrandom.hpp"

/**
 * @brief a random generator that always returns the same value, useful when you
 *         want to be able to replay scenarios (and so you need random that is not
 *         so randomized).
 */
class FixedRandom : public Random
{
public:
    FixedRandom(uint8_t fixedByte = 0xff) : value(fixedByte)
    {}

    void random(void * dest, size_t size) override
    {
        memset(dest, value, size);
    }

protected:
    uint8_t value;
};
