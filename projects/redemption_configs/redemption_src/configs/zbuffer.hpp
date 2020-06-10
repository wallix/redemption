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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <limits>
#include <string>
#include <array>
#include <chrono>
#include <type_traits>
#include <iosfwd>

#include "utils/sugar/zstring_view.hpp"


namespace configs
{

namespace detail
{
    template<class T,
        bool IsIntegral = std::is_integral<T>::value,
        bool IsEnum = std::is_enum<T>::value>
    struct str_buffer_size_impl
    {
        static_assert(IsIntegral || IsEnum, "missing str_buffer_size specialization");
    };

    template<class TInt>
    constexpr std::size_t integral_buffer_size()
    {
        return std::numeric_limits<TInt>::digits10 + 1 + std::numeric_limits<TInt>::is_signed;
    }

    template<class T>
    struct str_buffer_size_impl<T, true, false>
    {
        static const std::size_t value = integral_buffer_size<T>();
    };

    template<class T>
    struct str_buffer_size_impl<T, false, true>
    {
        static const std::size_t value = integral_buffer_size<std::underlying_type_t<T>>();
    };
} // namespace detail

template<class T>
struct str_buffer_size
: detail::str_buffer_size_impl<T>
{};

template<class T, class Ratio>
struct str_buffer_size<std::chrono::duration<T, Ratio>>
: str_buffer_size<T>
{};

template<> struct str_buffer_size<std::string>
{
    static const std::size_t value = 0;
};

template<std::size_t N> struct str_buffer_size<char[N]>
{
    static const std::size_t value = 0;
};

template<std::size_t N>
struct str_buffer_size<std::array<unsigned char, N>>
{
    static const std::size_t value = N*2;
};

namespace spec_types
{
    struct directory_path;
}

template<>
struct str_buffer_size<spec_types::directory_path>
{
    static const std::size_t value = 0;
};


template<class> struct spec_type {};

namespace spec_types
{
    class fixed_binary;
    class fixed_string;
    template<class T> class list;
    using ip = std::string;

    template<class T>
    struct underlying_type_for_range
    {
        using type = T;
    };

    template<class Rep, class Period>
    struct underlying_type_for_range<std::chrono::duration<Rep, Period>>
    {
        using type = Rep;
    };

    template<class T>
    using underlying_type_for_range_t = typename underlying_type_for_range<T>::type;

    template<class T, underlying_type_for_range_t<T> min, underlying_type_for_range_t<T> max>
    struct range {};

    struct directory_path
    {
        directory_path()
        : path("./")
        {}

        directory_path(std::string path)
        : path(std::move(path))
        {
            this->normalize();
        }

        directory_path(char const * path)
        : path(path)
        {
            this->normalize();
        }

        directory_path(zstring_view path)
        : path(path.begin(), path.end())
        {
            this->normalize();
        }

        directory_path(directory_path &&) = default;
        directory_path(directory_path const &) = default;

        directory_path & operator = (std::string new_path)
        {
            this->path = std::move(new_path);
            this->normalize();
            return *this;
        }

        directory_path & operator = (char const * new_path)
        {
            this->path = new_path;
            this->normalize();
            return *this;
        }

        directory_path & operator = (zstring_view new_path)
        {
            this->path.assign(new_path.begin(), new_path.end());
            this->normalize();
            return *this;
        }

        directory_path & operator = (directory_path &&) = default;
        directory_path & operator = (directory_path const &) = default;

        [[nodiscard]] char const * c_str() const noexcept { return this->path.c_str(); }

        [[nodiscard]] std::string const & as_string() const noexcept { return this->path; }

    private:
        void normalize()
        {
            if (this->path.empty()) {
                this->path = "./";
            }
            else {
                if (this->path.front() != '/') {
                    if (not(this->path.size() >= 2
                     && this->path[0] == '.'
                     && this->path[1] == '/')
                    ) {
                        this->path.insert(0, "./");
                    }
                }
                if (this->path.back() != '/') {
                    this->path += '/';
                }
            }
        }

        std::string path;
    };

    inline bool operator == (directory_path const & x, directory_path const & y)
    { return x.as_string() == y.as_string(); }

    inline bool operator == (std::string const & x, directory_path const & y)
    { return x == y.as_string(); }

    inline bool operator == (char const * x, directory_path const & y)
    { return x == y.as_string(); }

    inline bool operator == (directory_path const & x, std::string const & y)
    { return x.as_string() == y; }

    inline bool operator == (directory_path const & x, char const * y)
    { return x.as_string() == y; }

    inline bool operator != (directory_path const & x, directory_path const & y)
    { return !(x == y); }

    inline bool operator != (std::string const & x, directory_path const & y)
    { return !(x == y); }

    inline bool operator != (char const * x, directory_path const & y)
    { return !(x == y); }

    inline bool operator != (directory_path const & x, std::string const & y)
    { return !(x == y); }

    inline bool operator != (directory_path const & x, char const * y)
    { return !(x == y); }

    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> & operator << (std::basic_ostream<Ch, Tr> & out, directory_path const & path)
    { return out << path.as_string(); }
} // namespace spec_types

} // namespace configs
