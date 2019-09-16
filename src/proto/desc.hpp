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


template<class Data, class Name>
struct param
{
    using name = Name;
    using data_type = Data;

    Data data;
};

PROTO_IS_TYPE_VA(param);

template<class X, class... Name>
struct value
{
    using value_type = X;

    X value;
};

PROTO_IS_TYPE_VA(value);


template<class tag, class... xs>
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
tuple(Xs&&...) -> tuple<void, std::remove_reference_t<Xs>...>;

#define PROTO_IS_TUPLE_TAG(tag) PROTO_IS_TYPE_I(is_tuple_##name, tuple, (tag, T...), (class...T))

PROTO_HAS_TYPE(proto_basic_type);

namespace concepts
{
    template<class... Xs>
    using has_proto_basic_type = std::enable_if_t<(has_proto_basic_type_v<Xs> && ...)>;
}

class Definition;

PROTO_IS_TUPLE_TAG(Definition);

template<class... Xs>
tuple<Definition, std::decay_t<Xs>...> definition(Xs&&... xs)
{
    return tuple<Definition, std::decay_t<Xs>...>{FWD(xs)...};
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

#define PROTO_LOCAL_NAME(name)                                         \
    struct name : ::proto::label<name> {                               \
        using ::proto::label<name>::operator=;                         \
                                                                       \
        REDEMPTION_DIAGNOSTIC_PUSH                                     \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function") \
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-local-typedef")   \
        static auto mem() { return [](auto t){                         \
            struct name {                                              \
                using proto_type = typename decltype(t)::type;         \
                proto_type name;                                       \
            };                                                         \
            return ::proto::wrap_type<name>();                         \
        }; }                                                           \
        REDEMPTION_DIAGNOSTIC_POP                                      \
    } name {}

#define PROTO_NAME(name) inline constexpr struct name : ::proto::label<name> {} name {}

#define PROTO_NAME_AND_MEM(name)                    \
    inline constexpr                                \
    struct name : ::proto::label<name> {            \
        using proto_name_type = name;               \
        template<class X> struct value { X name; }; \
    } name {}


template<class Type>
struct data : Type
{
    using Type::Type;

    // template<class X>
    // constexpr auto operator=(X&& x) const
    // {
    //     return value<data, X&&>{static_cast<Type const&>(*this), FWD(x)};
    // }
    //
    // template<class X>
    // constexpr auto operator()(X&& x) const
    // {
    //     return value<data, X&&>{static_cast<Type const&>(*this), FWD(x)};
    // }

    template<class Name>
    constexpr auto operator[](label<Name>) const
    {
        return param<data, Name>{/*static_cast<Type const&>(*this)*/};
    }
};

PROTO_IS_TYPE1(data);

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
    inline constexpr data<types::type_name> value_name {};

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

        template<class charset> struct Stringdata { using proto_basic_type = Stringdata; };
        template<class charset> struct ZStringdata { using proto_basic_type = ZStringdata; };

        template<class Integer, class charset>
        struct StringSize { using proto_basic_type = StringSize; };

        template<class Integer, class charset>
        struct ZStringSize { using proto_basic_type = ZStringSize; };

        template<class Size, class data, class ZeroPolicy>
        struct String { using proto_basic_type = String; };
    }

    PROTO_ALIAS_TYPE(utf8_string_data, Utf8Stringdata, Stringdata<utf8_charset>);
    PROTO_ALIAS_TYPE(ascii_string_data, AsciiStringdata, Stringdata<ascii_charset>);
    PROTO_ALIAS_TYPE(unicode_string_data, UnicodeStringdata, Stringdata<unicode_charset>);

    PROTO_ALIAS_TYPE(utf8_zstring_data, Utf8Zstringdata, ZStringdata<utf8_charset>);
    PROTO_ALIAS_TYPE(ascii_zstring_data, AsciiZstringdata, ZStringdata<ascii_charset>);
    PROTO_ALIAS_TYPE(unicode_zstring_data, UnicodeZstringdata, ZStringdata<unicode_charset>);

#define PROTO_ALIAS_string_size(value_name, type_name, base, charset)                         \
    namespace types { template<class Integer> struct type_name : base<Integer, charset> {}; } \
    template<class Integer> constexpr inline auto value_name = types::type_name<Integer>{}

    PROTO_ALIAS_string_size(utf8_string_size, Utf8StringSize, StringSize, utf8_charset);
    PROTO_ALIAS_string_size(ascii_string_size, AsciiStringSize, StringSize, ascii_charset);
    PROTO_ALIAS_string_size(unicode_string_size, UnicodeStringSize, StringSize, unicode_charset);

    PROTO_ALIAS_string_size(utf8_zstring_size, Utf8ZStringSize, ZStringSize, utf8_charset);
    PROTO_ALIAS_string_size(ascii_zstring_size, AsciiZStringSize, ZStringSize, ascii_charset);
    PROTO_ALIAS_string_size(unicode_zstring_size, UnicodeZStringSize, ZStringSize, unicode_charset);

#undef PROTO_ALIAS_string_size

#define PROTO_ALIAS_string(value_name, type_name, string_size, string_data, zero_policy) \
    namespace types { template<class Integer> struct type_name                           \
      : String<types::string_size<Integer>, types::string_data, zero_policy> {}; }       \
    template<class Integer> constexpr inline auto value_name = types::type_name<Integer>{}

    PROTO_ALIAS_string(utf8_zstring_include_zero, Utf8ZStringIncludeZero,
        Utf8ZStringSize, Utf8Zstringdata, include_zero);
    PROTO_ALIAS_string(ascii_zstring_include_zero, AsciiZStringIncludeZero,
        AsciiZStringSize, AsciiZstringdata, include_zero);
    PROTO_ALIAS_string(unicode_zstring_include_zero, UnicodeZStringIncludeZero,
        UnicodeZStringSize, UnicodeZstringdata, include_zero);

    PROTO_ALIAS_string(utf8_zstring_exclude_zero, Utf8ZStringExcludeZero,
        Utf8ZStringSize, Utf8Zstringdata, exclude_zero);
    PROTO_ALIAS_string(ascii_zstring_exclude_zero, AsciiZStringExcludeZero,
        AsciiZStringSize, AsciiZstringdata, exclude_zero);
    PROTO_ALIAS_string(unicode_zstring_exclude_zero, UnicodeZStringExcludeZero,
        UnicodeZStringSize, UnicodeZstringdata, exclude_zero);

    PROTO_ALIAS_string(utf8_zstring, Utf8ZStringZero,
        Utf8ZStringSize, Utf8Zstringdata, no_zero);
    PROTO_ALIAS_string(ascii_zstring, AsciiZStringZero,
        AsciiZStringSize, AsciiZstringdata, no_zero);
    PROTO_ALIAS_string(unicode_zstring, UnicodeZStringZero,
        UnicodeZStringSize, UnicodeZstringdata, no_zero);

#undef PROTO_ALIAS_string
    /** @} */

#undef PROTO_ALIAS_TYPE

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
