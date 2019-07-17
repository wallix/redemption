/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#pragma once

#include <cstddef>
#include <cstdint>

#include "utils/sugar/bytes_view.hpp"

/**
 * \pre  \a out.size() >= \a modulus.size()
 * \return  the length of the big-endian number placed at out. ~size_t{} if error
 */
bytes_view mod_exp_direct(
    bytes_view out,
    const_bytes_view inr,
    const_bytes_view modulus,
    const_bytes_view exponent
);
