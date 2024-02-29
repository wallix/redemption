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

#ifndef PPOCR_SRC_STRATEGIES_RELATIONSHIP_INTERVAL_RELATIONSHIP_HPP
#define PPOCR_SRC_STRATEGIES_RELATIONSHIP_INTERVAL_RELATIONSHIP_HPP

#include "ppocr/strategies/utils/relationship.hpp"

namespace ppocr { namespace strategies {

template<class T, class R = T>
struct interval_relationship
{
    using value_type = T;
    using result_type = R;

    constexpr static bool is_contiguous = true;

    constexpr interval_relationship(T const & interval) noexcept
    : interval_(interval)
    {}

    result_type operator()(value_type const & a, value_type const & b) const
    { return utils::compute_relationship(a, b, interval_); }

    /// \return [0, 1]
    double dist(value_type const & a, value_type const & b) const
    { return static_cast<double>(operator()(a, b)) / 100.; }

    bool in_dist(value_type const & a, value_type const & b, value_type const & d) const
    { return (a < b ? (b > a + d) : (a > b + d)); }

    unsigned count() const { return static_cast<unsigned>(this->interval_) + 1u; }

private:
    value_type interval_;
};

} }

#endif
