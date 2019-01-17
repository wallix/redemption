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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <cassert>

template<class T>
struct uninit_checked
{
    T value;

    uninit_checked() = default;
    uninit_checked(uninit_checked &&) = default; /*NOLINT*/
    uninit_checked(uninit_checked const&) = default; /*NOLINT*/

    uninit_checked& operator=(uninit_checked &&) = default; /*NOLINT*/
    uninit_checked& operator=(uninit_checked const&) = default; /*NOLINT*/

    template<class U>
    uninit_checked& operator=(U&& x)
    noexcept(noexcept(*static_cast<T*>(nullptr) = static_cast<U&&>(x)))
    {
        value = static_cast<U&&>(x);
#ifndef NDEBUG
        is_initialized = true;
#endif
        return *this;
    }

    operator T& () noexcept
    {
        assert(this->is_initialized);
        return value;
    }

    operator T const& () const noexcept
    {
        assert(this->is_initialized);
        return value;
    }

#ifndef NDEBUG
    template<class U>
    uninit_checked(U&& x) noexcept(noexcept(T(static_cast<U&&>(x)))) /*NOLINT*/
    : value(static_cast<U&&>(x))
    {}
private:
    bool is_initialized = false;
#else
    // disable uninit_checked(value). Ok for uninit_checked{value};
    uninit_checked(T) = delete;
#endif
};
