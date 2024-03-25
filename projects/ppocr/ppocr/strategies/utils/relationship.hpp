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

#ifndef PPOCR_UTILS_RELATIONSHIP_HPP
#define PPOCR_UTILS_RELATIONSHIP_HPP

#include <type_traits>

#include <cassert>


namespace ppocr { namespace strategies { namespace utils {

template<class T, class U>
unsigned compute_relationship(T const & a, T const & b, U const & interval)
{
    using integer = typename std::conditional<std::is_signed<T>::value, int, unsigned>::type;
    auto d = integer{100} - ((a < b) ? b-a : a-b) * integer{100} / interval;
    assert(0 <= d && d <= 100);
    return unsigned(d);
}

} } }

#endif
