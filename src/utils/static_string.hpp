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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/bounded_array_view.hpp"
#include "utils/traits/is_null_terminated.hpp"
#include "utils/traits/static_array_desc.hpp"
#include "cxx/compiler_version.hpp"

#include <array>
#include <cstring>


namespace detail
{
    struct static_string_set_size;

    template<class StaticBufferDesc, std::size_t overflow_copy = 0>
    void memcpy_possibly_more(void* dest, void const* src, std::size_t n)
    {
        // compiler optimizes memcpy when size is compile-time
        constexpr std::size_t max_before_memcpy = REDEMPTION_COMP_IS_GCC ? 256 : 128;
        if constexpr (StaticBufferDesc::is_static
                   && StaticBufferDesc::size + overflow_copy <= max_before_memcpy
        ) {
            memcpy(dest, src, StaticBufferDesc::size + overflow_copy);
        }
        else {
            memcpy(dest, src, n + overflow_copy);
        }
    }
} // namespace detail


// N = size without null character
template<std::size_t N>
struct static_string
{
    static_string() noexcept
    {
        m_str[0] = '\0';
    }

    // C++20: default version when N < 128|256
    template<class C, class Bounds = sequence_to_size_bounds_t<C>>
    static_string(C const& cont) /* NOLINT(bugprone-forwarding-reference-overload) */
        noexcept(detail::is_noexcept_array_view_data_size_v<C const&>)
    {
        operator=(cont);
    }

    template<class C, class Bounds = sequence_to_size_bounds_t<C>>
    static_string& operator=(C const& cont) /* NOLINT(bugprone-forwarding-reference-overload) */
        noexcept(detail::is_noexcept_array_view_data_size_v<C const&>)
    {
        static_assert(Bounds::at_most <= N);

        constexpr bool is_zstr = is_null_terminated<C>::value;

        auto av = bounded_array_view_with<char, Bounds>(cont);
        m_len = checked_int(av.size());

        detail::memcpy_possibly_more<
            bounds_to_static_array_desc<Bounds>,
            is_zstr
        >(m_str.data(), av.data(), av.size());

        if constexpr (!is_zstr) {
            m_str[av.size()] = '\0';
        }

        return *this;
    }

    /// \pre i <= N
    char& operator[](std::size_t i) noexcept
    {
        return m_str[i];
    }

    /// \pre i <= N
    char operator[](std::size_t i) const noexcept
    {
        return m_str[i];
    }

    std::size_t size() const noexcept
    {
        return m_len;
    }

    char* data() noexcept
    {
        return m_str.data();
    }

    char const* data() const noexcept
    {
        return m_str.data();
    }

    bool empty() const noexcept
    {
        return m_len == 0;
    }

    char const* c_str() const noexcept
    {
        return m_str.data();
    }

    char* begin() noexcept
    {
        return data();
    }

    char const* begin() const noexcept
    {
        return data();
    }

    char* end() noexcept
    {
        return data() + size();
    }

    char const* end() const noexcept
    {
        return data() + size();
    }

private:
    friend class detail::static_string_set_size;

    detail::select_minimal_size_t<N> m_len = 0;
    std::array<char, N+1> m_str;
};


template<std::size_t MaxSize, class... Strings>
[[nodiscard]] inline auto static_str_concat(Strings const&... strs);

template<std::size_t N, class... Strings>
inline void static_str_assign(static_string<N>& str, Strings const&... strs);


template<std::size_t N>
struct is_null_terminated<static_string<N>>
: std::true_type
{};

namespace detail
{
    template<std::size_t N>
    struct sequence_to_size_bounds_impl<static_string<N>>
    {
        using type = size_bounds<0, N>;
    };


    template<class String>
    struct static_str_len
    {
        static constexpr std::size_t value = sequence_to_size_bounds_t<String>::at_most;
    };

    template<>
    struct static_str_len<char>
    {
        static constexpr std::size_t value = 1;
    };


    inline char to_static_string_view_or_char(char c) noexcept
    {
        return c;
    }

    template<class String>
    inline auto to_static_string_view_or_char(String const& str)
    {
        return bounded_array_view_with<char, sequence_to_size_bounds_t<String>>(str);
    }


    template<std::size_t AtLeast, std::size_t AtMost>
    inline char* append_from_bounded_av_or_char(char* s, bounded_array_view<char, AtLeast, AtMost> av) noexcept
    {
        detail::memcpy_possibly_more<
            static_array_desc<AtLeast == AtMost, AtMost>
        >(s, av.data(), av.size());
        return s + av.size();
    }

    inline char* append_from_bounded_av_or_char(char* s, char c) noexcept
    {
        *s = c;
        return s + 1;
    }


    struct static_string_set_size
    {
        template<std::size_t N>
        static void set_size(static_string<N>& s, std::size_t n) noexcept
        {
            s.m_len = checked_int(n);
        }
    };


    template<class... AvOrChar>
    inline std::size_t static_str_concat_impl(char* p, AvOrChar&&... strs) noexcept
    {
        char* e = p;
        (..., void(e = detail::append_from_bounded_av_or_char(e, strs)));
        *e = '\0';
        return std::size_t(e-p);
    }
} // namespace detail

// test_static_string.cpp fails to compile with gcc-8 and a default value for MaxSize
template<std::size_t MaxSize, class... Strings>
[[nodiscard]] inline auto static_str_concat(Strings const&... strs)
{
    constexpr auto max_size = (std::size_t() + ... + detail::static_str_len<Strings>::value);
    static_assert(max_size <= MaxSize);
    static_string<max_size> str;
    char* p = str.data();
    auto len = detail::static_str_concat_impl(p, detail::to_static_string_view_or_char(strs)...);
    detail::static_string_set_size::set_size(str, len);
    return str;
}

// workaround for gcc-8
template<class... Strings>
[[nodiscard]] inline auto static_str_concat(Strings const&... strs)
{
    return static_str_concat<4096>(strs...);
}

template<std::size_t N, class... Strings>
inline void static_str_assign(static_string<N>& str, Strings const&... strs)
{
    constexpr auto max_size = (std::size_t() + ... + detail::static_str_len<Strings>::value);
    static_assert(max_size <= N);
    char* p = str.data();
    auto len = detail::static_str_concat_impl(p, detail::to_static_string_view_or_char(strs)...);
    detail::static_string_set_size::set_size(str, len);
}
