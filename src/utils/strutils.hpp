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

#include <string>

#include <cstring>


template<class String, class... Strings>
[[nodiscard]] std::string str_concat(String&& str, Strings const&... strs);

template<class... Strings>
[[nodiscard]] std::string str_concat(std::string&& str, Strings const&... strs);

template<class... Strings>
void str_append(std::string& str, Strings const&... strs);

template<class... Strings>
void str_assign(std::string& str, Strings const&... strs);


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
inline std::size_t strlcpy(char (&dest)[N], chars_view src) noexcept
{
    return strlcpy(dest, src, N);
}

template<std::size_t N>
inline std::size_t strlcpy(char (&dest)[N], char const* src) noexcept
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
[[nodiscard]] inline bool strbcpy(char (&dest)[N], chars_view src) noexcept
{
    return strlcpy(dest, src, N) < N;
}

template<std::size_t N>
[[nodiscard]] inline bool strbcpy(char (&dest)[N], char const* src) noexcept
{
    return strlcpy(dest, src, N) < N;
}
/**
 * @}
 */

} // namespace utils

namespace detail
{
    inline chars_view to_string_view_or_char(chars_view av, int /*dummy*/) noexcept
    {
        return av;
    }

    inline chars_view to_string_view_or_char(char const* s, char /*dummy*/) noexcept
    {
        return {s, ::strlen(s)};
    }

    inline char to_string_view_or_char(char c, int /*dummy*/) noexcept
    {
        return c;
    }


    inline std::size_t len_from_av_or_char(chars_view av) noexcept
    {
        return av.size();
    }

    inline std::size_t len_from_av_or_char(char c) noexcept
    {
        (void)c;
        return 1;
    }


    inline char* append_from_av_or_char(char* s, chars_view av)
    {
        memcpy(s, av.data(), av.size());
        return s + av.size();
    }

    inline char* append_from_av_or_char(char* s, char c)
    {
        *s = c;
        return s + 1;
    }


    template<class... StringsOrChars>
    void str_concat_view(std::string& str, StringsOrChars&&... strs)
    {
        auto ipos = str.size();
        str.resize(str.size() + (... + len_from_av_or_char(strs)));
        auto p = str.data() + ipos;
        (..., void(p = append_from_av_or_char(p, strs)));
    }
} // namespace detail


template<class String, class... Strings>
[[nodiscard]] inline
std::string str_concat(String&& str, Strings const&... strs)
{
    std::string s;
    detail::str_concat_view(s, detail::to_string_view_or_char(str, 1),
                               detail::to_string_view_or_char(strs, 1)...);
    return s;
}

template<class... Strings>
[[nodiscard]] inline
std::string str_concat(std::string&& str, Strings const&... strs)
{
    detail::str_concat_view(str, detail::to_string_view_or_char(strs, 1)...);
    return std::move(str);
}


template<class... Strings>
inline void str_append(std::string& str, Strings const&... strs)
{
    detail::str_concat_view(str, detail::to_string_view_or_char(strs, 1)...);
}


template<class... Strings>
inline void str_assign(std::string& str, Strings const&... strs)
{
    str.clear();
    detail::str_concat_view(str, detail::to_string_view_or_char(strs, 1)...);
}
