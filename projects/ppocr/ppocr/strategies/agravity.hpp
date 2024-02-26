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

#ifndef PPOCR_STRATEGIES_AGRAVITY_HPP
#define PPOCR_STRATEGIES_AGRAVITY_HPP

#include <iosfwd>

namespace ppocr {

class Image;

namespace strategies
{
    struct agravity
    {
        static constexpr double null_angle() { return 100000.; }

        agravity() = default;
        agravity(double a) : a(a) {}

        agravity(const Image & img, const Image & img90);

        bool operator<(agravity const & other) const;

        bool operator==(agravity const & other) const;

        unsigned relationship(const agravity& other) const;

        double angle() const noexcept { return a; }

        friend std::istream & operator>>(std::istream &, agravity &);

    private:
        double a = null_angle();
    };

    std::ostream & operator<<(std::ostream &, agravity const &);
}

}

#endif
