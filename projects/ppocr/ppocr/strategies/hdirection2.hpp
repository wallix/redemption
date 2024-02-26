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

#ifndef PPOCR_SRC_STRATEGIES_HDIRECTION2_HPP
#define PPOCR_SRC_STRATEGIES_HDIRECTION2_HPP

#include "ppocr/strategies/relationship/interval.hpp"

namespace ppocr {

class Image;

namespace strategies {

struct hdirection2
{
    using value_type = unsigned;
    using relationship_type = interval_relationship<value_type>;

    static constexpr bool one_axis = true;

    value_type load(Image const & img, Image const & /*img90*/) const;

    relationship_type relationship() const;
    unsigned best_difference() const;
};

} }

#endif
