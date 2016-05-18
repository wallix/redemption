/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/array_view.hpp"

#include <cstring>

template<class T>
array_const_char type_name(T const * = nullptr)
#ifdef __clang__
{
  char const * s = __PRETTY_FUNCTION__;
  return {s + 43, s + strlen(s) - 1};
}
#elif defined(__GNUG__)
{
  char const * s = __PRETTY_FUNCTION__;
  return {s + 47, s + strlen(s) - 44};
}
#else
;
#endif
