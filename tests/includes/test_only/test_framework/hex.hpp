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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/compare_collection.hpp"

#include <type_traits>


/**
 * Declare hex8, hex16, hex32, hex64 and minimal_hex
 */

namespace ut
{
#define DEF_HEX(name, type)    \
    struct name                \
    {                          \
        using int_type = type; \
        int_type x;            \
        HEX_OP(name, |)        \
        HEX_OP(name, &)        \
        HEX_OP_BOOL(name, ==)  \
        HEX_OP_BOOL(name, !=)  \
        HEX_OP_BOOL(name, <)   \
        HEX_OP_BOOL(name, >)   \
        HEX_OP_BOOL(name, <=)  \
        HEX_OP_BOOL(name, >=)  \
    };                         \
    HEX_IDE_OP(name)
#define HEX_OP(name, op) name operator op (name const& other) const { return {int_type(x op other.x)}; }
#define HEX_OP_BOOL(name, op) bool operator op (name const& other) const { return x op other.x; }
#ifdef IN_IDE_PARSER
# define HEX_IDE_OP(T) /*for IDE*/                \
    bool operator == (T const&, uint64_t const&); \
    bool operator != (T const&, uint64_t const&); \
    bool operator == (uint64_t const&, T const&); \
    bool operator != (uint64_t const&, T const&);
#else
# define HEX_IDE_OP(T)
#endif

DEF_HEX(hex8, uint8_t)
DEF_HEX(hex16, uint16_t)
DEF_HEX(hex32, uint32_t)
DEF_HEX(hex64, uint64_t)
DEF_HEX(minimal_hex, uint64_t)

} // namespace ut

#if REDEMPTION_UNIT_TEST_FAST_CHECK
#define HEX_OSTREAM(T)
#else
namespace ut
{
    inline std::ostream& boost_test_print_type(std::ostream& out, minimal_hex const& h)
    {
        int n = 1;
        uint64_t x = h.x;
        if (x > 0xffffffffu) {
            n += 4;
            x >>= 32;
        }
        if (x > 0xffffu) {
            n += 2;
            x >>= 16;
        }
        if (x > 0xffu) {
            n += 1;
        }
        print_hex(out, h.x, n);
        return out;
    }

    namespace detail
    {
        template<class T> struct is_hex_int { static const bool value = false; };
        template<> struct is_hex_int<minimal_hex> { static const bool value = true; };

        template<class T, class U>
        assertion_result test_hex_EQ(T x, U y)
        {
            using printer = put_default_print_type_t;
            if constexpr (is_hex_int<T>::value && is_hex_int<U>::value) {
                return create_assertion_result(
                    (x.x == y.x), x, "!=", y, printer{});
            }
            else if constexpr (is_hex_int<T>::value) {
                const auto max = typename T::int_type(~0ull);
                if (max < y) {
                    return create_assertion_result(
                        (x.x == y), x, "!=", minimal_hex{uint64_t(y)}, printer{});
                }
                else {
                    return create_assertion_result(
                        (x.x == y), x, "!=", T{typename T::int_type(y)}, printer{});
                }
            }
            else {
                const auto max = typename U::int_type(~0ull);
                if (max < x) {
                    return create_assertion_result(
                        (x == y.x), minimal_hex{uint64_t(x)}, "!=", y, printer{});
                }
                else {
                    return create_assertion_result(
                        (x == y.x), U{typename U::int_type(x)}, "!=", y, printer{});
                }
            }
        }
    }
}

RED_TEST_DISPATCH_COMPARISON_EQ2(
    (class T, class U), (T), (U),
    (std::enable_if_t<(::ut::detail::is_hex_int<T>::value || ::ut::detail::is_hex_int<U>::value)>),
    ::ut::detail::test_hex_EQ)

#define HEX_OSTREAM(T) namespace ut {                                         \
    namespace detail                                                          \
    {                                                                         \
        template<> struct is_hex_int<T> { static const bool value = true; };  \
    }                                                                         \
    inline std::ostream& boost_test_print_type(std::ostream& out, T const& h) \
    {                                                                         \
        print_hex(out, h.x, sizeof(h.x));                                     \
        return out;                                                           \
    }                                                                         \
} // namespace ut
#endif

HEX_OSTREAM(hex8)
HEX_OSTREAM(hex16)
HEX_OSTREAM(hex32)
HEX_OSTREAM(hex64)

#undef HEX_OSTREAM
#undef HEX_OP_BOOL
#undef HEX_IDE_OP
#undef DEF_HEX
#undef HEX_OP
