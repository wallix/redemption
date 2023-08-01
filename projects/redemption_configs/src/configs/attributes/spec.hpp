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

#include <cstddef>
#include <type_traits>

#include <string_view>

#include <cassert>

#include "utils/sugar/array_view.hpp"


namespace cfg_desc
{

#define TYPE_REQUIEREMENT(T)                                           \
    static_assert(!std::is_arithmetic_v<T> || std::is_same_v<T, bool>, \
        "T cannot be an arithmetic type, "                             \
        "use types::u8, types::u16, types::s16, etc instead, "         \
        "eventually types::int_ or types::unsigned_")


#define MK_ENUM_OP(T)                                                     \
    constexpr T operator | (T x, T y)                                     \
    { return static_cast<T>(static_cast<int>(x) | static_cast<int>(y)); } \
    constexpr T operator & (T x, T y)                                     \
    { return static_cast<T>(static_cast<int>(x) & static_cast<int>(y)); } \
    constexpr T operator ~ (T x)                                          \
    { return static_cast<T>(~static_cast<int>(x)); }                      \
    constexpr T& operator |= (T& x, T y) { x = x | y; return x; }         \
    constexpr T& operator &= (T& x, T y) { x = x & y; return x; }


enum class TagList : unsigned
{
    None,
    Debug,
    Workaround,
};
MK_ENUM_OP(TagList)

struct Tags
{
    TagList value = TagList::None;
};

enum class DestSpecFile : uint8_t
{
    none         = 0,
    ini_only     = 1 << 0,
    global_spec  = 1 << 1,
    rdp          = 1 << 2,
    vnc          = 1 << 3,
    jh           = 1 << 4,
};
MK_ENUM_OP(DestSpecFile)


enum class ResetBackToSelector : bool { No, Yes };

enum class Loggable : uint8_t { No, Yes, OnlyWhenContainsPasswordString, };


struct names
{
    std::string_view all;
    std::string_view ini {};
    std::string_view acl {};
    std::string_view connpolicy {};
    std::string_view display {};

    std::string_view cpp_name() const { assert(!all.empty()); return all; }
    std::string_view ini_name() const { return ini.empty() ? all : ini; }
    std::string_view acl_name() const { return acl.empty() ? all : acl; }
    std::string_view connpolicy_name() const { return connpolicy.empty() ? all : connpolicy; }
};

namespace impl
{
    struct integer_base {};
    struct signed_base : integer_base {};
    struct unsigned_base : integer_base {};
}

namespace types
{
    struct u8 : impl::unsigned_base {};
    struct u16 : impl::unsigned_base {};
    struct u32 : impl::unsigned_base {};
    struct u64 : impl::unsigned_base {};

    struct i8 : impl::signed_base {};
    struct i16 : impl::signed_base {};
    struct i32 : impl::signed_base {};
    struct i64 : impl::signed_base {};

    struct unsigned_ : impl::unsigned_base {};
    struct int_ : impl::signed_base {};

    struct rgb {};

    template<unsigned Len> struct fixed_string {};
    template<unsigned Len> struct fixed_binary {};

    template<class T>
    struct list
    {
        TYPE_REQUIEREMENT(T);
    };

    struct ip_string {};

    struct dirpath {};

    template<class T, long min, long max>
    struct range
    {
        TYPE_REQUIEREMENT(T);
    };

    template<class T>
    struct megabytes
    {
        static_assert(std::is_base_of_v<impl::unsigned_base, T>);
    };
}

namespace cpp
{
    struct expr { char const * value; };
    #define CPP_EXPR(expression) ::cfg_desc::cpp::expr{#expression}
}



enum class SpecAttributes : uint16_t
{
    none            = 0,
    logged          = 1 << 0,
    hex             = 1 << 1,
    advanced        = 1 << 2,
    iptables        = 1 << 3,
    password        = 1 << 4,
    image           = 1 << 5,
    external        = 1 << 6,
    restart_service = 1 << 7,
};

MK_ENUM_OP(SpecAttributes)


enum class SesmanIO : uint8_t
{
    no_acl       = 0,
    acl_to_proxy = 1 << 0,
    proxy_to_acl = 1 << 1,
    rw           = acl_to_proxy | proxy_to_acl,
};

MK_ENUM_OP(SesmanIO)

struct SpecInfo
{
    DestSpecFile dest;
    SesmanIO acl_io;
    SpecAttributes attributes;
    ResetBackToSelector reset_back_to_selector;
    Loggable loggable;
    std::string_view image_path;
};

struct SesmanInfo
{
    SesmanIO acl_io;
    ResetBackToSelector reset_back_to_selector;
    Loggable loggable;

    operator SpecInfo () const
    {
        return {
            DestSpecFile::none,
            acl_io,
            SpecAttributes::none,
            reset_back_to_selector,
            loggable,
            std::string_view(),
        };
    }
};

template<bool has_image, bool is_compatible_connpolicy>
struct CheckedSpecAttributes
{
    SpecAttributes attr;
    std::string_view image_path {};

    template<bool has_image2, bool is_compatible_connpolicy2>
    constexpr CheckedSpecAttributes<
        has_image || has_image2,
        is_compatible_connpolicy || is_compatible_connpolicy2
    >
    operator | (CheckedSpecAttributes<has_image2, is_compatible_connpolicy2> other) const
    {
        static_assert(!(has_image && has_image2), "Image specified twice");

        return {
            attr | other.attr,
            has_image ? image_path : other.image_path,
        };
    }
};

namespace spec
{
    constexpr inline CheckedSpecAttributes<false, true> hex {SpecAttributes::hex};
    constexpr inline CheckedSpecAttributes<false, true> advanced {SpecAttributes::advanced};
    constexpr inline CheckedSpecAttributes<false, false> iptables {SpecAttributes::iptables};
    constexpr inline CheckedSpecAttributes<false, true> password {SpecAttributes::password};
    constexpr inline CheckedSpecAttributes<false, true> acl_only {SpecAttributes::external};
    constexpr inline CheckedSpecAttributes<false, false> restart_service {SpecAttributes::restart_service};
    constexpr inline CheckedSpecAttributes<false, false> logged {SpecAttributes::logged};

    constexpr CheckedSpecAttributes<true, false> image(std::string_view image_path)
    {
        return {SpecAttributes::image, image_path};
    }


    constexpr inline SesmanInfo no_acl {SesmanIO::no_acl, ResetBackToSelector::No, Loggable::No};

    constexpr SesmanInfo acl_to_proxy(ResetBackToSelector reset_back_to_selector, Loggable loggable)
    {
        return {SesmanIO::acl_to_proxy, reset_back_to_selector, loggable};
    }

    constexpr SesmanInfo proxy_to_acl(ResetBackToSelector reset_back_to_selector)
    {
        return {SesmanIO::proxy_to_acl, reset_back_to_selector, Loggable::No};
    }

    constexpr SesmanInfo acl_rw(ResetBackToSelector reset_back_to_selector, Loggable loggable)
    {
        return {SesmanIO::rw, reset_back_to_selector, loggable};
    }


    template<bool is_compatible_connpolicy = true>
    constexpr SpecInfo ini_only(SesmanInfo acl)
    {
        return {
            DestSpecFile::ini_only,
            acl.acl_io,
            SpecAttributes(),
            acl.reset_back_to_selector,
            acl.loggable,
            std::string_view(),
        };
    }

    template<bool has_image = false, bool is_compatible_connpolicy = true>
    constexpr SpecInfo global_spec(
        SesmanInfo acl,
        CheckedSpecAttributes<has_image, is_compatible_connpolicy> checked_attr = {})
    {
        return {
            DestSpecFile::global_spec,
            acl.acl_io,
            checked_attr.attr,
            acl.reset_back_to_selector,
            acl.loggable,
            checked_attr.image_path,
        };
    }

    template<bool has_image = false, bool is_compatible_connpolicy = true>
    constexpr SpecInfo external(
        CheckedSpecAttributes<has_image, is_compatible_connpolicy> checked_attr = {})
    {
        return {
            DestSpecFile::global_spec,
            SesmanIO(),
            checked_attr.attr | SpecAttributes::external,
            ResetBackToSelector::No,
            Loggable::No,
            checked_attr.image_path,
        };
    }

    constexpr SpecInfo connpolicy(
        DestSpecFile dest,
        Loggable loggable,
        CheckedSpecAttributes<false, true> checked_attr = {})
    {
        return {
            dest,
            SesmanIO::acl_to_proxy,
            checked_attr.attr,
            ResetBackToSelector::No,
            loggable,
            std::string_view(),
        };
    }
}

}
