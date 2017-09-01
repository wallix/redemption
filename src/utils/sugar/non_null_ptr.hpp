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

#include <cassert>


template<class T>
struct non_null_ptr
{
    using pointer = T*;
    using element_type = T;


    non_null_ptr(decltype(nullptr)) = delete;
    non_null_ptr(int) = delete;

    non_null_ptr(T * ptr) noexcept {
        *this = ptr;
    }

    non_null_ptr(non_null_ptr const &) noexcept = default;

    non_null_ptr& operator = (decltype(nullptr)) noexcept = delete;
    non_null_ptr& operator = (int) noexcept = delete;

    non_null_ptr& operator = (T * ptr) noexcept {
        assert(ptr);
        this->ptr_ = ptr;
        return *this;
    }

    non_null_ptr& operator = (non_null_ptr const &) noexcept = default;


    T * get() const noexcept { return this->ptr_; }
    T & operator*() const noexcept { return *this->ptr_; }
    T * operator->() const noexcept { return this->ptr_; }
    operator T * () const noexcept { return this->ptr_; }

private:
    T * ptr_;
};
