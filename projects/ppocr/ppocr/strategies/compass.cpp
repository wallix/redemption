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

#include "ppocr/strategies/compass.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/strategies/utils/relationship.hpp"
#include "ppocr/strategies/utils/horizontal_direction.hpp"
#include "ppocr/strategies/utils/cardinal_direction_io.hpp"

#include <ostream>
#include <istream>

namespace ppocr { namespace strategies {

static int horizontal_compass(const Image& img)
{
    utils::TopBottom d = utils::horizontal_direction(img);

    return (d.top > d.bottom) ? 1
        : (d.top < d.bottom) ? 3
        : 2;
}

compass::compass(const Image& img, const Image& img90)
: d(static_cast<cardinal_direction>(horizontal_compass(img) | horizontal_compass(img90) << 2))
{}

unsigned int compass::relationship(const compass& other) const
{ return utils::cardinal_relationship(d, other.d); }

std::istream& operator>>(std::istream& is, compass& d)
{ return is >> d.d; }

std::ostream& operator<<(std::ostream& os, const compass& d)
{ return os << d.direction(); }

} }
