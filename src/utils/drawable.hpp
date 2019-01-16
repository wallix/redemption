/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
              Poelen Jonathan
*/


#pragma once

#include <memory>

#include "utils/image_data_view.hpp"
#include "utils/colors.hpp"
#include "utils/rect.hpp"


using std::size_t; /*NOLINT*/

enum class DepthColor { color8 = 8, color15 = 15, color16 = 16, color24 = 24, color32 = 32 };

struct DrawableTraitColor24
{
    // 24 bpp
    static const uint8_t Bpp = 3;

    class color_t {
        uint8_t b;
        uint8_t g;
        uint8_t r;

    public:
        constexpr color_t(uint8_t b_, uint8_t g_, uint8_t r_) noexcept
        : b(b_)
        , g(g_)
        , r(r_)
        {}

        constexpr uint8_t red() const noexcept
        { return r; }

        constexpr uint8_t green() const noexcept
        { return g; }

        constexpr uint8_t blue() const noexcept
        { return b; }

        constexpr color_t operator~() const noexcept
        { return {uint8_t(~b), uint8_t(~g), uint8_t(~r)}; }
    };

    static uint8_t * assign(uint8_t * dest, color_t color)
    {
        *dest++ = color.blue();
        *dest++ = color.green();
        *dest++ = color.red();
        return dest;
    }

    template<class BinaryOp>
    static uint8_t * assign(uint8_t * dest, color_t color, BinaryOp op)
    {
        *dest = op(*dest, color.blue());  ++dest;
        *dest = op(*dest, color.green()); ++dest;
        *dest = op(*dest, color.red());   ++dest;
        return dest;
    }

    template<class BinaryOp>
    static uint8_t * assign(uint8_t * dest, color_t color, color_t color2, BinaryOp op)
    {
        *dest = op(*dest, color.blue(),  color2.blue());  ++dest;
        *dest = op(*dest, color.green(), color2.green()); ++dest;
        *dest = op(*dest, color.red(),   color2.red());   ++dest;
        return dest;
    }

    static constexpr color_t u32_to_color(uint32_t color) noexcept
    {
        return {uint8_t(color), uint8_t(color >> 8), uint8_t(color >> 16)};
    }

    static constexpr color_t u32bgr_to_color(uint32_t color) noexcept
    {
        return {uint8_t(color >> 16), uint8_t(color >> 8), uint8_t(color)};
    }

    struct fromColor8
    {
        const BGRPalette & palette;

        color_t operator()(const uint8_t * p) const
        {
            return u32_to_color(this->palette[*p].to_u32() & 0xFFFFFF);
        }
    };

    struct fromColor15
    {
        color_t operator()(const uint8_t * p) const
        {
            const RDPColor c = RDPColor::from((p[1] << 8) + p[0]);
            const BGRColor bgr = decode_color15()(c);
            return {bgr.blue(), bgr.green(), bgr.red()};
        }
    };

    struct fromColor16
    {
        color_t operator()(const uint8_t * p) const
        {
            const RDPColor c = RDPColor::from((p[1] << 8) + p[0]);
            const BGRColor bgr = decode_color16()(c);
            return {bgr.blue(), bgr.green(), bgr.red()};
        }
    };

    struct fromColor24
    {
        color_t operator()(const uint8_t * p) const
        {
            return {p[0], p[1], p[2]};
        }
    };
};

template<DepthColor BppIn>
struct DrawableTrait;

template<>
struct DrawableTrait<DepthColor::color24>
: DrawableTraitColor24
{};

namespace details
{
    std::unique_ptr<uint8_t[]> alloc_data(size_t rowsize, uint16_t height);
}

template<DepthColor BppIn>
struct DrawableImpl
{
    static_assert(BppIn != DepthColor::color8, "8 bit isn't supported");
    static_assert(BppIn != DepthColor::color15, "15 bit isn't supported");
    static_assert(BppIn != DepthColor::color16, "16 bit isn't supported");
    static_assert(BppIn != DepthColor::color32, "32 bit isn't supported");

    uint16_t width_;
    uint16_t height_;
    size_t rowsize_;

    std::unique_ptr<uint8_t[]> data_;

    using traits = DrawableTrait<BppIn>;
    using color_t = typename traits::color_t;

    static const uint8_t Bpp = traits::Bpp;

    DrawableImpl(unsigned width, unsigned height)
    : width_(width)
    , height_(height)
    , rowsize_(width * Bpp)
    , data_(details::alloc_data(this->rowsize_, this->height_))
    {}

    DrawableImpl(DrawableImpl const &) = delete;
    DrawableImpl& operator=(DrawableImpl const &) = delete;

    const uint8_t * data() const noexcept {
        return this->data_.get();
    }

    const uint8_t * data(int x, int y) const noexcept {
        return this->data() + (y * this->width_ + x) * Bpp;
    }

    uint16_t width() const noexcept {
        return this->width_;
    }

    uint16_t height() const noexcept {
        return this->height_;
    }

    unsigned size() const noexcept {
        return this->width_ * this->height_;
    }

    size_t rowsize() const noexcept {
        return this->rowsize_;
    }

    size_t pix_len() const noexcept {
        return this->rowsize_ * this->height_;
    }

    static constexpr uint8_t nbbytes_color() noexcept {
        return uint8_t(Bpp);
    }

    static constexpr uint8_t bpp() noexcept {
        return static_cast<uint8_t>(BppIn);
    }

    uint8_t * first_pixel() const noexcept {
        return this->data_.get();
    }

    uint8_t * first_pixel(int x, int y) const noexcept {
        return this->data_.get() + (y * this->width_ + x) * Bpp;
    }

    uint8_t * first_pixel(Rect rect) const noexcept {
        return this->first_pixel(rect.x, rect.y);
    }

    uint8_t * last_pixel() const noexcept {
        return this->data_.get() + this->pix_len();
    }

    uint8_t * row_data(int y) const noexcept {
        return this->first_pixel() + y * this->rowsize();
    }
};


class Ellipse;
class DrawablePointer;

class Drawable
{
    using DrawableImplPrivate = DrawableImpl<DepthColor::color24>;
    DrawableImplPrivate impl_;

public:
    Rect tracked_area {};
    bool tracked_area_changed = false;

    bool logical_frame_ended = true;

public:

    using Color = DrawableImplPrivate::color_t;
    static const size_t Bpp = DrawableImplPrivate::Bpp;


    Drawable(int width, int height)
    : impl_(width, height)
    {
    }

    void resize(int width, int height);

    uint8_t * first_pixel() noexcept
    {
        return this->impl_.first_pixel();
    }

    const uint8_t * data() const noexcept
    {
        return this->impl_.data();
    }

    Color u32_to_color(uint32_t color) const
    {
        return DrawableImplPrivate::traits::u32_to_color(color);
    }

    Color u32bgr_to_color(uint32_t color) const
    {
        return DrawableImplPrivate::traits::u32bgr_to_color(color);
    }

    const uint8_t * data(int x, int y) const noexcept
    {
        return this->impl_.data(x, y);
    }

    uint16_t width() const noexcept
    {
        return this->impl_.width();
    }

    uint16_t height() const noexcept
    {
        return this->impl_.height();
    }

    unsigned size() const noexcept
    {
        return this->impl_.size();
    }

    size_t rowsize() const noexcept
    {
        return this->impl_.rowsize();
    }

    size_t pix_len() const noexcept
    {
        return this->impl_.pix_len();
    }

    static constexpr uint8_t nbbytes_color() noexcept
    {
        return DrawableImplPrivate::nbbytes_color();
    }

    static constexpr uint8_t bpp() noexcept
    {
        return DrawableImplPrivate::bpp();
    }

    /*
     * The name doesn't say it : mem_blt COPIES a decoded bitmap from
     * a cache (data) and insert a subpart (srcx, srcy) to the local
     * image cache (this->impl_.first_pixel()) a the given position (rect).
     */
    void mem_blt(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy);

    void mem_blt_invert(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy);

    void mem_blt_ex( Rect rect
                   , const ConstImageDataView & bmp
                   , const uint16_t srcx
                   , const uint16_t srcy
                   , uint8_t rop);

    void draw_bitmap(Rect rect, const ConstImageDataView & bmp);

    void mem_3_blt( Rect rect
                  , const ConstImageDataView & bmp
                  , const uint16_t srcx
                  , const uint16_t srcy
                  , uint8_t rop
                  , const Color pattern_color);

    void black_color(Rect rect);
    void white_color(Rect rect);
    void invert_color(Rect rect);

// 2.2.2.2.1.1.1.6 Binary Raster Operation (ROP2_OPERATION)
//  The ROP2_OPERATION structure is used to define how the bits in a destination bitmap and a
//  selected brush or pen are combined by using Boolean operators.
// rop2Operation (1 byte): An 8-bit, unsigned integer. A raster-operation code that describes a
//  Boolean operation, in Reverse Polish Notation, to perform on the bits in a destination
//  bitmap (D) and selected brush or pen (P). This operation is a combination of the AND (a), OR
//  (o), NOT (n), and XOR (x) Boolean operators.
    void ellipse(const Ellipse & el, const uint8_t rop, const uint8_t fill, const Color color);

    // low level opaquerect,
    // mostly avoid clipping because we already took care of it
    // also we already swapped color if we are using BGR instead of RGB
    void opaquerect(Rect rect, const Color color);

    void draw_pixel(int16_t x, int16_t y, const Color color);

    // low level patblt,
    // mostly avoid clipping because we already took care of it
    void patblt(Rect rect, const uint8_t rop, const Color color);
    void patblt_ex(Rect rect, const uint8_t rop,
        const Color back_color, const Color fore_color,
        const uint8_t * brush_data, int8_t org_x, int8_t org_y);

    // low level destblt,
    // mostly avoid clipping because we already took care of it
    void destblt(Rect rect, const uint8_t rop);

    // low level scrblt, mostly avoid considering clipping
    // because we already took care of it
    void scrblt(unsigned srcx, unsigned srcy, Rect drect, uint8_t rop);

    void draw_line(
        int mix_mode,
        int16_t xStart, int16_t yStart,
        int16_t xEnd, int16_t yEnd,
        uint8_t rop, Color color, Rect clip
    );

    // nor horizontal nor vertical, use Bresenham
    void diagonal_line(int mix_mode, int x, int y, int endx, int endy, uint8_t rop, Color color);

    void vertical_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endy, uint8_t rop, Color color);

    void horizontal_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endx, uint8_t rop, Color color);

    void set_row(size_t rownum, const uint8_t * data, size_t data_length);

    void trace_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave);
    void clear_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave);

    operator ConstImageDataView () const
    {
        return ConstImageDataView{
            this->data(),
            this->width(), this->height(),
            this->rowsize(),
            ConstImageDataView::BytesPerPixel(this->Bpp),
            ConstImageDataView::Storage::TopToBottom
        };
    }

private:
    template <typename Op, class... Color>
    void mem_blt_op(
        Rect rect , const ConstImageDataView & bmp,
        const uint16_t srcx, const uint16_t srcy, Color... c);

    template <typename Op>
    void patblt_op(Rect rect, const Color color);

    template <typename Op>
    void patblt_op_ex(Rect rect, const uint8_t * brush_data, int8_t org_x, int8_t org_y,
        const Color back_color, const Color fore_color);

    template <typename Op>
    void scr_blt_op(uint16_t srcx, uint16_t srcy, Rect drect);
};

namespace gdi
{
    inline const MutableImageDataView get_mutable_image_view(Drawable & drawable)
    {
        return MutableImageDataView{
            drawable.first_pixel(),
            drawable.width(), drawable.height(),
            drawable.rowsize(),
            MutableImageDataView::BytesPerPixel(drawable.Bpp),
            MutableImageDataView::Storage::TopToBottom
        };
    }

    inline ConstImageDataView get_image_view(Drawable const & drawable)
    {
        return drawable;
    }
} // namespace gdi
