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
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/cast.hpp"
#include "cxx/diagnostic.hpp"

#include <cassert>


#define DECLARE_ENUM_MAP(var_name, converted_type, mapped_pairs)   \
    CREATE_ENUM_MAP_CLASS(var_name, converted_type, mapped_pairs); \
    inline constexpr var_name var_name {}

#define CREATE_ENUM_MAP_CLASS(class_name, converted_type, mapped_pairs) \
    CREATE_ENUM_MAP_CLASS_I(class_name, converted_type, mapped_pairs)

#define CREATE_ENUM_MAP_CLASS_I(class_name, converted_type, mapped_pairs) \
    struct class_name                                                     \
    {                                                                     \
        using mapped_type = converted_type;                               \
        using key_type = CREATE_ENUM_MAP_CLASS_KEY_TYPE1(                 \
            CREATE_ENUM_MAP_CLASS_KEY_TYPE mapped_pairs                   \
        );                                                                \
                                                                          \
        constexpr class_name()                                            \
        {                                                                 \
            [[maybe_unused]] bool completed = false;                      \
            constexpr unsigned len = sizeof(map) / sizeof(*map);          \
                                                                          \
            for (unsigned i = 0; i < len + 1; ++i) {                      \
                REDEMPTION_DIAGNOSTIC_PUSH()                              \
                REDEMPTION_DIAGNOSTIC_GCC_ERROR("-Wswitch")               \
                REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-default")      \
                switch (key_type(i)) {                                    \
                    CREATE_ENUM_MAP_CLASS_UNROLL(                         \
                        CREATE_ENUM_MAP_CLASS_CASE_NEXT1 mapped_pairs     \
                    )                                                     \
                }                                                         \
                REDEMPTION_DIAGNOSTIC_POP()                               \
                                                                          \
                assert(i == len);                                         \
                completed = true;                                         \
            }                                                             \
                                                                          \
            assert(completed);                                            \
        }                                                                 \
                                                                          \
        constexpr mapped_type operator()(key_type e) const noexcept       \
        {                                                                 \
            return map[underlying_cast(e)];                               \
        }                                                                 \
                                                                          \
    private:                                                              \
        mapped_type map[CREATE_ENUM_MAP_CLASS_UNROLL(                     \
            CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT1 mapped_pairs        \
        )] {};                                                            \
    }

#define CREATE_ENUM_MAP_CLASS_KEY_TYPE(e, v) e,
#define CREATE_ENUM_MAP_CLASS_KEY_TYPE1(x) CREATE_ENUM_MAP_CLASS_KEY_TYPE1_I(x)
#define CREATE_ENUM_MAP_CLASS_KEY_TYPE1_I(t, x) decltype(t)

#define CREATE_ENUM_MAP_CLASS_UNROLL(...) \
    CREATE_ENUM_MAP_CLASS_UNROLL_I(FINAL(), __VA_ARGS__)

#define CREATE_ENUM_MAP_CLASS_UNROLL_I(suffix, ...) \
    __VA_ARGS__ ## suffix

#define CREATE_ENUM_MAP_CLASS_CASE_NEXT1(e, v)                       \
    case e: map[i] = v; continue; /* NOLINT(bugprone-branch-clone)*/ \
    CREATE_ENUM_MAP_CLASS_CASE_NEXT2
#define CREATE_ENUM_MAP_CLASS_CASE_NEXT2(e, v)                       \
    case e: map[i] = v; continue; /* NOLINT(bugprone-branch-clone)*/ \
    CREATE_ENUM_MAP_CLASS_CASE_NEXT1

#define CREATE_ENUM_MAP_CLASS_CASE_NEXT1FINAL()
#define CREATE_ENUM_MAP_CLASS_CASE_NEXT2FINAL()

#define CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT1(e, v) \
    +1                                                  \
    CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT2
#define CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT2(e, v) \
    +1                                                  \
    CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT1

#define CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT1FINAL()
#define CREATE_ENUM_MAP_CLASS_COUNT_ELEMENT_NEXT2FINAL()
