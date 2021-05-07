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

#include "gdi/screen_info.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/colors.hpp"
#include "utils/rect.hpp"
#include "cxx/cxx.hpp"

#include <cassert>

enum class PredefinedPointer : uint8_t;

class RDPDstBlt;
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
#ifdef __EMSCRIPTEN__
class RDPBmpCache;
# include <array>
#endif

struct RDPBitmapData;
class Bitmap;
class GlyphCache;

class RDPColCache;
class RDPBrushCache;
class RDPSetSurfaceCommand;
class RDPSurfaceContent;
class RdpPointerView;

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
    } // namespace RAIL
}  // namespace RDP

namespace gdi {

struct Depth
{
    static constexpr Depth unspecified() { return Depth{0}; }
    static constexpr Depth depth8() { return Depth{1}; }
    static constexpr Depth depth15() { return Depth{2}; }
    static constexpr Depth depth16() { return Depth{3}; }
    static constexpr Depth depth24() { return Depth{4}; }

    static /*c++14: constexpr*/ Depth from_bpp(BitsPerPixel bpp) noexcept
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
        auto const depth = size_t(bpp) < sizeof(depth_table{}.table)/sizeof(depth_table{}.table[0])
            ? depth_table{}.table[int(bpp)]
            : unspecified();
        assert(depth != unspecified());
        return depth;
    }

    Depth() = default;

    constexpr Depth(Depth const &) = default;
    Depth & operator=(Depth const &) = default;

private:
    struct bpp_table { BitsPerPixel table[5] = {
        BitsPerPixel{0},
        BitsPerPixel{8},
        BitsPerPixel{15},
        BitsPerPixel{16},
        BitsPerPixel{24}
    }; };

public:
    [[nodiscard]] constexpr BitsPerPixel to_bpp() const noexcept {
        return bpp_table{}.table[unsigned(this->depth_)];
    }

    [[nodiscard]] constexpr bool is_defined() const noexcept { return !this->is_unspecified(); }
    [[nodiscard]] constexpr bool is_unspecified() const noexcept { return this->depth_ == unspecified().depth_; }
    [[nodiscard]] constexpr bool is_depth8() const noexcept { return this->depth_ == depth8().depth_; }
    [[nodiscard]] constexpr bool is_depth15() const noexcept { return this->depth_ == depth15().depth_; }
    [[nodiscard]] constexpr bool is_depth16() const noexcept { return this->depth_ == depth16().depth_; }
    [[nodiscard]] constexpr bool is_depth24() const noexcept { return this->depth_ == depth24().depth_; }

    [[nodiscard]] constexpr bool contains(Depth depth) const noexcept {
        return this->is_unspecified() || depth.is_unspecified() || (this->depth_ == depth.depth_);
    }

    [[nodiscard]] constexpr Depth const & depth_or(Depth const & default_depth) const noexcept {
        return this->is_unspecified() ? default_depth : *this;
    }

    [[nodiscard]] constexpr unsigned id() const noexcept { return this->depth_; }

private:
    enum class PrivateDepth { unspecified_, depth8_, depth15_, depth16_, depth24_, };
public:
    // for switch/case, == and !=
    constexpr operator PrivateDepth () const noexcept {
        return static_cast<PrivateDepth>(this->depth_);
    }

private:
    explicit constexpr Depth(uint8_t depth) noexcept : depth_(depth) {}

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

    [[nodiscard]] Depth depth() const { return this->depth_; }
    [[nodiscard]] BGRPalette const * palette() const { return this->palette_; }


    static ColorCtx depth8(BGRPalette && palette) = delete;
    static ColorCtx depth8(BGRPalette const & palette) { return {Depth::depth8(), &palette}; }

    static ColorCtx depth15() { return {Depth::depth15(), nullptr}; }
    static ColorCtx depth16() { return {Depth::depth16(), nullptr}; }
    static ColorCtx depth24() { return {Depth::depth24(), nullptr}; }

    static ColorCtx from_bpp(BitsPerPixel bpp, BGRPalette const * palette)
    { return {Depth::from_bpp(bpp), palette}; }

    static ColorCtx from_bpp(uint8_t bpp, BGRPalette const && palette) = delete;
    static ColorCtx from_bpp(BitsPerPixel bpp, BGRPalette const && palette) = delete;

    static ColorCtx from_bpp(BitsPerPixel bpp, BGRPalette const & palette)
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
        case Depth::unspecified(): break;
    }

    REDEMPTION_UNREACHABLE();
    return RDPColor{};
}


inline BGRColor color_decode(const RDPColor color, ColorCtx color_ctx) noexcept
{
    switch (color_ctx.depth()){
        case Depth::depth8(): return decode_color8()(color, *color_ctx.palette());
        case Depth::depth15(): return decode_color15()(color);
        case Depth::depth16(): return decode_color16()(color);
        case Depth::depth24(): return decode_color24()(color);
        case Depth::unspecified(): break;
    }

    REDEMPTION_UNREACHABLE();
    return BGRColor{0, 0, 0};
}


inline BGRColor color_decode(const RDPColor color, Depth depth, const BGRPalette & palette) noexcept
{
    return color_decode(color, ColorCtx{depth, palette});
}

struct CachePointerIndex
{
    static constexpr std::size_t MAX_POINTER_COUNT = 25;

    CachePointerIndex(uint16_t idx) noexcept
    : idx(idx)
    {
        assert(idx < MAX_POINTER_COUNT);
    }

    CachePointerIndex(PredefinedPointer pointer) noexcept
    : idx(MAX_POINTER_COUNT + uint16_t(pointer))
    {}

    bool is_predefined_pointer() const noexcept
    {
        return idx >= MAX_POINTER_COUNT;
    }

    PredefinedPointer as_predefined_pointer() const noexcept
    {
        assert(is_predefined_pointer());
        return PredefinedPointer(idx - MAX_POINTER_COUNT);
    }

    uint16_t cache_index() const noexcept
    {
        return idx;
    }

private:
    uint16_t idx;
};

// TODO: why doesn't GraphicApi give information on drawing surface size ?
// in vnc copyrect we need that to clip to the whole screen
struct GraphicApi : private noncopyable
{
    GraphicApi() = default;

    virtual ~GraphicApi() = default;

    virtual void set_palette(BGRPalette   const & /*unused*/) {}

    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
    virtual void draw(RDPDstBlt          const & cmd, Rect clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;

#ifdef __EMSCRIPTEN__
    struct CacheEntry
    {
        uint16_t nb_entries;
        uint16_t bmp_size;
        bool is_persistent;
    };
    virtual void set_bmp_cache_entries(std::array<CacheEntry, 3> const & /*cache_entries*/) = 0;
    virtual void draw(RDPBmpCache         const & /*cmd*/) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, ColorCtx color_ctx) = 0;
#else
    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, ColorCtx color_ctx, Bitmap const & bmp) = 0;
#endif

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
    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;
    virtual void draw(RDPSetSurfaceCommand const & cmd) = 0;
    virtual void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) = 0;

    // NOTE maybe in an other interface
    virtual void draw(const RDP::RAIL::NewOrExistingWindow            & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::WindowIcon                     & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::CachedIcon                     & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::DeletedWindow                  & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::DeletedNotificationIcons       & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & /*cmd*/) = 0;
    virtual void draw(const RDP::RAIL::NonMonitoredDesktop            & /*cmd*/) = 0;

    // TODO The 2 methods below should not exist and cache access be done before calling drawing orders
    virtual void draw(RDPColCache   const & /*cache*/) {}
    virtual void draw(RDPBrushCache const & /*cache*/) {}

    virtual void begin_update() {}
    virtual void end_update() {}

    virtual void sync() {}

    virtual void cached_pointer(CachePointerIndex cache_idx) = 0;

    /// \pre cache_idx.is_predefined_pointer() == false
    virtual void new_pointer(CachePointerIndex cache_idx, RdpPointerView const& cursor) = 0;

    // TODO berk, data within size
    virtual void set_row(std::size_t rownum, bytes_view data) { (void)rownum; (void)data; }
};


class NullGraphic : public GraphicApi
{
public:
    void draw(RDP::FrameMarker    const & /*cmd*/) override {}

    void draw(RDPDstBlt          const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPMultiDstBlt      const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPPatBlt           const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDP::RDPMultiPatBlt const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPOpaqueRect       const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPMultiOpaqueRect  const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPScrBlt           const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDP::RDPMultiScrBlt const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPLineTo           const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolygonSC        const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolygonCB        const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPPolyline         const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPEllipseSC        const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPEllipseCB        const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
    void draw(RDPBitmapData       const & /*cmd*/, Bitmap const & /*bmp*/) override {}
#ifdef __EMSCRIPTEN__
    void set_bmp_cache_entries(std::array<CacheEntry, 3> const & /*cache_entries*/) override {}
    void draw(RDPBmpCache         const & /*cmd*/) override {}
    void draw(RDPMemBlt           const & /*cmd*/, Rect /*clip*/) override {}
    void draw(RDPMem3Blt          const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/) override {}
#else
    void draw(RDPMemBlt           const & /*cmd*/, Rect /*clip*/, Bitmap const & /*bmp*/) override {}
    void draw(RDPMem3Blt          const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/, Bitmap const & /*bmp*/) override {}
#endif
    void draw(RDPGlyphIndex       const & /*cmd*/, Rect /*clip*/, ColorCtx /*color_ctx*/, GlyphCache const & /*gly_cache*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const & /*content*/) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow            & /*cmd*/) override {}
    void draw(const RDP::RAIL::WindowIcon                     & /*cmd*/) override {}
    void draw(const RDP::RAIL::CachedIcon                     & /*cmd*/) override {}
    void draw(const RDP::RAIL::DeletedWindow                  & /*cmd*/) override {}
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*cmd*/) override {}
    void draw(const RDP::RAIL::DeletedNotificationIcons       & /*cmd*/) override {}
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & /*cmd*/) override {}
    void draw(const RDP::RAIL::NonMonitoredDesktop            & /*cmd*/) override {}

    void draw(RDPColCache   const & /*cmd*/) override {}
    void draw(RDPBrushCache const & /*cmd*/) override {}

    void new_pointer(gdi::CachePointerIndex cache_idx, const RdpPointerView & cursor) override
    {
        (void)cache_idx;
        (void)cursor;
    }

    void cached_pointer(gdi::CachePointerIndex cache_idx) override
    {
        (void)cache_idx;
    }

public:
    NullGraphic() = default;
};

gdi::GraphicApi& null_gd() noexcept;

}  // namespace gdi
