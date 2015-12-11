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

#include <type_traits>


namespace gdi {

template<class Proxy, class InterfaceBase = GraphicDevice>
struct ProxyGD : InterfaceBase
{
    static_assert(std::is_base_of<GraphicDevice, InterfaceBase>::value, "InterfaceBase isn't a GraphicDevice");

    template<class... ProxyArgs>
    ProxyGD(ProxyArgs && ... args)
    : proxy_{std::forward<ProxyArgs>(args)...}
    {}

    template<class... Ts>
    ProxyGD(Proxy && proxy, Ts && ... args)
    : InterfaceBase{std::forward<Ts>(args)...}
    , proxy_(std::move(proxy))
    {}

    template<class... Ts>
    ProxyGD(Proxy const & proxy, Ts && ... args)
    : InterfaceBase{std::forward<Ts>(args)...}
    , proxy_(proxy)
    {}

    void draw(RDPDestBlt          const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPOpaqueRect       const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPScrBlt           const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPPolygonSC        const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPPolygonCB        const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPPolyline         const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPEllipseSC        const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }
    void draw(RDPEllipseCB        const & cmd, Rect const & clip) override { this->proxy_(*this, cmd, clip); }

    void draw(RDPMemBlt           const & cmd, Rect const & clip, Bitmap const & bmp) override {
        this->proxy_(*this, cmd, clip, bmp);
    }

    void draw(RDPMem3Blt          const & cmd, Rect const & clip, Bitmap const & bmp) override {
        this->proxy_(*this, cmd, clip, bmp);
    }

    void draw(RDPGlyphIndex       const & cmd, Rect const & clip, GlyphCache const & gly_cache) override {
        this->proxy_(*this, cmd, clip, gly_cache);
    }

    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override {
        this->proxy_(*this, cmd, bmp);
    }

    Proxy & get_proxy() { return this->proxy_; }
    Proxy const & get_proxy() const { return this->proxy_; }

private:
    Proxy proxy_;
};

template<class Proxy>
struct ProxySkipBase : Proxy
{
    using Proxy::Proxy;

    template<class Base, class... Ts>
    void operator()(Base const &, Ts const & ... args) {
        Proxy::operator()(args...);
    }
};


namespace detail_
{
    template<class T>
    typename std::pointer_traits<T>::element_type & to_ref(T & x) {
        return *x;
    }

    template<class T>
    T & to_ref(T & x) {
        return x;
    }
}

template<class Gd>
struct DrawProxy
{
    template<class... GdArgs>
    DrawProxy(GdArgs && ... args)
    : gd_(std::forward<GdArgs>(args)...)
    {}

    template<class Base, class... Ts>
    void operator()(Base const &, Ts const & ... args) {
        detail_::to_ref(this->gd_).draw(args...);
    }

private:
    Gd gd_;
};

}

#endif
