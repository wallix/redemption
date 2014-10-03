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

#ifndef REDEMPTION_UTILS_GET_S_HPP
#define REDEMPTION_UTILS_GET_S_HPP

#include <iterator>

template<class ForwardIterator>
class splitter
{
    ForwardIterator first_;
    ForwardIterator last_;
    ForwardIterator cur_;
    using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
    value_type sep_;

    struct range
    {
        ForwardIterator first_;
        ForwardIterator last_;

        std::size_t size() const
        { return this->last_ - this->first_; }

        const value_type & front() const
        { return *(this->first_); }

        value_type & front()
        { return *(this->first_); }

        const value_type & back() const
        { return *(this->last_-1); }

        value_type & back()
        { return *(this->last_-1); }

        ForwardIterator begin() const
        { return this->first_; }

        ForwardIterator end() const
        { return this->last_; }
    };

public:
    splitter(ForwardIterator first, ForwardIterator last, value_type sep)
    : first_(first)
    , last_(last)
    , cur_(first)
    , sep_(sep)
    {}

    range next() {
        this->first_ = this->cur_;
        while (this->cur_ != last_ && *this->cur_ != this->sep_ ) {
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

        iterator(splitter & s)
        : splitter_(s)
        , r_(s.next())
        {}

        iterator(splitter & s, int)
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


template<class ForwardIterator, class T = typename std::iterator_traits<ForwardIterator>::value_type>
splitter<ForwardIterator> get_split(ForwardIterator first, ForwardIterator last, T sep = T()) {
    return {first, last, sep};
}


inline splitter<const char *> get_line(const char * s, char sep = '\n') {
    return {s, s+strlen(s), sep};
}

inline splitter<char *> get_line(char * s, char sep = '\n') {
    return {s, s+strlen(s), sep};
}

#endif
