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

#pragma once

#include <type_traits>

#include <cstdint>
#include <cassert>
#include <string_view>
#include <cstring>
#include <vector>

#include "utils/sugar/array.hpp"
#include "utils/sugar/byte_ptr.hpp"

#include "cxx/cxx.hpp"

namespace detail
{
    template<class T, class R>
    struct filter_dangerous_implicit_array_view
    {
        using type = R;
    };

    template<std::size_t N, class R>
    struct filter_dangerous_implicit_array_view<char[N], R> {};

    template<std::size_t N, class R>
    struct filter_dangerous_implicit_array_view<uint8_t[N], R> {};

    template<std::size_t N, class R>
    struct filter_dangerous_implicit_array_view<const char[N], R> {};

    template<std::size_t N, class R>
    struct filter_dangerous_implicit_array_view<const uint8_t[N], R> {};

    template<class T, class R>
    struct filter_dangerous_implicit_array_view<T&, R> : filter_dangerous_implicit_array_view<T, R> {};
} // namespace detail

template<class T>
struct array_view
{
    using type = T;
    using iterator = T *;
    using const_iterator = T *;
    using value_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    using reference = T&;
    using const_reference = T const &;

    constexpr array_view() = default;
    constexpr array_view(array_view && other) = default;
    constexpr array_view(array_view const & other) = default;
    array_view & operator = (array_view && other) = default;
    array_view & operator = (array_view const & other) = default;

    constexpr array_view(std::nullptr_t /*null*/) noexcept
    : array_view(nullptr, nullptr)
    {}

    constexpr array_view(type * p, std::size_t sz) noexcept
    : p(p)
    , sz(sz)
    {}

    constexpr array_view(type * p, type * pright) noexcept
    : p(p)
    , sz(pright - p)
    {}

    template<class U, class = typename detail::filter_dangerous_implicit_array_view<U, decltype(
        *static_cast<type**>(nullptr) = utils::data(std::declval<U&&>()),
        *static_cast<std::size_t*>(nullptr) = utils::size(std::declval<U&&>())
    )>::type>
    constexpr array_view(U && x) /*NOLINT(bugprone-forwarding-reference-overload)*/
    noexcept(noexcept((void(utils::data(std::forward<U>(x))), utils::size(std::forward<U>(x)))))
    : p(utils::data(std::forward<U>(x)))
    , sz(utils::size(std::forward<U>(x)))
    {}

    template<class U, class = decltype(
        *static_cast<type**>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr array_view(array_view<U> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}

    [[nodiscard]] constexpr bool empty() const noexcept { return !this->sz; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return this->sz; }

    [[nodiscard]] constexpr type & front() const noexcept { /*assert(this->size());*/ return *this->p; }
    [[nodiscard]] constexpr type & back() const noexcept { /*assert(this->size());*/ return this->p[this->sz-1u]; }
    constexpr type & front() noexcept { assert(this->size()); return *this->p; }
    constexpr type & back() noexcept { assert(this->size()); return this->p[this->sz-1u]; }

    [[nodiscard]] constexpr type const * data() const noexcept { return this->p; }
    constexpr type * data() noexcept { return this->p; }

    [[nodiscard]] constexpr type const * begin() const noexcept { return this->data(); }
    [[nodiscard]] constexpr type const * end() const noexcept { return this->data() + this->size(); }
    constexpr type * begin() noexcept { return this->data(); }
    constexpr type * end() noexcept { return this->data() + this->size(); }

    constexpr type & operator[](std::size_t i) noexcept
    { assert(i < this->size()); return this->data()[i]; }
    constexpr type const & operator[](std::size_t i) const noexcept
    { assert(i < this->size()); return this->data()[i]; }

    // TODO free functions
    //@{
    [[nodiscard]]
    constexpr array_view first(std::size_t n) noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    [[nodiscard]]
    constexpr array_view last(std::size_t n) noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    [[nodiscard]]
    constexpr array_view<const T> first(std::size_t n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    [[nodiscard]]
    constexpr array_view<const T> last(std::size_t n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    [[nodiscard]]
    constexpr array_view from_offset(std::size_t offset) noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    [[nodiscard]]
    constexpr array_view subarray(std::size_t offset, std::size_t count) noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    // TODO frop_front
    [[nodiscard]]
    constexpr array_view<T const> from_offset(std::size_t offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    [[nodiscard]]
    constexpr array_view<T const> subarray(std::size_t offset, std::size_t count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    [[nodiscard]]
    constexpr array_view<T> drop_front(std::size_t count) noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view<T> drop_back(std::size_t count) noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view<T const> drop_front(std::size_t count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view<T const> drop_back(std::size_t count) const noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }
    //@}

private:
    type * p        = nullptr;
    std::size_t sz  = 0;
};

template<class T>
array_view(T&&) -> array_view<std::remove_pointer_t<decltype(utils::data(std::declval<T&&>()))>>;


template<class T>
constexpr array_view<T> make_array_view(array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T> make_array_view(T * x, std::size_t n) noexcept
{ return {x, n}; }

template<class T>
constexpr array_view<T> make_array_view(T * left, T * right) noexcept
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T const * left, T * right) noexcept
{ return {left, right}; }

template<class T>
constexpr array_view<const T> make_array_view(T * left, T const * right) noexcept
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T> make_array_view(T (&arr)[N]) noexcept
{ return {arr, N}; }

template<class Cont>
constexpr auto make_array_view(Cont & cont)
noexcept(noexcept(array_view<typename std::remove_pointer<decltype(cont.data())>::type>{cont}))
-> array_view<typename std::remove_pointer<decltype(cont.data())>::type>
{ return {cont}; }

template<class T>
constexpr array_view<T const> make_const_array_view(array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T const> make_const_array_view(T const * x, std::size_t n) noexcept
{ return {x, n}; }

template<class T>
constexpr array_view<const T> make_const_array_view(T const * left, T const * right) noexcept
{ return {left, right}; }

template<class T, std::size_t N>
constexpr array_view<T const> make_const_array_view(T const (&arr)[N]) noexcept
{ return {arr, N}; }


// TODO renamed to zstring_array
template<std::size_t N>
constexpr array_view<char const> cstr_array_view(char const (&str)[N]) noexcept
{ return {str, N-1}; }

// TODO renamed to zstring_array
// forbidden: array_view is for litterals
template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N]) = delete;


using array_view_u8 = array_view<uint8_t>;
using array_view_u16 = array_view<uint16_t>;
using array_view_u32 = array_view<uint32_t>;
using array_view_u64 = array_view<uint64_t>;
using array_view_const_u8 = array_view<uint8_t const>;
using array_view_const_u16 = array_view<uint16_t const>;
using array_view_const_u32 = array_view<uint32_t const>;
using array_view_const_u64 = array_view<uint64_t const>;

using array_view_s8 = array_view<int8_t>;
using array_view_s16 = array_view<int16_t>;
using array_view_s32 = array_view<int32_t>;
using array_view_s64 = array_view<int64_t>;
using array_view_const_s8 = array_view<int8_t const>;
using array_view_const_s16 = array_view<int16_t const>;
using array_view_const_s32 = array_view<int32_t const>;
using array_view_const_s64 = array_view<int64_t const>;

using chars_view = array_view<const char>;
using writable_chars_view = array_view<char>;
using array_view_char = array_view<char>;
using array_view_const_char = array_view<char const>;

constexpr array_view_const_char operator "" _av(char const * s, size_t len) noexcept
{
    return {s, len};
}

//constexpr array_view_const_u8 operator "" _av(unsigned char const * s, size_t len) noexcept
//{
//    return {s, len};
//}

static inline void ap_integer_increment_le(array_view_u8 number) {
    for (uint8_t& i : number) {
        if (i < 0xFF) {
            i++;
            break;
        }
        i = 0;
    }
}

static inline void ap_integer_decrement_le(array_view_u8 number) {
    for (uint8_t& i : number) {
        if (i > 0) {
            i--;
            break;
        }
        i = 0xFF;
    }
}

static inline bool are_buffer_equal(array_view_const_u8 a, array_view_const_u8 b)
{
    return a.size() == b.size() && (0 == memcmp(a.data(), b.data(), a.size()));
}

static inline std::pair<array_view_const_u8, array_view_const_u8> get_bytes_slice(array_view_const_u8 a, size_t n)
{
    return {{a.data(), n},{a.data()+n, a.size()-n}};
}


static inline std::vector<uint8_t> && operator<<(std::vector<uint8_t>&& v, array_view_const_u8 a)
{
    v.insert(v.end(), a.begin(), a.end());
    return std::move(v);
}

static inline std::vector<uint8_t> & operator<<(std::vector<uint8_t> & v, array_view_const_u8 a)
{
    v.insert(v.end(), a.begin(), a.end());
    return v;
}



