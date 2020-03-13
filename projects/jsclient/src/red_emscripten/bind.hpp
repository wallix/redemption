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

#include <emscripten/bind.h>

namespace redjs
{
    template<class T, class BaseSpecifier = emscripten::internal::NoBaseClass>
    struct class_ : emscripten::class_<T, BaseSpecifier>
    {
        using em_class = emscripten::class_<T, BaseSpecifier>;

        using em_class::em_class;

        template<typename... ConstructorArgs/*, typename... Policies*/>
        EMSCRIPTEN_ALWAYS_INLINE class_ const& constructor(/*Policies... policies*/) const
        {
            em_class::template constructor<ConstructorArgs...>(/*policies...*/);
            return *this;
        }

        template<typename... ConstructorArgs, class F/*, typename... Policies*/>
        EMSCRIPTEN_ALWAYS_INLINE const class_& constructor(F f/*, Policies... policies*/) const
        {
            em_class::constructor(static_cast<T*(*)(ConstructorArgs&&... args)>(f)/*, policies...*/);
            return *this;
        }

        template<class F>
        EMSCRIPTEN_ALWAYS_INLINE class_ const& function_ptr(char const* name, F f) const
        {
            this->function(name, +f, emscripten::allow_raw_pointers());
            return *this;
        }
    };

    template<class F, class... Policies>
    void function(const char* name, F f, Policies... policies)
    {
        emscripten::function(name, +f, policies...);
    }
}
