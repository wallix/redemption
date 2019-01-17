/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "utils/strutils.hpp"

#include <cstring>


namespace utils
{

bool ends_with(const char * str, size_t str_len, const char * suffix, size_t suffix_len)
{
    return ((str_len >= suffix_len) && (strcmp(str + str_len - suffix_len, suffix) == 0));
}

bool ends_case_with(const char * str, size_t str_len, const char * suffix, size_t suffix_len)
{
    return ((str_len >= suffix_len) && (strcasecmp(str + str_len - suffix_len, suffix) == 0));
}


std::size_t strlcpy(char* dest, array_view_const_char src, std::size_t n) noexcept
{
    auto const nsrc = src.size();
    if (n) {
        auto const ncp = std::min(n-1u, nsrc);
        std::memcpy(dest, src.data(), ncp);
        dest[ncp] = 0;
    }
    return nsrc;
}

std::size_t strlcpy(char* dest, char const* src, std::size_t n) noexcept
{
    return strlcpy(dest, array_view_const_char{src, strlen(src)}, n);
}


bool strbcpy(char* dest, array_view_const_char src, std::size_t n) noexcept
{
    return strlcpy(dest, src, n) < n;
}

bool strbcpy(char* dest, char const* src, std::size_t n) noexcept
{
    return strlcpy(dest, src, n) < n;
}

} // namespace utils
