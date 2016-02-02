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

#include <type_traits>

#include <cstdint>

#include "adapter_base.hpp"

#include "noncopyable.hpp"

#include <array>

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

struct GraphicDepth
{
    static constexpr GraphicDepth unspecified() { return {0}; }
    static constexpr GraphicDepth depth8() { return {1}; }
    static constexpr GraphicDepth depth15() { return {2}; }
    static constexpr GraphicDepth depth16() { return {3}; }
    static constexpr GraphicDepth depth24() { return {4}; }

    static constexpr GraphicDepth from_bpp(uint8_t bpp) {
        return {
            bpp == 8  ? depth8() :
            bpp == 15 ? depth15() :
            bpp == 16 ? depth16() :
            bpp == 24 ? depth24() :
            bpp == 32 ? depth24() :
            unspecified()
        };
    }

    constexpr GraphicDepth(GraphicDepth const &) = default;

private:
    struct bpp_table { int table[5] = {0, 8, 15, 16, 24}; };
public:
    constexpr uint8_t to_bpp(uint8_t bpp) const {
        return bpp_table{}.table[unsigned(this->depth_)];
    }

    constexpr bool is_defined() const { return !this->is_unspecified(); }
    constexpr bool is_unspecified() const { return this->depth_ == unspecified().depth_; }
    constexpr bool is_depth8() const { return this->depth_ == depth8().depth_; }
    constexpr bool is_depth15() const { return this->depth_ == depth15().depth_; }
    constexpr bool is_depth16() const { return this->depth_ == depth16().depth_; }
    constexpr bool is_depth24() const { return this->depth_ == depth24().depth_; }

    constexpr bool contains(GraphicDepth depth) const {
        return this->is_unspecified() || depth.is_unspecified() || (this->depth_ == depth.depth_);
    }

    constexpr GraphicDepth const & depth_or(GraphicDepth const & default_depth) const {
        return this->is_unspecified() ? default_depth : *this;
    }

    constexpr unsigned id() const { return this->depth_; }

private:
    enum class PrivateDepth { d0, d1, d2, d3, d4 };
public:
    // for switch/case, == and !=
    constexpr operator PrivateDepth () const { return static_cast<PrivateDepth>(this->depth_); }

private:
    constexpr GraphicDepth(uint8_t depth) : depth_(depth) {}

    uint8_t depth_;
};


struct GraphicApi : private noncopyable
{
    GraphicApi(GraphicDepth const & order_depths = GraphicDepth::unspecified())
    : order_depth_(order_depths)
    {}

    virtual ~GraphicApi() = default;

    virtual void set_pointer(Pointer      const & cursor) {}
    virtual void set_palette(BGRPalette   const & palette) {}

    virtual void draw(RDP::FrameMarker    const & order) = 0;
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

    GraphicDepth const & order_depth() const {
        return this->order_depth_;
    }

private:
    GraphicDepth order_depth_;

protected:
    void set_depths(GraphicDepth const & depths) {
        this->order_depth_ = depths;
    }
};

struct GraphicProxy
{
    struct draw_tag {};
    struct set_tag {};
    struct sync_tag {};
    struct set_row_tag {};

    template<class Api, class... Ts>
    void operator()(draw_tag, Api & api, Ts const & ... args) {
        api.draw(args...);
    }

    template<class Api>
    void operator()(set_tag, Api & api, Pointer const & pointer) {
        api.set_pointer(pointer);
    }

    template<class Api>
    void operator()(set_tag, Api & api, BGRPalette const & palette) {
        api.set_palette(palette);
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

    void set_pointer(Pointer    const & cursor) override {
        this->get_proxy()(GraphicProxy::set_tag{}, *this, cursor);
    }
    void set_palette(BGRPalette const & palette) override {
        this->get_proxy()(GraphicProxy::set_tag{}, *this, palette);
    }

    void draw(RDP::FrameMarker    const & order) override { this->draw_(order); }

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

namespace {
    struct GraphicCoreAccess
    {
        template<class Derived, class... Ts>
        static void draw(Derived & derived, Ts const & ... args) {
            derived.draw_impl(args...);
        }

        template<class Derived>
        static auto color_converter(Derived const & derived)
        -> decltype(derived.color_converter_impl()) {
            return derived.color_converter_impl();
        }

        template<class Derived, class Tag, class... Ts>
        static void graphic_proxy_func(Derived & derived, Tag tag, Ts const & ... args) {
            derived.graphic_proxy_func_impl(tag, args...);
        }

        template<class Derived>
        static auto get_gd_proxy(Derived & derived)
        -> decltype(derived.get_gd_proxy_impl()) {
            return derived.get_gd_proxy_impl();
        }

        template<class Derived>
        static auto get_gd_list(Derived & derived)
        -> decltype(derived.get_gd_list_impl()) {
            return derived.get_gd_list_impl();
        }

        template<class Derived, class Gd>
        static auto to_graphic_facade(Derived & derived, Gd & gd)
        -> decltype(derived.to_graphic_facade_impl(gd)) {
            return derived.to_graphic_facade_impl(gd);
        }
    };
}

template<class Derived, class InterfaceBase = GraphicApi, class CoreAccess = GraphicCoreAccess>
class GraphicBase : public InterfaceBase
{
    static_assert(std::is_base_of<GraphicApi, InterfaceBase>::value, "InterfaceBase isn't a GraphicApi");

    friend CoreAccess;

protected:
    using core_access = CoreAccess;
    using base_type = GraphicBase;

public:
    using InterfaceBase::InterfaceBase;

    void draw(RDP::FrameMarker    const & order) override { this->draw_(order); }

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

protected:
    Derived & derived() {
        return static_cast<Derived&>(*this);
    }

private:
    template<class... Ts>
    void draw_(Ts const & ... args) {
        CoreAccess::draw(this->derived(), args...);
    }

protected:
    template<class... Ts> void draw_impl(Ts const & ...) { }
};


template<class Derived, class InterfaceBase = GraphicApi, class CoreAccess = GraphicCoreAccess>
struct GraphicWrapper : GraphicBase<Derived, InterfaceBase, CoreAccess>
{
    using GraphicBase<Derived, InterfaceBase, CoreAccess>::GraphicBase;
    using base_type = GraphicWrapper;
    friend CoreAccess;

    void set_pointer(Pointer    const & pointer) override {
        CoreAccess::get_gd_proxy(this->derived()).set_pointer(pointer);
    }

    void set_palette(BGRPalette const & palette) override {
        CoreAccess::get_gd_proxy(this->derived()).set_palette(palette);
    }

    void sync() override {
        CoreAccess::get_gd_proxy(this->derived()).sync();
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        CoreAccess::get_gd_proxy(this->derived()).set_row(rownum, data);
    }

protected:
    template<class... Ts>
    void draw_impl(Ts const & ... args) {
        CoreAccess::get_gd_proxy(this->derived()).draw(args...);
    }
};


template<class Derived, class InterfaceBase = GraphicApi, class CoreAccess = GraphicCoreAccess>
struct GraphicDispatcher : GraphicBase<Derived, InterfaceBase, CoreAccess>
{
    using GraphicBase<Derived, InterfaceBase, CoreAccess>::GraphicBase;
    using base_type = GraphicDispatcher;
    friend CoreAccess;

    void set_pointer(Pointer    const & pointer) override {
        for (auto && gd : CoreAccess::get_gd_list(this->derived())) {
            CoreAccess::to_graphic_facade(this->derived(), gd).set_pointer(pointer);
        }
    }

    void set_palette(BGRPalette const & palette) override {
        for (auto && gd : CoreAccess::get_gd_list(this->derived())) {
            CoreAccess::to_graphic_facade(this->derived(), gd).set_palette(palette);
        }
    }

    void sync() override {
        for (auto && gd : CoreAccess::get_gd_list(this->derived())) {
            CoreAccess::to_graphic_facade(this->derived(), gd).sync();
        }
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        for (auto && gd : CoreAccess::get_gd_list(this->derived())) {
            CoreAccess::to_graphic_facade(this->derived(), gd).set_row(rownum, data);
        }
    }

protected:
    template<class... Ts>
    void draw_impl(Ts const & ... args) {
        for (auto && gd : CoreAccess::get_gd_list(this->derived())) {
            CoreAccess::to_graphic_facade(this->derived(), gd).draw(args...);
        }
    }

    GraphicApi & to_graphic_facade_impl(GraphicApi & gd) {
        return gd;
    }
};

template<class Derived, class InterfaceBase = GraphicApi, class CoreAccess = GraphicCoreAccess>
struct GraphicProx : GraphicBase<Derived, InterfaceBase, CoreAccess>
{
    using GraphicBase<Derived, InterfaceBase, CoreAccess>::GraphicBase;
    using base_type = GraphicProx;
    friend CoreAccess;

    friend CoreAccess;

    struct draw_tag {};
    struct set_tag {};
    struct sync_tag {};
    struct set_row_tag {};

    void set_pointer(Pointer    const & pointer) override {
        CoreAccess::graphic_proxy_func(this->derived(), set_tag{}, pointer);
    }

    void set_palette(BGRPalette const & palette) override {
        CoreAccess::graphic_proxy_func(this->derived(), set_tag{}, palette);
    }

    void sync() override {
        CoreAccess::graphic_proxy_func(this->derived(), sync_tag{});
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        CoreAccess::graphic_proxy_func(this->derived(), set_row_tag{}, rownum, data);
    }

protected:
    template<class... Ts> void draw_impl(Ts const & ... args) {
        CoreAccess::graphic_proxy_func(this->derived(), draw_tag{}, args...);
    }

    template<class... Ts> void graphic_proxy_func_impl(Ts const & ...) {}
};

}

#endif
