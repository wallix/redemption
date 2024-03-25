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

#ifndef PPOCR_SRC_STRATEGIES_RELATIONSHIP_ARRAY_COMPARE_RELATIONSHIP_HPP
#define PPOCR_SRC_STRATEGIES_RELATIONSHIP_ARRAY_COMPARE_RELATIONSHIP_HPP

#include <array>

namespace ppocr { namespace strategies {

template<class T, std::size_t N>
struct array_compare_relationship
{
    using value_type = std::array<T, N>;
    using result_type = unsigned;

    static result_type compute(value_type const & a, value_type const & b)
    {
        result_type n{};
        auto it = std::begin(a);
        for (auto const & i : b) {
            if (*it == i) {
                ++n;
            }
            ++it;
        }
        return result_type(n * result_type{100} / a.size());
    }

    /// \return [0, 1]
    static double dist(value_type const & a, value_type const & b)
    {
        return static_cast<double>(compute(a, b)) / 100.;
    }

    static bool in_dist(value_type const & a, value_type const & b, unsigned d)
    {
        return compute(a, b) >= d;
    }

    static unsigned count()
    {
        return 101;
    }
};

} }

#endif
