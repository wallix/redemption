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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#pragma once

#include <iterator>
#include <cstring>

#include "utils/sugar/range.hpp"


template<class ForwardIterator, class ValueT = typename std::iterator_traits<ForwardIterator>::value_type>
class splitter
{
    ForwardIterator first_;
    ForwardIterator last_;
    ForwardIterator cur_;
    using value_type = ValueT;
    value_type sep_;

    using range = ::range<ForwardIterator>;

public:
    splitter(ForwardIterator first, ForwardIterator last, value_type sep)
    : first_(first)
    , last_(last)
    , cur_(first)
    , sep_(sep)
    {}

    range next() {
        this->first_ = this->cur_;
        while (this->cur_ != last_ && !bool(this->sep_ == *this->cur_)) {
            ++this->cur_;
        }
        range res{this->first_, this->cur_};
        if (this->cur_ != this->last_ ) {
            ++this->cur_;
        }
        return res;
    }

    bool empty() const {
        return this->first_ == this->last_;
    }

private:
    class iterator {
        splitter & splitter_;
        range r_;

        friend class splitter;

        explicit iterator(splitter & s)
        : splitter_(s)
        , r_(s.next())
        {}

        explicit iterator(splitter & s, int /*unused*/)
        : splitter_(s)
        {}

    public:
        iterator& operator++() {
            this->r_ = this->splitter_.next();
            return *this;
        }

        const range & operator*() const {
            return this->r_;
        }

        const range* operator->() const {
            return &this->r_;
        }

        bool operator==(iterator const & other) const noexcept {
            return this->splitter_.first_ == other.splitter_.last_;
        }

        bool operator!=(iterator const & other) const noexcept {
            return !this->operator==(other);
        }
    };

public:
    iterator begin() {
        return iterator(*this);
    }

    iterator end() {
        return iterator(*this, 1);
    }
};


template<class ForwardIterator, class T>
splitter<ForwardIterator, typename std::decay<T>::type>
get_split(ForwardIterator first, ForwardIterator last, T && sep)
{
    return {first, last, std::forward<T>(sep)};
}

namespace adl_begin_end
{
    using std::begin;
    using std::end;

    template<class Cont>
    auto begin_(Cont && cont) -> decltype(begin(std::forward<Cont>(cont)))
    { return begin(cont); }

    template<class Cont>
    auto end_(Cont && cont) -> decltype(end(std::forward<Cont>(cont)))
    { return end(cont); }
}  // namespace adl_begin_end

namespace fn
{
    template<class Cont>
    auto begin(Cont & cont)
    -> decltype(adl_begin_end::begin_(cont))
    { return adl_begin_end::begin_(cont); }

    template<class Cont>
    auto end(Cont & cont)
    -> decltype(adl_begin_end::end_(cont))
    { return adl_begin_end::end_(cont); }
}  // namespace fn

template<class Cont>
struct container_traits
{
    using iterator = decltype(adl_begin_end::begin_(std::declval<Cont>()));
};

template<class T> struct container_traits<T*> { using iterator = T*; };
template<class T> struct container_traits<T*&> { using iterator = T*; };
template<class T> struct container_traits<T*&&> { using iterator = T*; };
template<class T, std::size_t n> struct container_traits<T[n]> { using iterator = T*; };
template<class T, std::size_t n> struct container_traits<T(&)[n]> { using iterator = T*; };

template<class Cont, class T>
splitter<typename container_traits<Cont>::iterator, typename std::decay<T>::type>
get_split(Cont && cont, T && sep)
{
    return {fn::begin(cont), fn::end(cont), std::forward<T>(sep)};
}


inline splitter<char const *> get_line(const char * s, char sep = '\n')
{
    return {s, s+strlen(s), sep};
}

inline splitter<char *> get_line(char * s, char sep = '\n')
{
    return {s, s+strlen(s), sep};
}

inline splitter<char const *> get_line(std::string const & s, char sep = '\n')
{
    return {s.data(), s.data()+s.size(), sep};
}

inline splitter<char *> get_line(std::string & s, char sep = '\n')
{
    return {&s[0], &s[0]+s.size(), sep};
}

splitter<char *> get_line(std::string && s, char sep = '\n') = delete;
splitter<char const *> get_line(std::string const && s, char sep = '\n') = delete;
