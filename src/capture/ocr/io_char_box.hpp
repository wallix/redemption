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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#pragma once

#include <vector>
#include <ostream>
#include <iomanip>

#include <mln/image/image2d.hh>

#include "labelize.hh"

inline void display_char_box(
    std::ostream & os, const mln::image2d<bool> & ima,
    const std::vector<ocr::label_attr_t> & attrs)
{
    using iterator = std::vector<ocr::label_attr_t>::const_iterator;
    iterator first = attrs.begin();
    iterator last = attrs.end();
    for (; first != last; ++first){
        const unsigned maxcol = first->bbox.pmax().col();
        const unsigned maxrow = first->bbox.pmax().row();
        const unsigned mincol = first->bbox.pmin().col();
        const unsigned minrow = first->bbox.pmin().row();
        os << std::setw(maxcol-mincol + 4) << std::setfill('X') << "\n";
        for (unsigned row = minrow; row <= maxrow; ++row) {
            os << "X";
            for (unsigned col = mincol; col <= maxcol; ++col) {
                os << (ima.at(row, col) ? '-' : 'X');
            }
            os << "X\n";
        }
        os << std::setw(maxcol-mincol + 5) << std::setfill('X') << "\n\n";
    }
}
