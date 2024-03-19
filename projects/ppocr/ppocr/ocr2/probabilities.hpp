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

#include <cstdlib>
#include <utility>

namespace ppocr { namespace ocr2 {

struct Probability
{
    unsigned i;
    double prob;
};

struct Probabilities
{
    using iterator = Probability *;
    using size_type = unsigned;

    Probabilities(size_type sz)
    : data(static_cast<Probability*>(
        aligned_alloc(alignof(Probability), sz * sizeof(Probability))))
    , current(data)
    {}

    Probabilities(Probabilities const &) = delete;
    Probabilities& operator=(Probabilities const &) = delete;

    void swap(Probabilities & p) noexcept
    {
        std::swap(p.data, data);
        std::swap(p.current, current);
    }

    ~Probabilities()
    {
        free(this->data);
    }

    iterator begin() const { return data; }
    iterator end() const { return current; }
    size_type size() const { return static_cast<size_type>(current - data); }
    bool empty() const { return current == data; }
    void push_back(Probability const & p) { *current++ = p; }
    void emplace_back(unsigned i) { current->i = i; ++current; }
    void emplace_back(unsigned i, double prob) { *current++ = {i, prob}; }
    void clear() { current = data; }

    Probability const & front() const { return *data; }
    Probability const & back() const { return *(current-1); }
    Probability const & operator[](size_type i) const { return data[i]; }

    void resize(size_type n)
    {
        current = data + n;
    }

private:
    Probability * data;
    Probability * current;
};

inline void swap(Probabilities & a, Probabilities & b) noexcept
{ a.swap(b); }

struct GtProb {
    bool operator()(Probability const & a, Probability const & b) const
    {
        return a.prob > b.prob;
    }
};

} }

#endif
