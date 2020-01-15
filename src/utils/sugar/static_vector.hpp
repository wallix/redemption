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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <type_traits>
#include <iterator>
#include <memory>
#include <cstdint>
#include <cassert>

namespace detail
{
    template<class T, std::size_t N>
    class static_vector_iterator;
}

template<class T, std::size_t N>
struct static_vector
{
    using iterator = detail::static_vector_iterator<T, N>;
    using const_iterator = detail::static_vector_iterator<T const, N>;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T&;

    static_vector() = default;

    static_vector(static_vector&& other) noexcept(noexcept(T(std::declval<T&&>())))
    : n(other.n)
    {
        std::uninitialized_move_n(other.data(), other.size(), this->data());
    }

    static_vector(static_vector const& other)
    : n(other.n)
    {
        std::uninitialized_copy_n(other.data(), other.size(), this->data());
    }

    ~static_vector()
    {
        this->clear();
    }

    static_vector& operator=(static_vector&& other) noexcept(noexcept(T(std::declval<T&&>())))
    {
        this->clear();
        std::uninitialized_move_n(other.data(), other.size(), this->data());
        this->n = other.n;
        return *this;
    }

    static_vector& operator=(static_vector const& other)
    {
        this->clear();
        std::uninitialized_copy_n(other.data(), other.size(), this->data());
        this->n = other.n;
        return *this;
    }

    void clear()
    {
        this->n = 0;
        std::destroy_n(this->data(), this->size());
    }

    template<class... Xs>
    T& emplace_back(Xs&&... args)
    {
        assert(this->size() < this->capacity());
        auto* p = new (this->data() + this->n) T{static_cast<Xs&&>(args)...}; /*NOLINT*/
        ++this->n;
        return *p;
    }

    template<class X>
    T& push_back(X&& arg)
    {
        assert(this->size() < this->capacity());
        auto* p = new (this->data() + this->n) T{static_cast<X&&>(arg)}; /*NOLINT*/
        ++this->n;
        return *p;
    }

    void pop_back() noexcept
    {
        assert(this->size());
        --this->n;
        this->data()[this->n].~T();
    }

    reference operator[](std::size_t i) noexcept
    {
        assert(i < this->size());
        return this->data()[i];
    }

    const_reference operator[](std::size_t i) const noexcept
    {
        assert(i < this->size());
        return this->data()[i];
    }

    reference front() noexcept
    {
        assert(!this->empty());
        return this->data()[0];
    }

    const_reference front() const noexcept
    {
        assert(!this->empty());
        return this->data()[0];
    }

    reference back() noexcept
    {
        assert(!this->empty());
        return this->data()[this->n - 1u];
    }

    const_reference back() const noexcept
    {
        assert(!this->empty());
        return this->data()[this->n - 1u];
    }

    size_type size() const noexcept
    {
        return this->n;
    }

    size_type capacity() const noexcept
    {
        return N;
    }

    bool empty() const noexcept
    {
        return this->n == 0;
    }

    const_pointer data() const noexcept
    {
        // TODO std::launder
        return reinterpret_cast<T const*>(this->buf);
    }

    pointer data() noexcept
    {
        // TODO std::launder
        return reinterpret_cast<T*>(this->buf);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator{this->data(), *this};
    }

    const_iterator end() const noexcept
    {
        return const_iterator{this->data()+n, *this};
    }

    iterator begin() noexcept
    {
        return iterator{this->data(), *this};
    }

    iterator end() noexcept
    {
        return iterator{this->data()+n, *this};
    }

private:
    using internal_size_type = std::conditional_t<
        (N > 16), uint32_t,
        std::conditional_t<(N > 8), uint16_t, uint8_t>
    >;

    std::aligned_storage_t<sizeof(T), alignof(T)> buf[N];
    internal_size_type n = 0;
};


namespace detail
{
    template<class T, std::size_t N>
    class static_vector_iterator
    {
        using static_vector_type = static_vector<std::remove_const_t<T>, N> const;
        using iterator_traits = std::iterator_traits<T*>;

    public:
        using difference_type = typename iterator_traits::difference_type;

        explicit static_vector_iterator() = default;

        explicit static_vector_iterator(T* p, [[maybe_unused]] static_vector_type& ref) noexcept
        : p(p)
#ifndef NDEBUG
        , ref(&ref)
#endif
        {}

        T& operator*() noexcept
        {
            assert(p);
            assert(this->is_valid_index(0));
            return *p;
        }

        T const& operator*() const noexcept
        {
            assert(p);
            assert(this->is_valid_index(0));
            return *p;
        }

        T& operator[](difference_type i) noexcept
        {
            assert(p);
            assert(this->is_valid_index(i));
            return p[i];
        }

        T const& operator[](difference_type i) const noexcept
        {
            assert(p);
            assert(this->is_valid_index(i));
            return p[i];
        }

        T* base() noexcept
        {
            return p;
        }

        T const* base() const noexcept
        {
            return p;
        }

        static_vector_iterator& operator++() noexcept
        {
            assert(p);
            assert(this->is_valid_it(1));
            ++this->p;
            return *this;
        }

        static_vector_iterator& operator--() noexcept
        {
            assert(p);
            assert(this->is_valid_it(-1));
            --this->p;
            return *this;
        }

        static_vector_iterator& operator+=(difference_type n) noexcept
        {
            assert(p);
            assert(this->is_valid_it(n));
            this->p += n;
            return *this;
        }

        static_vector_iterator& operator-=(difference_type n) noexcept
        {
            assert(p);
            assert(this->is_valid_it(-n));
            this->p -= n;
            return *this;
        }

        static_vector_iterator operator+(difference_type n) noexcept
        {
            auto it = *this;
            it += n;
            return it;
        }

        static_vector_iterator operator-(difference_type n) noexcept
        {
            auto it = *this;
            it -= n;
            return it;
        }

        bool operator==(static_vector_iterator const& other) const noexcept
        {
            return this->p == other.p;
        }

        bool operator!=(static_vector_iterator const& other) const noexcept
        {
            return this->p != other.p;
        }

    private:
        T* p = nullptr;

#ifndef NDEBUG
        static_vector_type* ref = nullptr;

        bool is_valid_index(difference_type n) const noexcept
        {
            return p >= ref->data() && p + n < ref->data() + ref->size();
        }

        bool is_valid_it(difference_type n) const noexcept
        {
            return p >= ref->data() && p + n <= ref->data() + ref->size();
        }
#endif
    };

    template<class T, class U, std::size_t N>
    std::enable_if_t<std::is_same<T const, U const>::value,
        typename static_vector_iterator<T, N>::difference_type>
    operator-(static_vector_iterator<T, N> const& a, static_vector_iterator<U, N> const& b) noexcept
    {
        assert(bool(a.base()) == bool(b.base()));
        return std::distance(b.base(), a.base());
    }

#define MK_OP(op)                                               \
    template<class T, class U, std::size_t N>                   \
    std::enable_if_t<std::is_same<T const, U const>::value,     \
        typename static_vector_iterator<T, N>::difference_type> \
    operator op (                                               \
        static_vector_iterator<T, N> const& a,                  \
        static_vector_iterator<U, N> const& b) noexcept         \
    {                                                           \
        return a.base() op b.base();                            \
    }

    MK_OP(==)
    MK_OP(!=)
    MK_OP(<)
    MK_OP(<=)
    MK_OP(>)
    MK_OP(>=)

#undef MK_OP
} // namespace detail

namespace std
{
    template<class T, std::size_t N>
    struct iterator_traits<::detail::static_vector_iterator<T, N>>
    : iterator_traits<T*>
    {};
}
