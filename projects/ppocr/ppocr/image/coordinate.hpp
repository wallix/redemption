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

struct Index {
    Index(unsigned x, unsigned y)
    : x_(x)
    , y_(y)
    {}

    Index() = default;

    unsigned x() const noexcept { return x_; }
    unsigned y() const noexcept { return y_; }

    bool operator == (Index const & other) const noexcept
    { return this->x_ == other.x_ && this->y_ == other.y_; }
    bool operator != (Index const & other) const noexcept
    { return !(*this == other); }

private:
    unsigned x_ = 0;
    unsigned y_ = 0;
};

struct Bounds {
    Bounds(unsigned w, unsigned h)
    : w_(w)
    , h_(h)
    {}

    Bounds() = default;

    unsigned area() const noexcept { return w_ * h_; }
    unsigned w() const noexcept { return w_; }
    unsigned h() const noexcept { return h_; }

    unsigned contains(Index const & idx) const
    { return idx.x() < w() && idx.y() < h(); }

    bool operator == (Bounds const & other) const noexcept
    { return this->w_ == other.w_ && this->h_ == other.h_; }
    bool operator != (Bounds const & other) const noexcept
    { return !(*this == other); }

private:
    unsigned w_ = 0;
    unsigned h_ = 0;
};


std::ostream & operator<<(std::ostream & os, Index const & idx);
std::istream & operator>>(std::istream & is, Index & idx);
std::ostream & operator<<(std::ostream & os, Bounds const & bnd);
std::istream & operator>>(std::istream & is, Bounds & bnd);

}

#endif
