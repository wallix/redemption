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

#ifndef PPOCR_SRC_STRATEGIES_PROPORTIONALITY_ZONE_HPP
#define PPOCR_SRC_STRATEGIES_PROPORTIONALITY_ZONE_HPP

#include "ppocr/strategies/utils/context.hpp"
#include "ppocr/strategies/utils/count_zone.hpp"

#include <vector>

namespace ppocr {

class Image;

namespace strategies {

struct proportionality_zone
{
    struct relationship_type
    {
        using value_type = std::vector<unsigned>;
        using result_type = unsigned;

        static result_type compute(value_type const & a, value_type const & b);

        /// \return [0, 1]
        static double dist(value_type const & a, value_type const & b);

        static bool in_dist(value_type const & a, value_type const & b, unsigned d);

        static unsigned count();
    };
    using value_type = relationship_type::value_type;

    using ctx_type = cache_context<utils::count_zone_fn>;

    static value_type load(Image const & img, Image const & img90, ctx_type& ctx);
};

} }

#endif
