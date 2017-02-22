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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#pragma once

#include "mln/image/image2d.hh"

namespace ocr
{
    template<class ImageValue>
    class Image2dView
    {
        mln::image2d<ImageValue> const & input;

    public:
        typedef ImageValue value_type;

        Image2dView(mln::image2d<ImageValue> const & input_)
        : input(input_)
        {}

        unsigned width() const
        { return input.ncols(); }

        unsigned height() const
        { return input.nrows(); }

        const ImageValue & operator()(unsigned row, unsigned col) const
        { return input.at(row, col); }

        const ImageValue & operator[](const ::mln::point2d & p) const
        { return input(p); }
    };
}
