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

namespace rvt
{

struct Color
{
    constexpr Color() noexcept = default;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b) noexcept
    : r(r)
    , g(g)
    , b(b)
    {}

    constexpr uint8_t red() const noexcept { return r; }
    constexpr uint8_t green() const noexcept { return g; }
    constexpr uint8_t blue() const noexcept { return b; }

private:
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

constexpr bool operator == (Color const & c1, Color const & c2) noexcept
{
    return c1.red() == c2.red()
        && c1.green() == c2.green()
        && c1.blue() == c2.blue();
}

constexpr bool operator != (Color const & c1, Color const & c2) noexcept
{ return !operator==(c1, c2); }

}
