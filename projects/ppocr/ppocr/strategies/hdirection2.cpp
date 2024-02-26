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

#include "ppocr//strategies/hdirection2.hpp"

#include "ppocr/strategies/utils/horizontal_direction.hpp"

#include "ppocr/image/image.hpp"


namespace ppocr { namespace strategies {

hdirection2::value_type hdirection2::load(Image const & img, Image const & /*img90*/) const
{
    auto const area = img.height() / 2 * img.width();
    if (!area) {
        return 50;
    }
    auto const top_bottom = utils::horizontal_direction(img);
    auto const ret = (area + top_bottom.top - top_bottom.bottom) * 100 / (area * 2);
    return ret;
}

hdirection2::relationship_type hdirection2::relationship() const
{ return {100}; }

unsigned hdirection2::best_difference() const
{ return 5; }

} }
