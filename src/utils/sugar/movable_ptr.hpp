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

#include <utility>
#include <cassert>


template<class T>
struct movable_ptr
{
    movable_ptr() = default;
    movable_ptr(T* p) noexcept : p(p) {}

    movable_ptr(movable_ptr const&) = delete;
    movable_ptr& operator=(movable_ptr const&) = delete;

    movable_ptr(movable_ptr&& other) noexcept
      : p(std::exchange(other.p, nullptr))
    {}

    movable_ptr& operator=(movable_ptr&& other) noexcept
    {
        p = std::exchange(other.p, nullptr);
        return *this;
    }

    movable_ptr& operator=(std::nullptr_t) noexcept
    {
        p = nullptr;
        return *this;
    }

    T* operator->() const noexcept
    {
        return p;
    }

    T& operator*() const noexcept
    {
        assert(p);
        return *p;
    }

    T* get() const noexcept
    {
        return p;
    }

    operator T* () const
    {
        return p;
    }

private:
    T* p = nullptr;
};
