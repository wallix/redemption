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

#include <type_traits>
#include <algorithm>
#include <limits>
#include <cassert>


// analogous to static_cast<> for integral types
// with an assert macro if the conversion is overflow or underflow.
template <class Dst, class Src>
/*c++14 constexpr*/ Dst checked_cast(Src value);


// analogous to static_cast<> for integral types,
// except that use std::clamp if the conversion is overflow or underflow.
template <class Dst, class Src>
/*c++14 constexpr*/ Dst saturated_cast(Src value);


// analogous to static_cast<> for integral types
// with an static_assert if the conversion is possibly overflow or underflow.
template <class Dst, class Src>
constexpr Dst safe_cast(Src value);


// integer type with checked_cast
template<class T>
struct checked_int
{
    template<class U>
    /*c++14 constexpr*/ checked_int(U i) noexcept
    : i(checked_cast<T>(i))
    {}

    checked_int & operator = (checked_int const &) = default;

    template<class U>
    checked_int & operator = (U i)
    {
        this->i = checked_cast<T>(i);
        return *this;
    }

    operator T () const noexcept { return this->i; }

private:
    T i;
};


// integer type with saturated_cast
template<class T>
struct saturated_int
{
    template<class U>
    /*c++14 constexpr*/ saturated_int(U i) noexcept
    : i(saturated_cast<T>(i))
    {}

    saturated_int & operator = (saturated_int const &) = default;

    template<class U>
    saturated_int & operator = (U i)
    {
        this->i = saturated_cast<T>(i);
        return *this;
    }

    operator T () const noexcept { return this->i; }

private:
    T i;
};


// integer type with safe_int
template<class T>
struct safe_int
{
    template<class U>
    constexpr safe_int(U i) noexcept
    : i(safe_cast<T>(i))
    {}

    /*c++14 constexpr*/ safe_int & operator = (safe_int const &) = default;

    template<class U>
    /*c++14 constexpr*/ safe_int & operator = (U i)
    {
        this->i = saturated_cast<T>(i);
        return *this;
    }

    constexpr operator T () const noexcept { return this->i; }

private:
    T i;
};


// Implementation


namespace detail
{
    template<class T, class = typename std::is_enum<T>::type>
    struct underlying_type_or_integral
    { using type = T; };

    template<class T>
    struct underlying_type_or_integral<T, std::true_type>
    { using type = typename std::underlying_type<T>::type; };

    template<class T>
    using underlying_type_or_integral_t = typename underlying_type_or_integral<T>::type;

    template <class Dst, class Src>
    constexpr int check_int(Src const &) noexcept
    {
        static_assert(std::is_integral<Src>::value || std::is_enum<Src>::value, "Argument must be an integral.");
        static_assert(std::is_integral<Dst>::value || std::is_enum<Dst>::value, "Dst must be an integral.");
        return 1;
    }

    template <class Dst, class Src>
    /*c++14 constexpr*/ Dst checked_cast(Src value)
    {
    #ifndef NDEBUG
        using dst_limits = std::numeric_limits<Dst>;
        assert(dst_limits::max() >= value);
        assert(dst_limits::min() <= value);
    # endif
        return static_cast<Dst>(value);
    }

    template <class Dst, class Src>
    /*c++14 constexpr*/ Dst saturated_cast(Src value)
    {
        if (std::is_signed<Dst>::value == std::is_signed<Src>::value && sizeof(Dst) >= sizeof(Src)) {
            return static_cast<Dst>(value);
        }

        using dst_limits = std::numeric_limits<Dst>;
        Dst const new_max_value = dst_limits::max() < value ? dst_limits::max() : static_cast<Dst>(value);
        if (!std::is_signed<Dst>::value && std::is_signed<Src>::value) {
            return value < 0 ? Dst{0} : new_max_value;
        }
        if (!std::is_signed<Src>::value) {
            return new_max_value;
        }
        return dst_limits::min() > value ? dst_limits::min() : new_max_value;
    }

    using ull = unsigned long long;
    using ll = long long;

    template<class From, class To>
    struct is_safe_convertible
    {
        using S = std::numeric_limits<From>;
        using D = std::numeric_limits<To>;

        static const bool value
          = (std::is_signed<From>::value ? std::is_signed<To>::value : true)
          && ll(D::min()) <= ll(S::min()) && ull(S::max()) <= ull(D::max())
        ;
    };
}


template <class Dst, class Src>
/*c++14 constexpr*/ Dst checked_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value), "");
    using dst_type = detail::underlying_type_or_integral_t<Dst>;
    using src_type = detail::underlying_type_or_integral_t<Src>;
    return static_cast<Dst>(detail::checked_cast<dst_type>(static_cast<src_type>(value)));
}

template <class Dst>
constexpr Dst checked_cast(Dst value)
{
    static_assert(detail::check_int<Dst>(value), "");
    return value;
}


template <class Dst, class Src>
/*c++14 constexpr*/ Dst saturated_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value), "");
    using dst_type = detail::underlying_type_or_integral_t<Dst>;
    using src_type = detail::underlying_type_or_integral_t<Src>;
    return static_cast<Dst>(detail::saturated_cast<dst_type>(static_cast<src_type>(value)));
}

template <class Dst>
constexpr Dst saturated_cast(Dst value)
{
    static_assert(detail::check_int<Dst>(value), "");
    return value;
}


template<class From, class To>
using is_safe_convertible = std::integral_constant<bool, detail::is_safe_convertible<
    detail::underlying_type_or_integral_t<From>,
    detail::underlying_type_or_integral_t<To>
>::value>;

template <class Dst, class Src>
constexpr Dst safe_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value), "");
    static_assert(is_safe_convertible<Src, Dst>::value, "Unsafe conversion.");
    return static_cast<Dst>(value);
}

template <class Dst>
constexpr Dst safe_cast(Dst value)
{
    static_assert(detail::check_int<Dst>(value), "");
    return value;
}
