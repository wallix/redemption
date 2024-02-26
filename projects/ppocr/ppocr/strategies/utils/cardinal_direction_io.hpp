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

#ifndef PPOCR_STRATEGIES_UTILS_CARDINAL_DIRECTION_IO_HPP
#define PPOCR_STRATEGIES_UTILS_CARDINAL_DIRECTION_IO_HPP

#include <ostream>
#include <istream>
#include <type_traits>

#include "ppocr/strategies/utils/cardinal_direction.hpp"


namespace ppocr { namespace strategies { namespace utils {

inline std::ostream & operator<<(std::ostream & os, CardinalDirection d)
{ return os << static_cast<std::underlying_type<decltype(d)>::type>(d); }

inline std::istream & operator>>(std::istream & is, CardinalDirection & d)
{
    std::underlying_type<CardinalDirection>::type i;
    is >> i;
    d = static_cast<CardinalDirection>(i);
    return is;
}

inline std::ostream & operator<<(std::ostream & os, CardinalDirection2 d)
{ return os << static_cast<std::underlying_type<decltype(d)>::type>(d); }

inline std::istream & operator>>(std::istream & is, CardinalDirection2 & d)
{
    std::underlying_type<CardinalDirection2>::type i;
    is >> i;
    d = static_cast<CardinalDirection2>(i);
    return is;
}

} } }


#endif
