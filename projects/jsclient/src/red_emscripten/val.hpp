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
#include <cassert>


namespace redjs
{
    inline emscripten::val emval_from_view(bytes_view av)
    {
        return emscripten::val(emscripten::typed_memory_view(av.size(), av.data()));
    }

    inline emscripten::val emval_from_view(u16_array_view av)
    {
        return emscripten::val(emscripten::typed_memory_view(av.size(), av.data()));
    }


    template<class T>
    struct from_memory_offset_t
    {
        static_assert(!sizeof(T), "only pointer or reference");
    };

    template<class T>
    struct from_memory_offset_t<T*>
    {
        T* operator()(intptr_t iptr) const noexcept
        {
            return reinterpret_cast<T*>(iptr); /* NOLINT */
        }

        T* operator()(uintptr_t iptr) const noexcept
        {
            return reinterpret_cast<T*>(iptr); /* NOLINT */
        }

        template<class U>
        T* operator()(U iptr) = delete;
    };

    template<class T>
    struct from_memory_offset_t<T&>
    {
        T& operator()(intptr_t iptr) const noexcept
        {
            assert(iptr);
            return *reinterpret_cast<T*>(iptr); /* NOLINT */
        }

        T& operator()(uintptr_t iptr) const noexcept
        {
            assert(iptr);
            return *reinterpret_cast<T*>(iptr); /* NOLINT */
        }

        template<class U>
        T& operator()(U iptr) = delete;
    };

    template<class T>
    constexpr inline from_memory_offset_t<T> from_memory_offset {};

    template<class T>
    uintptr_t to_memory_offset(T* p)
    {
      return reinterpret_cast<uintptr_t>(p); /* NOLINT */
    }

    template<class T>
    uintptr_t to_memory_offset(T& ref)
    {
      return reinterpret_cast<uintptr_t>(&ref); /* NOLINT */
    }

    template<class T>
    uintptr_t to_memory_offset(T&& ref) = delete;


    template<class T>
    struct EmValPtr;

    template<>
    struct EmValPtr<void>
    {
        static uintptr_t i(void const* p) noexcept
        {
            return reinterpret_cast<uintptr_t>(p); /* NOLINT */
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

    inline emscripten::val const& emval_call_arg(emscripten::val const& val) noexcept
    {
        return val;
    }

    template<class T>
    inline auto emval_call_arg(T const& x) noexcept
    {
        if constexpr (std::is_integral_v<T>) {
            static_assert(sizeof(T) <= 4, "uint64_t and int64_t are not supported");
            return x;
        }
        else if constexpr (std::is_enum_v<T>) {
            auto i = std::underlying_type_t<T>(x);
            static_assert(sizeof(i) <= 4, "enum on uint64_t or int64_t are not supported");
            return i;
        }
        else {
            return emval_from_view(array_view{x});
        }
    }

    template<class ReturnType = void, class... Args>
    inline ReturnType emval_call(emscripten::val const& v, char const* name, Args const&... args)
    {
        return v.call<ReturnType>(name, emval_call_arg(args)...);
    }
}
