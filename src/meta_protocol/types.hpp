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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_META_PROTOCOL_TYPES_HPP
#define REDEMPTION_SRC_META_PROTOCOL_TYPES_HPP

#include <initializer_list>
#include <type_traits>
#include <functional> // is_placeholder
#include <cstdint>

namespace meta_protocol {

namespace types {

    template<class T> struct type_base;
    template<class T> using type_base_t = typename type_base<T>::type;

    struct le_tag {};
    struct be_tag {};

    /**
     * Constants fixed width integer types
     * @{
     */
    template<class T, T x, class Tag> struct integral_base {};
    template<class T, T x, class Tag> struct type_base<integral_base<T, x, Tag>> { using type = T; };

#define MK(T, name, Tag) template<T x> using name = integral_base<T, x, Tag>

    MK(int8_t, s8, void);
    MK(uint8_t, u8, void);

    MK(int16_t, s16_be, be_tag);
    MK(int16_t, s16_le, le_tag);
    MK(uint16_t, u16_be, be_tag);
    MK(uint16_t, u16_le, le_tag);

    MK(int32_t, s32_be, be_tag);
    MK(int32_t, s32_le, le_tag);
    MK(uint32_t, u32_be, be_tag);
    MK(uint32_t, u32_le, le_tag);

    MK(int64_t, s64_be, be_tag);
    MK(int64_t, s64_le, le_tag);
    MK(uint64_t, u64_be, be_tag);
    MK(uint64_t, u64_le, le_tag);

#undef MK
    /** @} */

    /**
     * Parametrable fixed width integer types
     * @{
     */
    template<class T, class Tag> struct dyn_base { T x; };
    template<class T, class Tag> struct type_base<dyn_base<T, Tag>> { using type = T; };

#define MK(T, name, Tag) using name = dyn_base<T, Tag>

    MK(int8_t, dyn_s8, void);
    MK(uint8_t, dyn_u8, void);

    MK(int16_t, dyn_s16_be, be_tag);
    MK(int16_t, dyn_s16_le, le_tag);
    MK(uint16_t, dyn_u16_be, be_tag);
    MK(uint16_t, dyn_u16_le, le_tag);

    MK(int32_t, dyn_s32_be, be_tag);
    MK(int32_t, dyn_s32_le, le_tag);
    MK(uint32_t, dyn_u32_be, be_tag);
    MK(uint32_t, dyn_u32_le, le_tag);

    MK(int64_t, dyn_s64_be, be_tag);
    MK(int64_t, dyn_s64_le, le_tag);
    MK(uint64_t, dyn_u64_be, be_tag);
    MK(uint64_t, dyn_u64_le, le_tag);

#undef MK
    /** @} */

    /**
     * Expression with fixed width integer types
     * @{
     */
    template<class T, class Expr, class Tag> struct expr_base { Expr expr; };
    template<class T, class Expr, class Tag> struct type_base<expr_base<T, Expr, Tag>> { using type = T; };

#define MK(T, name, Tag) template<class Expr> using name = expr_base<T, Expr, Tag>

    MK(int8_t, expr_s8, void);
    MK(uint8_t, expr_u8, void);

    MK(int16_t, expr_s16_be, be_tag);
    MK(int16_t, expr_s16_le, le_tag);
    MK(uint16_t, expr_u16_be, be_tag);
    MK(uint16_t, expr_u16_le, le_tag);

    MK(int32_t, expr_s32_be, be_tag);
    MK(int32_t, expr_s32_le, le_tag);
    MK(uint32_t, expr_u32_be, be_tag);
    MK(uint32_t, expr_u32_le, le_tag);

    MK(int64_t, expr_s64_be, be_tag);
    MK(int64_t, expr_s64_le, le_tag);
    MK(uint64_t, expr_u64_be, be_tag);
    MK(uint64_t, expr_u64_le, le_tag);

#undef MK
    /** @} */
}

/**
 * \brief make a constant fixed width integer
 * @{
 */
template<int8_t X> types::u8<X> s8() { return {}; }
template<uint8_t X> types::u8<X> u8() { return {}; }

template<int16_t X> types::u16_be<X> s16_be() { return {}; }
template<int16_t X> types::u16_le<X> s16_le() { return {}; }
template<uint16_t X> types::u16_be<X> u16_be() { return {}; }
template<uint16_t X> types::u16_le<X> u16_le() { return {}; }

template<int32_t X> types::u32_be<X> s32_be() { return {}; }
template<int32_t X> types::u32_le<X> s32_le() { return {}; }
template<uint32_t X> types::u32_be<X> u32_be() { return {}; }
template<uint32_t X> types::u32_le<X> u32_le() { return {}; }

template<int64_t X> types::u64_be<X> s64_be() { return {}; }
template<int64_t X> types::u64_le<X> s64_le() { return {}; }
template<uint64_t X> types::u64_be<X> u64_be() { return {}; }
template<uint64_t X> types::u64_le<X> u64_le() { return {}; }
/** @} */


/**
 * \brief make a parametrable fixed width integer
 * @{
 */
types::dyn_s8 s8(int8_t x) { return {x}; }
types::dyn_u8 u8(uint8_t x) { return {x}; }

types::dyn_s16_be s16_be(int16_t x) { return {x}; }
types::dyn_s16_le s16_le(int16_t x) { return {x}; }
types::dyn_u16_be u16_be(uint16_t x) { return {x}; }
types::dyn_u16_le u16_le(uint16_t x) { return {x}; }

types::dyn_s32_be s32_be(int32_t x) { return {x}; }
types::dyn_s32_le s32_le(int32_t x) { return {x}; }
types::dyn_u32_be u32_be(uint32_t x) { return {x}; }
types::dyn_u32_le u32_le(uint32_t x) { return {x}; }

types::dyn_s64_be s64_be(int64_t x) { return {x}; }
types::dyn_s64_le s64_le(int64_t x) { return {x}; }
types::dyn_u64_be u64_be(uint64_t x) { return {x}; }
types::dyn_u64_le u64_le(uint64_t x) { return {x}; }
/** @} */

namespace detail_ {
    template<class Expr, template<class> class Tpl>
    using enable_if_expr = typename std::enable_if<!std::is_integral<Expr>::value, Tpl<Expr>>::type;
}

/**
 * \brief make a expression fixed width integer
 * @{
 */
template<class Expr> detail_::enable_if_expr<Expr, types::expr_s8> s8(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u8> u8(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::expr_s16_be> s16_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_s16_le> s16_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u16_be> u16_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u16_le> u16_le(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::expr_s32_be> s32_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_s32_le> s32_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u32_be> u32_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u32_le> u32_le(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::expr_s64_be> s64_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_s64_le> s64_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u64_be> u64_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::expr_u64_le> u64_le(Expr expr) { return {expr}; }
/** @} */


// template<int, class T> struct placeholder
// {
//     using type = T;
//
//     template<int new_N>
//     using bind = placeholder<new_N, T>;
//
//     template<class... Ts>
//     T & operator()(Ts && ... args) const {
//
//     }
// };
//
// //@{
// template<class T> using _1 = placeholder<1, T>;
// template<class T> using _2 = placeholder<2, T>;
// template<class T> using _3 = placeholder<3, T>;
// template<class T> using _4 = placeholder<4, T>;
// template<class T> using _5 = placeholder<5, T>;
// template<class T> using _6 = placeholder<6, T>;
// template<class T> using _7 = placeholder<7, T>;
// template<class T> using _8 = placeholder<8, T>;
// template<class T> using _9 = placeholder<9, T>;
// //@}


// template<class...> struct packet_type {};
//
// template<class... Ts>
// packet_type<Ts...> packet(Ts...) {
//     return {};
// }
//
// template<class Layout, class... Ts, class... Args>
// void write(Layout layout, packet_type<Ts...>, Args const & ... args) {
//     (void)std::initializer_list<int>{((layout(Ts{}, args...)), 1)...};
// }
//
// template<class Packet>
// struct write_fn
// {
//     template<class Layout, class... Args>
//     void operator()(Layout && layout, Args const & ... args) const {
//         write(layout, Packet::description, args...);
//     }
//
// //     auto values() {}
// };
//
// template<class T> struct static_const { static constexpr T value = T{}; };
// template<class T> constexpr T static_const<T>::value;

}

// namespace std {
//   template<int I>
//   struct is_placeholder< ::meta_protocol::placeholder<I>> : public std::integral_constant<int, I> {};
// }

#endif

