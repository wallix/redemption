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

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/sized_sequence.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "cxx/cxx.hpp"

#include <cstdint>
#include <cassert>


namespace detail
{
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


#if REDEMPTION_COMP_GCC_VERSION_LESS(9, 0, 0)
# define gcc_8_fix_decltemplate template<class = void>
# define gcc_8_fix_default_impl {}
#else
# define gcc_8_fix_decltemplate
# define gcc_8_fix_default_impl = default;
#endif


template<class T, std::size_t Size>
struct sized_array_view
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

    static constexpr std::size_t extent = Size;

public:
    // C++20: enable when Size == 0
    constexpr sized_array_view() = delete;
    constexpr sized_array_view(sized_array_view && other) = default;
    constexpr sized_array_view(sized_array_view const & other) = default;
    constexpr sized_array_view & operator = (sized_array_view && other) = default;
    constexpr sized_array_view & operator = (sized_array_view const & other) = default;

    // C++20: enable when Size == 0
    constexpr sized_array_view(std::nullptr_t /*null*/) = delete;


    static constexpr sized_array_view assumed(const_pointer p) noexcept
    {
        return sized_array_view(p);
    }

    static constexpr sized_array_view assumed(sized_array_view av) noexcept
    {
        assert(size() <= av.size());
        return sized_array_view(av.data());
    }

    template<class C, std::size_t N = sequence_to_static_size_t<C>::value>
    static constexpr sized_array_view assumed(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    {
        static_assert(size() <= N);
        return sized_array_view(utils::data(static_cast<C&&>(a)));
    }


    template<class C, class = std::enable_if_t<
        (sequence_to_static_size_t<C>::value == Size),
        typename detail::filter_dangerous_implicit_array_view<C, decltype(
            *static_cast<const_pointer*>(nullptr) = utils::data(std::declval<C&&>())
        )>::type
    >>
    constexpr sized_array_view(C&& a) noexcept(detail::is_noexcept_array_view_data_v<C&&>)
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
        return Size;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept/*(disable_if_n<0>::value)*/
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept/*(disable_if_n<0>::value)*/
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
    constexpr const_reference operator[](size_type i) const noexcept/*(disable_if_n<0>::value)*/
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
    constexpr sized_array_view<T, N> first() const noexcept
    {
        static_assert(N <= size());
        return sized_array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<N>::value>
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
    [[nodiscard]] constexpr sized_array_view<T, n> last() const noexcept
    {
        static_assert(n <= size());
        return sized_array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<N>::value>
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
    constexpr sized_array_view<T, size() - offset> from_offset() const noexcept
    {
        static_assert(offset <= size());
        return sized_array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Offset>::value>
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
    constexpr sized_array_view<T, count> subarray(size_type offset) const noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) const noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }

    template<std::size_t offset, std::size_t count>
    [[nodiscard]]
    constexpr sized_array_view<T, count> subarray() const noexcept
    {
        static_assert(offset <= size() && count <= size() - offset);
        return sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Offset, class Count>
    [[nodiscard]]
    constexpr auto
    subarray(Offset const& /*offset*/, Count const& /*n*/) const noexcept
    -> decltype(subarray<
        detail::static_uint<Offset>::value,
        detail::static_uint<Count>::value
    >(0))
    {
        return subarray<
            detail::static_uint<Offset>::value,
            detail::static_uint<Count>::value
        >();
    }

    [[nodiscard]]
    constexpr array_view<T> drop_front(size_type count) const noexcept
    {
        assert(count <= this->size());
        return {this->data() + count, this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - count> drop_front() const noexcept
    {
        static_assert(count <= size());
        return sized_array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    constexpr sized_array_view<T, size() - count> drop_back() const noexcept
    {
        static_assert(count <= size());
        return sized_array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    explicit constexpr sized_array_view(const_pointer p) noexcept
    : p(p)
    {}

    const_pointer p = nullptr;
};


template<class T>
sized_array_view(T&&) -> sized_array_view<
    detail::value_type_array_view_from_t<T&&>,
    sequence_to_static_size_t<T&&>::value
>;


template<class T, std::size_t Size>
struct writable_sized_array_view
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

    static constexpr std::size_t extent = Size;

public:
    // C++20: enable when Size == 0
    constexpr writable_sized_array_view() = delete;
    constexpr writable_sized_array_view(writable_sized_array_view && other) = default;
    constexpr writable_sized_array_view(writable_sized_array_view const & other) = default;
    constexpr writable_sized_array_view & operator = (writable_sized_array_view && other) = default;
    constexpr writable_sized_array_view & operator = (writable_sized_array_view const & other) = default;

    // C++20: enable when Size == 0
    constexpr writable_sized_array_view(std::nullptr_t /*null*/) = delete;

    static constexpr writable_sized_array_view assumed(pointer p) noexcept
    {
        return writable_sized_array_view(p);
    }

    static constexpr writable_sized_array_view assumed(writable_sized_array_view av) noexcept
    {
        assert(size() <= av.size());
        return writable_sized_array_view(av.data());
    }

    template<class C, std::size_t N = sequence_to_static_size_t<C>::value>
    static constexpr writable_sized_array_view assumed(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    {
        static_assert(size() <= N);
        return writable_sized_array_view(utils::data(static_cast<C&&>(a)));
    }

    template<class C, class = std::enable_if_t<
        (sequence_to_static_size_t<C>::value == Size),
        typename detail::filter_dangerous_implicit_array_view<C, decltype(
            *static_cast<pointer*>(nullptr) = utils::data(std::declval<C&&>())
        )>::type
    >>
    explicit constexpr writable_sized_array_view(C&& a)
        noexcept(detail::is_noexcept_array_view_data_v<C&&>)
    : p(utils::data(static_cast<C&&>(a)))
    {}

    template<class U, class = decltype(
        *static_cast<pointer*>(nullptr) = static_cast<U*>(nullptr)
    )>
    constexpr writable_sized_array_view(writable_sized_array_view<U, Size> av) noexcept
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
        return Size;
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept/*(disable_if_n<0>::value)*/
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept/*(disable_if_n<0>::value)*/
    {
        assert(this->size());
        return this->p[this->sz-1u];
    }

    [[nodiscard]]
    constexpr reference front() noexcept/*(disable_if_n<0>::value)*/
    {
        assert(this->size());
        return *this->p;
    }

    [[nodiscard]]
    constexpr reference back() noexcept/*(disable_if_n<0>::value)*/
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
    constexpr reference operator[](size_type i) noexcept/*(disable_if_n<0>::value)*/
    {
        assert(i < this->size());
        return this->data()[i];
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type i) const noexcept/*(disable_if_n<0>::value)*/
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
    constexpr writable_sized_array_view<T, N> first() noexcept
    {
        static_assert(N <= size());
        return writable_sized_array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, detail::static_uint<N>::value>
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
    constexpr sized_array_view<T, N> first() const noexcept
    {
        static_assert(N <= size());
        return sized_array_view<T, N>::assumed(p);
    }

    template<class N>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<N>::value>
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
    [[nodiscard]] constexpr writable_sized_array_view<T, n> last() noexcept
    {
        static_assert(n <= size());
        return writable_sized_array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, detail::static_uint<N>::value>
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
    [[nodiscard]] constexpr sized_array_view<T, n> last() const noexcept
    {
        static_assert(n <= size());
        return sized_array_view<T, n>::assumed(p + (size() - n));
    }

    template<class N>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<N>::value>
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
    constexpr writable_sized_array_view<T, size() - offset> from_offset() noexcept
    {
        static_assert(offset <= size());
        return writable_sized_array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, size() - detail::static_uint<Offset>::value>
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
    constexpr sized_array_view<T, size() - offset> from_offset() const noexcept
    {
        static_assert(offset <= size());
        return sized_array_view<T, size() - offset>::assumed(p + offset);
    }

    template<class Offset>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Offset>::value>
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
    constexpr writable_sized_array_view<T, count> subarray(size_type offset) noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return writable_sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }

    template<std::size_t offset, std::size_t count>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, count> subarray() noexcept
    {
        static_assert(offset <= size() && count <= size() - offset);
        return writable_sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Offset, class Count>
    [[nodiscard]]
    constexpr auto
    subarray(size_type offset, Offset const& /*offset*/, Count const& /*n*/) noexcept
    -> decltype(subarray<
        detail::static_uint<Offset>::value,
        detail::static_uint<Count>::value
    >(0))
    {
        return subarray<
            detail::static_uint<Offset>::value,
            detail::static_uint<Count>::value
        >(offset);
    }

    [[nodiscard]]
    constexpr array_view<T> subarray(size_type offset, size_type count) const noexcept
    {
        assert(offset <= this->size() && count <= this->size() - offset);
        return {this->data() + offset, count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr sized_array_view<T, count> subarray(size_type offset) const noexcept
    {
        static_assert(count <= size());
        assert(offset <= this->size() && count <= this->size() - offset);
        return sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, detail::static_uint<Count>::value>
    subarray(size_type offset, Count const& /*n*/) const noexcept
    {
        return subarray<detail::static_uint<Count>::value>(offset);
    }

    template<std::size_t offset, std::size_t count>
    [[nodiscard]]
    constexpr sized_array_view<T, count> subarray() const noexcept
    {
        static_assert(offset <= size() && count <= size() - offset);
        return sized_array_view<T, count>::assumed(p + offset);
    }

    template<class Offset, class Count>
    [[nodiscard]]
    constexpr auto
    subarray(Offset const& /*offset*/, Count const& /*n*/) const noexcept
    -> decltype(subarray<
        detail::static_uint<Offset>::value,
        detail::static_uint<Count>::value
    >(0))
    {
        return subarray<
            detail::static_uint<Offset>::value,
            detail::static_uint<Count>::value
        >();
    }


    [[nodiscard]]
    constexpr writable_array_view<T> drop_front(size_type count) noexcept
    {
        assert(count <= this->size());
        return writable_array_view<T>{this->data() + count, this->size() - count};
    }

    template<std::size_t count>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, size() - count> drop_front() noexcept
    {
        static_assert(count <= size());
        return writable_sized_array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    constexpr sized_array_view<T, size() - count> drop_front() const noexcept
    {
        static_assert(count <= size());
        return sized_array_view<T, size() - count>::assumed(p + count);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    constexpr writable_sized_array_view<T, size() - count> drop_back() noexcept
    {
        static_assert(count <= size());
        return writable_sized_array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr writable_sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    constexpr sized_array_view<T, size() - count> drop_back() const noexcept
    {
        static_assert(count <= size());
        return sized_array_view<T, size() - count>::assumed(p);
    }

    template<class Count>
    [[nodiscard]]
    constexpr sized_array_view<T, size() - detail::static_uint<Count>::value>
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
    explicit constexpr writable_sized_array_view(pointer p) noexcept
    : p(p)
    {}

    pointer p    = nullptr;
    size_type sz = 0;
};

#undef gcc_8_fix_decltemplate
#undef gcc_8_fix_decltemplate


namespace detail
{
    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<sized_array_view<T, N>>
    : std::integral_constant<std::size_t, N>
    {};

    template<class T, std::size_t N>
    struct sequence_to_static_size_impl<writable_sized_array_view<T, N>>
    : std::integral_constant<std::size_t, N>
    {};
} // namespace detail


template<class T>
writable_sized_array_view(T&&) -> writable_sized_array_view<
    detail::value_type_array_view_from_t<T&&>,
    sequence_to_static_size_t<T&&>::value
>;


template<class T, std::size_t Size>
constexpr sized_array_view<T, Size>
make_sized_array_view(sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class T, std::size_t Size>
constexpr sized_array_view<T, Size>
make_sized_array_view(writable_sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class Cont>
constexpr auto make_sized_array_view(Cont const& cont)
    noexcept(noexcept(sized_array_view{cont}))
-> decltype(sized_array_view{cont})
{
    return {cont};
}

template<class T, std::size_t N>
constexpr sized_array_view<T, N>
make_sized_array_view(T const (&arr)[N]) noexcept
{
    return sized_array_view<T, N>::assumed(&arr[0]);
}


// TODO renamed to zstring_array
template<std::size_t N>
constexpr sized_array_view<char, N-1>
cstr_sized_array_view(char const (&str)[N]) noexcept
{
    return sized_array_view<char, N-1>::assumed(str);
}

// TODO renamed to zstring_array
// forbidden: sized_array_view is for litterals
template<std::size_t N>
sized_array_view<char, N-1> cstr_sized_array_view(char (&str)[N]) = delete;


template<class T, std::size_t Size>
constexpr writable_sized_array_view<T, Size>
make_writable_sized_array_view(writable_sized_array_view<T, Size> av) noexcept
{
    return av;
}

template<class Cont>
constexpr auto make_writable_sized_array_view(Cont& cont)
    noexcept(noexcept(writable_sized_array_view{cont}))
-> decltype(writable_sized_array_view{cont})
{
    return writable_sized_array_view{cont};
}

template<class T, std::size_t N>
constexpr writable_sized_array_view<T, N>
make_writable_sized_array_view(T (&arr)[N]) noexcept
{
    return writable_sized_array_view<T, N>::assumed(&arr[0]);
}


template<std::size_t N, class T, class AV
    = sized_array_view<detail::value_type_array_view_from_t<T&&>, N>>
constexpr AV make_assumed_sized_array_view(T&& x)
    noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
{
    return AV::assumed(static_cast<T&&>(x));
}

template<std::size_t N, class T>
constexpr sized_array_view<T, N> make_assumed_sized_array_view(T const* x) noexcept
{
    return sized_array_view<T, N>::assumed(x);
}

template<std::size_t N, class T, class AV
    = writable_sized_array_view<detail::value_type_array_view_from_t<T&&>, N>>
constexpr AV make_assumed_writable_sized_array_view(T&& x)
    noexcept(noexcept(AV::assumed(static_cast<T&&>(x))))
{
    return AV::assumed(static_cast<T&&>(x));
}

template<std::size_t N, class T>
constexpr writable_sized_array_view<T, N>
make_assumed_writable_sized_array_view(T* x) noexcept
{
    return writable_sized_array_view<T, N>::assumed(x);
}


template<std::size_t N> using sized_chars_view = sized_array_view<char, N>;
template<std::size_t N> using sized_u8_array_view = sized_array_view<std::uint8_t, N>;
template<std::size_t N> using sized_writable_u8_array_view = writable_sized_array_view<std::uint8_t, N>;
