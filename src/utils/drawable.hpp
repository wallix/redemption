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

#include <utility>
#include <memory>

#include "utils/log.hpp"
#include "core/error.hpp"
#include "utils/image_data_view.hpp"
#include "utils/colors.hpp"
#include "utils/rect.hpp"
#include "utils/ellipse.hpp"
#include "utils/bitfu.hpp"
#include "utils/drawable_pointer.hpp"
#include "gdi/image_frame_api.hpp"

using std::size_t;

namespace Ops {
    using u8 = uint8_t;

    struct CopySrc
    {
       constexpr u8 operator()(u8 /*target*/, u8 source) const noexcept
       {
           return source;
       }
    };

    struct InvertSrc
    {
       constexpr u8 operator()(u8 /*target*/, u8 source) const noexcept
       {
           return ~source;
       }
    };

    struct InvertTarget
    {
       constexpr u8 operator()(u8 target, u8 /*source*/) const noexcept
       {
           return ~target;
       }
    };


    struct Op_0xB8 // PSDPxax
    {
        constexpr u8 operator()(u8 target, u8 source, u8 pattern) const noexcept
        {
            return ((target ^ pattern) & source) ^ pattern;
        }
    };

// 2.2.2.2.1.1.1.6 Binary Raster Operation (ROP2_OPERATION)
//  The ROP2_OPERATION structure is used to define how the bits in a destination bitmap and a
//  selected brush or pen are combined by using Boolean operators.
// rop2Operation (1 byte): An 8-bit, unsigned integer. A raster-operation code that describes a
//  Boolean operation, in Reverse Polish Notation, to perform on the bits in a destination
//  bitmap (D) and selected brush or pen (P). This operation is a combination of the AND (a), OR
//  (o), NOT (n), and XOR (x) Boolean operators.

    struct Op2_0x01 // R2_BLACK 0
    {
       constexpr u8 operator()(u8 /*target*/, u8 /*source*/) const noexcept
       {
           return 0x00;
       }
    };

    struct Op2_0x02 // R2_NOTMERGEPEN DPon
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return ~(target | source);
        }
    };

    struct Op2_0x03 // R2_MASKNOTPEN DPna
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (target & ~source);
        }
    };

    typedef InvertSrc Op2_0x04; // R2_NOTCOPYPEN Pn

    struct Op2_0x05 // R2_MASKPENNOT PDna
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (source & ~target);
        }
    };

    typedef InvertTarget Op2_0x06; // R2_NOT Dn

    struct Op2_0x07 // R2_XORPEN DPx
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (target ^ source);
        }
    };

    struct Op2_0x08 // R2_NOTMASKPEN DPan
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return ~(target & source);
        }
    };

    struct Op2_0x09 // R2_MASKPEN DPa
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (target & source);
        }
    };

    struct Op2_0x0A // R2_NOTXORPEN DPxn
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return ~(target ^ source);
        }
    };

    // struct Op2_0x0B // R2_NOP D
    // {
    //     constexpr u8 operator()(u8 target, u8 source) const noexcept
    //     {
    //         return target;
    //     }
    // };

    struct Op2_0x0C // R2_MERGENOTPEN DPno
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (target | ~source);
        }
    };

    typedef CopySrc Op2_0x0D; // R2_COPYPEN P

    struct Op2_0x0E // R2_MERGEPENNOT PDno
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (source | ~target);
        }
    };

    struct Op2_0x0F // R2_MERGEPEN PDo
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (target | source);
        }
    };

    struct Op2_0x10 // R2_WHITE 1
    {
       constexpr u8 operator()(u8 /*target*/, u8 /*source*/) const noexcept
       {
           return 0xFF;
       }
    };


    typedef Op2_0x02 Op_0x05;
    typedef InvertSrc Op_0x0F;
    typedef Op2_0x05 Op_0x50;
    typedef Op2_0x07 Op_0x5A;
    typedef Op2_0x08 Op_0x5F;
    typedef Op2_0x09 Op_0xA0;
    typedef Op2_0x0A Op_0xA5;
    typedef Op2_0x0C Op_0xAF;
    typedef CopySrc Op_0xF0;
    typedef Op2_0x0E Op_0xF5;
    typedef Op2_0x0F Op_0xFA;

    struct Op_0x11 // NOTSRCERASE DSon
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return ~(target | source);
        }
    };

    typedef Op2_0x03 Op_0x22;
    typedef InvertSrc Op_0x33;
    typedef Op2_0x05 Op_0x44;
    typedef Op2_0x07 Op_0x66;
    typedef Op2_0x08 Op_0x77;
    typedef Op2_0x09 Op_0x88;
    typedef Op2_0x0A Op_0x99;
    typedef Op2_0x0C Op_0xBB;
    typedef CopySrc Op_0xCC;
    typedef Op2_0x0E Op_0xDD;
    typedef Op2_0x0F Op_0xEE;
}


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

auto DrawableImplAlloc = [](size_t rowsize_, uint16_t height_) {
        size_t sz = rowsize_ * height_;
        if (sz == 0) {
            throw Error(ERR_RECORDER_EMPTY_IMAGE);
        }
        uint8_t * data = new (std::nothrow) uint8_t[sz]; /*NOLINT*/
        // done this way because otherwise clang raise a zero-size-array is an extension warning
        memset(data, 0, sz);

        if (nullptr == data) {
            throw Error(ERR_RECORDER_FRAME_ALLOCATION_FAILED);
        }
        return data;
    };

template<DepthColor BppIn>
class DrawableImpl
{
    static_assert(BppIn != DepthColor::color8, "8 bit isn't supported");
    static_assert(BppIn != DepthColor::color15, "15 bit isn't supported");
    static_assert(BppIn != DepthColor::color16, "16 bit isn't supported");
    static_assert(BppIn != DepthColor::color32, "32 bit isn't supported");

    using u8 = uint8_t;
    using u16 = uint16_t;
    using P = u8 *;
    using cP = u8 const *;

    uint16_t width_;
    uint16_t height_;
    size_t rowsize_;

    P data_;

public:
    using traits = DrawableTrait<BppIn>;
    using color_t = typename traits::color_t;

    static const uint8_t Bpp = traits::Bpp;

    DrawableImpl(unsigned width, unsigned height)
    : width_(width)
    , height_(height)
    , rowsize_(width * Bpp)
    // , data_([this]{
    //     size_t sz = this->rowsize_ * this->height_;
    //     if (sz == 0) {
    //         throw Error(ERR_RECORDER_EMPTY_IMAGE);
    //     }
    //     uint8_t * data = new (std::nothrow) uint8_t[sz]; /*NOLINT*/
    //     // done this way because otherwise clang raise a zero-size-array is an extension warning
    //     memset(data, 0, sz);
    //     if (nullptr == data) {
    //         throw Error(ERR_RECORDER_FRAME_ALLOCATION_FAILED);
    //     }
    //     return data;
    // }())
    , data_(DrawableImplAlloc(this->rowsize_, this->height_))
    {}

    DrawableImpl(DrawableImpl const &) = delete;
    DrawableImpl& operator=(DrawableImpl const &) = delete;

public:
    ~DrawableImpl()
    {
        delete[] this->data_;
    }

    void resize(unsigned width, unsigned height) {
        uint16_t saved_height_  = this->height_;
        size_t   saved_rowsize_ = this->rowsize_;
        P        saved_data_    = this->data_;


        this->width_   = width;
        this->height_  = height;
        this->rowsize_ = this->width_ * Bpp;
        this->data_    =  DrawableImplAlloc(this->rowsize_, this->height_);

        P      src         = saved_data_;
        P      dest        = this->data_;
        size_t min_rowsize = std::min(saved_rowsize_, this->rowsize_);
        for (uint16_t row_index = 0, row_count = std::min(saved_height_, this->height_);
             row_index < row_count; ++row_index) {
            memcpy(dest, src, min_rowsize);

            src  += saved_rowsize_;
            dest += this->rowsize_;
        }

        delete[] saved_data_;
    }

    const uint8_t * data() const noexcept {
        return this->data_;
    }

    const uint8_t * data(int x, int y) const noexcept {
        return this->data_ + (y * this->width_ + x) * Bpp;
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
        return this->data_;
    }

    uint8_t * first_pixel(int x, int y) const noexcept {
        return this->data_ + (y * this->width_ + x) * Bpp;
    }

    uint8_t * first_pixel(Rect rect) const noexcept {
        return this->first_pixel(rect.x, rect.y);
    }

    uint8_t * last_pixel() const noexcept {
        return this->data_ + this->pix_len();
    }

    uint8_t * row_data(int y) const noexcept {
        return this->first_pixel() + y * this->rowsize();
    }

private:
    template<class>
    struct AssignOp;
    struct Assign;
    struct Invert;

public:
    void opaque_rect(Rect rect, const color_t color)
    {
        P const base = this->first_pixel(rect);

        apply_for_line(base, rect.cx, Assign{color});

        P target = base;
        const size_t line_size = this->rowsize();
        cP pe = target + line_size * rect.cy;
        while ((target += line_size) < pe) {
            memcpy(target, base, rect.cx * Bpp);
        }
    }

    void draw_pixel(int16_t x, int16_t y, const color_t color)
    {
        traits::assign(this->first_pixel(x, y), color);
    }

    template<class Op, class... Col>
    void mem_blt(Rect rect, const ConstImageDataView & bmp, const uint16_t srcx, const uint16_t srcy, Op op, Col... c)
    {
        // TODO implements with ConstImageDataView::Storage::TopToBottom
        assert(bmp.storage_type() == ConstImageDataView::Storage::BottomToTop && "other is unimplemented");

        P dest = this->first_pixel(rect);
        cP src = bmp.data(srcx, bmp.height() - srcy - 1);
        const size_t n = rect.cx * Bpp;
        const uint8_t bmp_bpp = bmp.bits_per_pixel();
        const size_t bmp_line_size = bmp.line_size();

        if (bmp_bpp == this->bpp()) {
            const size_t line_size = this->rowsize();
            for (cP ep = dest + line_size * rect.cy; dest < ep; dest += line_size, src -= bmp_line_size) {
                this->copy(dest, src, n, op, c...);
            }
        }
        else {
            switch (bmp_bpp) {
                case 8: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                    bmp.bytes_per_pixel(), bmp_line_size, op, typename traits::fromColor8{bmp.palette()}, c...); break;
                case 15: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                    bmp.bytes_per_pixel(), bmp_line_size, op, typename traits::fromColor15{}, c...); break;
                case 16: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                    bmp.bytes_per_pixel(), bmp_line_size, op, typename traits::fromColor16{}, c...); break;
                case 24: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                    bmp.bytes_per_pixel(), bmp_line_size, op, typename traits::fromColor24{}, c...); break;
                default: ;
            }
        }
    }

private:
    template<class Op, class ToColor, class... Col>
    void spe_mem_blt(
        P dest, cP src, u16 cx, u16 cy, size_t bmp_Bpp, size_t bmp_line_size, Op op, ToColor to_color, Col... c)
    {
        const size_t line_size = this->rowsize();
        const size_t destn = cx * Bpp;
        const size_t srcn = cx * bmp_Bpp;
        const size_t inc_line = line_size - destn;
        const size_t inc_bmp_line = bmp_line_size + srcn;

        for (cP ep = dest + line_size * cy; dest < ep; dest += inc_line, src -= inc_bmp_line) {
            const cP dest_e = dest + destn;
            while (dest != dest_e) {
                dest = traits::assign(dest, to_color(src), c..., op);
                src += bmp_Bpp;
            }
        }
    }

public:
    void draw_bitmap(Rect rect, ConstImageDataView bmp)
    {
        this->mem_blt(rect, bmp, 0, 0, Ops::CopySrc{});
    }

    void component_rect(Rect rect, uint8_t c)
    {
        P p = this->first_pixel(rect);
        const size_t step = this->rowsize();
        const size_t n = rect.cx * Bpp;
        for (cP pe = p + rect.cy * step; p < pe; p += step) {
            memset(p, c, n);
        }
    }

    template<typename Op2>
    void draw_ellipse(const Ellipse & el, const uint8_t fill, const color_t color)
    {
        Op2 op;
        const int cX = el.center_x();
        const int cY = el.center_y();
        const int rX = el.radius_x();
        const int rY = el.radius_y();
        const int rXcarre = rX*rX;
        const int rYcarre = rY*rY;
        int errX = 0;
        int pX = rX;
        int pY = 0;
        int borX = rYcarre*rX;
        int borY = 0;

        this->colordot(cX+pX, cY, color, op);
        this->colordot(cX-pX, cY, color, op);
        if (fill) {
            this->colorline(cX-pX + 1, cY, 2*pX - 1, color, op);
        }
        if (errX > pX*rYcarre) {
            errX -= (2*pX - 1)*rYcarre;
            pX--;
            borX -= rYcarre;
        }
        errX += (2*pY + 1)*rXcarre;
        pY++;
        borY += rXcarre;
        int lastchange = 0;
        while ((borX > borY) && (pY <= rY)) {
            lastchange = 0;
            this->colordot(cX+pX, cY+pY, color, op);
            this->colordot(cX+pX, cY-pY, color, op);
            this->colordot(cX-pX, cY+pY, color, op);
            this->colordot(cX-pX, cY-pY, color, op);
            if (fill) {
                this->colorline(cX-pX + 1, cY+pY, 2*pX - 1, color, op);
                this->colorline(cX-pX + 1, cY-pY, 2*pX - 1, color, op);
            }
            if (errX > pX*rYcarre) {
                errX -= (2*pX - 1)*rYcarre;
                pX--;
                borX -= rYcarre;
                lastchange = 1;
            }
            errX += (2*pY + 1)*rXcarre;
            pY++;
            borY += rXcarre;
        }
        int lastpX = pX + lastchange;
        int lastpY = pY - 1;
        int errY = 0;
        pX = 0;
        pY = rY;
        if ((fill && ((pX < lastpX) && (pY > lastpY))) ||
            (!fill && ((pX < lastpX) || (pY > lastpY)))) {
            this->colordot(cX, cY+pY, color, op);
            this->colordot(cX, cY-pY, color, op);
            if (errY > pY*rXcarre) {
                errY -= (2*pY - 1)*rXcarre;
                pY--;
                if (fill && pY > lastpY) {
                    this->colorline(cX, cY + pY, 2*pX + 1, color, op);
                }
            }
            errY += (2*pX + 1)*rYcarre;
            pX++;
        }
        while (((fill && (pX <= lastpX && pY > lastpY)) ||
                (!fill && ((pX < lastpX) || (pY > lastpY)))) &&
               (pX <= rX)) {
            this->colordot(cX+pX, cY+pY, color, op);
            this->colordot(cX+pX, cY-pY, color, op);
            this->colordot(cX-pX, cY+pY, color, op);
            this->colordot(cX-pX, cY-pY, color, op);
            if (errY > pY*rXcarre) {
                errY -= (2*pY - 1)*rXcarre;
                pY--;
                if (fill && (pY > lastpY)) {
                    this->colorline(cX-pX, cY+pY, 2*pX+1, color, op);
                    this->colorline(cX-pX, cY-pY, 2*pX+1, color, op);
                }
            }
            errY += (2*pX + 1)*rYcarre;
            pX++;
        }
    }

private:
    template <typename Op2>
    void colordot(int x, int y, color_t color, Op2 op2)
    {
        if (!(x >= 0 &&
              y >= 0 &&
              x < this->width() &&
              y < this->height())) {
            return;
        }
        P p = this->first_pixel(x, y);
        traits::assign(p, color, op2);
    }

    template <typename Op2>
    void colorline(int x, int y, int l, color_t color, Op2)
    {
        if (!(y >= 0 &&
              y < this->height())) {
                return;
        }
        if (x < 0) {
            l += x;
            x = 0;
        }
        else if ((x + l) >= this->width()) {
            l = this->width() - x;
        }
        P p = this->first_pixel(x, y);
        this->apply_for_line(p, l, AssignOp<Op2>{color});
    }

public:
    template <typename Op>
    void patblt_op_ex(
        Rect rect, const uint8_t * brush_data, int8_t org_x, int8_t org_y,
        const color_t back_color, const color_t fore_color)
    {
        // TODO org_x is not used
        (void)org_x;
        // TODO org_y is not used
        (void)org_y;

        P const base = this->first_pixel(rect);
        P       p    = base;

        int const origin_x = rect.x/* - org_x*/;
        int const origin_y = rect.y/* - org_y*/;

        for (size_t y = 0, cy = rect.cy; y < cy; ++y) {
            p = base + this->rowsize() * y;
            const uint8_t brush = brush_data[(y + origin_y) % 8];
            for (size_t x = 0, cx = rect.cx; x < cx; ++x) {
                if (brush & ((1 << 7) >> ((x + origin_x) % 8))) {
                    p = traits::assign(p, back_color, Op());
                }
                else {
                    p = traits::assign(p, fore_color, Op());
                }
            }
        }
    }

    template <typename Op>
    void scr_blt_op(Rect rect, uint16_t srcx, uint16_t srcy)
    {
        const int16_t deltax = static_cast<int16_t>(srcx - rect.x);
        const int16_t deltay = static_cast<int16_t>(srcy - rect.y);
        const Rect srect = rect.offset(deltax, deltay);
        const bool is_overlap = srect.has_intersection(rect);

        if (is_overlap) {
            const Rect overlap = srect.intersect(rect);
            if (rect.y < srcy) {
                this->scr_blt_op_nooverlap(rect, srcx, srcy, Op());
            }
            else if (rect.y == srcy) {
                this->scr_blt_op_overlap(rect, srcx, srcy, Op());
            }
            else {
                if (srcy + overlap.cy > overlap.y) {
                    P dest = this->first_pixel(rect.x, rect.bottom());
                    cP src = this->first_pixel(srcx, srcy + rect.cy);
                    cP esrc = src - this->rowsize() * (srcy + rect.cy - overlap.y);
                    while (src > esrc) {
                        dest -= this->rowsize();
                        src -= this->rowsize();
                        this->copy(dest, src, rect.cx * this->Bpp, Op());
                    }
                }
                else {
                    const Rect dest1(rect.x, rect.bottom() - ((srcy + rect.cy - overlap.y)), rect.cx, (srcy + rect.cy - overlap.y));
                    this->scr_blt_op_nooverlap(dest1, dest1.x + deltax, dest1.y + deltay, Op());
                }

                const Rect dest_safe(rect.x, rect.y, rect.cx, rect.cy - overlap.cy);
                this->scr_blt_op_nooverlap(dest_safe, dest_safe.x + deltax, dest_safe.y + deltay, Op());
            }
        }
        else {
            this->scr_blt_op_nooverlap(rect, srect.x, srect.y, Op());
        }
    }

private:
    void scr_blt_op_overlap(Rect const rect_dest, size_t srcx, size_t srcy, Ops::CopySrc)
    {
        this->scr_blt_impl(rect_dest, srcx, srcy, [](P dest, cP src, size_t n) {
            memmove(dest, src, n);
        });
    }

    template <typename Op>
    void scr_blt_op_overlap(Rect const rect_dest, size_t srcx, size_t srcy, Op op)
    {
        P dest = this->first_pixel(rect_dest);
        cP src = this->first_pixel(srcx, srcy);
        const size_t n = rect_dest.cx * Bpp;
        if (src + n < dest || dest < src) {
            this->scr_blt_op_nooverlap(rect_dest, srcx, srcy, op);
        }
        else {
            const size_t d = static_cast<size_t>((src + n) - dest);
            if (dest + d > dest - (n - d)) {
                this->scr_blt_impl(dest, src, n, rect_dest.cy, [this, d, op](P dest, cP src, size_t n) {
                    P edest = dest + n;
                    cP esrc = src + n;
                    while (edest > dest + (n - d)) {
                        --edest;
                        --esrc;
                        *edest = op(*edest, *esrc);
                    }
                    this->copy(dest, src, n - d, op);
                });
            }
            else {
                this->scr_blt_impl(dest, src, n, rect_dest.cy, [this, d, op](P dest, cP src, size_t n) {
                    this->copy(dest + (n - d), src + (n - d), d, op);
                    this->copy(dest, src, n - d, op);
                });
            }
        }
    }

    template <typename Op>
    void scr_blt_op_nooverlap(Rect const rect_dest, size_t srcx, size_t srcy, Op op)
    {
        this->scr_blt_impl(rect_dest, srcx, srcy, [this, op](P dest, cP src, size_t n) {
            this->copy(dest, src, n, op);
        });
    }

    template <typename F>
    void scr_blt_impl(Rect const rect_dest, size_t srcx, size_t srcy, F f)
    {
        this->scr_blt_impl(this->first_pixel(rect_dest), this->first_pixel(srcx, srcy), rect_dest.cx * Bpp, rect_dest.cy, f);
    }

    template <typename F>
    void scr_blt_impl(P dest, cP src, size_t n, size_t cy, F f)
    {
        for (P e = dest + this->rowsize() * cy; e != dest; ) {
            f(dest, src, n);
            dest += this->rowsize();
            src += this->rowsize();
        }
    }

public:
    // nor horizontal nor vertical, use Bresenham
    template<class Op>
    void diagonal_line(int x, int y, int endx, int endy, color_t color, Op op)
    {
        assert(x <= endx);

        // Prep
        const int dx = endx - x;
        const int dy = (endy >= y) ? (endy - y) : (y - endy);
        const int sy = (endy >= y) ? 1 : -1;
        int err = dx - dy;

        while (true) {
            traits::assign(this->first_pixel(x, y), color, op);

            if ((x >= endx) && (y == endy)) {
                break;
            }

            // Calculating pixel position
            const int e2 = err * 2; //prevents use of floating point
            if (e2 > -dy) {
                err -= dy;
                x++;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
    }

    template<class Op>
    void vertical_line(uint16_t x, uint16_t y, uint16_t endy, color_t color, Op op)
    {
        assert(y <= endy);

        P p = this->first_pixel(x, y);
        P pe = p + (endy - y + 1) * this->rowsize();
        for (; p != pe; p += this->rowsize()) {
            traits::assign(p, color, op);
        }
    }

    template<class Op>
    void horizontal_line(uint16_t x, uint16_t y, uint16_t endx, color_t color, Op)
    {
        assert(x <= endx);

        this->apply_for_line(this->first_pixel(x, y), endx - x + 1, AssignOp<Op>{color});
    }

    template <typename Op>
    void patblt_op(Rect rect, color_t color, Op)
    {
        this->apply_for_rect(rect, AssignOp<Op>{color});
    }

    void patblt_op(Rect rect, color_t color, Ops::InvertSrc)
    {
        this->apply_for_rect(rect, Assign{~color});
    }

    void patblt_op(Rect rect, color_t color, Ops::CopySrc)
    {
        this->apply_for_rect(rect, Assign{color});
    }

    void invert_color(Rect rect)
    {
        this->apply_for_rect(rect, Invert{});
    }

private:
    struct Assign {
        color_t color;

        P operator()(P dest) const
        { return traits::assign(dest, color); }
    };

    template<class Op>
    struct AssignOp {
        color_t color;

        P operator()(P dest) const
        { return traits::assign(dest, color, Op()); }
    };

    struct Invert {
        P operator()(P dest) const
        { *dest ^= 0xff; return ++dest; }
    };

    template<class Op>
    void copy(uint8_t * dest, const uint8_t * src, size_t n, Op op)
    {
        const uint8_t * e = dest + n;
        for (; dest != e; ++dest, ++src) {
            *dest = op(*dest, *src);
        }
    }

    void copy(uint8_t * dest, const uint8_t * src, size_t n, Ops::CopySrc)
    {
       memcpy(dest, src, n);
    }

    template<class Op>
    void copy(uint8_t * dest, const uint8_t * src, size_t n, Op op, color_t c)
    {
        const uint8_t * e = dest + n;
        while (dest != e) {
            *dest = op(*dest, *src, c.blue());  ++dest; ++src;
            *dest = op(*dest, *src, c.green()); ++dest; ++src;
            *dest = op(*dest, *src, c.red());   ++dest; ++src;
        }
    }

    template<class F>
    P apply_for_line(P p, size_t n, F f)
    {
        for (cP pe = p + n * Bpp; p != pe; ) {
            p = f(p);
        }
        return p;
    }

    template<class F>
    void apply_for_rect(Rect rect, F f)
    {
        P p = this->first_pixel(rect);
        const size_t line_size = this->rowsize();
        const size_t cx = rect.cx * Bpp;
        const size_t inc_line = static_cast<size_t>(line_size - cx);
        for (cP pe = p + rect.cy * line_size; p != pe; p += inc_line) {
            for (cP pe2 = p + cx; p != pe2; ) {
                p = f(p);
            }
        }
    }
};


class Drawable
{
    using DrawableImplPrivate = DrawableImpl<DepthColor::color24>;
    DrawableImplPrivate impl_;
    DrawableImplPrivate & impl() noexcept { return this->impl_; }
    const DrawableImplPrivate & impl() const noexcept { return this->impl_; }

public:
    Rect tracked_area;
    bool tracked_area_changed;

    bool logical_frame_ended;

public:

    using Color = DrawableImplPrivate::color_t;
    static const size_t Bpp = DrawableImplPrivate::Bpp;


    Drawable(int width, int height)
    : impl_(width, height)
    , tracked_area(0, 0, 0, 0)
    , tracked_area_changed(false)
    , logical_frame_ended(true)
    {
    }

    void resize(int width, int height) {
        this->impl_.resize(width, height);

        this->tracked_area         = Rect(0, 0, 0, 0);
        this->tracked_area_changed = false;

        this->logical_frame_ended = true;
    }

    uint8_t * first_pixel() noexcept {
        return this->impl().first_pixel();
    }

    const uint8_t * data() const noexcept {
        return this->impl().data();
    }

    Color u32_to_color(uint32_t color) const {
        return DrawableImplPrivate::traits::u32_to_color(color);
    }

    Color u32bgr_to_color(uint32_t color) const {
        return DrawableImplPrivate::traits::u32bgr_to_color(color);
    }

    const uint8_t * data(int x, int y) const noexcept {
        return this->impl().data(x, y);
    }

    uint16_t width() const noexcept {
        return this->impl().width();
    }

    uint16_t height() const noexcept {
        return this->impl().height();
    }

    unsigned size() const noexcept {
        return this->impl().size();
    }

    size_t rowsize() const noexcept {
        return this->impl().rowsize();
    }

    size_t pix_len() const noexcept {
        return this->impl().pix_len();
    }

    static constexpr uint8_t nbbytes_color() noexcept {
        return DrawableImplPrivate::nbbytes_color();
    }

    static constexpr uint8_t bpp() noexcept {
        return DrawableImplPrivate::bpp();
    }

public:
    /*
     * The name doesn't say it : mem_blt COPIES a decoded bitmap from
     * a cache (data) and insert a subpart (srcx, srcy) to the local
     * image cache (this->impl().first_pixel()) a the given position (rect).
     */
    void mem_blt(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy) {
        this->mem_blt_op<Ops::CopySrc>(rect, bmp, srcx, srcy);
    }

    void mem_blt_invert(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy) {
        this->mem_blt_op<Ops::InvertSrc>(rect, bmp, srcx, srcy);
    }

private:
    template <typename Op, class... Color>
    void mem_blt_op( Rect rect
                   , const ConstImageDataView & bmp
                   , const uint16_t srcx
                   , const uint16_t srcy
                   , Color... c) {
        if (bmp.width() < srcx || bmp.height() < srcy) {
            return ;
        }

        const int16_t mincx = std::min<int16_t>(bmp.width() - srcx, std::min<int16_t>(this->width() - rect.x, rect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.height() - srcy, std::min<int16_t>(this->height() - rect.y, rect.cy));

        if (mincx <= 0 || mincy <= 0) {
            return;
        }
        const Rect trect(rect.x, rect.y, mincx, mincy);

        if (this->tracked_area.has_intersection(trect)) {
            this->tracked_area_changed = true;
        }

        this->impl().mem_blt(trect, bmp, srcx, srcy, Op(), c...);
    }

public:
    void mem_blt_ex( Rect rect
                   , const ConstImageDataView & bmp
                   , const uint16_t srcx
                   , const uint16_t srcy
                   , uint8_t rop) {
        switch (rop) {
            // +------+-------------------------------+
            // | 0x22 | ROP: 0x00220326               |
            // |      | RPN: DSna                     |
            // +------+-------------------------------+
            case 0x22:
                this->mem_blt_op<Ops::Op_0x22>(rect, bmp, srcx, srcy);
                break;
            // +------+-------------------------------+
            // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
            // |      | RPN: DSx                      |
            // +------+-------------------------------+
            case 0x66:
                this->mem_blt_op<Ops::Op_0x66>(rect, bmp, srcx, srcy);
                break;
            // +------+-------------------------------+
            // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
            // |      | RPN: DSa                      |
            // +------+-------------------------------+
            case 0x88:
                this->mem_blt_op<Ops::Op_0x88>(rect, bmp, srcx, srcy);
                break;
            // +------+-------------------------------+
            // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
            // |      | RPN: DSo                      |
            // +------+-------------------------------+
            case 0xEE:
                this->mem_blt_op<Ops::Op_0xEE>(rect, bmp, srcx, srcy);
                break;
            // +------+-------------------------------+
            // | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
            // |      | RPN: DSno                     |
            // +------+-------------------------------+
            case 0xBB:
                this->mem_blt_op<Ops::Op_0xBB>(rect, bmp, srcx, srcy);
                break;

            default:
                LOG(LOG_INFO, "Drawable::mem_blt_ex(): unimplemented rop=%X", rop);
                break;
        }
    }

    void draw_bitmap(Rect rect, const ConstImageDataView & bmp) {
        this->mem_blt_op<Ops::CopySrc>(rect, bmp, 0, 0);
    }

    void mem_3_blt( Rect rect
                  , const ConstImageDataView & bmp
                  , const uint16_t srcx
                  , const uint16_t srcy
                  , uint8_t rop
                  , const Color pattern_color) {
        switch (rop) {
            // +------+-------------------------------+
            // | 0xB8 | ROP: 0x00B8074A               |
            // |      | RPN: PSDPxax                  |
            // +------+-------------------------------+
            case 0xB8:
                this->mem_blt_op<Ops::Op_0xB8>(rect, bmp, srcx, srcy, pattern_color);
            break;

            default:
                LOG(LOG_INFO, "Drawable::mem_3_blt(): unimplemented rop=%X", rop);
            break;
        }
    }

    void black_color(Rect rect)
    {
        const Rect trect = rect.intersect(this->width(), this->height());

        if (this->tracked_area.has_intersection(trect)) {
            this->tracked_area_changed = true;
        }

        this->impl().component_rect(trect, 0);
    }

    void white_color(Rect rect)
    {
        const Rect trect = rect.intersect(this->width(), this->height());

        if (this->tracked_area.has_intersection(rect)) {
            this->tracked_area_changed = true;
        }

        this->impl().component_rect(trect, 0xFF);
    }

private:
    void invert_color(Rect rect)
    {
        const Rect trect = rect.intersect(this->width(), this->height());

        if (this->tracked_area.has_intersection(trect)) {
            this->tracked_area_changed = true;
        }

        this->impl().invert_color(trect);
    }

// 2.2.2.2.1.1.1.6 Binary Raster Operation (ROP2_OPERATION)
//  The ROP2_OPERATION structure is used to define how the bits in a destination bitmap and a
//  selected brush or pen are combined by using Boolean operators.
// rop2Operation (1 byte): An 8-bit, unsigned integer. A raster-operation code that describes a
//  Boolean operation, in Reverse Polish Notation, to perform on the bits in a destination
//  bitmap (D) and selected brush or pen (P). This operation is a combination of the AND (a), OR
//  (o), NOT (n), and XOR (x) Boolean operators.

public:
    void ellipse(const Ellipse & el, const uint8_t rop, const uint8_t fill, const Color color) {
        if (this->tracked_area.has_intersection(el.get_rect())) {
            this->tracked_area_changed = true;
        }
        switch (rop) {
        case 0x01: // R2_BLACK
            this->impl().draw_ellipse<Ops::Op2_0x01>(el, fill, color);
            break;
        case 0x02: // R2_NOTMERGEPEN
            this->impl().draw_ellipse<Ops::Op2_0x02>(el, fill, color);
            break;
        case 0x03: // R2_MASKNOTPEN
            this->impl().draw_ellipse<Ops::Op2_0x03>(el, fill, color);
            break;
        case 0x04: // R2_NOTCOPYPEN
            this->impl().draw_ellipse<Ops::Op2_0x04>(el, fill, color);
            break;
        case 0x05: // R2_MASKPENNOT
            this->impl().draw_ellipse<Ops::Op2_0x05>(el, fill, color);
            break;
        case 0x06:  // R2_NOT
            this->impl().draw_ellipse<Ops::Op2_0x06>(el, fill, color);
            break;
        case 0x07:  // R2_XORPEN
            this->impl().draw_ellipse<Ops::Op2_0x07>(el, fill, color);
            break;
        case 0x08:  // R2_NOTMASKPEN
            this->impl().draw_ellipse<Ops::Op2_0x08>(el, fill, color);
            break;
        case 0x09:  // R2_MASKPEN
            this->impl().draw_ellipse<Ops::Op2_0x09>(el, fill, color);
            break;
        case 0x0A:  // R2_NOTXORPEN
            this->impl().draw_ellipse<Ops::Op2_0x0A>(el, fill, color);
            break;
        case 0x0B:  // R2_NOP
            break;
        case 0x0C:  // R2_MERGENOTPEN
            this->impl().draw_ellipse<Ops::Op2_0x0C>(el, fill, color);
            break;
        case 0x0D:  // R2_COPYPEN
            this->impl().draw_ellipse<Ops::Op2_0x0D>(el, fill, color);
            break;
        case 0x0E:  // R2_MERGEPENNOT
            this->impl().draw_ellipse<Ops::Op2_0x0E>(el, fill, color);
            break;
        case 0x0F:  // R2_MERGEPEN
            this->impl().draw_ellipse<Ops::Op2_0x0F>(el, fill, color);
            break;
        case 0x10: // R2_WHITE
            this->impl().draw_ellipse<Ops::Op2_0x10>(el, fill, color);
            break;
        default:
            this->impl().draw_ellipse<Ops::Op2_0x0D>(el, fill, color);
            break;
        }
    }

    // low level opaquerect,
    // mostly avoid clipping because we already took care of it
    // also we already swapped color if we are using BGR instead of RGB
    void opaquerect(Rect rect, const Color color)
    {
        if (this->tracked_area.has_intersection(rect)) {
            this->tracked_area_changed = true;
        }
        this->impl().opaque_rect(rect, color);
    }

    void draw_pixel(int16_t x, int16_t y, const Color color)
    {
        if (this->tracked_area.has_intersection(x, y)) {
            this->tracked_area_changed = true;
        }
        this->impl().draw_pixel(x, y, color);
    }

private:
    template <typename Op>
    void patblt_op(Rect rect, const Color color)
    {
        if (this->tracked_area.has_intersection(rect)) {
            this->tracked_area_changed = true;
        }
        this->impl().patblt_op(rect, color, Op());
    }

public:
    // low level patblt,
    // mostly avoid clipping because we already took care of it
    void patblt(Rect rect, const uint8_t rop, const Color color)
    {
        switch (rop) {
            // +------+-------------------------------+
            // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
            // |      | RPN: 0                        |
            // +------+-------------------------------+
            case 0x00: // blackness
                this->black_color(rect);
                break;
            // +------+-------------------------------+
            // | 0x05 | ROP: 0x000500A9               |
            // |      | RPN: DPon                     |
            // +------+-------------------------------+
            case 0x05:
                this->patblt_op<Ops::Op_0x05>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0x0F | ROP: 0x000F0001               |
                // |      | RPN: Pn                       |
                // +------+-------------------------------+
            case 0x0F:
                this->patblt_op<Ops::Op_0x0F>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0x50 | ROP: 0x00500325               |
                // |      | RPN: PDna                     |
                // +------+-------------------------------+
            case 0x50:
                this->patblt_op<Ops::Op_0x50>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
                // |      | RPN: Dn                       |
                // +------+-------------------------------+
            case 0x55: // inversion
                this->invert_color(rect);
                break;
            // +------+-------------------------------+
            // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
            // |      | RPN: DPx                      |
            // +------+-------------------------------+
            case 0x5A:
                this->patblt_op<Ops::Op_0x5A>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0x5F | ROP: 0x005F00E9               |
                // |      | RPN: DPan                     |
                // +------+-------------------------------+
            case 0x5F:
                this->patblt_op<Ops::Op_0x5F>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xA0 | ROP: 0x00A000C9               |
                // |      | RPN: DPa                      |
                // +------+-------------------------------+
            case 0xA0:
                this->patblt_op<Ops::Op_0xA0>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xA5 | ROP: 0x00A50065               |
                // |      | RPN: PDxn                     |
                // +------+-------------------------------+
            case 0xA5:
                this->patblt_op<Ops::Op_0xA5>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xAA | ROP: 0x00AA0029               |
                // |      | RPN: D                        |
                // +------+-------------------------------+
            case 0xAA: // change nothing
                break;
                // +------+-------------------------------+
                // | 0xAF | ROP: 0x00AF0229               |
                // |      | RPN: DPno                     |
                // +------+-------------------------------+
            case 0xAF:
                this->patblt_op<Ops::Op_0xAF>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
            case 0xF0:
                this->patblt_op<Ops::Op_0xF0>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xF5 | ROP: 0x00F50225               |
                // |      | RPN: PDno                     |
                // +------+-------------------------------+
            case 0xF5:
                this->patblt_op<Ops::Op_0xF5>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xFA | ROP: 0x00FA0089               |
                // |      | RPN: DPo                      |
                // +------+-------------------------------+
            case 0xFA:
                this->patblt_op<Ops::Op_0xFA>(rect, color);
                break;
                // +------+-------------------------------+
                // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
                // |      | RPN: 1                        |
                // +------+-------------------------------+
            case 0xFF: // whiteness
                this->white_color(rect);
                break;
            default:
                // should not happen, do nothing
                break;
        }
    }

private:
    template <typename Op>
    void patblt_op_ex(Rect rect, const uint8_t * brush_data, int8_t org_x, int8_t org_y,
        const Color back_color, const Color fore_color)
    {
        if (this->tracked_area.has_intersection(rect)) {
            this->tracked_area_changed = true;
        }

        this->impl().patblt_op_ex<Op>(rect, brush_data, org_x, org_y, back_color, fore_color);
    }

public:
    void patblt_ex(Rect rect, const uint8_t rop,
        const Color back_color, const Color fore_color,
        const uint8_t * brush_data, int8_t org_x, int8_t org_y)
    {
        switch (rop)
        {
        // +------+-------------------------------+
        // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
        // |      | RPN: P                        |
        // +------+-------------------------------+
        case 0xF0:
            this->patblt_op_ex<Ops::Op_0xF0>(rect, brush_data, org_x, org_y, back_color, fore_color);
            break;
        // +------+-------------------------------+
        // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
        // |      | RPN: DPx                      |
        // +------+-------------------------------+
        case 0x5A:
            this->patblt_op_ex<Ops::Op_0x5A>(rect, brush_data, org_x, org_y, back_color, fore_color);
            break;
        default:
            // should not happen
            LOG(LOG_INFO, "Unsupported parameters for PatBlt Primary Drawing Order!");
            this->patblt(rect, rop, back_color);
            break;
        }
    }

    // low level destblt,
    // mostly avoid clipping because we already took care of it
    void destblt(Rect rect, const uint8_t rop)
    {
        switch (rop) {
            case 0x00: // blackness
                this->black_color(rect);
                break;
            case 0x55: // inversion
                this->invert_color(rect);
                break;
            case 0xAA: // change nothing
                break;
            case 0xFF: // whiteness
                this->white_color(rect);
                break;
            default:
                // should not happen
                break;
        }
    }

    template <typename Op>
    void scr_blt_op(uint16_t srcx, uint16_t srcy, Rect drect)
    {
        if (this->tracked_area.has_intersection(drect)) {
            this->tracked_area_changed = true;
        }

        this->impl().scr_blt_op<Op>(drect, srcx, srcy);
    }

public:
    // low level scrblt, mostly avoid considering clipping
    // because we already took care of it
    void scrblt(unsigned srcx, unsigned srcy, Rect drect, uint8_t rop)
    {
        switch (rop) {
            // +------+-------------------------------+
            // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
            // |      | RPN: 0                        |
            // +------+-------------------------------+
            case 0x00:
                this->black_color(drect);
                break;
                // +------+-------------------------------+
                // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
                // |      | RPN: DSon                     |
                // +------+-------------------------------+
            case 0x11:
                this->scr_blt_op<Ops::Op_0x11>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x22 | ROP: 0x00220326               |
                // |      | RPN: DSna                     |
                // +------+-------------------------------+
            case 0x22:
                this->scr_blt_op<Ops::Op_0x22>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
                // |      | RPN: Sn                       |
                // +------+-------------------------------+
            case 0x33:
                this->scr_blt_op<Ops::Op_0x33>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
                // |      | RPN: SDna                     |
                // +------+-------------------------------+
            case 0x44:
                this->scr_blt_op<Ops::Op_0x44>(srcx, srcy, drect);
                break;

                // +------+-------------------------------+
                // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
                // |      | RPN: Dn                       |
                // +------+-------------------------------+
            case 0x55:
                this->invert_color(drect);
                break;
                // +------+-------------------------------+
                // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
                // |      | RPN: DSx                      |
                // +------+-------------------------------+
            case 0x66:
                this->scr_blt_op<Ops::Op_0x66>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x77 | ROP: 0x007700E6               |
                // |      | RPN: DSan                     |
                // +------+-------------------------------+
            case 0x77:
                this->scr_blt_op<Ops::Op_0x77>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
                // |      | RPN: DSa                      |
                // +------+-------------------------------+
            case 0x88:
                this->scr_blt_op<Ops::Op_0x88>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0x99 | ROP: 0x00990066               |
                // |      | RPN: DSxn                     |
                // +------+-------------------------------+
            case 0x99:
                this->scr_blt_op<Ops::Op_0x99>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0xAA | ROP: 0x00AA0029               |
                // |      | RPN: D                        |
                // +------+-------------------------------+
            case 0xAA: // nothing to change
                break;
                // +------+-------------------------------+
                // | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
                // |      | RPN: DSno                     |
                // +------+-------------------------------+
            case 0xBB:
                this->scr_blt_op<Ops::Op_0xBB>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
                // |      | RPN: S                        |
                // +------+-------------------------------+
            case 0xCC:
                this->scr_blt_op<Ops::Op_0xCC>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0xDD | ROP: 0x00DD0228               |
                // |      | RPN: SDno                     |
                // +------+-------------------------------+
            case 0xDD:
                this->scr_blt_op<Ops::Op_0xDD>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
                // |      | RPN: DSo                      |
                // +------+-------------------------------+
            case 0xEE:
                this->scr_blt_op<Ops::Op_0xEE>(srcx, srcy, drect);
                break;
                // +------+-------------------------------+
                // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
                // |      | RPN: 1                        |
                // +------+-------------------------------+
            case 0xFF:
                this->white_color(drect);
                break;
            default:
                // should not happen
                break;
        }
    }

    void draw_line(
        int mix_mode,
        int16_t xStart, int16_t yStart,
        int16_t xEnd, int16_t yEnd,
        uint8_t rop, Color color, Rect clip
    ) {
        LineEquation equa(xStart, yStart, xEnd, yEnd);

        if (not equa.resolve(clip)) {
            return;
        }

        int startx = equa.segin.a.x;
        int starty = equa.segin.a.y;
        int endx = equa.segin.b.x;
        int endy = equa.segin.b.y;

        if (startx == endx){
            this->vertical_line(
                mix_mode,
                startx, starty,
                endy,
                rop, color
            );
        }
        else if (starty == endy){
            this->horizontal_line(
                mix_mode,
                startx, starty,
                endx,
                rop, color
            );
        }
        else {
            this->diagonal_line(
                mix_mode,
                startx, starty,
                endx, endy,
                rop, color
            );
        }
    }

    // nor horizontal nor vertical, use Bresenham
    void diagonal_line(int mix_mode, int x, int y, int endx, int endy, uint8_t rop, Color color)
    {
        // TODO mix_mode is not used
        (void)mix_mode;

        if (endx <= x) {
            std::swap(x, endx);
            std::swap(y, endy);
        }

        const Rect line_rect = Rect(x, y, 1, 1).enlarge_to(endx, endy);
        if (this->tracked_area.has_intersection(line_rect)) {
            this->tracked_area_changed = true;
        }

        if (rop == 0x06) {
            this->impl().diagonal_line(x, y, endx, endy, color, Ops::InvertTarget());
        }
        else {
            this->impl().diagonal_line(x, y, endx, endy, color, Ops::CopySrc());
        }
    }

    void vertical_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endy, uint8_t rop, Color color)
    {
        // TODO mix_mode is not used
        (void)mix_mode;

        if (endy < y) {
            std::swap(y, endy);
        }

        const Rect line_rect = Rect(x, y, 1, 1).enlarge_to(x+1, endy);
        if (this->tracked_area.has_intersection(line_rect)) {
            this->tracked_area_changed = true;
        }

        if (rop == 0x06) {
            this->impl().vertical_line(x, y, endy, color, Ops::InvertTarget());
        }
        else {
            this->impl().vertical_line(x, y, endy, color, Ops::CopySrc());
        }
    }

    void horizontal_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endx, uint8_t rop, Color color)
    {
        // TODO mix_mode is not used
        (void)mix_mode;

        if (endx < x) {
            std::swap(x, endx);
        }

        const Rect line_rect = Rect(x, y, 1, 1).enlarge_to(endx, y+1);
        if (this->tracked_area.has_intersection(line_rect)) {
            this->tracked_area_changed = true;
        }

        if (rop == 0x06) {
            this->impl().horizontal_line(x, y, endx, color, Ops::InvertTarget());
        }
        else {
            this->impl().horizontal_line(x, y, endx, color, Ops::CopySrc());
        }
    }

    void set_row(size_t rownum, const uint8_t * data, size_t data_length)
    {
        memcpy(this->impl().row_data(rownum), data, std::min(this->rowsize(), data_length));
    }

    void trace_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave) {
        for (DrawablePointer::ContiguousPixels const & contiguous_pixels : current_pointer->contiguous_pixels_view()) {
            if (contiguous_pixels.x + x < 0 || contiguous_pixels.y + y < 0) {
                continue;
            }
            uint8_t * pixel_begin = this->impl().first_pixel(contiguous_pixels.x + x, contiguous_pixels.y + y);
            uint8_t * pixel_end = pixel_begin + contiguous_pixels.data_size;
            uint8_t * line_begin = this->impl().row_data(contiguous_pixels.y+y);
            uint8_t * line_end = line_begin + this->impl().rowsize();
            if (pixel_end > line_end) {
                pixel_end = line_end;
            }
            if (line_end >= this->impl().last_pixel()) {
                continue;
            }
            if (line_begin < this->impl().first_pixel()) {
                continue;
            }
            size_t offset = (pixel_begin < line_begin) ? line_begin - pixel_begin : 0;
            if (pixel_end > pixel_begin+offset){
                memcpy(psave+offset, pixel_begin+offset, pixel_end-pixel_begin-offset);
                memcpy(pixel_begin + offset, contiguous_pixels.data + offset, pixel_end-pixel_begin-offset);
            }
            psave += contiguous_pixels.data_size;
        }
    }

    void clear_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave) {
        for (DrawablePointer::ContiguousPixels const & contiguous_pixels : current_pointer->contiguous_pixels_view()) {
            if (contiguous_pixels.x + x < 0 || contiguous_pixels.y + y < 0) {
                continue;
            }
            uint8_t * pixel_begin = this->impl().first_pixel(contiguous_pixels.x + x, contiguous_pixels.y + y);
            uint8_t * pixel_end = pixel_begin + contiguous_pixels.data_size;
            uint8_t * line_begin = this->impl().row_data(contiguous_pixels.y+y);
            uint8_t * line_end = line_begin + this->impl().rowsize();
            if (pixel_end > line_end) {
                pixel_end = line_end;
            }
            if (line_end >= this->impl().last_pixel()) {
                continue;
            }
            if (line_begin < this->impl().first_pixel()) {
                continue;
            }
            size_t offset = (pixel_begin < line_begin) ? line_begin - pixel_begin : 0;
            if (pixel_end > pixel_begin+offset){
                memcpy(pixel_begin+offset, psave+offset, pixel_end-pixel_begin-offset);
            }
            psave += contiguous_pixels.data_size;
        }
    }

public:
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
}
