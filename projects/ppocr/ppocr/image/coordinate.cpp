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

#include "ppocr/image/coordinate.hpp"

#include <ostream>
#include <istream>

namespace ppocr {

std::ostream & operator<<(std::ostream & os, Index const & idx) {
    return os << idx.x() << ' ' << idx.y();
}

std::ostream & operator<<(std::ostream & os, Bounds const & bnd) {
    return os << bnd.width() << ' ' << bnd.height();
}

std::istream & operator>>(std::istream & is, Index & idx) {
    unsigned x, y;
    if (is >> x >> y) {
        idx = Index(x, y);
    }
    return is;
}

std::istream & operator>>(std::istream & is, Bounds & bnd) {
    unsigned w, h;
    if (is >> w >> h) {
        bnd = Bounds(w, h);
    }
    return is;
}

}
