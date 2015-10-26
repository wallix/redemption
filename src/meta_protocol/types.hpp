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
#include <cstring> // strlen

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
    template<class T, T x, class Tag> struct out_integral {};
    template<class T, T x, class Tag> struct type_base<out_integral<T, x, Tag>> { using type = T; };
    template<class T, T x, class Tag> struct is_protocol_type<out_integral<T, x, Tag>> : std::true_type {};

    template<int8_t x> using out_s8 = out_integral<int8_t, x, void>;
    template<uint8_t x> using out_u8 = out_integral<uint8_t, x, void>;

    template<int16_t x> using out_s16_be = out_integral<int16_t, x, be_tag>;
    template<int16_t x> using out_s16_le = out_integral<int16_t, x, le_tag>;
    template<uint16_t x> using out_u16_be = out_integral<uint16_t, x, be_tag>;
    template<uint16_t x> using out_u16_le = out_integral<uint16_t, x, le_tag>;

    template<int32_t x> using out_s32_be = out_integral<int32_t, x, be_tag>;
    template<int32_t x> using out_s32_le = out_integral<int32_t, x, le_tag>;
    template<uint32_t x> using out_u32_be = out_integral<uint32_t, x, be_tag>;
    template<uint32_t x> using out_u32_le = out_integral<uint32_t, x, le_tag>;

    template<int64_t x> using out_s64_be = out_integral<int64_t, x, be_tag>;
    template<int64_t x> using out_s64_le = out_integral<int64_t, x, le_tag>;
    template<uint64_t x> using out_u64_be = out_integral<uint64_t, x, be_tag>;
    template<uint64_t x> using out_u64_le = out_integral<uint64_t, x, le_tag>;
    /** @} */

    /**
     * @{
     * Parametrable fixed width integer types
     */
    template<class T, class Tag> struct out_dyn { T x; };
    template<class T, class Tag> struct type_base<out_dyn<T, Tag>> { using type = T; };
    template<class T, class Tag> struct is_protocol_type<out_dyn<T, Tag>> : std::true_type {};

    using out_dyn_s8 = out_dyn<int8_t, void>;
    using out_dyn_u8 = out_dyn<uint8_t, void>;

    using out_dyn_s16_be = out_dyn<int16_t, be_tag>;
    using out_dyn_s16_le = out_dyn<int16_t, le_tag>;
    using out_dyn_u16_be = out_dyn<uint16_t, be_tag>;
    using out_dyn_u16_le = out_dyn<uint16_t, le_tag>;

    using out_dyn_s32_be = out_dyn<int32_t, be_tag>;
    using out_dyn_s32_le = out_dyn<int32_t, le_tag>;
    using out_dyn_u32_be = out_dyn<uint32_t, be_tag>;
    using out_dyn_u32_le = out_dyn<uint32_t, le_tag>;

    using out_dyn_s64_be = out_dyn<int64_t, be_tag>;
    using out_dyn_s64_le = out_dyn<int64_t, le_tag>;
    using out_dyn_u64_be = out_dyn<uint64_t, be_tag>;
    using out_dyn_u64_le = out_dyn<uint64_t, le_tag>;
    /** @} */

    /**
     * @{
     * Expression with fixed width integer types
     */
    template<class T, class Expr, class Tag> struct out_expr { Expr expr; };
    template<class T, class Expr, class Tag> struct type_base<out_expr<T, Expr, Tag>> { using type = T; };
    template<class T, class Expr, class Tag> struct is_protocol_type<out_expr<T, Expr, Tag>> : std::true_type {};

    template<class Expr> using out_expr_s8 = out_expr<int8_t, Expr, void>;
    template<class Expr> using out_expr_u8 = out_expr<uint8_t, Expr, void>;

    template<class Expr> using out_expr_s16_be = out_expr<int16_t, Expr, be_tag>;
    template<class Expr> using out_expr_s16_le = out_expr<int16_t, Expr, le_tag>;
    template<class Expr> using out_expr_u16_be = out_expr<uint16_t, Expr, be_tag>;
    template<class Expr> using out_expr_u16_le = out_expr<uint16_t, Expr, le_tag>;

    template<class Expr> using out_expr_s32_be = out_expr<int32_t, Expr, be_tag>;
    template<class Expr> using out_expr_s32_le = out_expr<int32_t, Expr, le_tag>;
    template<class Expr> using out_expr_u32_be = out_expr<uint32_t, Expr, be_tag>;
    template<class Expr> using out_expr_u32_le = out_expr<uint32_t, Expr, le_tag>;

    template<class Expr> using out_expr_s64_be = out_expr<int64_t, Expr, be_tag>;
    template<class Expr> using out_expr_s64_le = out_expr<int64_t, Expr, le_tag>;
    template<class Expr> using out_expr_u64_be = out_expr<uint64_t, Expr, be_tag>;
    template<class Expr> using out_expr_u64_le = out_expr<uint64_t, Expr, le_tag>;
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
template<int8_t X> types::out_u8<X> out_s8() { return {}; }
template<uint8_t X> types::out_u8<X> out_u8() { return {}; }

template<int16_t X> types::out_u16_be<X> out_s16_be() { return {}; }
template<int16_t X> types::out_u16_le<X> out_s16_le() { return {}; }
template<uint16_t X> types::out_u16_be<X> out_u16_be() { return {}; }
template<uint16_t X> types::out_u16_le<X> out_u16_le() { return {}; }

template<int32_t X> types::out_u32_be<X> out_s32_be() { return {}; }
template<int32_t X> types::out_u32_le<X> out_s32_le() { return {}; }
template<uint32_t X> types::out_u32_be<X> out_u32_be() { return {}; }
template<uint32_t X> types::out_u32_le<X> out_u32_le() { return {}; }

template<int64_t X> types::out_u64_be<X> out_s64_be() { return {}; }
template<int64_t X> types::out_u64_le<X> out_s64_le() { return {}; }
template<uint64_t X> types::out_u64_be<X> out_u64_be() { return {}; }
template<uint64_t X> types::out_u64_le<X> out_u64_le() { return {}; }
/** @} */


/**
 * @{
 * \brief make a parametrable fixed width integer
 */
inline types::out_dyn_s8 out_s8(int8_t x) { return {x}; }
inline types::out_dyn_u8 out_u8(uint8_t x) { return {x}; }

inline types::out_dyn_s16_be out_s16_be(int16_t x) { return {x}; }
inline types::out_dyn_s16_le out_s16_le(int16_t x) { return {x}; }
inline types::out_dyn_u16_be out_u16_be(uint16_t x) { return {x}; }
inline types::out_dyn_u16_le out_u16_le(uint16_t x) { return {x}; }

inline types::out_dyn_s32_be out_s32_be(int32_t x) { return {x}; }
inline types::out_dyn_s32_le out_s32_le(int32_t x) { return {x}; }
inline types::out_dyn_u32_be out_u32_be(uint32_t x) { return {x}; }
inline types::out_dyn_u32_le out_u32_le(uint32_t x) { return {x}; }

inline types::out_dyn_s64_be out_s64_be(int64_t x) { return {x}; }
inline types::out_dyn_s64_le out_s64_le(int64_t x) { return {x}; }
inline types::out_dyn_u64_be out_u64_be(uint64_t x) { return {x}; }
inline types::out_dyn_u64_le out_u64_le(uint64_t x) { return {x}; }
/** @} */

namespace detail_ {
    template<class Expr, template<class> class Tpl>
    using enable_if_expr = typename std::enable_if<!std::is_integral<Expr>::value, Tpl<Expr>>::type;
}

/**
 * @{
 * \brief make a expression fixed width integer
 */
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s8> out_s8(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u8> out_u8(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s16_be>
out_s16_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s16_le>
out_s16_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u16_be>
out_u16_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u16_le>
out_u16_le(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s32_be>
out_s32_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s32_le>
out_s32_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u32_be>
out_u32_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u32_le>
out_u32_le(Expr expr) { return {expr}; }

template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s64_be>
out_s64_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_s64_le>
out_s64_le(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u64_be>
out_u64_be(Expr expr) { return {expr}; }
template<class Expr> detail_::enable_if_expr<Expr, types::out_expr_u64_le>
out_u64_le(Expr expr) { return {expr}; }
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
    struct out_bytes
    {
        uint8_t const * data_;
        std::size_t sz_;

        constexpr uint8_t const * data() const { return this->data_; }
        constexpr size_t size() const { return this->sz_; }
    };

    template<> struct is_protocol_type<out_bytes> : std::true_type {};

    template<std::size_t N>
    struct out_array
    {
        uint8_t const (&data_) [N];
        constexpr uint8_t const * data() const { return this->data_; }
        constexpr size_<N> size() const { return {}; }
    };

    template<std::size_t N> struct is_protocol_type<out_array<N>> : std::true_type {};
    /** @} */

    std::size_t sizeof_(out_bytes const & x) {
        return x.size();
    }

    template<std::size_t N>
    size_<N> sizeof_(out_array<N> const &) {
        return {};
    }

}

/**
 * @{
 * \brief make a sequence of byte
 */
inline types::out_bytes out_bytes(char const * data, std::size_t sz) {
    return {reinterpret_cast<uint8_t const *>(data), sz};
}

inline types::out_bytes out_bytes(signed char const * data, std::size_t sz) {
    return {reinterpret_cast<uint8_t const *>(data), sz};
}

inline types::out_bytes out_bytes(unsigned char const * data, std::size_t sz) {
    return {data, sz};
}

template<std::size_t N>
types::out_bytes str(char const * s) {
    return {reinterpret_cast<uint8_t const *>(s), strlen(s)};
}

// avort NULL sequence
types::out_bytes out_bytes(int, std::size_t) = delete;
types::out_bytes out_bytes(decltype(nullptr), std::size_t) = delete;


template<std::size_t N>
types::out_array<N> out_array(char const (&arr)[N]) {
    return {reinterpret_cast<uint8_t const (&)[N]>(arr)};
}

template<std::size_t N>
types::out_array<N> out_array(signed char const (&arr)[N]) {
    return {reinterpret_cast<uint8_t const (&)[N]>(arr)};
}

template<std::size_t N>
types::out_array<N> out_array(unsigned char const (&arr)[N]) {
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

