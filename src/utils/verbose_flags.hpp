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
#include <cinttypes>


#define REDEMPTION_DECLARE_ENUM_OPS(Prefix, enum_name)                 \
    Prefix enum_name operator | (enum_name x, enum_name y) noexcept    \
    {                                                                  \
        using int_type = std::underlying_type_t<enum_name>;            \
        return enum_name(int_type(x) | int_type(y));                   \
    }                                                                  \
    Prefix enum_name operator & (enum_name x, enum_name y) noexcept    \
    {                                                                  \
        using int_type = std::underlying_type_t<enum_name>;            \
        return enum_name(int_type(x) & int_type(y));                   \
    }                                                                  \
                                                                       \
    Prefix enum_name& operator |= (enum_name& x, enum_name y) noexcept \
    { return x = x | y; }                                              \
    Prefix enum_name& operator &= (enum_name& x, enum_name y) noexcept \
    { return x = x & y; }                                              \
                                                                       \
    Prefix enum_name operator ~ (enum_name x) noexcept                 \
    { return enum_name(~std::underlying_type_t<enum_name>(x)); }


#define REDEMPTION_VERBOSE_FLAGS_DEC_OPS(Prefix, enum_name) \
    enum class enum_name : uint32_t;                        \
    REDEMPTION_DECLARE_ENUM_OPS(Prefix, enum_name)


#define REDEMPTION_VERBOSE_FLAGS(visibility, verbose_member_name) \
    REDEMPTION_VERBOSE_FLAGS_DEC_OPS(constexpr friend, Verbose)   \
    visibility: Verbose const verbose_member_name;                \
    public: enum class Verbose : uint32_t


#define REDEMPTION_VERBOSE_FLAGS_DEF(enum_name)            \
    REDEMPTION_VERBOSE_FLAGS_DEC_OPS(constexpr, enum_name) \
    enum class enum_name : uint32_t
