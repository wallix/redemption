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

#ifndef PPOCR_STRATEGIES_DIRECTION_HPP
#define PPOCR_STRATEGIES_DIRECTION_HPP

#include <iosfwd>

#include "ppocr/strategies/utils/cardinal_direction.hpp"

namespace ppocr {

class Image;

namespace strategies
{
    struct direction
    {
        using cardinal_direction = utils::CardinalDirection2;

        direction() = default;

        direction(const Image & img, const Image & img90);

        direction(cardinal_direction direction) : d(direction) {}

        bool operator<(direction const & other) const
        { return d < other.d; }

        bool operator==(direction const & other) const
        { return d == other.d; }

        unsigned relationship(const direction& other) const;

        cardinal_direction id() const noexcept { return d; }

        friend std::istream & operator>>(std::istream &, direction &);

    private:
        cardinal_direction d = cardinal_direction::NONE;
    };

    std::ostream & operator<<(std::ostream &, direction const &);
}

}

#endif
