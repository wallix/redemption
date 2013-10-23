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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
              Meng Tan

*/

#ifndef _REDEMPTION_UTILS_ELLIPSE_HPP_
#define _REDEMPTION_UTILS_ELLIPSE_HPP_

#include <algorithm>
#include <iosfwd>
#include <stdint.h>
#include "rect.hpp"
#include "log.hpp"


#define SQ(X) X*X

struct Ellipse {
    int16_t centerx;
    int16_t centery;
    uint16_t radiusx;
    uint16_t radiusy;

    Ellipse() : centerx(0), centery(0), radiusx(0), radiusy(0) {}

    Ellipse(int centerx, int centery, uint16_t radiusx, uint16_t radiusy)
        : centerx(centerx), centery(centery), radiusx(radiusx), radiusy(radiusy)
    {
        if (((radiusx-1)|(radiusy-1)) & 0x8000) {
            this->centerx = 0;
            this->centery = 0;
            this->radiusx = 0;
            this->radiusy = 0;
        }
    }

    Ellipse(const Rect & rect)
        : centerx(rect.getCenteredX())
        , centery(rect.getCenteredY())
        , radiusx(rect.cx / 2)
        , radiusy(rect.cy / 2)
    {}

    int16_t left() const {
        return this->centerx - this->radiusx;
    }
    int16_t top() const {
        return this->centery - this->radiusy;
    }
    int16_t right() const {
        return this->centerx + this->radiusx;
    }
    int16_t bottom() const {
        return this->centery + this->radiusy;
    }

    uint16_t width() const {
        return this->radiusx * 2;
    }
    uint16_t height() const {
        return this->radiusy * 2;
    }

    Rect get_rect() const {
        return Rect(this->left(), this->top(), this->width(), this->height());
    }

    bool contains_pt(int x, int y) const {
        if (this->radiusy == 0 || this->radiusy == 0)
            return false;
        return (SQ((x - this->centerx) / this->radiusx) +
                SQ((y - this->centery) / this->radiusy)) <= 1;
    }
    bool equal(const Ellipse & other) const {
        return (other.centerx == this->centerx &&
                other.centery == this->centery &&
                other.radiusx == this->radiusx &&
                other.radiusy == this->radiusy);
    }

};

#endif
