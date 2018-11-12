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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include <cassert>
#include <iterator>


template<class Iterator>
class safe_iterator
{
    using _iterator_traits = std::iterator_traits<Iterator>;

public:
    using iterator_category = typename _iterator_traits::iterator_category;
    using value_type = typename _iterator_traits::value_type;
    using difference_type = typename _iterator_traits::difference_type;
    using pointer = typename _iterator_traits::pointer;
    using reference = typename _iterator_traits::reference;

    safe_iterator() = default;
    safe_iterator(safe_iterator &&) = default;
    safe_iterator(safe_iterator const&) = default;

    safe_iterator& operator=(safe_iterator &&) = default;
    safe_iterator& operator=(safe_iterator const&) = default;

#ifdef NDEBUG
    explicit safe_iterator(Iterator it, Iterator const&)
      : it(std::move(it))
    {}

    explicit safe_iterator(Iterator it, std::size_t)
      : it(std::move(it))
    {}

    explicit safe_iterator(Iterator it, Iterator const&, Iterator const&)
      : it(std::move(it))
    {}
#else
    explicit safe_iterator(Iterator it, Iterator end)
      : it(std::move(it))
      , begin(it)
      , end(std::move(end))
    {}

    explicit safe_iterator(Iterator it, std::size_t n)
      : it(std::move(it))
      , begin(it)
      , end(this->begin + n)
    {}

    explicit safe_iterator(Iterator it, Iterator begin, Iterator end)
      : it(std::move(it))
      , begin(std::move(begin))
      , end(std::move(end))
    {}
#endif

    safe_iterator& operator++()
    {
        check_range(1);
        ++it;
        return *this;
    }

    safe_iterator& operator--()
    {
        check_range(-1);
        --it;
        return *this;
    }

    safe_iterator operator++(int)
    {
        check_range(1);
        auto old = *this;
        ++*this;
        return old;
    }

    safe_iterator operator--(int)
    {
        check_range(-1);
        auto old = *this;
        --*this;
        return old;
    }

    safe_iterator& operator+=(difference_type n)
    {
        check_range(n);
        it += n;
        return *this;
    }

    safe_iterator& operator-=(difference_type n)
    {
        check_range(-n);
        it -= n;
        return *this;
    }

    safe_iterator operator+(difference_type n) const
    {
#ifdef NDEBUG
        return safe_iterator(it+n, it);
#else
        check_range(n);
        return safe_iterator(it+n, begin, end);
#endif
    }

    safe_iterator operator-(difference_type n) const
    {
#ifdef NDEBUG
        return safe_iterator(it+n, it);
#else
        check_range(-n);
        return safe_iterator(it-n, begin, end);
#endif
    }

    difference_type operator-(safe_iterator const& other) const
    {
        assert(it >= other.it);
        return it - other.it;
    }

    reference operator*()
    {
        check_at(0);
        return *it;
    }

    reference operator[](difference_type i)
    {
        check_at(i);
        return it[i];
    }

    bool operator==(safe_iterator const& other) const
    {
        return it == other.it;
    }

    bool operator!=(safe_iterator const& other) const
    {
        return it != other.it;
    }

    bool operator<(safe_iterator const& other) const
    {
        return it < other.it;
    }

    bool operator>(safe_iterator const& other) const
    {
        return it > other.it;
    }

    bool operator<=(safe_iterator const& other) const
    {
        return it <= other.it;
    }

    bool operator>=(safe_iterator const& other) const
    {
        return it >= other.it;
    }

    Iterator base() const
    {
        return it;
    }

private:
    Iterator it {};
#ifdef NDEBUG
    void check_range(difference_type) const  {}
    void check_at(difference_type) const  {}
#else
    Iterator begin {};
    Iterator end {};

    void check_range(difference_type n) const
    {
        if (n < 0) {
            if (bool(begin)) {
                assert(it - begin >= n);
            }
        }
        else if (bool(end)) {
            assert(end - it >= n);
        }
    }

    void check_at(difference_type i) const
    {
        if (i <= 0) {
            if (bool(begin)) {
                assert(it - begin >= i);
            }
        }
        else if (bool(end)) {
            assert(end - it > i);
        }
    }
#endif
};


template<class Iterator>
safe_iterator<Iterator> operator+(
    typename safe_iterator<Iterator>::difference_type n,
    safe_iterator<Iterator> const& it)
{
    return it + n;
}

template<class Iterator>
safe_iterator<Iterator> operator-(
    typename safe_iterator<Iterator>::difference_type n,
    safe_iterator<Iterator> const& it)
{
    return it - n;
}
