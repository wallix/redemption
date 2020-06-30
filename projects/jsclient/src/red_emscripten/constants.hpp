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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include <boost/preprocessor/seq/for_each.hpp>

// usage: RED_JS_BINDING_CONSTANTS(ScopeName, ((name1, value1))((name2, value2)))
#define RED_JS_BINDING_CONSTANTS(name, S)             \
    static struct RedJSBindingInitializer_##name {    \
        RedJSBindingInitializer_##name(){             \
            RED_JS_BINDING_CONSTANTS_I({              \
                let i = 0;                            \
                Module.name = Object.freeze({         \
                    BOOST_PP_SEQ_FOR_EACH(            \
                        RED_JS_CONSTANT_ELEMENT,      \
                        RED_JS_CONSTANT_EXTRACT_NAME, \
                        S                             \
                    )                                 \
                });                                   \
            } BOOST_PP_SEQ_FOR_EACH(                  \
                RED_JS_CONSTANT_ELEMENT,              \
                RED_JS_CONSTANT_EXTRACT_VALUE,        \
                S                                     \
            ));                                       \
        }                                             \
    } RedJSBindingInitializer_##name##_instance

#define RED_JS_BINDING_CONSTANTS_I(...) EM_ASM(__VA_ARGS__)

#define RED_JS_CONSTANT_ELEMENT(r, data, elem) data elem

#define RED_JS_CONSTANT_EXTRACT_NAME(name, x) name: arguments[i++],
#define RED_JS_CONSTANT_EXTRACT_VALUE(name, x), x
