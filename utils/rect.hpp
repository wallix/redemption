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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Rect class : Copyright (C) Christophe Grosjean 2009

*/

#ifndef _REDEMPTION_UTILS_RECT_HPP_
#define _REDEMPTION_UTILS_RECT_HPP_

#include <algorithm>
#include <iosfwd>
#include <stdint.h>
#include "log.hpp"

struct Rect {
    int16_t x;
    int16_t y;
    uint16_t cx;
    uint16_t cy;

    struct RectIterator {
        virtual ~RectIterator() {}
        virtual void callback(const Rect & rect) = 0;
    };

    TODO("Right should be included inside rect (and undefined for empty rect)");
    uint16_t right() const {
        return static_cast<uint16_t>(this->x + this->cx);
    }

    TODO("Bottom should be included inside rect (and undefined for empty rect)");
    uint16_t bottom() const {
        return static_cast<uint16_t>(this->y + this->cy);
    }

    Rect() : x(0), y(0), cx(0), cy(0) {
    }

    Rect(int left, int top, uint16_t width, uint16_t height)
        : x(left), y(top), cx(width), cy(height)
    {
        // fast detection of overflow, works for valid width/height range 0..4096
        if (((width-1)|(height-1)) & 0x8000){
            this->x = 0;
            this->y = 0;
            this->cx = 0;
            this->cy = 0;
        }
    }

    bool contains_pt(int x, int y) const {
        return    x  >= this->x
               && y  >= this->y
               && x < (this->x + this->cx)
               && y < (this->y + this->cy);
    }

    // special cases: contains returns true
    // - if both rects are empty
    // - if inner rect is empty
    bool contains(const Rect & inner) const {
        return (inner.x >= this->x
             && inner.y >= this->y
             && inner.x + inner.cx <= this->x + this->cx
             && inner.y + inner.cy <= this->y + this->cy);
    }

    bool equal(const Rect & other) const {
        return (other.x == this->x
             && other.y == this->y
             && other.x + other.cx == this->x + this->cx
             && other.y + other.cy == this->y + this->cy);
    }

    bool operator==(const Rect &other) const {
        return this->equal(other);
    }

    // Rect constructor ensures that any empty rect will be (0, 0, 0, 0)
    // hence testing cx or cy is enough
    bool isempty() const {
        return this->cx == 0;
    }

    int getCenteredX() const {
        return this->x + (this->cx / 2);
    }

    int getCenteredY() const {
        return this->y + (this->cy / 2);
    }

    Rect wh() const {
        return Rect(0, 0, this->cx, this->cy);
    }

    // compute a new rect containing old rect and given point
    Rect enlarge_to(int x, int y) const {
        if (this->isempty()){
            return Rect(x, y, 1, 1);
        }
        else {
            const int x0 = std::min<int>(this->x, x);
            const int y0 = std::min<int>(this->y, y);
            const int x1 = std::max<int>(this->x + this->cx - 1, x);
            const int y1 = std::max<int>(this->y + this->cy - 1, y);
            return Rect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
        }
    }

    Rect offset(int dx, int dy) const {
        return Rect(this->x + dx, this->y + dy, this->cx, this->cy);
    }

    Rect shrink(uint16_t margin) const {
        return Rect(this->x + margin, this->y + margin,
                    static_cast<uint16_t>(this->cx - margin * 2),
                    static_cast<uint16_t>(this->cy - margin * 2));
    }

    Rect upper_side() const {
        return Rect(this->x, this->y, this->cx, 1);
    }

    Rect left_side() const {
        return Rect(this->x, this->y, 1, this->cy);
    }

    Rect lower_side() const {
        return Rect(this->x, this->y + this->cy - 1, this->cx, 1);
    }

    Rect right_side() const {
        return Rect(this->x + this->cx - 1, this->y, 1, this->cy);
    }

    Rect intersect(uint16_t width, uint16_t height) const
    {
        return this->intersect(Rect(0, 0, width, height));
    }

    Rect intersect(const Rect & in) const
    {
        int max_x = std::max(in.x, this->x);
        int max_y = std::max(in.y, this->y);
        int min_right = std::min<int>(in.x + in.cx, this->x + this->cx);
        int min_bottom = std::min<int>(in.y + in.cy, this->y + this->cy);

        return Rect(max_x, max_y, min_right - max_x, min_bottom - max_y);
    }

    bool has_intersection(const Rect & in) const
    {
        return (this->cx &&
            (std::min<int>(in.x + in.cx, this->x + this->cx) -
                 std::max(in.x, this->x) > 0)
        );
    }

    // Ensemblist difference
    void difference(const Rect & a, RectIterator & it) const
    {
        const Rect & intersect = this->intersect(a);

        if (!intersect.isempty()) {
            if (intersect.y  > this->y) {
                it.callback(Rect(this->x, this->y, this->cx,
                                 static_cast<uint16_t>(intersect.y - this->y)));
            }
            if (intersect.x > this->x) {
                it.callback(Rect(this->x, intersect.y,
                                 static_cast<uint16_t>(intersect.x - this->x), intersect.cy));
            }
            if (this->x + this->cx > intersect.x + intersect.cx) {
                it.callback(Rect(intersect.x + intersect.cx, intersect.y,
                                this->x + this->cx - (intersect.x + intersect.cx),
                                intersect.cy));
            }
            if (this->y + this->cy > intersect.y + intersect.cy) {
                it.callback(Rect(this->x, intersect.y + intersect.cy,
                                this->cx,
                                this->y + this->cy - (intersect.y + intersect.cy)));
            }
        }
        else {
            it.callback(*this);
        }
    }

    friend inline std::ostream & operator<<(std::ostream& os, const Rect &r) {
        return os << "(" << r.x << ", " << r.y << ", " << r.cx << ", " << r.cy << ")";
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

    enum {
        IN = 0x00,
        UP = 0x01,
        DOWN = 0x02,
        LEFT = 0x04,
        RIGHT = 0x08
    };
    // Region of a point outside rect
    // 0x00 means inside
    int region_pt(int x, int y) const {
        int res = 0;
        if (x < this->x) {
            res |= LEFT;
        }
        else if (x > this->x + this->cx) {
            res |= RIGHT;
        }
        if (y < this->y) {
            res |= UP;
        }
        else if (y > this->y + this->cy) {
            res |= DOWN;
        }
        return res;
    }

};
struct Point {
    int x;
    int y;

    Point(int x, int y)
        : x(x)
        , y(y)
    {}

};

struct Segment {
    Point a;
    Point b;

    Segment(const Point & a, const Point & b)
        : a(a)
        , b(b)
    {}
};

struct LineEquation {
    Segment seg;
    Segment segin;

    int dX;
    int dY;
    int c;

    LineEquation(int aX, int aY, int bX, int bY)
        : seg(Segment(Point(aX, aY), Point(bX, bY)))
        , segin(Segment(Point(0, 0), Point(0, 0)))
        , dX(aX - bX)
        , dY(aY - bY)
        , c(bY*aX - aY*bX)
    {
    }

    int compute_x(int y) {
        return (this->dX*y - this->c) / this->dY;
    }

    int compute_y(int x) {
        return (this->dY*x + this->c) / this->dX;
    }

    bool compute_intersection(const Rect & rect, int region, int & x, int & y) {
        int interX = 0;
        int interY = 0;
        bool found = false;
        if (region & Rect::LEFT) {
            int tmpy = this->compute_y(rect.x);
            if (tmpy >= rect.y && tmpy <= (rect.y + rect.cy)) {
                found = true;
                interX = rect.x;
                interY = tmpy;
            }
        }
        else if (region & Rect::RIGHT) {
            int tmpy = this->compute_y(rect.x + rect.cx);
            if (tmpy >= rect.y && tmpy <= (rect.y + rect.cy)) {
                found = true;
                interX = rect.x + rect.cx;
                interY = tmpy;
            }
        }
        if (region & Rect::UP) {
            int tmpx = this->compute_x(rect.y);
            if (tmpx >= rect.x && tmpx <= (rect.x + rect.cx)) {
                found = true;
                interX = tmpx;
                interY = rect.y;
            }
        }
        else if (region & Rect::DOWN) {
            int tmpx = this->compute_x(rect.y + rect.cy);
            if (tmpx >= rect.x && tmpx <= (rect.x + rect.cx)) {
                found = true;
                interX = tmpx;
                interY = rect.y + rect.cy;
            }
        }
        if (found) {
            x = interX;
            y = interY;
        }
        return found;
    }

    bool resolve(const Rect & rect) {
        int aPosition = rect.region_pt(this->seg.a.x, this->seg.a.y);
        int bPosition = rect.region_pt(this->seg.b.x, this->seg.b.y);

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

// helper class used to compute differences between two rectangles
class DeltaRect {
    public:
    int dleft;
    int dtop;
    int dheight;
    int dwidth;

    DeltaRect(const Rect & r1, const Rect & r2) {
        this->dtop = r1.y - r2.y;
        this->dleft = r1.x - r2.x;
        this->dheight = r1.cy - r2.cy;
        this->dwidth = r1.cx - r2.cx;
    }

    bool fully_relative(){
        return (abs(this->dleft) < 128)
            && (abs(this->dtop) < 128)
            && (abs(this->dwidth) < 128)
            && (abs(this->dheight) < 128)
            ;
    }
};


#endif
