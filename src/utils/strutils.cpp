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
#include <algorithm>
#include <functional>

namespace utils
{

bool starts_with(chars_view str, chars_view prefix) noexcept
{
    return str.size() >= prefix.size()
        && strncmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool ends_with(chars_view str, chars_view suffix) noexcept
{
    return str.size() >= suffix.size()
        && strncmp(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
}

bool ends_case_with(chars_view str, chars_view suffix) noexcept
{
    return str.size() >= suffix.size()
        && strncasecmp(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
}


std::size_t strlcpy(char* dest, chars_view src, std::size_t n) noexcept
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
    return strlcpy(dest, chars_view{src, strlen(src)}, n);
}


void str_replace_inplace(std::string& str,
                         std::string_view tag,
                         std::string_view replacement)
{
    assert(!tag.empty());

    std::boyer_moore_searcher searcher(tag.begin(), tag.end());

    std::ptrdiff_t i = 0;

    for (;;) {
        auto rng = searcher(str.begin(), str.end());
        if (rng.first != rng.second) {
            i = rng.first - str.begin();
            i += std::ptrdiff_t(replacement.size());
            str.replace(rng.first, rng.second, replacement.begin(), replacement.end());
        }
        else {
            break;
        }
    }
}

void str_replace_inplace_between_pattern(std::string& str,
                                         char pattern,
                                         std::string_view replacement)
{
    auto it = str.begin();

    for (;;) {
        auto open_it = std::find(it, str.end(), pattern);
        if (open_it != str.end()) {
            auto close_it = std::find(open_it+1, str.end(), pattern);
            if (close_it != str.end()) {
                ++close_it;
                auto i = open_it - str.begin();
                i += std::ptrdiff_t(replacement.size());
                str.replace(open_it, close_it, replacement.begin(), replacement.end());
                it = str.begin() + i;
                continue;
            }
        }
        break;
    }
}

} // namespace utils
