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
  Copyright (C) Wallix 2010-2019
  Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/cxx.hpp"

#if REDEMPTION_HAS_INCLUDE(<charconv>)
#  include <charconv>
#else

#include "utils/sugar/numerics/safe_conversions.hpp"

#include <system_error>
#include <cstdio>
#include <cstdio>
#include <cinttypes>

namespace std
{

struct to_chars_result
{
    char* ptr;
    std::errc ec;
};

inline to_chars_result to_chars(char* first, char* last, ::safe_int<uint64_t> value)
{
    int r = std::snprintf(first, last-first, "%" PRIu64, value.underlying());
    if (r < 0 || r > last-first) {
        return {first, std::errc::value_too_large};
    }
    return {first + r, std::errc{}};
}

}

#endif
