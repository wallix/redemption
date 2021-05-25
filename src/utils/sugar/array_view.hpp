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

#include "utils/sugar/sized_sequence.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "cxx/cxx.hpp"

#include <cstdint>
#include <cassert>


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

    template<class U>
    constexpr bool is_convertible_with_two_ptr(...)
    {
        return false;
    }

    template<class U, class P>
    constexpr auto is_convertible_with_two_ptr(P)
    -> decltype(bool(((void)U(P(), P()), true)))
    {
        return true;
    }

    template<template<class...> class U>
    constexpr bool is_convertible_with_two_ptr(...)
    {
        return false;
    }

    template<template<class...> class U, class P>
    constexpr auto is_convertible_with_two_ptr(P)
    -> decltype(bool(((void)U(P(), P()), true)))
    {
        return true;
    }

    template<class T, class = void>
    struct select_extent
    {
        static constexpr std::size_t value = std::size_t(-1);
    };

    template<class T>
    struct select_extent<T, std::void_t<sequence_to_static_size_t<T>>>
    {
        static constexpr std::size_t value = sequence_to_static_size_t<T>::value;
    };

    template<class C>
    inline constexpr bool is_noexcept_array_view_data_v
      = noexcept(utils::data(std::declval<C>()));

    template<class C>
    inline constexpr bool is_noexcept_array_view_size_v
      = noexcept(utils::size(std::declval<C>()));

    template<class C>
    inline constexpr bool is_noexcept_array_view_data_size_v
      = is_noexcept_array_view_data_v<C> && is_noexcept_array_view_size_v<C>;

    template<class T>
    struct static_uint
    {};

    template<class T, T N>
    struct static_uint<std::integral_constant<T, N>>
    : std::enable_if<(N >= 0), std::integral_constant<std::size_t, N>>
    {};

    template<class T>
    struct is_constructible_with_elements : std::false_type
    {};

    template<class T>
    struct is_constructible_with_sub_elements : std::false_type
    {};

    template<class T, std::size_t N>
    struct is_constructible_with_sub_elements<std::array<T, N>> : std::true_type
    {};

    template<class F, std::size_t... Ints>
    constexpr auto unroll_indexes(std::index_sequence<Ints...>, F&& f)
    {
        return f(std::integral_constant<std::size_t, Ints>()...);
    }
} // namespace detail


inline constexpr std::size_t dynamic_extent = std::size_t(-1);


template<class T, std::size_t Extent = dynamic_extent>
struct array_view;

template<class T, std::size_t Extent = dynamic_extent>
struct writable_array_view;


template<class T>
struct array_view<T, dynamic_extent>
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

    static constexpr std::size_t extent = dynamic_extent;

    constexpr array_view() = default;
    constexpr array_view(array_view && other) = default;
    constexpr array_view(array_view const & other) = default;
    constexpr array_view & operator = (array_view && other) = default;
    constexpr array_view & operator = (array_view const & other) = default;

    constexpr array_view(std::nullptr_t /*null*/) noexcept
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

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return !this->sz;
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return this->sz;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type i) const noexcept
    {
        assert(i < this->size());
        return this->data()[i];
    }

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
    constexpr C as() const
    {
        if constexpr (detail::is_convertible_with_two_ptr<C>(pointer()))
            return C(this->begin(), this->end());
        else
            return C(this->data(), this->size());
    }

    template<template<class...> class C>
    constexpr auto as() const
    {
        if constexpr (detail::is_convertible_with_two_ptr<C>(pointer()))
            return C(this->begin(), this->end());
        else
            return C(this->data(), this->size());
    }
    //@}

private:
    pointer p    = nullptr;
    size_type sz = 0;
};


template<class T, std::size_t Extent>
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

    static constexpr std::size_t extent = Extent;

private:
    template<std::size_t N, class U = std::true_type>
    using enable_if_n = std::enable_if_t<(N == Extent), U>;

    template<std::size_t N, class U = std::true_type>
    using disable_if_n = std::enable_if_t<(N != Extent), U>;

public:
    constexpr array_view() noexcept(enable_if_n<0>::value) = default;
    constexpr array_view(array_view && other) = default;
    constexpr array_view(array_view const & other) = default;
    constexpr array_view & operator = (array_view && other) = default;
    constexpr array_view & operator = (array_view const & other) = default;

    constexpr array_view(std::nullptr_t /*null*/) noexcept(enable_if_n<0>::value)
    {}

    static constexpr array_view assumed(const_pointer p) noexcept
    {
        return array_view(p);
    }

    static constexpr array_view assumed(array_view<T> av) noexcept
    {
        assert(size() <= av.size());
        return array_view(av.data());
    }

    template<class C, std::size_t N = sequence_to_static_size_t<C>::value>
    static constexpr array_view assumed(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    {
        static_assert(size() <= N);
        return array_view(utils::data(static_cast<C&&>(a)));
    }

    template<class C, class = std::enable_if_t<
        (sequence_to_static_size_t<C>::value == Extent),
        typename detail::filter_dangerous_implicit_array_view<C, decltype(
            *static_cast<const_pointer*>(nullptr) = utils::data(std::declval<C&&>())
        )>::type
    >>
    constexpr array_view(C&& a) noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    : p(utils::data(static_cast<C&&>(a)))
    {}


    [[nodiscard]]
    static constexpr bool empty() noexcept
    {
        return !size();
    }

    [[nodiscard]]
    static constexpr size_type size() noexcept
    {
        return Extent;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type i) const noexcept(disable_if_n<0>::value)
    {
        assert(i < this->size());
        return this->data()[i];
    }

    // TODO free functions
    //@{
    [[nodiscard]]
    constexpr array_view<T> first(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    template<std::size_t N>
    [[nodiscard]]
    constexpr array_view<T, N> first() const noexcept
    {
        static_assert(N <= size());
        return array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<N>::value>
    first(N const& /*n*/) const noexcept
    {
        return first<detail::static_uint<N>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> last(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    template<std::size_t n>
    [[nodiscard]] constexpr array_view<T, n> last() const noexcept
    {
        static_assert(n <= size());
        return array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<N>::value>
    last(N const& /*n*/) const noexcept
    {
        return last<detail::static_uint<N>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> from_offset(size_type offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    template<std::size_t offset>
    [[nodiscard]]
    constexpr array_view<T, size() - offset> from_offset() const noexcept
    {
        static_assert(offset <= size());
        return array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Offset>::value>
    from_offset(Offset const& /*n*/) const noexcept
    {
        return from_offset<detail::static_uint<Offset>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> subarray(size_type offset, size_type count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, count> subarray(size_type offset) const noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) const noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }

    [[nodiscard]]
    constexpr array_view<T> drop_front(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, size() - count> drop_front() const noexcept
    {
        static_assert(count <= size());
        return array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Count>::value>
    drop_front(Count const& /*n*/) const noexcept
    {
        return drop_front<detail::static_uint<Count>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> drop_back(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, size() - count> drop_back() const noexcept
    {
        static_assert(count <= size());
        return array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Count>::value>
    drop_back(Count const& /*n*/) const noexcept
    {
        return drop_back<detail::static_uint<Count>::value>();
    }

    template<class C>
    constexpr C as() const
    {
        if constexpr (detail::is_constructible_with_sub_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{{p[i]...}};
            });
        }
        else if constexpr (detail::is_constructible_with_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{p[i]...};
            });
        }
        else {
            return array_view<T>(p, size()).template as<C>();
        }
    }

    template<template<class...> class C>
    constexpr auto as() const
    {
        return array_view<T>(p, size()).template as<C>();
    }

    template<template<class, std::size_t> class C>
    constexpr auto as() const
    {
        return as<C<T, size()>>();
    }
    //@}

private:
    explicit constexpr array_view(const_pointer p) noexcept
    : p(p)
    {}

    const_pointer p = nullptr;
};


template<class T>
array_view(T&&) -> array_view<
    detail::value_type_array_view_from_t<T&&>,
    detail::select_extent<T&&>::value
>;

template<class T>
array_view(T const*, T const*) -> array_view<T>;

template<class T>
array_view(T const*, std::size_t) -> array_view<T>;


template<class T>
struct writable_array_view<T, dynamic_extent>
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

    static constexpr std::size_t extent = dynamic_extent;

    constexpr writable_array_view() = default;
    constexpr writable_array_view(writable_array_view && other) = default;
    constexpr writable_array_view(writable_array_view const & other) = default;
    constexpr writable_array_view & operator = (writable_array_view && other) = default;
    constexpr writable_array_view & operator = (writable_array_view const & other) = default;

    constexpr writable_array_view(std::nullptr_t /*null*/) noexcept
    {}

    explicit constexpr writable_array_view(pointer p, size_type sz) noexcept
    : p(p)
    , sz(sz)
    {}

    explicit constexpr writable_array_view(pointer first, const_pointer last) noexcept
    : p(first)
    , sz(checked_int{last - first})
    {}

    template<class C, class = typename detail::filter_dangerous_implicit_array_view<C, decltype(
        *static_cast<pointer*>(nullptr) = utils::data(std::declval<C&&>()),
        *static_cast<size_type*>(nullptr) = utils::size(std::declval<C&&>())
    )>::type>
    explicit constexpr writable_array_view(C&& a)
        noexcept(detail::is_noexcept_array_view_data_size_v<C&&>)
    : p(utils::data(static_cast<C&&>(a)))
    , sz(utils::size(static_cast<C&&>(a)))
    {}

    template<class U, class = decltype(
        *static_cast<pointer*>(nullptr) = static_cast<U*>(nullptr)
    ), std::size_t UExtent>
    constexpr writable_array_view(writable_array_view<U, UExtent> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return !this->sz;
    }

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return this->sz;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr reference front() noexcept
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr reference back() noexcept
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr pointer data() noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr reference operator[](size_type i) noexcept
    {
        assert(i < this->size());
        return this->data()[i];
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type i) const noexcept
    {
        assert(i < this->size());
        return this->data()[i];
    }

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
    constexpr C as()
    {
        if constexpr (detail::is_convertible_with_two_ptr<C>(pointer()))
            return C(this->begin(), this->end());
        else
            return C(this->data(), this->size());
    }

    template<class C>
    constexpr C as() const
    {
        if constexpr (detail::is_convertible_with_two_ptr<C>(pointer()))
            return C(this->begin(), this->end());
        else
            return C(this->data(), this->size());
    }
    //@}

private:
    pointer p    = nullptr;
    size_type sz = 0;
};


template<class T, std::size_t Extent>
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

    static constexpr std::size_t extent = Extent;

private:
    template<std::size_t N, class U = std::true_type>
    using enable_if_n = std::enable_if_t<(N == Extent), U>;

    template<std::size_t N, class U = std::true_type>
    using disable_if_n = std::enable_if_t<(N != Extent), U>;

public:
    constexpr writable_array_view() noexcept(enable_if_n<0>::value) = default;
    constexpr writable_array_view(writable_array_view && other) = default;
    constexpr writable_array_view(writable_array_view const & other) = default;
    constexpr writable_array_view & operator = (writable_array_view && other) = default;
    constexpr writable_array_view & operator = (writable_array_view const & other) = default;

    constexpr writable_array_view(std::nullptr_t /*null*/) noexcept(enable_if_n<0>::value)
    {}

    static constexpr writable_array_view assumed(pointer p) noexcept
    {
        return writable_array_view(p);
    }

    static constexpr writable_array_view assumed(writable_array_view<T> av) noexcept
    {
        assert(size() <= av.size());
        return writable_array_view(av.data());
    }

    template<class C, std::size_t N = sequence_to_static_size_t<C>::value>
    static constexpr writable_array_view assumed(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    {
        static_assert(size() <= N);
        return writable_array_view(utils::data(static_cast<C&&>(a)));
    }

    template<class C, class = std::enable_if_t<
        (sequence_to_static_size_t<C>::value == Extent),
        typename detail::filter_dangerous_implicit_array_view<C, decltype(
            *static_cast<pointer*>(nullptr) = utils::data(std::declval<C&&>())
        )>::type
    >>
    explicit constexpr writable_array_view(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    : p(utils::data(static_cast<C&&>(a)))
    {}

    template<class U, class = decltype(
        *static_cast<pointer*>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr writable_array_view(writable_array_view<U, Extent> av) noexcept
    : p(av.data())
    , sz(av.size())
    {}


    [[nodiscard]]
    static constexpr bool empty() noexcept
    {
        return !size();
    }

    [[nodiscard]]
    static constexpr size_type size() noexcept
    {
        return Extent;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr reference front() noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr reference back() noexcept(disable_if_n<0>::value)
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr pointer data() noexcept
    {
        return this->p;
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept
    {
        return this->data();
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return this->data() + this->size();
    }

    [[nodiscard]]
    constexpr reference operator[](size_type i) noexcept(disable_if_n<0>::value)
    {
        assert(i < this->size());
        return this->data()[i];
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type i) const noexcept(disable_if_n<0>::value)
    {
        assert(i < this->size());
        return this->data()[i];
    }

    // TODO free functions
    //@{
    [[nodiscard]]
    constexpr writable_array_view<T> first(size_type n) noexcept
    {
        assert(n <= this->size());
        return writable_array_view<T>{this->data(), n};
    }

    template<std::size_t N>
    [[nodiscard]]
    constexpr writable_array_view<T, N> first() noexcept
    {
        static_assert(N <= size());
        return writable_array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr writable_array_view<T, detail::static_uint<N>::value>
    first(N const& /*n*/) noexcept
    {
        return first<detail::static_uint<N>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> first(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data(), n};
    }

    template<std::size_t N>
    [[nodiscard]]
    constexpr array_view<T, N> first() const noexcept
    {
        static_assert(N <= size());
        return array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<N>::value>
    first(N const& /*n*/) const noexcept
    {
        return first<detail::static_uint<N>::value>();
    }


    [[nodiscard]]
    constexpr writable_array_view<T> last(size_type n) noexcept
    {
        assert(n <= this->size());
        return writable_array_view<T>{this->data() + this->size() - n, n};
    }

    template<std::size_t n>
    [[nodiscard]] constexpr writable_array_view<T, n> last() noexcept
    {
        static_assert(n <= size());
        return writable_array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr writable_array_view<T, detail::static_uint<N>::value>
    last(N const& /*n*/) noexcept
    {
        return last<detail::static_uint<N>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> last(size_type n) const noexcept
    {
        assert(n <= this->size());
        return {this->data() + this->size() - n, n};
    }

    template<std::size_t n>
    [[nodiscard]] constexpr array_view<T, n> last() const noexcept
    {
        static_assert(n <= size());
        return array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<N>::value>
    last(N const& /*n*/) const noexcept
    {
        return last<detail::static_uint<N>::value>();
    }


    [[nodiscard]]
    constexpr writable_array_view<T> from_offset(size_type offset) noexcept
    {
        assert(offset <= this->size());
        return writable_array_view<T>{this->data() + offset, this->size() - offset};
    }

    template<std::size_t offset>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - offset> from_offset() noexcept
    {
        static_assert(offset <= size());
        return writable_array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - detail::static_uint<Offset>::value>
    from_offset(Offset const& /*n*/) noexcept
    {
        return from_offset<detail::static_uint<Offset>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> from_offset(size_type offset) const noexcept
    {
        assert(offset <= this->size());
        return {this->data() + offset, this->size() - offset};
    }

    template<std::size_t offset>
    [[nodiscard]]
    constexpr array_view<T, size() - offset> from_offset() const noexcept
    {
        static_assert(offset <= size());
        return array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Offset>::value>
    from_offset(Offset const& /*n*/) const noexcept
    {
        return from_offset<detail::static_uint<Offset>::value>();
    }


    [[nodiscard]]
    constexpr writable_array_view<T> subarray(size_type offset, size_type count) noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return writable_array_view<T>{this->data() + offset, count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr writable_array_view<T, count> subarray(size_type offset) noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return writable_array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }

    [[nodiscard]]
    constexpr array_view<T> subarray(size_type offset, size_type count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, count> subarray(size_type offset) const noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) const noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }


    [[nodiscard]]
    constexpr writable_array_view<T> drop_front(size_type count) noexcept
    {
        assert(count <= this->size());
        return writable_array_view<T>{this->data() + count, this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - count> drop_front() noexcept
    {
        static_assert(count <= size());
        return writable_array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - detail::static_uint<Count>::value>
    drop_front(Count const& /*n*/) noexcept
    {
        return drop_front<detail::static_uint<Count>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> drop_front(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, size() - count> drop_front() const noexcept
    {
        static_assert(count <= size());
        return array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Count>::value>
    drop_front(Count const& /*n*/) const noexcept
    {
        return drop_front<detail::static_uint<Count>::value>();
    }


    [[nodiscard]]
    constexpr writable_array_view<T> drop_back(size_type count) noexcept
    {
        assert(count <= this->size());
        return writable_array_view<T>{this->data(), this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - count> drop_back() noexcept
    {
        static_assert(count <= size());
        return writable_array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_array_view<T, size() - detail::static_uint<Count>::value>
    drop_back(Count const& /*n*/) noexcept
    {
        return drop_back<detail::static_uint<Count>::value>();
    }

    [[nodiscard]]
    constexpr array_view<T> drop_back(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data(), this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr array_view<T, size() - count> drop_back() const noexcept
    {
        static_assert(count <= size());
        return array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr array_view<T, size() - detail::static_uint<Count>::value>
    drop_back(Count const& /*n*/) const noexcept
    {
        return drop_back<detail::static_uint<Count>::value>();
    }


    template<class C>
    constexpr C as()
    {
        if constexpr (detail::is_constructible_with_sub_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{{p[i]...}};
            });
        }
        else if constexpr (detail::is_constructible_with_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{p[i]...};
            });
        }
        else {
            return writable_array_view<T>(p, size()).template as<C>();
        }
    }

    template<template<class...> class C>
    constexpr auto as()
    {
        return writable_array_view<T>(p, size()).template as<C>();
    }

    template<template<class, std::size_t> class C>
    constexpr auto as()
    {
        return as<C<T, size()>>();
    }

    template<class C>
    constexpr C as() const
    {
        if constexpr (detail::is_constructible_with_sub_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{{p[i]...}};
            });
        }
        else if constexpr (detail::is_constructible_with_elements<C>::value) {
            return detail::unroll_indexes(std::make_index_sequence<size()>(), [this](auto... i) {
                return C{p[i]...};
            });
        }
        else {
            return array_view<T>(p, size()).template as<C>();
        }
    }

    template<template<class...> class C>
    constexpr auto as() const
    {
        return array_view<T>(p, size()).template as<C>();
    }

    template<template<class, std::size_t> class C>
    constexpr auto as() const
    {
        return as<C<T, size()>>();
    }
    //@}

private:
    explicit constexpr writable_array_view(pointer p) noexcept
    : p(p)
    {}

    pointer p    = nullptr;
    size_type sz = 0;
};


namespace detail
{
    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<array_view<T, N>>
    : std::integral_constant<std::size_t, N>
    {};

    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<writable_array_view<T, N>>
    : std::integral_constant<std::size_t, N>
    {};

    template<class T>
    struct sequence_to_static_size_impl<array_view<T, dynamic_extent>>
    {};

    template<class T>
    struct sequence_to_static_size_impl<writable_array_view<T, dynamic_extent>>
    {};
} // namespace detail


template<class T>
writable_array_view(T&&) -> writable_array_view<
    detail::value_type_array_view_from_t<T&&>,
    detail::select_extent<T&&>::value
>;

template<class T>
writable_array_view(T*, T const*) -> writable_array_view<T>;

template<class T>
writable_array_view(T*, std::size_t) -> writable_array_view<T>;



template<class T, std::size_t Extent>
constexpr array_view<T, Extent>
make_array_view(array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T, std::size_t Extent>
constexpr array_view<T, Extent>
make_array_view(writable_array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T>
constexpr array_view<T>
make_array_view(T const* x, std::size_t n) noexcept
{
    return {x, n};
}

template<class T>
constexpr array_view<T>
make_array_view(T const* first, T const* last) noexcept
{
    return {first, last};
}

template<class Cont>
constexpr auto make_array_view(Cont const& cont)
    noexcept(noexcept(array_view{cont}))
-> decltype(array_view{cont})
{
    return {cont};
}

template<class T, std::size_t N>
constexpr array_view<T, N>
make_array_view(T const (&arr)[N]) noexcept
{
    return array_view<T, N>::assumed(&arr[0]);
}


// TODO renamed to zstring_array
template<std::size_t N>
constexpr array_view<char>
cstr_array_view(char const (&str)[N]) noexcept
{
    return array_view<char>(str, N-1);
}

// TODO renamed to zstring_array
// forbidden: array_view is for litterals
template<std::size_t N>
array_view<char> cstr_array_view(char (&str)[N]) = delete;


template<class T, std::size_t Extent>
constexpr writable_array_view<T, Extent>
make_writable_array_view(writable_array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T>
constexpr writable_array_view<T>
make_writable_array_view(T* x, std::size_t n) noexcept
{
    return writable_array_view<T>{x, n};
}

template<class T>
constexpr writable_array_view<T>
make_writable_array_view(T* first, T* last) noexcept
{
    return writable_array_view<T>{first, last};
}

template<class Cont>
constexpr auto make_writable_array_view(Cont& cont)
    noexcept(noexcept(writable_array_view{cont}))
-> decltype(writable_array_view{cont})
{
    return writable_array_view{cont};
}

template<class T, std::size_t N>
constexpr writable_array_view<T, N>
make_writable_array_view(T (&arr)[N]) noexcept
{
    return writable_array_view<T, N>::assumed(&arr[0]);
}


template<class T, std::size_t Extent>
constexpr array_view<T>
make_dynamic_array_view(array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T, std::size_t Extent>
constexpr array_view<T>
make_dynamic_array_view(writable_array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T>
constexpr array_view<T>
make_dynamic_array_view(T const* x, std::size_t n) noexcept
{
    return {x, n};
}

template<class T>
constexpr array_view<T>
make_dynamic_array_view(T const* first, T const* last) noexcept
{
    return {first, last};
}

template<class Cont>
constexpr auto make_dynamic_array_view(Cont const& cont)
    noexcept(noexcept(make_array_view(cont)))
-> decltype(make_dynamic_array_view(make_array_view(cont)))
{
    return make_array_view(cont);
}


template<class T, std::size_t Extent>
constexpr writable_array_view<T>
make_dynamic_writable_array_view(writable_array_view<T, Extent> av) noexcept
{
    return av;
}

template<class T>
constexpr writable_array_view<T>
make_dynamic_writable_array_view(T* x, std::size_t n) noexcept
{
    return writable_array_view<T>{x, n};
}

template<class T>
constexpr writable_array_view<T>
make_dynamic_writable_array_view(T* first, T* last) noexcept
{
    return writable_array_view<T>{first, last};
}

template<class Cont>
constexpr auto make_dynamic_writable_array_view(Cont& cont)
    noexcept(noexcept(make_writable_array_view(cont)))
-> decltype(make_dynamic_writable_array_view(make_writable_array_view(cont)))
{
    return make_writable_array_view(cont);
}


template<std::size_t N>
struct sized_array_view_t
{
    template<class T, class AV
        = array_view<detail::value_type_array_view_from_t<T&&>, N>>
    constexpr AV operator()(T&& x) const
        noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
    {
        return AV::assumed(static_cast<T&&>(x));
    }
};

template<std::size_t N>
struct writable_sized_array_view_t
{
    template<class T, class AV
        = writable_array_view<detail::value_type_array_view_from_t<T&&>, N>>
    constexpr AV operator()(T&& x) const
        noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
    {
        return AV::assumed(static_cast<T&&>(x));
    }
};

template<std::size_t N>
inline constexpr sized_array_view_t<N> sized_array_view {};

template<std::size_t N>
inline constexpr writable_sized_array_view_t<N> writable_sized_array_view {};


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

template<std::size_t N> using sized_u8_array_view = array_view<std::uint8_t, N>;
template<std::size_t N> using sized_writable_u8_array_view = writable_array_view<std::uint8_t, N>;


// TODO array_view<char, len> (c++20)
constexpr chars_view operator "" _av(char const * s, size_t len) noexcept
{
    return {s, len};
}
