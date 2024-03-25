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

#ifndef PPOCR_SRC_STRATEGIES_ZONE_HPP
#define PPOCR_SRC_STRATEGIES_ZONE_HPP

#include <array>

#include "ppocr/strategies/relationship/array_compare.hpp"
#include "ppocr/strategies/utils/count_zone.hpp"
#include "ppocr/strategies/utils/context.hpp"

namespace ppocr {

class Image;

namespace strategies {

struct zone {
    enum {
        top_left_is_letter,
        bottom_right_is_letter,
        number_top_alternations,
        number_right_alternations,
        number_bottom_alternations,
        number_left_alternations,
        number_internal_alternations,
        number_index
    };

    using relationship_type = array_compare_relationship<unsigned, number_index>;
    using value_type = relationship_type::value_type;

    using ctx_type = cache_context<utils::count_zone_fn>;

    static value_type load(const Image & img, const Image & img90, ctx_type& ctx);
};

} }

#endif
