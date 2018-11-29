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
Copyright (C) Wallix 2010
Author(s): Christophe Grosjean, Jonathan Poelen
*/

#pragma once

#include "utils/sugar/bytes_view.hpp"

#include <cstddef>

// hexdump for humans
void hexdump(const_byte_ptr data, size_t size, unsigned line_length = 16);
void hexdump_av(const_bytes_view data, unsigned line_length = 16);

/**
 *  hexdump for c++ integer array.
 *  0x23, 0x53 .....
 */
void hexdump_d(const_byte_ptr data, size_t size, unsigned line_length = 16);
void hexdump_av_d(const_bytes_view data, unsigned line_length = 16);

// also available for 16 bits items arrays, size in bytes and must be even
void hexdump16_d(const uint16_t * data, size_t size);


/**
 *  hexdump for c++ raw string.
 *  "\\x23\\x53"
 */
void hexdump_c(const_byte_ptr data, size_t size, unsigned line_length = 16);
void hexdump_av_c(const_bytes_view data, unsigned line_length = 16);
