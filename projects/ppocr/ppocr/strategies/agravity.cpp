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

#include "ppocr/strategies/agravity.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/math/almost_equal.hpp"
#include "ppocr/strategies/utils/relationship.hpp"
#include "ppocr/strategies/utils/horizontal_gravity.hpp"

#include <ostream>
#include <istream>
#include <iomanip>

#include <cfloat>
#include <cmath>


namespace ppocr { namespace strategies {

agravity::agravity(const Image& img, const Image& img90)
{
    utils::TopBottom const g1 = utils::horizontal_gravity(img);
    utils::TopBottom const g2 = utils::horizontal_gravity(img90);

    using Signed = long;

    auto const h1 = Signed(g1.top) - Signed(g1.bottom);
    auto const h2 = Signed(g2.top) - Signed(g2.bottom);

    if (h1 || h2) {
        this->a = std::asin(double(h1) / std::sqrt(h1*h1+h2*h2));
    }
}

bool agravity::operator==(const agravity& other) const
{ return almost_equal(this->a, other.a, 2); }

bool agravity::operator<(agravity const & other) const
{ return a < other.a && !(*this == other); }

unsigned agravity::relationship(const agravity& other) const
{
    // disable -Wfloat-equal
    std::equal_to<double> eq;
    if (eq(angle(), null_angle()) || eq(other.angle(), null_angle())) {
        return eq(other.angle(), angle()) ? 100 : 0;
    }
    return utils::compute_relationship(angle(), other.angle(), M_PI);
}

std::istream& operator>>(std::istream& is, agravity& ag)
{ return is >> ag.a; }

std::ostream& operator<<(std::ostream& os, const agravity& ag)
{
    auto const flags = os.setf(std::ios_base::fixed | std::ios_base::scientific, std::ios_base::floatfield);
    auto const precision = os.precision(std::numeric_limits<double>::max_exponent10);
    os << ag.angle();
    os.precision(precision);
    os.setf(flags, std::ios_base::floatfield);
    return os;
}

} }
