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

#ifndef PPOCR_BOX_HPP
#define PPOCR_BOX_HPP

#include "ppocr/image/coordinate.hpp"

#include <iosfwd>


namespace ppocr {

struct Box
{
    Box() = default;

    Box(Index const & idx,  Bounds const & bnd)
    : idx_(idx)
    , bounds_(bnd)
    {}

    Box(Bounds const & bnd)
    : bounds_(bnd)
    {}

    size_t x() const noexcept { return idx_.x(); }
    size_t y() const noexcept { return idx_.y(); }

    size_t w() const noexcept { return bounds_.w(); }
    size_t h() const noexcept { return bounds_.h(); }

    size_t bottom() const noexcept { return y()+h()-1; }
    size_t right() const noexcept { return x()+w()-1; }
    size_t left() const noexcept { return x(); }
    size_t top() const noexcept { return y(); }

    Index const & index() const noexcept { return idx_; }
    Bounds const & bounds() const noexcept { return bounds_; }

    explicit operator bool() const noexcept { return w() && h(); }

    bool operator == (Box const & other) const noexcept
    { return this->bounds_ == other.bounds_; }
    bool operator != (Box const & other) const noexcept
    { return !(*this == other); }

private:
    Index idx_;
    Bounds bounds_;
};

std::ostream & operator<<(std::ostream & os, Box const & box);
std::istream & operator>>(std::istream & is, Box & box);

}

#endif
