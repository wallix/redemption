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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/rect.hpp"


struct LineEquation {
    struct Point {
        int x;
        int y;
    };


    struct Segment {
        Point a;
        Point b;
    };

    Segment seg;
    Segment segin;

    int dX;
    int dY;
    int c;

    LineEquation(int aX, int aY, int bX, int bY)
        : seg{Segment{Point{aX, aY}, Point{bX, bY}}}
        , segin{Segment{Point{0, 0}, Point{0, 0}}}
        , dX(aX - bX)
        , dY(aY - bY)
        , c(bY*aX - aY*bX)
    {
    }


    //             |                         |
    //             |                         |
    //             |                         |
    //    UP       |         UP              |      UP
    //    LEFT     |                         |      RIGHT
    //             |                         |
    //-------------+-------------------------+--------------------
    //             |/ / / / / / / / / / / / /|
    //             | / / / / / / / / / / / / |
    //             |/ / / / / / / / / / / / /|
    //    LEFT     | / / / /   IN  / / / / / |     RIGHT
    //             |/ / / / / Rect  / / / / /|
    //             | / / / / / / / / / / / / |
    //             |/ / / / / / / / / / / / /|
    //-------------+-------------------------+-------------------
    //             |                         |
    //    DOWN     |         DOWN            |     DOWN
    //    LEFT     |                         |     RIGHT
    //             |                         |
    //             |                         |

    enum  t_region {
        IN = 0x00,
        UP = 0x01,
        DOWN = 0x02,
        LEFT = 0x04,
        RIGHT = 0x08
    };
    // Region of a point outside rect
    // 0x00 means inside
    static t_region region_pt(Rect const& rect, int16_t x, int16_t y)
    {
        int res = IN;
        if (x < rect.x) {
            res |= LEFT;
        }
        else if (x >= rect.eright()) {
            res |= RIGHT;
        }
        if (y < rect.y) {
            res |= UP;
        }
        else if (y >= rect.ebottom()) {
            res |= DOWN;
        }
        return static_cast<t_region>(res);
    }

    [[nodiscard]] int compute_x(int y) const {
        return (this->dX*y - this->c) / this->dY;
    }

    [[nodiscard]] int compute_y(int x) const {
        return (this->dY*x + this->c) / this->dX;
    }

    bool compute_intersection(Rect rect, int region, int & x, int & y) const {
        int interX = 0;
        int interY = 0;
        bool found = false;

        if (region & LEFT) {
            int tmpy = this->compute_y(rect.x);
            if (tmpy >= rect.y && tmpy < rect.ebottom()) {
                found = true;
                interX = rect.x;
                interY = tmpy;
            }
        }
        else if (region & RIGHT) {
            int tmpy = this->compute_y(rect.eright() - 1);
            if (tmpy >= rect.y && tmpy < rect.ebottom()) {
                found = true;
                interX = rect.eright() - 1;
                interY = tmpy;
            }
        }

        if (region & UP) {
            int tmpx = this->compute_x(rect.y);
            if (tmpx >= rect.x && tmpx < rect.eright()) {
                found = true;
                interX = tmpx;
                interY = rect.y;
            }
        }
        else if (region & DOWN) {
            int tmpx = this->compute_x(rect.ebottom() - 1);
            if (tmpx >= rect.x && tmpx < rect.eright()) {
                found = true;
                interX = tmpx;
                interY = rect.ebottom() - 1;
            }
        }

        if (found) {
            x = interX;
            y = interY;
        }

        return found;
    }

    bool resolve(Rect rect) {
        t_region aPosition = region_pt(rect, int16_t(this->seg.a.x), int16_t(this->seg.a.y));
        t_region bPosition = region_pt(rect, int16_t(this->seg.b.x), int16_t(this->seg.b.y));

        if (aPosition & bPosition) {
            return false;
        }
        bool exist = true;
        if (!aPosition) {
            this->segin.a.x = this->seg.a.x;
            this->segin.a.y = this->seg.a.y;
        }
        else {
            exist &= this->compute_intersection(rect, aPosition, this->segin.a.x, this->segin.a.y);
        }

        if (!bPosition) {
            this->segin.b.x = this->seg.b.x;
            this->segin.b.y = this->seg.b.y;
        }
        else {
            exist &= this->compute_intersection(rect, bPosition, this->segin.b.x, this->segin.b.y);
        }
        return exist;
    }
};
