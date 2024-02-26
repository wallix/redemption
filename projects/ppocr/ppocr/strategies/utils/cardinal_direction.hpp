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

#ifndef PPOCR_STRATEGIES_UTILS_CARDINAL_DIRECTION_HPP
#define PPOCR_STRATEGIES_UTILS_CARDINAL_DIRECTION_HPP

namespace ppocr { namespace strategies { namespace utils {

enum class CardinalDirection : unsigned
{
    NONE  = 2 + (2 << 2),
    SOUTH = 1 + (2 << 2),
    NORTH = 3 + (2 << 2),
    WEST  = 2 + (1 << 2),
    EAST  = 2 + (3 << 2),
    NORTH_EAST = 3 + (3 << 2),
    NORTH_WEST = 3 + (1 << 2),
    SOUTH_EAST = 1 + (3 << 2),
    SOUTH_WEST = 1 + (1 << 2),
};

enum class CardinalDirection2 : unsigned
{
    NONE   = 3 + (3 << 3),
    NORTH2 = 5 + (3 << 3),
    NORTH  = 4 + (3 << 3),
    SOUTH  = 2 + (3 << 3),
    SOUTH2 = 1 + (3 << 3),
    EAST2  = 3 + (5 << 3),
    EAST   = 3 + (4 << 3),
    WEST   = 3 + (2 << 3),
    WEST2  = 3 + (1 << 3),
    NORTH_EAST   = (NORTH  & 7) | (EAST  & (7<<3)),
    NORTH2_EAST  = (NORTH2 & 7) | (EAST  & (7<<3)),
    NORTH_EAST2  = (NORTH  & 7) | (EAST2 & (7<<3)),
    NORTH2_EAST2 = (NORTH2 & 7) | (EAST2 & (7<<3)),
    NORTH_WEST   = (NORTH  & 7) | (WEST  & (7<<3)),
    NORTH2_WEST  = (NORTH2 & 7) | (WEST  & (7<<3)),
    NORTH_WEST2  = (NORTH  & 7) | (WEST2 & (7<<3)),
    NORTH2_WEST2 = (NORTH2 & 7) | (WEST2 & (7<<3)),
    SOUTH_EAST   = (SOUTH  & 7) | (EAST  & (7<<3)),
    SOUTH2_EAST  = (SOUTH2 & 7) | (EAST  & (7<<3)),
    SOUTH_EAST2  = (SOUTH  & 7) | (EAST2 & (7<<3)),
    SOUTH2_EAST2 = (SOUTH2 & 7) | (EAST2 & (7<<3)),
    SOUTH_WEST   = (SOUTH  & 7) | (WEST  & (7<<3)),
    SOUTH2_WEST  = (SOUTH2 & 7) | (WEST  & (7<<3)),
    SOUTH_WEST2  = (SOUTH  & 7) | (WEST2 & (7<<3)),
    SOUTH2_WEST2 = (SOUTH2 & 7) | (WEST2 & (7<<3)),
};

} } }

#endif
