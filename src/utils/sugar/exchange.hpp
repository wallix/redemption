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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#pragma once

#if __cplusplus > 201103L
#include <utility>
using std::exchange;
#else
#include <algorithm>
/// Assign @p new_val to @p obj and return its previous value.
template <typename T, typename U = T>
T exchange(T & obj, U && new_val)
{
  T old_val = std::move(obj);
  obj = std::forward<U>(new_val);
  return old_val;
}
#endif

