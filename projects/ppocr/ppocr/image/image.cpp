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

#include "ppocr/image/image.hpp"
#include <ostream>

#include <cassert>

namespace ppocr {

namespace {
    using cP = Pixel const *;
    using P = Pixel *;
}

Image::Image(Bounds const& bounds, PtrImageData data)
: data_(std::move(data))
, bounds_(bounds)
{}

void section(Image const & from, Pixel * data, Index const & idx, Bounds const & bnd)
{
    cP d = from.data(idx);
    for (unsigned y = 0; y != bnd.height(); ++y) {
        data = std::copy(d, d+bnd.width(), data);
        d += from.width();
    }
}

Image Image::section(Index const& section_idx, Bounds const& section_bnd) const
{
    assert(bounds_.contains(section_idx));
    assert(section_bnd.width() + section_idx.x() <= width() && section_bnd.height() + section_idx.y() <= height());
    PtrImageData data(new Pixel[section_bnd.area()]);
    ::ppocr::section(*this, data.get(), section_idx, section_bnd);
    return {{section_bnd.width(), section_bnd.height()}, std::move(data)};
}

void rotate90(Image const & from, Pixel * data)
{
    for (unsigned x = from.width(); x; ) {
        --x;
        for (cP d = from.data() + x, e = d + from.area(); d != e; d += from.width()) {
            *data++ = *d;
        }
    }
}

Image Image::rotate90() const
{
    PtrImageData data(new Pixel[this->area()]);
    ::ppocr::rotate90(*this, data.get());
    return {{this->height(), this->width()}, std::move(data)};
}

Image Image::clone() const
{
    return this->section({}, this->bounds());
}

bool operator==(const Image& a, const Image& b)
{
    return a.width() == b.width()
        && a.height() == b.height()
        && std::equal(a.data(), a.data_end(), b.data());
}

std::ostream & operator<<(std::ostream & os, Image const & image)
{
    os.fill(':');
    os.width(image.width()+3);
    os << ":\n";
    cP p = image.data_.get();
    for (unsigned h = 0; h != image.height(); ++h) {
        os << ':';
        os.write(p, image.width());
        os << ":\n";
        p += image.width();
    }
    os.width(image.width()+3);
    os << ":\n";
    return os;
}

}
