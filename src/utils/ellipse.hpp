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


#pragma once

#include <cstddef>
#include "utils/rect.hpp"

class Ellipse {
    int16_t  leftRect;
    int16_t  topRect;
    int16_t  rightRect;
    int16_t  bottomRect;

public:
    Ellipse()
    : Ellipse(0, 0, 0, 0)
    {}

    Ellipse(int16_t leftRect, int16_t topRect, int16_t rightRect, int16_t bottomRect)
    : leftRect(leftRect)
    , topRect(topRect)
    , rightRect(rightRect)
    , bottomRect(bottomRect)
    {}

    static Ellipse since_center(int centerx, int centery, uint16_t radiusx, uint16_t radiusy) {
        return Ellipse(centerx - radiusx, centery - radiusy, centerx + radiusx, centery + radiusy);
    }

    explicit Ellipse(const Rect r)
    : Ellipse(r.x, r.y, r.eright(), r.ebottom())
    {}

    [[nodiscard]] int16_t ileft() const {
        return this->leftRect;
    }
    [[nodiscard]] int16_t itop() const {
        return this->topRect;
    }
    [[nodiscard]] int16_t eright() const {
        return this->rightRect;
    }
    [[nodiscard]] int16_t ebottom() const {
        return this->bottomRect;
    }

    [[nodiscard]] uint16_t width() const {
        return static_cast<uint16_t>(this->eright() - this->ileft());
    }
    [[nodiscard]] uint16_t height() const {
        return static_cast<uint16_t>(this->ebottom() - this->itop());
    }

    [[nodiscard]] uint16_t radius_x() const {
        return this->width() / 2;
    }
    [[nodiscard]] uint16_t radius_y() const {
        return this->height() / 2;
    }

    [[nodiscard]] int16_t center_x() const {
        return this->ileft() + (this->eright() - this->ileft()) / 2;
    }
    [[nodiscard]] int16_t center_y() const {
        return this->itop() + (this->ebottom() - this->itop()) / 2;
    }

    [[nodiscard]] Rect get_rect() const {
        return Rect(this->ileft(), this->itop(), this->width(), this->height());
    }

    bool operator == (const Ellipse & other) const noexcept {
        return (other.leftRect == this->leftRect &&
                other.topRect == this->topRect &&
                other.rightRect == this->rightRect &&
                other.bottomRect == this->bottomRect);
    }
};

