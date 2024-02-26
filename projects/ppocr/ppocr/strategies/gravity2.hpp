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

#ifndef PPOCR_STRATEGIES_GRAVITY2_HPP
#define PPOCR_STRATEGIES_GRAVITY2_HPP

#include <iosfwd>

namespace ppocr {

class Image;

namespace strategies
{
    struct gravity2
    {
        gravity2() = default;

        gravity2(const Image & img, const Image & img90);

        gravity2(int d) : d(d) {}

        bool operator<(gravity2 const & other) const
        { return d < other.d; }

        bool operator==(gravity2 const & other) const
        { return d == other.d; }

        unsigned relationship(const gravity2& other) const;

        unsigned id() const noexcept { return d; }

        friend std::istream & operator>>(std::istream &, gravity2 &);

    private:
        unsigned d = 0;
    };

    std::ostream & operator<<(std::ostream &, gravity2 const &);
}

}

#endif
