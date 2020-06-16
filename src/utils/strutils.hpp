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

#pragma once

#include "utils/sugar/array_view.hpp"

namespace utils
{

bool ends_with(chars_view str, chars_view suffix) noexcept;
bool ends_case_with(chars_view str, chars_view suffix) noexcept;

/**
 * \resume copies up to \c n - 1 characters from the NUL-terminated string \c src to \c dst, NUL-terminating the result.
 * \return total length of the string they tried to create.
 * @{
 */
std::size_t strlcpy(char* dest, chars_view src, std::size_t n) noexcept;
std::size_t strlcpy(char* dest, char const* src, std::size_t n) noexcept;

template<std::size_t N>
std::size_t strlcpy(char (&dest)[N], chars_view src) noexcept
{
    return strlcpy(dest, src, N);
}

template<std::size_t N>
std::size_t strlcpy(char (&dest)[N], char const* src) noexcept
{
    return strlcpy(dest, src, N);
}
/**
 * @}
 */


/**
 * \resume copies up to \c n - 1 characters from the NUL-terminated string \c src to \c dst, NUL-terminating the result.
 * \return false if \c src is too long
 * @{
 */
[[nodiscard]] inline bool strbcpy(char* dest, chars_view src, std::size_t n) noexcept
{
    return strlcpy(dest, src, n) < n;
}

[[nodiscard]] inline bool strbcpy(char* dest, char const* src, std::size_t n) noexcept
{
    return strlcpy(dest, src, n) < n;
}

template<std::size_t N>
[[nodiscard]] bool strbcpy(char (&dest)[N], chars_view src) noexcept
{
    return strlcpy(dest, src, N) < N;
}

template<std::size_t N>
[[nodiscard]] bool strbcpy(char (&dest)[N], char const* src) noexcept
{
    return strlcpy(dest, src, N) < N;
}
/**
 * @}
 */

} // namespace utils
