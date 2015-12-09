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

#ifndef REDEMPTION_GDI_DRAWABLE_GD_HPP
#define REDEMPTION_GDI_DRAWABLE_GD_HPP

#include "drawable.hpp"
#include "proxy_gd.hpp"

namespace gdi {

struct DrawableGd : ProxyGD<ProxyDraw<Drawable>>
{
    using Drawable::TypeId;
    using Drawable::GraphicDispatcher;

    DrawableGd() = default;

    TypeId add_gdi(GraphicDevicePtr && pgdi) {
        return this->drawable_.add_gdi(std::move(pgdi));
    }

    template<class FilterClass>
    TypeId add_filter(FilterClass && filter) {
        return this->drawable_.add_filter(std::forward<FilterClass>(filter));
    }

private:
    Drawable drawable_;
};

}

#endif
