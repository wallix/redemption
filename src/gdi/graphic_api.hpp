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

#ifndef REDEMPTION_GDI_GRAPHIC_DEVICE_HPP
#define REDEMPTION_GDI_GRAPHIC_DEVICE_HPP

#include <cstdint>
#include <type_traits>

#include "adapter_base.hpp"

#include "noncopyable.hpp"


class BGRPalette;
class RDPDestBlt;
class RDPMultiDstBlt;
class RDPPatBlt;
class RDPOpaqueRect;
class RDPMultiOpaqueRect;
class RDPScrBlt;
class RDPMemBlt;
class RDPMem3Blt;
class RDPLineTo;
class RDPGlyphIndex;
class RDPPolygonSC;
class RDPPolygonCB;
class RDPPolyline;
class RDPEllipseSC;
class RDPEllipseCB;

class RDPBitmapData;
class Pointer;
class Rect;
class Bitmap;
class GlyphCache;

class RDPColCache;
class RDPBrushCache;

namespace RDP {
    class RDPMultiPatBlt;
    class RDPMultiScrBlt;
    class FrameMarker;

    namespace RAIL {
        class NewOrExistingWindow;
        class WindowIcon;
        class CachedIcon;
        class DeletedWindow;
    }
}


namespace gdi {

struct GraphicApi : private noncopyable
{
    virtual ~GraphicApi() = default;

    virtual void draw(RDP::FrameMarker    const & order) = 0;
    virtual void draw(Pointer             const & cursor) = 0;
    virtual void draw(BGRPalette          const & palette) = 0;
    virtual void draw(RDPDestBlt          const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPPatBlt           const & cmd, Rect const & clip) = 0;
    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPOpaqueRect       const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect const & clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPLineTo           const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPPolygonSC        const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPPolygonCB        const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPPolyline         const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPEllipseSC        const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPEllipseCB        const & cmd, Rect const & clip) = 0;
    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect const & clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect const & clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPGlyphIndex       const & cmd, Rect const & clip, GlyphCache const & gly_cache) = 0;

    // NOTE maybe in an other interface
    virtual void draw(const RDP::RAIL::NewOrExistingWindow &) {}
    virtual void draw(const RDP::RAIL::WindowIcon          &) {}
    virtual void draw(const RDP::RAIL::CachedIcon          &) {}
    virtual void draw(const RDP::RAIL::DeletedWindow       &) {}

    // TODO("The 2 methods below should not exist and cache access be done before calling drawing orders")
    virtual void draw(RDPColCache   const & cmd) {}
    virtual void draw(RDPBrushCache const & cmd) {}

    virtual void sync() {}

    // TODO berk, data within size
    virtual void set_row(std::size_t rownum, const uint8_t * data) {}
};

struct GraphicProxy
{
    struct draw_tag {};
    struct sync_tag {};
    struct set_row_tag {};

    template<class Api, class... Ts>
    void operator()(draw_tag, Api & api, Ts const & ... args) {
        api.draw(args...);
    }

    template<class Api>
    void operator()(sync_tag, Api & api) {
        api.sync();
    }

    template<class Api>
    void operator()(set_row_tag, Api & api, std::size_t rownum, const uint8_t * data) {
        api.set_row(rownum, data);
    }
};

template<class Proxy, class InterfaceBase = GraphicApi>
struct GraphicAdapter : AdapterBase<Proxy, InterfaceBase>
{
    static_assert(std::is_base_of<GraphicApi, InterfaceBase>::value, "InterfaceBase isn't a GraphicApi");

    using AdapterBase<Proxy, InterfaceBase>::AdapterBase;

    void draw(RDP::FrameMarker    const & order) override { this->draw_(order); }
    void draw(Pointer             const & cursor) override { this->draw_(cursor); }
    void draw(BGRPalette          const & palette) override { this->draw_(palette); }

    void draw(RDPDestBlt          const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPOpaqueRect       const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPScrBlt           const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPPolygonSC        const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPPolygonCB        const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPPolyline         const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPEllipseSC        const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }
    void draw(RDPEllipseCB        const & cmd, Rect const & clip) override { this->draw_(cmd, clip); }

    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_(cmd, bmp); }

    void draw(RDPMemBlt           const & cmd, Rect const & clip, Bitmap const & bmp) override {
        this->draw_(cmd, clip, bmp);
    }

    void draw(RDPMem3Blt          const & cmd, Rect const & clip, Bitmap const & bmp) override {
        this->draw_(cmd, clip, bmp);
    }

    void draw(RDPGlyphIndex       const & cmd, Rect const & clip, GlyphCache const & gly_cache) override {
        this->draw_(cmd, clip, gly_cache);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) { this->draw_(order); }
    void draw(const RDP::RAIL::WindowIcon          & order) { this->draw_(order); }
    void draw(const RDP::RAIL::CachedIcon          & order) { this->draw_(order); }
    void draw(const RDP::RAIL::DeletedWindow       & order) { this->draw_(order); }

    void draw(RDPColCache   const & cmd) override { this->draw_(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_(cmd); }

    void sync() override { this->get_proxy()(GraphicProxy::sync_tag{}, *this); }

    void set_row(std::size_t rownum, const uint8_t * data) override {
      this->get_proxy()(GraphicProxy::set_row_tag{}, *this, rownum, data);
    }

private:
  template<class... Ts>
  void draw_(Ts const & ... args) {
    this->get_proxy()(GraphicProxy::draw_tag{}, *this, args...);
  }
};

}

#endif
