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

#include "ppocr/strategies/zone.hpp"
#include "ppocr/image/image.hpp"
#include "ppocr/strategies/utils/count_zone.hpp"


namespace ppocr { namespace strategies {

zone::value_type zone::load(const Image& img, const Image& /*img*/) const
{
    utils::ZoneInfo zone_info = utils::count_zone(img);

    zone::value_type ret;

    ret[top_left_is_letter] = is_pix_letter(img[{0, 0}]);
    ret[bottom_right_is_letter] = is_pix_letter(img[{img.width()-1, img.height()-1}]);
    ret[number_top_alternations] = zone_info.top().count_non_zero();
    ret[number_right_alternations] = zone_info.right().count_non_zero();
    ret[number_bottom_alternations] = zone_info.bottom().count_non_zero();
    ret[number_left_alternations] = zone_info.left().count_non_zero();

    for (unsigned i = 0; i < zone_info.len; ++i) {
        zone_info.top()[i] += zone_info.right()[i] + zone_info.bottom()[i] + zone_info.left()[i];
    }

    ret[number_internal_alternations] = zone_info.count_zone - 1 - zone_info.top().count_non_zero();

    return ret;
}

} }
