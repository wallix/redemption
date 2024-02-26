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

#ifndef PPOCR_COORDINATE_HPP
#define PPOCR_COORDINATE_HPP

#include <cstddef>
#include <iosfwd>


namespace ppocr {

using std::size_t;

struct Index {
    Index(size_t x, size_t y)
    : x_(x)
    , y_(y)
    {}

    Index() = default;

    size_t x() const noexcept { return x_; }
    size_t y() const noexcept { return y_; }

    bool operator == (Index const & other) const noexcept
    { return this->x_ == other.x_ && this->y_ == other.y_; }
    bool operator != (Index const & other) const noexcept
    { return !(*this == other); }

private:
    size_t x_ = 0;
    size_t y_ = 0;
};

struct Bounds {
    Bounds(size_t w, size_t h)
    : w_(w)
    , h_(h)
    {}

    Bounds() = default;

    size_t area() const noexcept { return w_ * h_; }
    size_t w() const noexcept { return w_; }
    size_t h() const noexcept { return h_; }

    size_t contains(Index const & idx) const
    { return idx.x() < w() && idx.y() < h(); }

    bool operator == (Bounds const & other) const noexcept
    { return this->w_ == other.w_ && this->h_ == other.h_; }
    bool operator != (Bounds const & other) const noexcept
    { return !(*this == other); }

private:
    size_t w_ = 0;
    size_t h_ = 0;
};


std::ostream & operator<<(std::ostream & os, Index const & idx);
std::istream & operator>>(std::istream & is, Index & idx);
std::ostream & operator<<(std::ostream & os, Bounds const & bnd);
std::istream & operator>>(std::istream & is, Bounds & bnd);

}

#endif
