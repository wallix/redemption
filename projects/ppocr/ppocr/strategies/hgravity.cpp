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

#include "ppocr//strategies/hgravity.hpp"

#include "ppocr/strategies/utils/horizontal_gravity.hpp"

#include "ppocr/image/image.hpp"

namespace ppocr { namespace strategies {

hgravity::value_type hgravity::load(Image const & img, Image const & /*img90*/) const
{
    auto const top_bottom = utils::horizontal_gravity(img);
    auto const sum = top_bottom.top + top_bottom.bottom;
    return sum ? (sum + top_bottom.top - top_bottom.bottom) * 100 / sum : 100;
}

hgravity::relationship_type hgravity::relationship() const
{ return {200}; }

unsigned hgravity::best_difference() const
{ return 10; }

} }
