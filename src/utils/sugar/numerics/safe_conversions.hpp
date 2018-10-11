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

#include "cxx/diagnostic.hpp"

#include <type_traits>
#include <algorithm>
#include <limits>
#include <cassert>


// analogous to static_cast<> for integral types
// with an assert macro if the conversion is overflow or underflow.
template <class Dst, class Src>
constexpr Dst checked_cast(Src value);


// analogous to static_cast<> for integral types,
// except that use std::clamp if the conversion is overflow or underflow.
template <class Dst, class Src>
constexpr Dst saturated_cast(Src value);


// analogous to static_cast<> for integral types
// with an static_assert if the conversion is possibly overflow or underflow.
template <class Dst, class Src>
constexpr Dst safe_cast(Src value);


// integer type with checked_cast
template<class T>
struct checked_int
{
    using value_type = T;

    template<class U>
    constexpr checked_int(U i) noexcept
    : i(checked_cast<T>(i))
    {}

    constexpr checked_int(T i) noexcept
    : i(i)
    {}

    template<class U>
    operator U () const noexcept { return checked_cast<U>(i); }

    operator T () const noexcept { return this->i; }

    T underlying() const noexcept { return this->i; }

private:
    T i;
};


// integer type with saturated_cast
template<class T>
struct saturated_int
{
    using value_type = T;

    template<class U>
    constexpr saturated_int(U i) noexcept
    : i(saturated_cast<T>(i))
    {}

    constexpr saturated_int(T i) noexcept
    : i(i)
    {}

    template<class U>
    operator U () const noexcept { return saturated_cast<U>(i); }

    operator T () const noexcept { return this->i; }

    constexpr T underlying() const noexcept { return this->i; }

private:
    T i;
};


// integer type with safe_cast
template<class T>
struct safe_int
{
    using value_type = T;

    template<class U>
    constexpr safe_int(U i) noexcept
    : i(safe_cast<T>(i))
    {}

    constexpr safe_int(T i) noexcept
    : i(i)
    {}

    template<class U>
    operator U () const noexcept { return safe_cast<U>(i); }

    constexpr operator T () const noexcept { return this->i; }

    constexpr T underlying() const noexcept { return this->i; }

private:
    T i;
};


template<class T> inline checked_int<T>   make_checked_int(T i)   { return {i}; }
template<class T> inline saturated_int<T> make_saturated_int(T i) { return {i}; }
template<class T> inline safe_int<T>      make_safe_int(T i)      { return {i}; }


namespace std
{
    template<class T> struct underlying_type< ::safe_int<T>     > { using type = T; };
    template<class T> struct underlying_type< ::checked_int<T>  > { using type = T; };
    template<class T> struct underlying_type< ::saturated_int<T>> { using type = T; };
}

// Implementation


namespace detail
{
    template<class T, class = void>
    struct underlying_type_or_integral
    { using type = T; };

    template<class T>
    struct underlying_type_or_integral<T, std::enable_if_t<std::is_enum_v<T>>>
    { using type = std::underlying_type_t<T>; };

    template<class T>
    using underlying_type_or_integral_t = typename underlying_type_or_integral<T>::type;

    template<class T>
    struct type_
    {
        using type = T;
    };

    template <class Dst, class Src>
    constexpr int check_int(Src const & /*unused*/) noexcept
    {
        static_assert(std::is_integral<Src>::value || std::is_enum<Src>::value, "Argument must be an integral.");
        static_assert(std::is_integral<Dst>::value || std::is_enum<Dst>::value, "Dst must be an integral.");
        return 1;
    }

    template <class Dst, class Src>
    constexpr Dst checked_cast(type_<Dst> /*unused*/, Src value)
    {
    #ifndef NDEBUG
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wsign-compare")
        using dst_limits = std::numeric_limits<Dst>;
        assert(dst_limits::max() >= value);
        assert(dst_limits::min() <= value);
        REDEMPTION_DIAGNOSTIC_POP
    # endif
        return static_cast<Dst>(value);
    }

    template <class Dst>
    constexpr Dst checked_cast(type_<Dst> /*unused*/, Dst value)
    {
        return value;
    }

    template <class Dst, class Src>
    constexpr Dst saturated_cast(type_<Dst> /*unused*/, Src value)
    {
        if constexpr (std::is_signed<Dst>::value == std::is_signed<Src>::value && sizeof(Dst) >= sizeof(Src)) {
            return static_cast<Dst>(value);
        }

        using dst_limits = std::numeric_limits<Dst>;
        Dst const new_max_value = dst_limits::max() < value ? dst_limits::max() : static_cast<Dst>(value);
        if constexpr (!std::is_signed<Dst>::value && std::is_signed<Src>::value) {
            return value < 0 ? Dst{0} : new_max_value;
        }
        if constexpr (!std::is_signed<Src>::value) {
            return new_max_value;
        }
        return dst_limits::min() > value ? dst_limits::min() : new_max_value;
    }

    template <class Dst>
    constexpr Dst saturated_cast(type_<Dst> /*unused*/, Dst value)
    {
        return value;
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

    template<class From>
    struct is_safe_convertible<From, From>
    {
        static const bool value = true;
    };
}  // namespace detail


template <class Dst, class Src>
constexpr Dst checked_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value) );
    using dst_type = detail::underlying_type_or_integral_t<Dst>;
    using src_type = detail::underlying_type_or_integral_t<Src>;
    return static_cast<Dst>(detail::checked_cast(detail::type_<dst_type>{}, static_cast<src_type>(value)));
}


template <class Dst, class Src>
constexpr Dst saturated_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value) );
    using dst_type = detail::underlying_type_or_integral_t<Dst>;
    using src_type = detail::underlying_type_or_integral_t<Src>;
    return static_cast<Dst>(detail::saturated_cast(detail::type_<dst_type>{}, static_cast<src_type>(value)));
}


template<class From, class To>
using is_safe_convertible = std::integral_constant<bool, detail::is_safe_convertible<
    detail::underlying_type_or_integral_t<From>,
    detail::underlying_type_or_integral_t<To>
>::value>;

template <class Dst, class Src>
constexpr Dst safe_cast(Src value)
{
    static_assert(detail::check_int<Dst>(value) );
    static_assert(is_safe_convertible<Src, Dst>::value, "Unsafe conversion.");
    return static_cast<Dst>(value);
}
