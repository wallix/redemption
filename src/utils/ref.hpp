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
Author(s): Proxy Team
*/

#pragma once

template<class T>
struct Ref
{
    Ref(T& ref) noexcept : _p(&ref) {}
    Ref(T&& ref) = delete;

    operator T& () const noexcept { return *_p; }
    T& get() const noexcept { return *_p; }

private:
    T* _p;
};

template<class T>
struct Ref<T const>
{
    Ref(Ref&&) noexcept = default;
    Ref(Ref const&) noexcept = default;

    Ref(Ref<T> const& other) noexcept : Ref(other.get()) {}

    Ref(T const& ref) noexcept : _p(&ref) {}
    Ref(T const&& ref) = delete;
    Ref(T&& ref) = delete;

    operator T const& () const noexcept { return *_p; }
    T const& get() const noexcept { return *_p; }

private:
    T const* _p;
};

template<class T>
using CRef = Ref<T const>;


template<class T>
struct ExplicitRef : Ref<T>
{
    explicit ExplicitRef(T& ref) noexcept : Ref<T>(ref) {}
    explicit ExplicitRef(T&& ref) = delete;
};

template<class T>
struct ExplicitCRef : Ref<T const>
{
    explicit ExplicitCRef(T const& ref) noexcept : Ref<T const>(ref) {}
    explicit ExplicitCRef(T const&& ref) = delete;
    explicit ExplicitCRef(T&& ref) = delete;
};

template<class T>
struct ExplicitRef<T const> : ExplicitCRef<T>
{
    using ExplicitCRef<T>::ExplicitCRef;
};
