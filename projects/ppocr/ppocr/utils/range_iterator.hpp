/*
* Copyright (C) 2016 Wallix
* 
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
* 
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
* 
* You should have received a copy of the GNU Lesser General Public License along
* with this library; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef PPOCR_SRC_UTILS_RANGE_ITERATOR_HPP
#define PPOCR_SRC_UTILS_RANGE_ITERATOR_HPP

#include <utility>
#include <iterator>
#include <type_traits>


namespace ppocr {

namespace detail_ {
    struct DefaultProxy {
        DefaultProxy() noexcept {}
        template<class T> T const & operator()(T const & x) const { return x; }
        template<class T> T& operator()(T& x) const { return x; }
        template<class T> T operator()(T&& x) const { return std::forward<T>(x); }
    };

    template<class T>
    struct pointer_iterator
    {
        using value_type = T;
        using reference = T&;
        using const_reference = T const &;
        using iterator_tag = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;

        pointer_iterator(T * p = nullptr) noexcept
        : p_(p)
        {}

        pointer_iterator & operator++() noexcept { ++p_; return *this; }
        pointer_iterator operator++(int) noexcept { return {p_++}; }

        T & operator*() noexcept { return *p_; }
        T const & operator*() const noexcept { return *p_; }

        T & operator[](std::size_t i) noexcept { return p_[i]; }
        T const & operator[](std::size_t i) const noexcept { return p_[i]; }

        pointer_iterator operator+(std::ptrdiff_t n) const noexcept { return {p_+n}; }
        pointer_iterator & operator+=(std::ptrdiff_t n) noexcept { p_ += n; return *this; }
        pointer_iterator & operator-=(std::ptrdiff_t n) noexcept { p_ -= n; return *this; }

        std::ptrdiff_t operator-(pointer_iterator const & other) const noexcept { return p_ - other.p_; }

        bool operator == (pointer_iterator const & other) const noexcept { return p_ == other.p_; }
        bool operator != (pointer_iterator const & other) const noexcept { return p_ != other.p_; }
        bool operator < (pointer_iterator const & other) const noexcept { return p_ < other.p_; }
        bool operator > (pointer_iterator const & other) const noexcept { return p_ > other.p_; }
        bool operator <= (pointer_iterator const & other) const noexcept { return p_ <= other.p_; }
        bool operator >= (pointer_iterator const & other) const noexcept { return p_ >= other.p_; }

    private:
        T * p_;
    };

    template<class Iterator>
    using force_iterator_class = typename std::conditional<
        std::is_pointer<Iterator>::value,
        pointer_iterator<typename std::remove_pointer<Iterator>::type>,
        Iterator
    >::type;
}

template<class IteratorBase, class Proxy>
struct proxy_iterator : detail_::force_iterator_class<IteratorBase>, private Proxy
{
    using iterator_base = detail_::force_iterator_class<IteratorBase>;

    proxy_iterator(IteratorBase base, Proxy proxy)
    : iterator_base(base)
    , Proxy(proxy)
    {}

    proxy_iterator(IteratorBase base)
    : iterator_base(base)
    {}

private:
    iterator_base & base_() { return static_cast<iterator_base&>(*this); }
    Proxy & proxy_() { return static_cast<Proxy&>(*this); }

public:
    auto operator*() -> decltype(this->proxy_()(*this->base_()))
    { return this->proxy_()(*this->base_()); }

    auto operator->() -> decltype(&this->proxy_()(*this->base_()))
    { return &this->proxy_()(*this->base_()); }

    auto operator[](std::size_t i) -> decltype(this->proxy_()(*(this->base_()+i)))
    { return this->proxy_()(*(this->base_()+i)); }
};

template<class Iterator, class Proxy = detail_::DefaultProxy>
class range_iterator : Proxy
{
    Iterator first_;
    Iterator last_;

    Proxy const & proxy_() const { return static_cast<Proxy const &>(*this); }

public:
    range_iterator(Iterator first, Iterator last)
    : first_(first)
    , last_(last)
    {}

    range_iterator(Iterator first, Iterator last, Proxy proxy)
    : Proxy(proxy)
    , first_(first)
    , last_(last)
    {}

    proxy_iterator<Iterator, Proxy> begin() const { return {this->first_, this->proxy_()}; }
    proxy_iterator<Iterator, Proxy> end() const { return {this->last_, this->proxy_()}; }

    auto operator[](std::size_t i) const -> decltype(this->proxy_()(*(this->first_+i)))
    { return this->proxy_()(*(this->first_+i)); }

    auto front() const -> decltype(this->proxy_()(*this->first_))
    { return this->proxy_()(*this->first_); }

    auto back() const -> decltype(this->proxy_()(*(this->last_-1)))
    { return this->proxy_()(*(this->last_-1)); }

    auto size() const -> decltype(this->last_ - this->first_)
    { return this->last_ - this->first_; }

    bool empty() const { return this->first_ == this->last_; }
};

}

#endif
