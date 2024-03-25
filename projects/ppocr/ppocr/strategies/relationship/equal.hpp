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

#ifndef PPOCR_SRC_STRATEGIES_RELATIONSHIP_EQUAL_RELATIONSHIP_HPP
#define PPOCR_SRC_STRATEGIES_RELATIONSHIP_EQUAL_RELATIONSHIP_HPP

#include <cstddef>

namespace ppocr { namespace strategies {

template<class T>
struct equal_relationship
{
    using value_type = T;
    using result_type = bool;

    constexpr static bool is_contiguous = true;

    static result_type compute(value_type const & a, value_type const & b)
    {
        return a == b;
    }

    /// \return [0, 1]
    static double dist(value_type const & a, value_type const & b)
    {
        return a == b ? 1. : 0.;
    }

    static bool in_dist(value_type const & a, value_type const & b, unsigned)
    {
        return a == b;
    }

    static unsigned count()
    {
        return 2;
    }
};

} }

#endif
