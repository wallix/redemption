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

#ifndef PPOCR_SRC_OCR2_PROBABILITIES_HPP
#define PPOCR_SRC_OCR2_PROBABILITIES_HPP

#include <new>
#include <utility>
#include <algorithm>

namespace ppocr { namespace ocr2 {

struct Probability
{
    unsigned i;
    double prob;

    Probability(unsigned i) noexcept
    : i(i)
    {}

    Probability(unsigned i, double prob) noexcept
    : i(i)
    , prob(prob)
    {}
};

struct Probabilities
{
    using iterator = Probability *;
    using size_type = unsigned;

    Probabilities(size_type sz)
    : data(static_cast<Probability*>(::operator new(sz * sizeof(Probability))))
    , current(data)
    {}

    template<class It>
    Probabilities(It first, It last)
    : data(static_cast<Probability*>(::operator new((last-first) * sizeof(Probability))))
    , current(data + (last-first))
    { std::copy(first, last, data); }

    Probabilities(Probabilities &&) = delete;
    Probabilities(Probabilities const &) = delete;

    void swap(Probabilities & p) noexcept
    {
        std::swap(p.data, data);
        std::swap(p.current, current);
    }

    ~Probabilities() {
      ::operator delete(this->data);
    }

    iterator begin() const { return data; }
    iterator end() const { return current; }
    size_type size() const { return current - data; }
    bool empty() const { return current == data; }
    void push_back(Probability const & p) { *current++ = p; }
    template<class... Args>
    void emplace_back(Args const & ... args) { *current++ = {args...}; }
    void clear() { current = data; }

    Probability const & front() const { return *data; }
    Probability const & back() const { return *(current-1); }
    Probability const & operator[](size_type i) const { return data[i]; }

    void resize(size_type n) {
        current = data + n;
    }

private:
    Probability * data;
    Probability * current;
};

inline void swap(Probabilities & a, Probabilities & b) noexcept
{ a.swap(b); }

struct GtProb {
    GtProb() {}
    bool operator()(Probability const & a, Probability const & b) const {
        return a.prob > b.prob;
    }
};

} }

#endif
