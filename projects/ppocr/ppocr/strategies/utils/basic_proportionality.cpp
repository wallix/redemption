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

#include "ppocr/strategies/utils/basic_proportionality.hpp"
#include "ppocr/strategies/utils/relationship.hpp"
#include <cassert>

#include <ostream>
#include <istream>


namespace ppocr { namespace strategies {

void details_::check_interval(unsigned x, unsigned interval)
{
    assert(x <= interval);
    (void)x;
    (void)interval;
}

unsigned proportionality_base::relationship(const proportionality_base& other, unsigned interval) const
{ return utils::compute_relationship(value(), other.value(), interval); }

std::istream& operator>>(std::istream& is, proportionality_base & prop)
{
    decltype(prop.value()) proportion;
    is >> proportion;
    prop = proportionality_base(proportion);
    return is;
}

std::ostream& operator<<(std::ostream& os, const proportionality_base & prop)
{ return os << prop.value(); }

} }
