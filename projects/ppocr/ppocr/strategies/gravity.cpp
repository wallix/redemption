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

#include "ppocr/strategies/gravity.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/strategies/utils/relationship.hpp"
#include "ppocr/strategies/utils/horizontal_gravity.hpp"
#include "ppocr/strategies/utils/horizontal_zone.hpp"
#include "ppocr/strategies/utils/cardinal_direction_io.hpp"

#include <ostream>
#include <istream>

#include <type_traits>

namespace ppocr { namespace strategies {

gravity::gravity(const Image& img, const Image& img90)
: d(static_cast<cardinal_direction>(
    utils::horizontal_zone_1_2_3_4_5(img, utils::horizontal_gravity(img))
 | (utils::horizontal_zone_1_2_3_4_5(img90, utils::horizontal_gravity(img90)) << 3)
))
{}

unsigned gravity::relationship(const gravity& other) const
{ return utils::cardinal_relationship(d, other.d); }

std::istream& operator>>(std::istream& is, gravity& d)
{ return is >> d.d; }

std::ostream& operator<<(std::ostream& os, const gravity& d)
{ return os << d.id(); }

} }
