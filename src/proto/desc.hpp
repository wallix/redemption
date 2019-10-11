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

#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/string_c.hpp"
#include "cxx/diagnostic.hpp"


#include <functional>
#include <type_traits>
#include <cstdint>


#define FWD(x) static_cast<decltype(x)&&>(x)

#define PROTO_ID(...) __VA_ARGS__

#define PROTO_IS_SAME_TYPE(name)                                               \
    template<class T> using is_##name = typename std::is_same<T, name>::value; \
    template<class T> inline constexpr bool is_##name##_v = std::is_same<T, name>::value

#define PROTO_IS_TYPE_I(name, Type, spe_params, type_params) \
    namespace detail                                         \
    {                                                        \
        template<class>                                      \
        struct name##_impl : std::false_type {};             \
                                                             \
        template<PROTO_ID type_params>                       \
        struct name##_impl<Type<PROTO_ID spe_params>>        \
        : std::true_type {};                                 \
    }                                                        \
                                                             \
    template<class T>                                        \
    using name = typename detail::name##_impl<T>::type;      \
    template<class T>                                        \
    inline constexpr bool name##_v = detail::name##_impl<T>::value

#define PROTO_IS_TYPE(name, spe_params, type_params) \
    PROTO_IS_TYPE_I(is_##name, name, spe_params, type_params)

#define PROTO_IS_TYPE_VA(name) PROTO_IS_TYPE(name, (T...), (class...T))
#define PROTO_IS_TYPE1(name) PROTO_IS_TYPE(name, (T), (class T))
#define PROTO_IS_TYPE2(name) PROTO_IS_TYPE(name, (T, U), (class T, class U))
#define PROTO_IS_TYPE_TPL(name) PROTO_IS_TYPE(name, (Tpl...), (template<class> class... Tpl))

#define PROTO_HAS_TYPE(type_name)                                            \
    namespace detail {                                                       \
        template<class T, class = void>                                      \
        struct has_##type_name##_impl : std::false_type {};                  \
                                                                             \
        template<class T>                                                    \
        struct has_##type_name##_impl<T, std::void_t<typename T::type_name>> \
        : std::true_type {};                                                 \
    }                                                                        \
                                                                             \
    template<class T> using has_##type_name                                  \
      = typename detail::has_##type_name##_impl<T>::type;                    \
    template<class T> inline constexpr bool has_##type_name##_v              \
      = detail::has_##type_name##_impl<T>::value

namespace proto
{

template<std::size_t n>
struct static_size {};

template<std::size_t min, std::size_t max>
struct range_size {};

template<std::size_t n>
using limited_size = range_size<0, n>;

struct dyn_size {};

PROTO_IS_TYPE(static_size, (n), (std::size_t n));
PROTO_IS_TYPE(range_size, (min, max), (std::size_t min, std::size_t max));

template<class T>
using is_dynamic_size = typename std::is_same<T, dyn_size>::type;

namespace tags
{
    class static_buffer_tag {};
    class view_buffer_tag {};
    class limited_buffer_tag {};
}

template<class T, T v>
using integral_constant = std::integral_constant<T, v>;
PROTO_IS_TYPE(integral_constant, (T, v), (class T, T v));

template<class T> using data_type_t = typename T::data_type;
template<class T> using value_type_t = typename T::value_type;
template<class T> using name_t = typename T::name;
template<class T> using proto_basic_type_t = typename T::proto_basic_type;

template<class Data, class Name>
struct param
{
    using name = Name;
    using data_type = Data;
};

PROTO_IS_TYPE_VA(param);

template<class X, class Name>
struct value
{
    using name = Name;
    using value_type = X;

    X value;
};

PROTO_IS_TYPE_VA(value);

template<class Data, class... Names>
struct lazy_value
{
    using data_type = Data;

    Data data;
};

PROTO_IS_TYPE_VA(lazy_value);

struct as_param
{
    using proto_basic_type = as_param;
};

PROTO_IS_SAME_TYPE(as_param);

template<class Data, class Name>
struct param_and_lazy_value
{
    using name = Name;
    using data_type = Data;

    static lazy_value<proto::as_param, Name> value() { return {}; }
    static param<Data, Name> type() { return {}; }
};

PROTO_IS_TYPE_VA(param_and_lazy_value);


template<class... xs>
struct tuple : xs...
{
    template<class F>
    decltype(auto) apply(F&& f) &
    {
        return FWD(f)(static_cast<xs&>(*this)...);
    }

    template<class F>
    decltype(auto) apply(F&& f) &&
    {
        return FWD(f)(static_cast<xs&&>(*this)...);
    }

    template<class F>
    decltype(auto) apply(F&& f) const&
    {
        return FWD(f)(static_cast<xs const&>(*this)...);
    }
};

template<class... Xs>
tuple(Xs const&...) -> tuple<Xs...>;

PROTO_HAS_TYPE(proto_basic_type);

namespace concepts
{
    template<class... Xs>
    using has_proto_basic_type = std::enable_if_t<(has_proto_basic_type_v<Xs> && ...)>;
}

template<class Name, class X>
struct named
{
    using name = Name;
    using value = X;

    X x;
};

class no_type {};


template<class Name>
struct label
{
    using name = Name;

    template<class X>
    constexpr auto operator=(X&& x) const
    {
        return value<X&&, Name>{FWD(x)};
    }

    template<class X>
    constexpr auto operator()(X&& x) const
    {
        return value<X&&, Name>{FWD(x)};
    }
};

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
template<class C, C... cs>
jln::string_c<cs...> operator ""_n () noexcept
{ return {}; }
REDEMPTION_DIAGNOSTIC_POP

template<class T>
struct wrap_type
{
    using type = T;
};

#define PROTO_DECL_CLASS_NAME(classname, memname, T)              \
    struct classname {                                            \
        using proto_type = T;                                     \
        proto_type memname;                                       \
        proto_type const& proto_value() const { return memname; } \
        static char const* proto_name() { return #classname; }    \
    }

#define PROTO_LOCAL_NAME(name)                                             \
    struct name : ::proto::label<name> {                                   \
        using ::proto::label<name>::operator=;                             \
                                                                           \
        REDEMPTION_DIAGNOSTIC_PUSH                                         \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function")     \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-local-typedef")       \
        static auto mem() { return [](auto t){                             \
            PROTO_DECL_CLASS_NAME(name, name, typename decltype(t)::type); \
            return ::proto::wrap_type<name>();                             \
        }; }                                                               \
        REDEMPTION_DIAGNOSTIC_POP                                          \
    } name {}

#define PROTO_CLASS_NAME(classname, name)                \
    template<class T = void>                             \
    PROTO_DECL_CLASS_NAME(classname, name, T);           \
                                                         \
    template<>                                           \
    struct classname<void> : ::proto::label<classname<>> \
    {                                                    \
        using ::proto::label<classname<>>::operator=;    \
                                                         \
        static auto mem() { return [](auto t){           \
            return ::proto::wrap_type<                   \
                classname<typename decltype(t)::type>    \
            >();                                         \
        }; }                                             \
    }

#define PROTO_GLOBAL_NAME(classname, name) \
    PROTO_CLASS_NAME(classname, name);     \
    inline constexpr classname<> name

#define PROTO_USE_CLASS_NAME(classname) struct classname<>


template<class Type>
struct data : Type
{
    template<class Name>
    constexpr param_and_lazy_value<Type, Name> operator[](Name const&) const
    {
        return {};
    }
};

template<template<class...> class Tpl>
struct tpl_data
{
    template<class... Type>
    constexpr data<Tpl<Type...>> operator()(Type const&...) const
    {
        return {};
    }
};

template<template<class...> class Tpl>
struct value_data
{
    template<class T> using bind = Tpl<T>;

    template<class Name>
    constexpr lazy_value<Tpl<as_param>, Name> operator[](Name const&) const
    {
        return {};
    }

    template<class... Type>
    constexpr data<Tpl<Type...>> operator()(Type const&...) const
    {
        return {};
    }
};

PROTO_IS_TYPE1(data);
PROTO_IS_TYPE_TPL(value_data);

class dummy {};

namespace datas
{
    template<class Desc, typename Desc::value_type val>
    struct static_value
    {
        using proto_basic_type = Desc;
    };

    namespace types
    {
        struct BinaryData { using proto_basic_type = BinaryData; };
    }

    inline constexpr data<types::BinaryData> binary_data {};


    namespace types
    {
        class le_tag {};
        class be_tag {};
    }

#define PROTO_ALIAS_TYPE(value_name, type_name, ...)       \
    namespace types { struct type_name : __VA_ARGS__ {}; } \
    inline constexpr ::proto::data<types::type_name> value_name {}

#define PROTO_ALIAS_TPL_TYPE(value_name, type_name, ...)                        \
    namespace types { template<class Type> struct type_name : __VA_ARGS__ {}; } \
    inline constexpr ::proto::tpl_data<types::type_name> value_name {}

#define PROTO_ALIAS_TPL_VALUE_TYPE(value_name, type_name)      \
    namespace types { template<class Type> struct type_name { \
        using proto_basic_type = type_name; }; }              \
    inline constexpr ::proto::value_data<types::type_name> value_name {}

    /**
    * fixed width integer types
    * @{
    */
    namespace types
    {
        template<class T, class Endianess>
        struct Integer
        {
            using proto_basic_type = Integer;

            using value_type = T;

            static_assert(std::is_integral_v<T>);
        };
    }

    PROTO_ALIAS_TYPE(s8, S8, Integer<int8_t, void>);
    PROTO_ALIAS_TYPE(u8, U8, Integer<uint8_t, void>);

    PROTO_ALIAS_TYPE(s16_be, S16_be, Integer<int16_t, be_tag>);
    PROTO_ALIAS_TYPE(s16_le, S16_le, Integer<int16_t, le_tag>);
    PROTO_ALIAS_TYPE(u16_be, U16_be, Integer<uint16_t, be_tag>);
    PROTO_ALIAS_TYPE(u16_le, U16_le, Integer<uint16_t, le_tag>);

    PROTO_ALIAS_TYPE(s32_be, S32_be, Integer<int32_t, be_tag>);
    PROTO_ALIAS_TYPE(s32_le, S32_le, Integer<int32_t, le_tag>);
    PROTO_ALIAS_TYPE(u32_be, U32_be, Integer<uint32_t, be_tag>);
    PROTO_ALIAS_TYPE(u32_le, U32_le, Integer<uint32_t, le_tag>);

    PROTO_ALIAS_TYPE(s64_be, S64_be, Integer<int64_t, be_tag>);
    PROTO_ALIAS_TYPE(s64_le, S64_le, Integer<int64_t, le_tag>);
    PROTO_ALIAS_TYPE(u64_be, U64_be, Integer<uint64_t, be_tag>);
    PROTO_ALIAS_TYPE(u64_le, U64_le, Integer<uint64_t, le_tag>);
    /** @} */

    /**
    * string types
    * @{
    */
    namespace types
    {
        class ascii_charset {};
        class unicode_charset {};
        class utf8_charset {};

        class include_zero {};
        class exclude_zero {};
        class no_zero {};

        template<class charset> struct StringData { using proto_basic_type = StringData; };
        template<class charset> struct ZStringData { using proto_basic_type = ZStringData; };

        template<class Integer, class charset>
        struct StringSize { using proto_basic_type = StringSize; };

        template<class Integer, class charset>
        struct ZStringSize { using proto_basic_type = ZStringSize; };

        template<class Size, class data, class ZeroPolicy>
        struct String { using proto_basic_type = String; };
    }

    PROTO_ALIAS_TYPE(utf8_string_data, Utf8StringData, StringData<utf8_charset>);
    PROTO_ALIAS_TYPE(ascii_string_data, AsciiStringData, StringData<ascii_charset>);
    PROTO_ALIAS_TYPE(unicode_string_data, UnicodeStringData, StringData<unicode_charset>);

    PROTO_ALIAS_TYPE(utf8_zstring_data, Utf8ZStringData, ZStringData<utf8_charset>);
    PROTO_ALIAS_TYPE(ascii_zstring_data, AsciiZStringData, ZStringData<ascii_charset>);
    PROTO_ALIAS_TYPE(unicode_zstring_data, UnicodeZStringData, ZStringData<unicode_charset>);

#define PROTO_ALIAS_string_size(value_name, type_name, base, charset) \
    PROTO_ALIAS_TPL_TYPE(value_name, type_name, base<Type, charset>)

    PROTO_ALIAS_string_size(utf8_string_size, Utf8StringSize, StringSize, utf8_charset);
    PROTO_ALIAS_string_size(ascii_string_size, AsciiStringSize, StringSize, ascii_charset);
    PROTO_ALIAS_string_size(unicode_string_size, UnicodeStringSize, StringSize, unicode_charset);

    PROTO_ALIAS_string_size(utf8_zstring_size, Utf8ZStringSize, ZStringSize, utf8_charset);
    PROTO_ALIAS_string_size(ascii_zstring_size, AsciiZStringSize, ZStringSize, ascii_charset);
    PROTO_ALIAS_string_size(unicode_zstring_size, UnicodeZStringSize, ZStringSize, unicode_charset);

#undef PROTO_ALIAS_string_size

#define PROTO_ALIAS_string(value_name, type_name, string_size, string_data, zero_policy) \
    PROTO_ALIAS_TPL_TYPE(value_name, type_name, \
        String<Type, types::string_data, zero_policy>)

    PROTO_ALIAS_string(utf8_zstring_include_zero, Utf8ZStringIncludeZero,
        Utf8ZStringSize, Utf8ZStringData, include_zero);
    PROTO_ALIAS_string(ascii_zstring_include_zero, AsciiZStringIncludeZero,
        AsciiZStringSize, AsciiZStringData, include_zero);
    PROTO_ALIAS_string(unicode_zstring_include_zero, UnicodeZStringIncludeZero,
        UnicodeZStringSize, UnicodeZStringData, include_zero);

    PROTO_ALIAS_string(utf8_zstring_exclude_zero, Utf8ZStringExcludeZero,
        Utf8ZStringSize, Utf8ZStringData, exclude_zero);
    PROTO_ALIAS_string(ascii_zstring_exclude_zero, AsciiZStringExcludeZero,
        AsciiZStringSize, AsciiZStringData, exclude_zero);
    PROTO_ALIAS_string(unicode_zstring_exclude_zero, UnicodeZStringExcludeZero,
        UnicodeZStringSize, UnicodeZStringData, exclude_zero);

    PROTO_ALIAS_string(utf8_string, Utf8String,
        Utf8StringSize, Utf8StringData, no_zero);
    PROTO_ALIAS_string(ascii_string, AsciiString,
        AsciiStringSize, AsciiStringData, no_zero);
    PROTO_ALIAS_string(unicode_string, UnicodeString,
        UnicodeStringSize, UnicodeStringData, no_zero);

#undef PROTO_ALIAS_string
    /** @} */

    namespace types
    {
        template<class Size> struct PktSize
        {
            using data_size = Size;
            using proto_basic_type = PktSize;
        };

        template<class Size> struct NextPktSize
        {
            using data_size = Size;
            using proto_basic_type = NextPktSize;
        };
    }

    inline constexpr tpl_data<types::PktSize> pkt_size {};
    inline constexpr tpl_data<types::NextPktSize> next_pkt_size {};

    namespace values
    {
        PROTO_ALIAS_TPL_VALUE_TYPE(size_bytes, SizeBytes);
        PROTO_ALIAS_TPL_VALUE_TYPE(data, Data);

        PROTO_ALIAS_TYPE(bytes, Bytes, Data<datas::types::U8>);
    }

#undef PROTO_ALIAS_TYPE
#undef PROTO_ALIAS_TPL_TYPE
#undef PROTO_ALIAS_TPL_VALUE_TYPE

}

namespace layouts
{
    class binary_tag {};

    namespace tags
    {
        using ::proto::layouts::binary_tag;
    }

    template<class Tag, class T>
    struct unknown_element
    {
        static_assert(!std::is_same_v<T, T>, "unknown type");
    };

    struct binary_layout
    {
        template<class T>
        struct sizeof_ : unknown_element<binary_tag, T> {};

        template<class T, class Endianess>
        struct sizeof_<datas::types::Integer<T, Endianess>>
        : std::integral_constant<std::size_t, sizeof(T)>
        {};

        template<class Desc, auto x>
        struct sizeof_<datas::static_value<Desc, x>>
        : sizeof_<Desc>
        {};
    };
}

namespace utils
{
    template<class X>
    decltype(auto) maybe_static_value(X&& x)
    {
        using T = std::decay_t<X>;
        if constexpr (is_value_v<T>)
        {
            using I = std::decay_t<decltype(x.x)>;
            if constexpr (is_integral_constant_v<I>)
            {
                return datas::static_value<std::decay_t<decltype(x.desc)>, I::value>{};
            }
            else
            {
                return FWD(x);
            }
        }
        else
        {
            static_assert(is_data_v<T>, "invalide type, accept def or value");
            return FWD(x);
        }
    }
}

namespace algorithms
{
    namespace detail
    {
        template<class X>
        auto read_write_function_element(X&& /*x*/)
        {
            using T = std::decay_t<X>;
            if constexpr (is_data_v<T>)
            {

            }
        }
    }

    template<class... Xs>
    auto read_write_function(Xs&&... xs)
    {
        return tuple<std::decay_t<Xs>...>{FWD(xs)...};
    }
}

}

#undef FWD
