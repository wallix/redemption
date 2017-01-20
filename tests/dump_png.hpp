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

#include <cstdio>
#include <cstdlib>

#include "utils/drawable.hpp"
#include "utils/png.hpp"


inline void dump_png(const char * filename, const Drawable & drawable)
{
    if (FILE * f = fopen(filename, "wb")) {
        ::dump_png24(f, drawable.data(), drawable.width(), drawable.height(), drawable.rowsize(), true);
        ::fclose(f);
    }
}

inline void dump_png(const char * filename, const Bitmap & bmp)
{
    if (FILE * f = fopen(filename, "wb")) {
        dump_png24(f, bmp.data(), bmp.cx(), bmp.cy(), bmp.line_size(), true);
        ::fclose(f);
    }
}
