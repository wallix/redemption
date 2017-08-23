/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file, region management
*/

#pragma once

#include "utils/rect.hpp"

#include <vector>

struct SubRegion
{
    std::vector<Rect> rects;

    void subtract_rect(const Rect rect)
    {
        std::vector<Rect> new_rects;

        std::size_t count = this->rects.size();
        for (std::size_t i = 0; i < count; i++) {
            Rect rect1 = this->rects[i];
            if (!rect.contains(rect1)) {
                const Rect sect = rect.intersect(rect1);

                if (sect.isempty()) {
                    new_rects.push_back(rect1);
                }
                else {
                    const Rect a(rect1.x, rect1.y, rect1.cx, sect.y - rect1.y);
                    const Rect b(rect1.x, sect.y, sect.x - rect1.x, sect.cy);
                    const Rect c(sect.right(), sect.y, rect1.right() - sect.right(), sect.cy);
                    const Rect d(rect1.x, sect.bottom(), rect1.cx, rect1.bottom() - sect.bottom());
                    if (!a.isempty()) {
                        new_rects.push_back(a);
                    }
                    if (!b.isempty()) {
                        new_rects.push_back(b);
                    }
                    if (!c.isempty()) {
                        new_rects.push_back(c);
                    }
                    if (!d.isempty()) {
                        new_rects.push_back(d);
                    }
                }
            }
        }
        this->rects = std::move(new_rects);
    }

    void add_rect(const Rect & rect)
    {
        this->rects.push_back(rect);
    }
};
