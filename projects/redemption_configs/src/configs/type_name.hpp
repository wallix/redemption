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

#include <type_traits>
#include <string_view>
#include <string>

#include <cstddef>
#include <chrono>
#include <vector>

#include "configs/attributes/spec.hpp"


struct string_type_name
{
    char const * first;
    char const * last;

    std::string_view to_sv() const
    {
        return std::string_view{first, std::size_t(last - first)};
    }
};

namespace detail
{
    template<class T, bool = std::is_enum<T>::value>
    struct type_name_impl;

    template<class T>
    struct type_name_impl<T, true>
    {
        static string_type_name impl()
        {
            char const * s = __PRETTY_FUNCTION__;
            std::size_t sz = sizeof(__PRETTY_FUNCTION__) - 1;
#ifdef __clang__
            return {s + 47, s + (47 + (sz-68)/2)};
#elif defined(__GNUG__)
            return {s + 74, s + sz - 1};
#endif
        }
    };
}

template<class T>
std::string_view type_name(T const * = nullptr)
{ return detail::type_name_impl<T>::impl().to_sv(); }

#define CONFIG_DEFINE_TYPE_NAME(name, /*type*/...)               \
    namespace detail {                                           \
        template<bool B> struct type_name_impl<__VA_ARGS__, B> { \
            static string_type_name impl() {                     \
                char const * s = name;                           \
                return {s, s + (sizeof(name) - 1)};              \
            }                                                    \
        };                                                       \
    }

#define CONFIG_DEFINE_TYPE_NAME2(/*type*/...) \
    CONFIG_DEFINE_TYPE_NAME(#__VA_ARGS__, __VA_ARGS__)

#define CONFIG_DEFINE_TYPE(type) \
    CONFIG_DEFINE_TYPE_NAME(#type, type)

CONFIG_DEFINE_TYPE_NAME2(std::string)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::hours)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::minutes)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::seconds)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::milliseconds)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::duration<unsigned, std::ratio<1, 10>>)
CONFIG_DEFINE_TYPE_NAME2(std::chrono::duration<unsigned, std::ratio<1, 100>>)
CONFIG_DEFINE_TYPE_NAME2(std::vector<uint8_t>)
CONFIG_DEFINE_TYPE_NAME("uint8_t", cfg_desc::types::u8)
CONFIG_DEFINE_TYPE_NAME("uint16_t", cfg_desc::types::u16)
CONFIG_DEFINE_TYPE_NAME("uint32_t", cfg_desc::types::u32)
CONFIG_DEFINE_TYPE_NAME("uint64_t", cfg_desc::types::u64)
CONFIG_DEFINE_TYPE_NAME("int8_t", cfg_desc::types::i8)
CONFIG_DEFINE_TYPE_NAME("int16_t", cfg_desc::types::i16)
CONFIG_DEFINE_TYPE_NAME("int32_t", cfg_desc::types::i32)
CONFIG_DEFINE_TYPE_NAME("int64_t", cfg_desc::types::i64)
CONFIG_DEFINE_TYPE_NAME("int", cfg_desc::types::int_)
CONFIG_DEFINE_TYPE_NAME("unsigned", cfg_desc::types::unsigned_)
