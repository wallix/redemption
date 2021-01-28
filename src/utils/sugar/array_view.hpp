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

#include "utils/sugar/array.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "cxx/cxx.hpp"

namespace detail
{
    template<class T, class R>
    struct filter_dangerous_implicit_array_view
    {
        using type = R;
    };

    // probably a buffer type
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

    template<class T>
    using value_type_array_view_from_t
        = std::remove_cv_t<std::remove_pointer_t<decltype(utils::data(std::declval<T>()))>>;
} // namespace detail

template<class T>
struct array_view
{
    using element_type = T const;
    using value_type = std::remove_cv_t<T>;
    using reference = T const&;
    using iterator = T const*;
    using pointer = T const*;
    using const_reference = T const&;
    using const_iterator = T const*;
    using const_pointer = T const*;
    using size_type = std::size_t;

    constexpr array_view() = default;
    constexpr array_view(array_view && other) = default;
    constexpr array_view(array_view const & other) = default;
    array_view & operator = (array_view && other) = default;
    array_view & operator = (array_view const & other) = default;

    constexpr array_view(std::nullptr_t /*null*/) noexcept
    : array_view()
    {}

    constexpr array_view(const_pointer p, size_type sz) noexcept
    : p(p)
    , sz(sz)
    {}

    constexpr array_view(const_pointer first, const_pointer last) noexcept
    : p(first)
    , sz(checked_int{last - first})
    {}

    template<class U, class = typename detail::filter_dangerous_implicit_array_view<U, decltype(
        *static_cast<const_pointer*>(nullptr) = utils::data(std::declval<U&&>()),
        *static_cast<size_type*>(nullptr) = utils::size(std::declval<U&&>())
    )>::type>
    constexpr array_view(U && x) /*NOLINT(bugprone-forwarding-reference-overload)*/
    noexcept(noexcept((void(utils::data(static_cast<U&&>(x))), utils::size(static_cast<U&&>(x)))))
    : p(utils::data(static_cast<U&&>(x)))
    , sz(utils::size(static_cast<U&&>(x)))
    {}

    template<class U, class = decltype(
        *static_cast<const_pointer*>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr array_view(array_view<U> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}

    [[nodiscard]] constexpr bool empty() const noexcept { return !this->sz; }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->sz; }

    [[nodiscard]] constexpr const_reference front() const noexcept { assert(this->size()); return *this->p; }
    [[nodiscard]] constexpr const_reference back() const noexcept { assert(this->size()); return this->p[this->sz-1u]; }

    [[nodiscard]] constexpr const_pointer data() const noexcept { return this->p; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return this->data(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return this->data() + this->size(); }

    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept
    { assert(i < this->size()); return this->data()[i]; }

    // TODO free functions
    //@{
    [[nodiscard]]
    constexpr array_view first(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    [[nodiscard]]
    constexpr array_view last(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    [[nodiscard]]
    constexpr array_view from_offset(size_type offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    [[nodiscard]]
    constexpr array_view subarray(size_type offset, size_type count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    [[nodiscard]]
    constexpr array_view drop_front(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view drop_back(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }

    template<class C>
    C as() const
    {
        return C(this->begin(), this->end());
    }
    //@}

private:
    pointer p    = nullptr;
    size_type sz = 0;
};

template<class T>
array_view(T&&) -> array_view<detail::value_type_array_view_from_t<T&&>>;


template<class T>
struct writable_array_view
{
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using reference = T&;
    using iterator = T*;
    using pointer = T*;
    using const_reference = T const&;
    using const_iterator = T const*;
    using const_pointer = T const*;
    using size_type = std::size_t;

    constexpr writable_array_view() = default;
    constexpr writable_array_view(writable_array_view && other) = default;
    constexpr writable_array_view(writable_array_view const & other) = default;
    writable_array_view & operator = (writable_array_view && other) = default;
    writable_array_view & operator = (writable_array_view const & other) = default;

    constexpr writable_array_view(std::nullptr_t /*null*/) noexcept
    : writable_array_view()
    {}

    explicit constexpr writable_array_view(pointer p, size_type sz) noexcept
    : p(p)
    , sz(sz)
    {}

    explicit constexpr writable_array_view(pointer first, pointer last) noexcept
    : p(first)
    , sz(last - first)
    {}

    template<class U, class = typename detail::filter_dangerous_implicit_array_view<U, decltype(
        *static_cast<pointer*>(nullptr) = utils::data(std::declval<U&&>()),
        *static_cast<size_type*>(nullptr) = utils::size(std::declval<U&&>())
    )>::type>
    explicit constexpr writable_array_view(U && x) /*NOLINT(bugprone-forwarding-reference-overload)*/
    noexcept(noexcept((void(utils::data(static_cast<U&&>(x))), utils::size(static_cast<U&&>(x)))))
    : p(utils::data(static_cast<U&&>(x)))
    , sz(utils::size(static_cast<U&&>(x)))
    {}

    template<class U, class = decltype(
        *static_cast<pointer*>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr writable_array_view(writable_array_view<U> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}

    [[nodiscard]] constexpr bool empty() const noexcept { return !this->sz; }

    [[nodiscard]] constexpr size_type size() const noexcept { return this->sz; }

    [[nodiscard]] constexpr const_reference front() const noexcept { assert(this->size()); return *this->p; }
    [[nodiscard]] constexpr const_reference back() const noexcept { assert(this->size()); return this->p[this->sz-1u]; }
    [[nodiscard]] constexpr reference front() noexcept { assert(this->size()); return *this->p; }
    [[nodiscard]] constexpr reference back() noexcept { assert(this->size()); return this->p[this->sz-1u]; }

    [[nodiscard]] constexpr const_pointer data() const noexcept { return this->p; }
    [[nodiscard]] constexpr pointer data() noexcept { return this->p; }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return this->data(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return this->data() + this->size(); }
    [[nodiscard]] constexpr iterator begin() noexcept { return this->data(); }
    [[nodiscard]] constexpr iterator end() noexcept { return this->data() + this->size(); }

    [[nodiscard]]constexpr reference operator[](size_type i) noexcept
    { assert(i < this->size()); return this->data()[i]; }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept
    { assert(i < this->size()); return this->data()[i]; }

    // TODO free functions
    //@{
    [[nodiscard]]
    constexpr writable_array_view first(size_type n) noexcept
    {
        assert(n <= this->size());
        return writable_array_view{this->data(), n};
    }

    [[nodiscard]]
    constexpr writable_array_view last(size_type n) noexcept
    {
        assert(n <= this->size());
        return writable_array_view{this->data() + this->size() - n, n};
    }

    [[nodiscard]]
    constexpr array_view<value_type> first(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    [[nodiscard]]
    constexpr array_view<value_type> last(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    [[nodiscard]]
    constexpr writable_array_view from_offset(size_type offset) noexcept
    {
        assert(offset <= this->size());
        return writable_array_view{this->data() + offset, this->size() - offset};
    }

    [[nodiscard]]
    constexpr writable_array_view subarray(size_type offset, size_type count) noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return writable_array_view{this->data() + offset, count};
    }

    [[nodiscard]]
    constexpr array_view<value_type> from_offset(size_type offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    [[nodiscard]]
    constexpr array_view<value_type> subarray(size_type offset, size_type count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    [[nodiscard]]
    constexpr writable_array_view drop_front(size_type count) noexcept
    {
        assert(count <= this->size());
        return writable_array_view{this->data() + count, this->size() - count};
    }

    [[nodiscard]]
    constexpr writable_array_view drop_back(size_type count) noexcept
    {
        assert(count <= this->size());
        return writable_array_view{this->data(), this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view<value_type> drop_front(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    [[nodiscard]]
    constexpr array_view<value_type> drop_back(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }

    template<class C>
    C as() const
    {
        return C(this->begin(), this->end());
    }
    //@}

private:
    pointer p    = nullptr;
    size_type sz = 0;
};

template<class T>
writable_array_view(T&&) -> writable_array_view<detail::value_type_array_view_from_t<T&&>>;


template<class T>
constexpr array_view<T> make_array_view(array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T> make_array_view(writable_array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr array_view<T> make_array_view(T const* x, std::size_t n) noexcept
{ return {x, n}; }

template<class T>
constexpr array_view<T> make_array_view(T const* first, T const* last) noexcept
{ return {first, last}; }

template<class Cont>
constexpr auto make_array_view(Cont const& cont)
noexcept(noexcept(array_view<detail::value_type_array_view_from_t<Cont const&>>{cont}))
-> decltype(array_view<detail::value_type_array_view_from_t<Cont const&>>{cont})
{ return {cont}; }

template<class T, std::size_t N>
constexpr array_view<T> make_array_view(T const (&arr)[N]) noexcept
{ return {arr, N}; }


// TODO renamed to zstring_array
template<std::size_t N>
constexpr array_view<char> cstr_array_view(char const (&str)[N]) noexcept
{ return {str, N-1}; }

// TODO renamed to zstring_array
// forbidden: array_view is for litterals
template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N]) = delete;


template<class T>
constexpr writable_array_view<T> make_writable_array_view(writable_array_view<T> av) noexcept
{ return av; }

template<class T>
constexpr writable_array_view<T> make_writable_array_view(T* x, std::size_t n) noexcept
{ return writable_array_view{x, n}; }

template<class T>
constexpr writable_array_view<T> make_writable_array_view(T* first, T* last) noexcept
{ return writable_array_view{first, last}; }

template<class Cont>
constexpr auto make_writable_array_view(Cont& cont)
noexcept(noexcept(writable_array_view<detail::value_type_array_view_from_t<Cont&>>{cont}))
-> decltype(writable_array_view<detail::value_type_array_view_from_t<Cont&>>{cont})
{ return writable_array_view<detail::value_type_array_view_from_t<Cont&>>{cont}; }

template<class T, std::size_t N>
constexpr writable_array_view<T> make_writable_array_view(T (&arr)[N]) noexcept
{ return writable_array_view{arr, N}; }


using u8_array_view = array_view<std::uint8_t>;
using u16_array_view = array_view<std::uint16_t>;
using u32_array_view = array_view<std::uint32_t>;
using u64_array_view = array_view<std::uint64_t>;

using writable_u8_array_view = writable_array_view<std::uint8_t>;
using writable_u16_array_view = writable_array_view<std::uint16_t>;
using writable_u32_array_view = writable_array_view<std::uint32_t>;
using writable_u64_array_view = writable_array_view<std::uint64_t>;

using s8_array_view = array_view<std::int8_t>;
using s16_array_view = array_view<std::int16_t>;
using s32_array_view = array_view<std::int32_t>;
using s64_array_view = array_view<std::int64_t>;

using writable_s8_array_view = writable_array_view<std::int8_t>;
using writable_s16_array_view = writable_array_view<std::int16_t>;
using writable_s32_array_view = writable_array_view<std::int32_t>;
using writable_s64_array_view = writable_array_view<std::int64_t>;

using chars_view = array_view<char>;
using writable_chars_view = writable_array_view<char>;

constexpr chars_view operator "" _av(char const * s, size_t len) noexcept
{
    return {s, len};
}
