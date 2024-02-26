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

#ifndef PPOCR_CAPTURE_RDP_PPOCR_CACHE_HPP
#define PPOCR_CAPTURE_RDP_PPOCR_CACHE_HPP

#include "ppocr/ocr2/glyphs.hpp"

#include "ppocr/image/image.hpp"
#include "ppocr/utils/image_compare.hpp"

#include <map>
#include <vector>
#include <functional>

namespace ppocr { namespace ocr2 {

using view_ref = std::reference_wrapper<View const>;
using view_ref_list = std::vector<view_ref>;

struct def_img_compare {
    def_img_compare() {}
    bool operator()(ppocr::Image const & a, ppocr::Image const & b) const
    { return ppocr::image_compare(a, b) < 0; }
};
using image_cache_type_t = std::map<ppocr::Image, view_ref_list, def_img_compare>;

} }

#endif
