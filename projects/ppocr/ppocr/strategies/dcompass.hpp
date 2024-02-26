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

#ifndef PPOCR_STRATEGIES_DCOMPASS_HPP
#define PPOCR_STRATEGIES_DCOMPASS_HPP

#include <iosfwd>

#include "ppocr/strategies/utils/cardinal_direction.hpp"

namespace ppocr {

class Image;

namespace strategies
{
    struct dcompass
    {
        using cardinal_direction = utils::CardinalDirection;

        dcompass() = default;

        dcompass(cardinal_direction d) : d(d) {}

        dcompass(const Image & img, const Image & img90);

        bool operator<(dcompass const & other) const
        { return d < other.d; }

        bool operator==(dcompass const & other) const
        { return d == other.d; }

        unsigned relationship(const dcompass& other) const;

        cardinal_direction direction() const noexcept { return d; }

        friend std::istream & operator>>(std::istream &, dcompass &);

    private:
        cardinal_direction d = cardinal_direction::NONE;
    };

    std::ostream & operator<<(std::ostream &, dcompass const &);
}

}

#endif
