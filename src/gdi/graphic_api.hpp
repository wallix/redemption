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

#include "utils/sugar/noncopyable.hpp"
#include "utils/colors.hpp"
#include "utils/rect.hpp"

#include <cassert>


class Font;

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
        class NewOrExistingNotificationIcons;
        class DeletedNotificationIcons;
        class ActivelyMonitoredDesktop;
        class NonMonitoredDesktop;
    }
}

namespace gdi {

struct Depth
{
    static constexpr Depth unspecified() { return {0}; }
    static constexpr Depth depth8() { return {1}; }
    static constexpr Depth depth15() { return {2}; }
    static constexpr Depth depth16() { return {3}; }
    static constexpr Depth depth24() { return {4}; }

    static /*c++14: constexpr*/ Depth from_bpp(uint8_t bpp)
    {
        struct depth_table {
            Depth table[33] = {
                unspecified(),  unspecified(),  unspecified(),  unspecified(),
                unspecified(),  unspecified(),  unspecified(),  unspecified(),
                depth8(),       unspecified(),  unspecified(),  unspecified(),
                unspecified(),  unspecified(),  unspecified(),  depth15(),
                depth16(),      unspecified(),  unspecified(),  unspecified(),
                unspecified(),  unspecified(),  unspecified(),  unspecified(),
                depth24(),      unspecified(),  unspecified(),  unspecified(),
                unspecified(),  unspecified(),  unspecified(),  unspecified(),
                depth24(), // TODO useless ?
            };
        };
        auto const depth = bpp < sizeof(depth_table{}.table)/sizeof(depth_table{}.table[0])
            ? depth_table{}.table[int(bpp)]
            : unspecified();
        assert(depth != unspecified());
        return depth;
    }

    Depth() = default;

    constexpr Depth(Depth const &) = default;
    Depth & operator=(Depth const &) = default;

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

    constexpr bool contains(Depth depth) const {
        return this->is_unspecified() || depth.is_unspecified() || (this->depth_ == depth.depth_);
    }

    constexpr Depth const & depth_or(Depth const & default_depth) const {
        return this->is_unspecified() ? default_depth : *this;
    }

    constexpr unsigned id() const { return this->depth_; }

private:
    enum class PrivateDepth { unspecified_, depth8_, depth15_, depth16_, depth24_, };
public:
    // for switch/case, == and !=
    constexpr operator PrivateDepth () const { return static_cast<PrivateDepth>(this->depth_); }

private:
    constexpr Depth(uint8_t depth) : depth_(depth) {}

    uint8_t depth_;
};

constexpr bool operator < (Depth const & depth1, Depth const & depth2) {
    return depth1.id() < depth2.id();
}

constexpr bool operator > (Depth const & depth1, Depth const & depth2) {
    return (depth2 < depth1);
}

constexpr bool operator <= (Depth const & depth1, Depth const & depth2) {
    return !(depth2 < depth1);
}

constexpr bool operator >= (Depth const & depth1, Depth const & depth2) {
    return !(depth1 < depth2);
}


struct ColorCtx
{
    ColorCtx(Depth depth, BGRPalette const & palette)
    : depth_(depth)
    , palette_(&palette)
    {}

    ColorCtx(Depth depth, BGRPalette const * palette)
    : depth_(depth)
    , palette_(palette)
    {
        assert(depth == Depth::depth8() ? bool(palette) : true);
    }

    Depth depth() const { return this->depth_; }
    BGRPalette const * palette() const { return this->palette_; }


    static ColorCtx depth8(BGRPalette && palette) = delete;
    static ColorCtx depth8(BGRPalette const & palette) { return {Depth::depth8(), &palette}; }

    static ColorCtx depth15() { return {Depth::depth15(), nullptr}; }
    static ColorCtx depth16() { return {Depth::depth16(), nullptr}; }
    static ColorCtx depth24() { return {Depth::depth24(), nullptr}; }

    static ColorCtx from_bpp(uint8_t bpp, BGRPalette const * palette)
    { return {Depth::from_bpp(bpp), palette}; }

    static ColorCtx from_bpp(uint8_t bpp, BGRPalette const && palette) = delete;
    static ColorCtx from_bpp(uint8_t bpp, BGRPalette const & palette)
    { return {Depth::from_bpp(bpp), &palette}; }

private:
    Depth depth_;
    BGRPalette const * palette_;
};


inline RDPColor color_encode(const BGRColor color, Depth depth) noexcept
{
    switch (depth){
        case Depth::depth8(): return encode_color8()(color);
        case Depth::depth15(): return encode_color15()(color);
        case Depth::depth16(): return encode_color16()(color);
        case Depth::depth24(): return encode_color24()(color);
        case Depth::unspecified(): default:;
    }

    REDASSERT(false);
    return RDPColor{};
}


inline BGRColor color_decode(const RDPColor color, ColorCtx color_ctx) noexcept
{
    switch (color_ctx.depth()){
        case Depth::depth8(): return decode_color8()(color, *color_ctx.palette());
        case Depth::depth15(): return decode_color15()(color);
        case Depth::depth16(): return decode_color16()(color);
        case Depth::depth24(): return decode_color24()(color);
        case Depth::unspecified(): default:;
    }

    REDASSERT(false);
    return BGRColor{0, 0, 0};
}


inline BGRColor color_decode(const RDPColor color, Depth depth, const BGRPalette & palette) noexcept
{
    return color_decode(color, ColorCtx{depth, palette});
}


struct GraphicApi : private noncopyable
{
    GraphicApi() {}

    virtual ~GraphicApi() = default;

    virtual void set_pointer(Pointer      const &) {}
    virtual void set_palette(BGRPalette   const &) {}

    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
    virtual void draw(RDPDestBlt          const & cmd, Rect clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;

    virtual void draw(RDPPatBlt           const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPOpaqueRect       const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPLineTo           const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonSC        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonCB        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolyline         const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseSC        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseCB        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, ColorCtx color_ctx, Bitmap const & bmp) = 0;
    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;

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
};


class NullGraphic final : public GraphicApi
{
public:
    void draw(RDP::FrameMarker    const &) override {}
    void draw(RDPDestBlt          const &, Rect) override {}
    void draw(RDPMultiDstBlt      const &, Rect) override {}
    void draw(RDPPatBlt           const &, Rect, ColorCtx) override {}
    void draw(RDP::RDPMultiPatBlt const &, Rect, ColorCtx) override {}
    void draw(RDPOpaqueRect       const &, Rect, ColorCtx) override {}
    void draw(RDPMultiOpaqueRect  const &, Rect, ColorCtx) override {}
    void draw(RDPScrBlt           const &, Rect) override {}
    void draw(RDP::RDPMultiScrBlt const &, Rect) override {}
    void draw(RDPLineTo           const &, Rect, ColorCtx) override {}
    void draw(RDPPolygonSC        const &, Rect, ColorCtx) override {}
    void draw(RDPPolygonCB        const &, Rect, ColorCtx) override {}
    void draw(RDPPolyline         const &, Rect, ColorCtx) override {}
    void draw(RDPEllipseSC        const &, Rect, ColorCtx) override {}
    void draw(RDPEllipseCB        const &, Rect, ColorCtx) override {}
    void draw(RDPBitmapData       const &, Bitmap const &) override {}
    void draw(RDPMemBlt           const &, Rect, Bitmap const &) override {}
    void draw(RDPMem3Blt          const &, Rect, ColorCtx, Bitmap const &) override {}
    void draw(RDPGlyphIndex       const &, Rect, ColorCtx, GlyphCache const &) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow            &) override {}
    void draw(const RDP::RAIL::WindowIcon                     &) override {}
    void draw(const RDP::RAIL::CachedIcon                     &) override {}
    void draw(const RDP::RAIL::DeletedWindow                  &) override {}
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons &) override {}
    void draw(const RDP::RAIL::DeletedNotificationIcons       &) override {}
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       &) override {}
    void draw(const RDP::RAIL::NonMonitoredDesktop            &) override {}

    void draw(RDPColCache   const &) override {}
    void draw(RDPBrushCache const &) override {}

    static gdi::NullGraphic & instance()
    {
        static gdi::NullGraphic gd;
        return gd;
    }

public:
    NullGraphic()
    {}
};

inline gdi::GraphicApi & null_gd() noexcept
{
    return gdi::NullGraphic::instance();
}


struct TextMetrics
{
    int width = 0;
    int height = 0;

    TextMetrics(const Font & font, const char * unicode_text);
};


// TODO implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)
// TODO: is it still used ? If yes move it somewhere else. Method from internal mods ?
void server_draw_text(
    GraphicApi & drawable, Font const & font,
    int16_t x, int16_t y, const char * text,
    RDPColor fgcolor, RDPColor bgcolor,
    ColorCtx color_ctx,
    Rect clip
);

}
