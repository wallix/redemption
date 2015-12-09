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

#ifndef REDEMPTION_GDI_PROXY_GD_HPP
#define REDEMPTION_GDI_PROXY_GD_HPP

#include "graphic_device.hpp"
#include "meta/meta.hpp"

namespace gdi {

template<class Proxy>
struct ProxyGD : GraphicDevice
{
    template<class... ProxyArgs>
    ProxyGD(ProxyArgs && ... args)
    : proxy_(std::forward<ProxyArgs>(args)...)
    {}

    void draw(RDPOpaqueRect const & cmd, Rect const & clip) {
        this->proxy_(cmd, clip);
    }

private:
    Proxy proxy_;
};


namespace detail_
{
    template<class T, class = void>
    struct gd_draw_wrapper
    { using type = T; };

    template<class Ptr>
    struct gd_draw_wrapper<Ptr, meta::void_t<typename std::pointer_traits<Ptr>::element_type>>
    {
        struct type {
            template<class Cmd>
            void draw(Cmd const & cmd, Rect const & clip) {
                this->p->draw(cmd, clip);
            }
            Ptr p;
        };
    };
}

template<class Gd>
struct ProxyDraw
{
    template<class... GdArgs>
    ProxyDraw(GdArgs && ... args)
    : gd_(std::forward<GdArgs>(args)...)
    {}

    template<class Cmd>
    void draw(Cmd const & cmd, Rect const & clip) {
        this->gd_.draw(cmd, clip);
    }

private:
    typename detail_::gd_draw_wrapper<Gd>::type gd_;
};

}

#endif
