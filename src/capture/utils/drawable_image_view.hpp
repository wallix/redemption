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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/drawable.hpp"
#include "mln/core/point.hh"

class DrawableImageView
{
    const Drawable & drawable;
    unsigned w;
    unsigned h;

public:
    DrawableImageView(const Drawable & drawable_)
    : drawable(drawable_)
    , w(drawable_.width())
    , h(drawable_.height() > 50 ? drawable_.height() - 50 : 0)
    {}

    unsigned width() const
    { return this->w; }

    unsigned height() const
    { return this->h; }

    struct Color
    {
        const uint8_t * c;

        uint8_t blue()  const { return c[0]; }
        uint8_t green() const { return c[1]; }
        uint8_t red()   const { return c[2]; }
    };

    typedef Color value_type;

    Color operator()(unsigned row, unsigned col) const
    { return {this->drawable.data(col, row)}; }

    Color operator[](::mln::point2d p) const
    { return {this->drawable.data(p.col(), p.row())}; }
};
