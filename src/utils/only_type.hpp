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

#include <type_traits>


template<class T>
struct only_type
{
    only_type() = default;
    only_type(only_type&& /*other*/) = default;
    only_type(only_type const& /*other*/) = default;

    template<class U, class = std::enable_if_t<std::is_same_v<T, std::decay_t<U>>, T>>
    constexpr only_type(U&& x) noexcept(noexcept(T(static_cast<U&&>(x))))
    : value_(static_cast<U&&>(x))
    {}

    T& value() & noexcept
    {
        return value_;
    }

    [[nodiscard]] T const& value() const& noexcept
    {
        return value_;
    }

    T&& value() && noexcept
    {
        return static_cast<T&&>(value);
    }

public:
    T value_;
};
