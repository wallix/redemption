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

#ifndef REDEMPTION_UTILS_APPLY_FOR_DELIM_HPP
#define REDEMPTION_UTILS_APPLY_FOR_DELIM_HPP

namespace detail {
  struct ignore_blank_fn
  {
    bool operator()(char c) const noexcept
    { return c == ' ' || c == '\t'; }
  };
}

template<class Fn, class IgnoreFn = detail::ignore_blank_fn>
void apply_for_delim(const char * cstr, char delim, Fn fn, IgnoreFn ignore = IgnoreFn())
{
  while (*cstr) {
    while (ignore(*cstr)) {
      ++cstr;
    }

    fn(cstr);

    while (*cstr && *cstr != delim) {
      ++cstr;
    }
    if (*cstr) {
      ++cstr;
    }
  }
}

#endif
