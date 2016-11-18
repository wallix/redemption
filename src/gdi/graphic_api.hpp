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

#pragma once

#include <type_traits>
#include <utility>

#include <cstdint>
#include <iostream>

#include "utils/sugar/noncopyable.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/caches/glyphcache.hpp"


struct BGRPalette;
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

struct RDPBitmapData;
struct Pointer;
struct Rect;
class Bitmap;

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
        class NewOrExistingNotificationIcons;
        class DeletedNotificationIcons;
        class ActivelyMonitoredDesktop;
        class NonMonitoredDesktop;
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
    GraphicDepth & operator=(GraphicDepth const &) = default;

private:
    struct bpp_table { uint8_t table[5] = {0, 8, 15, 16, 24}; };
public:
    constexpr uint8_t to_bpp() const {
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
    enum class PrivateDepth { unspecified_, depth8_, depth15_, depth16_, depth24_, };
public:
    // for switch/case, == and !=
    constexpr operator PrivateDepth () const { return static_cast<PrivateDepth>(this->depth_); }

private:
    constexpr GraphicDepth(uint8_t depth) : depth_(depth) {}

    uint8_t depth_;
};

constexpr bool operator < (GraphicDepth const & depth1, GraphicDepth const & depth2) {
    return depth1.id() < depth2.id();
}

constexpr bool operator > (GraphicDepth const & depth1, GraphicDepth const & depth2) {
    return (depth2 < depth1);
}

constexpr bool operator <= (GraphicDepth const & depth1, GraphicDepth const & depth2) {
    return !(depth2 < depth1);
}

constexpr bool operator >= (GraphicDepth const & depth1, GraphicDepth const & depth2) {
    return !(depth1 < depth2);
}


struct GraphicApi : private noncopyable
{
    GraphicApi(GraphicDepth const & order_depths = GraphicDepth::unspecified())
    : order_depth_(order_depths)
    {}

    virtual ~GraphicApi() = default;

    virtual void set_pointer(Pointer      const &) {}
    virtual void set_palette(BGRPalette   const &) {}

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
    virtual void draw(const RDP::RAIL::NewOrExistingWindow            &) {}
    virtual void draw(const RDP::RAIL::WindowIcon                     &) {}
    virtual void draw(const RDP::RAIL::CachedIcon                     &) {}
    virtual void draw(const RDP::RAIL::DeletedWindow                  &) {}
    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons &) {}
    virtual void draw(const RDP::RAIL::DeletedNotificationIcons       &) {}
    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop       &) {}
    virtual void draw(const RDP::RAIL::NonMonitoredDesktop            &) {}

    // TODO The 2 methods below should not exist and cache access be done before calling drawing orders
    virtual void draw(RDPColCache   const &) {}
    virtual void draw(RDPBrushCache const &) {}

    virtual void begin_update() {}
    virtual void end_update() {}

    virtual void sync() {}

    // TODO berk, data within size
    virtual void set_row(std::size_t rownum, const uint8_t * data) { (void)rownum; (void)data; }

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


struct GraphicCoreAccess
{
    template<class Derived, class... Ts>
    static void draw(Derived & derived, Ts const & ... args) {
        derived.draw_impl(args...);
    }

    template<class Derived>
    static auto graphic_proxy(Derived & derived)
    -> decltype(derived.get_graphic_proxy()) {
        return derived.get_graphic_proxy();
    }

    template<class Derived>
    static auto color_converter(Derived const & derived)
    -> decltype(derived.get_color_converter()) {
        return derived.get_color_converter();
    }
};


/**
 * \code
 * struct Graphic : gdi::GraphicBase<Graphic>
 * {
 * private:
 *   friend gdi::GraphicCoreAccess;
 *
 *   template<class Cmd, class... Args>
 *   void draw_impl(Cmd const & cmd, Args const & ...) { log(cmd_name(cmd)); }
 * };
 * \endcode
 */
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

    void draw(const RDP::RAIL::NewOrExistingWindow            & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::WindowIcon                     & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::CachedIcon                     & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::DeletedWindow                  & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & order) override { this->draw_(order); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & order) override { this->draw_(order); }

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
};


/**
 * \code
 * struct Graphic : gdi::GraphicProxyBase<Graphic>
 * {
 * private:
 *   gdi::GraphicApi & internal;
 *
 *   friend gdi::GraphicCoreAccess;
 *
 *   gdi::GraphicApi & get_graphic_proxy() { return this->internal; }
 * };
 * \endcode
 */
template<class Derived, class InterfaceBase = GraphicApi, class CoreAccess = GraphicCoreAccess>
struct GraphicProxyBase : GraphicBase<Derived, InterfaceBase, CoreAccess>
{
    using GraphicBase<Derived, InterfaceBase, CoreAccess>::GraphicBase;
    using base_type = GraphicProxyBase;
    friend CoreAccess;

    void set_pointer(Pointer    const & pointer) override {
        CoreAccess::graphic_proxy(this->derived()).set_pointer(pointer);
    }

    void set_palette(BGRPalette const & palette) override {
        CoreAccess::graphic_proxy(this->derived()).set_palette(palette);
    }

    void sync() override {
        CoreAccess::graphic_proxy(this->derived()).sync();
    }

    void set_row(std::size_t rownum, const uint8_t * data) override {
        CoreAccess::graphic_proxy(this->derived()).set_row(rownum, data);
    }

    void begin_update() override {
        CoreAccess::graphic_proxy(this->derived()).begin_update();
    }

    void end_update() override {
        CoreAccess::graphic_proxy(this->derived()).end_update();
    }

protected:
    template<class... Ts>
    void draw_impl(Ts const & ... args) {
        CoreAccess::graphic_proxy(this->derived()).draw(args...);
    }
};


struct draw_tag {};
struct set_tag {};
struct sync_tag {};
struct set_row_tag {};
struct begin_update_tag{};
struct end_update_tag{};

template<class Graphic>
struct GraphicUniformProxy
{
    using proxy_type = GraphicUniformProxy;

    GraphicUniformProxy() = default;
    GraphicUniformProxy(Graphic && graphic) : graphic_(std::move(graphic)) {}
    GraphicUniformProxy(Graphic const & graphic) : graphic_(graphic) {}

    template<class Graphic_>
    GraphicUniformProxy(Graphic_ && graphic)
    : graphic_(std::forward<Graphic_>(graphic))
    {}

    template<class... Ts>
    void draw(Ts const & ... args) {
        this->graphic_(draw_tag{}, args...);
    }

    void set_pointer(Pointer    const & pointer) {
        this->graphic_(set_tag{}, pointer);
    }

    void set_palette(BGRPalette const & palette) {
        this->graphic_(set_tag{}, palette);
    }

    void sync() {
        this->graphic_(sync_tag{});
    }

    void set_row(std::size_t rownum, const uint8_t * data) {
        this->graphic_(set_row_tag{}, rownum, data);
    }

    void begin_update() {
        this->graphic_(begin_update_tag{});
    }

    void end_update() {
        this->graphic_(end_update_tag{});
    }

private:
    Graphic graphic_;
};


template<class Graphic>
struct GraphicUniformDistribution
{
    Graphic graphic_;

    template<class... Ts>
    void operator()(draw_tag, Ts const & ... args) {
        this->graphic_.draw(args...);
    }

    void operator()(set_tag, Pointer const & pointer) {
        this->graphic_.set_pointer(pointer);
    }

    void operator()(set_tag, BGRPalette const & palette) {
        this->graphic_.set_palette(palette);
    }

    void operator()(sync_tag) {
        this->graphic_.sync();
    }

    void operator()(set_row_tag, std::size_t rownum, const uint8_t * data) {
        this->graphic_.set_row(rownum, data);
    }

    void operator()(begin_update_tag) {
        this->graphic_.begin_update();
    }

    void operator()(end_update_tag) {
        this->graphic_.end_update();
    }
};


struct self_fn
{
    template<class T> T & operator()(T & x) const { return x; }
    template<class T> T operator()(T && x) const { return std::move(x); }

    template<class T> T & operator()(std::reference_wrapper<T> x) const { return x; }
};

template<class GraphicList, class Projection = self_fn>
struct GraphicUniformDispatcherList
{
    GraphicList & graphics_;
    Projection projection_;

    template<class Tag, class... Ts>
    void operator()(Tag tag, Ts const & ... args) {
        for (auto && graphic : this->graphics_) {
            this->projection_(graphic)(tag, args...);
        }
    }
};


template<class Projection>
struct GraphicUniformProjection
{
    Projection projection_;

    template<class Graphic>
    auto operator()(Graphic & graphic)
    -> GraphicUniformDistribution<decltype(projection_(graphic))>
    {
        return {this->projection_(graphic)};
    }
};

/**
 * \code
 * struct GraphicDispatcher : gdi::GraphicProxyBase<Graphic>
 * {
 * private:
 *   std::vector<std::reference_wrapper<gdi::GraphicApi>> graphics;
 *
 *   friend gdi::GraphicCoreAccess;
 *
 *   GraphicDispatcherList<std::vector<std::reference_wrapper<gdi::GraphicApi>>>
 *   get_graphic_proxy() { return this->graphics; }
 * };
 * \endcode
 */
template<class GraphicList, class Projection = self_fn>
struct GraphicDispatcherList
: GraphicUniformProxy<
    GraphicUniformDispatcherList<
        GraphicList,
        GraphicUniformProjection<Projection>
    >
>
{
    GraphicDispatcherList(GraphicList & graphic_list, Projection proj = Projection{})
    : GraphicDispatcherList::proxy_type{{graphic_list, GraphicUniformProjection<Projection>{proj}}}
    {}
};


class BlackoutGraphic final : public GraphicBase<BlackoutGraphic>
{
    friend gdi::GraphicCoreAccess;

    template<class... Args>
    void draw_impl(Args const & ...) {}

    using base_type_ = GraphicBase<BlackoutGraphic>;

public:
    using base_type_::base_type_;
};


inline gdi::GraphicApi & null_gd() {
    static gdi::BlackoutGraphic gd;
    return gd;
}


struct TextMetrics
{
    int width = 0;
    int height = 0;

    TextMetrics(const Font & font, const char * unicode_text)
    {
        UTF8toUnicodeIterator unicode_iter(unicode_text);
        uint16_t height_max = 0;
        for (; uint32_t c = *unicode_iter; ++unicode_iter) {
            const FontChar & font_item = font.glyph_or_unknown(c);
            this->width += font_item.incby;
            height_max = std::max(height_max, font_item.height);
        }
        this->height = height_max;
    }
};


// TODO implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
static inline void server_draw_text(
                GraphicApi & drawable,
                Font const & font, int16_t x, int16_t y, const char * text,
                uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
{
    // TODO static not const is a bad idea
    static GlyphCache mod_glyph_cache;

    UTF8toUnicodeIterator unicode_iter(text);

    while (*unicode_iter) {
        int total_width = 0;
        int total_height = 0;
        uint8_t data[256];
        auto data_begin = std::begin(data);
        const auto data_end = std::end(data)-2;

        const int cacheId = 7;
        int distance_from_previous_fragment = 0;
        while (data_begin != data_end) {
            const uint32_t charnum = *unicode_iter;
            if (!charnum) {
                break ;
            }
            ++unicode_iter;

            int cacheIndex = 0;
            FontChar const * font_item = font.glyph_at(charnum);
            if (!font_item) {
                LOG(LOG_WARNING, "server_draw_text() - character not defined >0x%02x<", charnum);
                font_item = &font.unknown_glyph();
            }

            // TODO avoid passing parameters by reference to get results
            const GlyphCache::t_glyph_cache_result cache_result =
                mod_glyph_cache.add_glyph(*font_item, cacheId, cacheIndex);
            (void)cache_result; // supress warning

            *data_begin = cacheIndex;
            ++data_begin;
            *data_begin = distance_from_previous_fragment;
            ++data_begin;
            distance_from_previous_fragment = font_item->incby;
            total_width += font_item->incby;
            total_height = std::max(uint16_t(total_height), font_item->height);
        }

        const Rect bk(x, y, total_width + 1, total_height + 1);

        RDPGlyphIndex glyphindex(
            cacheId,            // cache_id
            0x03,               // fl_accel
            0x0,                // ui_charinc
            1,                  // f_op_redundant,
            fgcolor,            // BackColor (text color)
            bgcolor,            // ForeColor (color of the opaque rectangle)
            bk,                 // bk
            bk,                 // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                reinterpret_cast<const uint8_t *>("\xaa\x55\xaa\x55\xaa\x55\xaa\x55")),
            x,                  // glyph_x
            y + total_height,   // glyph_y
            data_begin - data,  // data_len in bytes
            data                // data
        );

        x += total_width;

        drawable.draw(glyphindex, clip, mod_glyph_cache);
    }
}

}
