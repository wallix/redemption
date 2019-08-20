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

#include "utils/sugar/bytes_view.hpp"

#include <emscripten/val.h>

#include <type_traits>


namespace redjs
{
    inline emscripten::val emval_from_view(const_bytes_view av)
    {
        return emscripten::val(emscripten::typed_memory_view(av.size(), av.data()));
    }

    template<class T>
    struct EmValPtr;

    template<>
    struct EmValPtr<void>
    {
        static uintptr_t i(void const* p) noexcept
        {
            return reinterpret_cast<uintptr_t>(p);
        }
    };

    template<> struct EmValPtr<char> : EmValPtr<void> {};
    template<> struct EmValPtr<signed char> : EmValPtr<void> {};
    template<> struct EmValPtr<unsigned char> : EmValPtr<void> {};

    template<class T> struct EmValPtr<T const> : EmValPtr<T> {};

    template<class T>
    inline uintptr_t emval_call_arg(T* p) noexcept
    {
        return EmValPtr<T>::i(p);
    }

    template<class T>
    inline T const& emval_call_arg(T const& x) noexcept
    {
        static_assert(std::is_integral_v<T>);
        static_assert(sizeof(T) != 8, "uint64_t and int64_t are not supported");
        return x;
    }

    template<class ReturnType = void, class... Args>
    inline ReturnType emval_call(emscripten::val const& v, char const* name, Args const&... args)
    {
        return v.call<ReturnType>(name, emval_call_arg(args)...);
    }
}
