/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include <cstddef>


// A multi-sz contains a sequence of null-terminated strings,
//  terminated by an empty string (\0) so that the last two
//  characters are both null terminators.
void SOHSeparatedStringsToMultiSZ(char * dest, std::size_t dest_size, const char * src);

// A multi-sz contains a sequence of null-terminated strings,
//  terminated by an empty string (\0) so that the last two
//  characters are both null terminators.
void MultiSZCopy(char * dest, std::size_t dest_size, const char * src);
