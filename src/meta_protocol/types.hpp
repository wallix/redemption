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

#include <type_traits>
#include <cstdint>

namespace meta_protocol {

/// \ingroup meta_protocol

template<std::size_t N>
struct size_ : std::integral_constant<std::size_t, N>
{};

namespace types {

    /**
     * \defgroup Integer width integer types
     * @{
     */

    template<class T> struct type_base;
    template<class T> using type_base_t = typename type_base<T>::type;

    template<class T> struct is_protocol_type : std::false_type {};

    struct le_tag {};
    struct be_tag {};

    /**
     * @{
     * Constants fixed width integer types
     */
    template<class T, T x, class Tag> struct integral {};
    template<class T, T x, class Tag> struct type_base<integral<T, x, Tag>> { using type = T; };
    template<class T, T x, class Tag> struct is_protocol_type<integral<T, x, Tag>> : std::true_type {};

    template<int8_t x> using s8 = integral<int8_t, x, void>;
    template<uint8_t x> using u8 = integral<uint8_t, x, void>;

    template<int16_t x> using s16_be = integral<int16_t, x, be_tag>;
    template<int16_t x> using s16_le = integral<int16_t, x, le_tag>;
    template<uint16_t x> using u16_be = integral<uint16_t, x, be_tag>;
    template<uint16_t x> using u16_le = integral<uint16_t, x, le_tag>;

    template<int32_t x> using s32_be = integral<int32_t, x, be_tag>;
    template<int32_t x> using s32_le = integral<int32_t, x, le_tag>;
    template<uint32_t x> using u32_be = integral<uint32_t, x, be_tag>;
    template<uint32_t x> using u32_le = integral<uint32_t, x, le_tag>;

    template<int64_t x> using s64_be = integral<int64_t, x, be_tag>;
    template<int64_t x> using s64_le = integral<int64_t, x, le_tag>;
    template<uint64_t x> using u64_be = integral<uint64_t, x, be_tag>;
    template<uint64_t x> using u64_le = integral<uint64_t, x, le_tag>;
    /** @} */

    /**
     * @{
     * Parametrable fixed width integer types
     */
    template<class T, class Tag> struct dyn { T x; };
    template<class T, class Tag> struct type_base<dyn<T, Tag>> { using type = T; };
    template<class T, class Tag> struct is_protocol_type<dyn<T, Tag>> : std::true_type {};

    using dyn_s8 = dyn<int8_t, void>;
    using dyn_u8 = dyn<uint8_t, void>;

    using dyn_s16_be = dyn<int16_t, be_tag>;
    using dyn_s16_le = dyn<int16_t, le_tag>;
    using dyn_u16_be = dyn<uint16_t, be_tag>;
    using dyn_u16_le = dyn<uint16_t, le_tag>;

    using dyn_s32_be = dyn<int32_t, be_tag>;
    using dyn_s32_le = dyn<int32_t, le_tag>;
    using dyn_u32_be = dyn<uint32_t, be_tag>;
    using dyn_u32_le = dyn<uint32_t, le_tag>;

    using dyn_s64_be = dyn<int64_t, be_tag>;
    using dyn_s64_le = dyn<int64_t, le_tag>;
    using dyn_u64_be = dyn<uint64_t, be_tag>;
    using dyn_u64_le = dyn<uint64_t, le_tag>;
    /** @} */

    /**
     * @{
     * Expression with fixed width integer types
     */
    template<class T, class Expr, class Tag> struct expr { Expr expr; };
    template<class T, class Expr, class Tag> struct type_base<expr<T, Expr, Tag>> { using type = T; };
    template<class T, class Expr, class Tag> struct is_protocol_type<expr<T, Expr, Tag>> : std::true_type {};

    template<class Expr> using expr_s8 = expr<int8_t, Expr, void>;
    template<class Expr> using expr_u8 = expr<uint8_t, Expr, void>;

    template<class Expr> using expr_s16_be = expr<int16_t, Expr, be_tag>;
    template<class Expr> using expr_s16_le = expr<int16_t, Expr, le_tag>;
    template<class Expr> using expr_u16_be = expr<uint16_t, Expr, be_tag>;
    template<class Expr> using expr_u16_le = expr<uint16_t, Expr, le_tag>;

    template<class Expr> using expr_s32_be = expr<int32_t, Expr, be_tag>;
    template<class Expr> using expr_s32_le = expr<int32_t, Expr, le_tag>;
    template<class Expr> using expr_u32_be = expr<uint32_t, Expr, be_tag>;
    template<class Expr> using expr_u32_le = expr<uint32_t, Expr, le_tag>;

    template<class Expr> using expr_s64_be = expr<int64_t, Expr, be_tag>;
    template<class Expr> using expr_s64_le = expr<int64_t, Expr, le_tag>;
    template<class Expr> using expr_u64_be = expr<uint64_t, Expr, be_tag>;
    template<class Expr> using expr_u64_le = expr<uint64_t, Expr, le_tag>;
    /** @} */


    template<class T>
    size_<sizeof(types::type_base_t<T>)> sizeof_(T const &) {
        return {};
    }
}

/**
 * @{
 * \brief make a constant fixed width integer
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
 * @{
 * \brief make a parametrable fixed width integer
 */
inline types::dyn_s8 s8(int8_t x) { return {x}; }
inline types::dyn_u8 u8(uint8_t x) { return {x}; }

inline types::dyn_s16_be s16_be(int16_t x) { return {x}; }
inline types::dyn_s16_le s16_le(int16_t x) { return {x}; }
inline types::dyn_u16_be u16_be(uint16_t x) { return {x}; }
inline types::dyn_u16_le u16_le(uint16_t x) { return {x}; }

inline types::dyn_s32_be s32_be(int32_t x) { return {x}; }
inline types::dyn_s32_le s32_le(int32_t x) { return {x}; }
inline types::dyn_u32_be u32_be(uint32_t x) { return {x}; }
inline types::dyn_u32_le u32_le(uint32_t x) { return {x}; }

inline types::dyn_s64_be s64_be(int64_t x) { return {x}; }
inline types::dyn_s64_le s64_le(int64_t x) { return {x}; }
inline types::dyn_u64_be u64_be(uint64_t x) { return {x}; }
inline types::dyn_u64_le u64_le(uint64_t x) { return {x}; }
/** @} */

namespace detail_ {
    template<class Expr, template<class> class Tpl>
    using enable_if_expr = typename std::enable_if<!std::is_integral<Expr>::value, Tpl<Expr>>::type;
}

/**
 * @{
 * \brief make a expression fixed width integer
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


namespace types {

    struct none {};

    size_<0> sizeof_(types::none) {
        return {};
    }

    /**
     * \defgroup Condition
     * @{
     */
    template<class Condition, class If, class Else>
    struct if_
    {
        Condition cond;
        If yes;
        Else no;
    };

    template<class Cond, class If, class Else> struct is_protocol_type<if_<Cond, If, Else>> : std::true_type {};
    /** @} */

}

namespace detail_ {
    struct bool_function
    {
        bool_function(bool val)
        : val_(val)
        {}

        bool operator()() const {
            return val_;
        }

    private:
        bool val_;
    };
    template<class T, bool = std::is_integral<T>::value> struct to_function { using type = T; };
    template<class T> struct to_function<T, true> { using type = bool_function; };

    template<class T> struct is_condition_impl : std::false_type {};

    template<class Cond, class If, class Else>
    struct is_condition_impl<types::if_<Cond, If, Else>> : std::true_type {};
}

template<class T> using is_condition = typename detail_::is_condition_impl<
    typename std::remove_reference<typename std::decay<T>::type>::type
>::type;

/**
 * @{
 * \brief make a conditional type
 */
template<class Cond, class If>
types::if_<typename detail_::to_function<Cond>::type, If, types::none>
if_(Cond cond, If yes) {
    return {cond, yes, {}};
}

template<class Cond, class If, class Else>
types::if_<typename detail_::to_function<Cond>::type, If, Else>
if_(Cond cond, If yes, Else no) {
    return {cond, yes, no};
}
/** @} */


namespace types {

    /**
     * \defgroup Sequence
     * @{
     */
    struct bytes
    {
        uint8_t const * data;
        std::size_t sz;
    };

    template<> struct is_protocol_type<bytes> : std::true_type {};

    template<std::size_t N>
    struct array
    {
        uint8_t const (&data) [N];
    };

    template<std::size_t N> struct is_protocol_type<array<N>> : std::true_type {};
    /** @} */

    std::size_t sizeof_(bytes const & x) {
        return x.sz;
    }

    template<std::size_t N>
    size_<N> sizeof_(array<N> const &) {
        return {};
    }

}

/**
 * @{
 * \brief make a sequence of byte
 */
inline types::bytes bytes(char const * data, std::size_t sz) {
    return {reinterpret_cast<uint8_t const *>(data), sz};
}

inline types::bytes bytes(signed char const * data, std::size_t sz) {
    return {reinterpret_cast<uint8_t const *>(data), sz};
}

inline types::bytes bytes(unsigned char const * data, std::size_t sz) {
    return {data, sz};
}

// avort NULL sequence
types::bytes bytes(int, std::size_t) = delete;
types::bytes bytes(decltype(nullptr), std::size_t) = delete;


template<std::size_t N>
types::array<N> array(char const (&arr)[N]) {
    return {reinterpret_cast<uint8_t const (&)[N]>(arr)};
}

template<std::size_t N>
types::array<N> array(signed char const (&arr)[N]) {
    return {reinterpret_cast<uint8_t const (&)[N]>(arr)};
}

template<std::size_t N>
types::array<N> array(unsigned char const (&arr)[N]) {
    return {arr};
}
/** @} */



template<std::size_t n1, std::size_t n2>
size_<n1+n2> operator+(size_<n1> const &, size_<n2> const &) {
    return {};
}

template<std::size_t n2>
std::size_t operator+(std::size_t n1, size_<n2> const &) {
    return n1 + n2;
}

template<std::size_t n1>
std::size_t operator+(size_<n1> const &, std::size_t n2) {
    return n1 + n2;
}


using types::sizeof_;

}

#endif

