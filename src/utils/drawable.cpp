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

#include <utility>

#include "utils/drawable.hpp"

#include "core/error.hpp"
#include "utils/bitfu.hpp"
#include "utils/bitmap.hpp"
#include "utils/drawable_pointer.hpp"
#include "utils/ellipse.hpp"
#include "utils/ellipse.hpp"
#include "utils/log.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"


using std::size_t; /*NOLINT*/

namespace Ops
{
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

    using Op2_0x04 = InvertSrc; // R2_NOTCOPYPEN Pn

    struct Op2_0x05 // R2_MASKPENNOT PDna
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return (source & ~target);
        }
    };

    using Op2_0x06 = InvertTarget; // R2_NOT Dn

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

    using Op2_0x0D = CopySrc; // R2_COPYPEN P

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


    using Op_0x05 = Op2_0x02;
    using Op_0x0F = InvertSrc;
    using Op_0x50 = Op2_0x05;
    using Op_0x5A = Op2_0x07;
    using Op_0x5F = Op2_0x08;
    using Op_0xA0 = Op2_0x09;
    using Op_0xA5 = Op2_0x0A;
    using Op_0xAF = Op2_0x0C;
    using Op_0xF0 = CopySrc;
    using Op_0xF5 = Op2_0x0E;
    using Op_0xFA = Op2_0x0F;

    struct Op_0x11 // NOTSRCERASE DSon
    {
        constexpr u8 operator()(u8 target, u8 source) const noexcept
        {
            return ~(target | source);
        }
    };

    using Op_0x22 = Op2_0x03;
    using Op_0x33 = InvertSrc;
    using Op_0x44 = Op2_0x05;
    using Op_0x66 = Op2_0x07;
    using Op_0x77 = Op2_0x08;
    using Op_0x88 = Op2_0x09;
    using Op_0x99 = Op2_0x0A;
    using Op_0xBB = Op2_0x0C;
    using Op_0xCC = CopySrc;
    using Op_0xDD = Op2_0x0E;
    using Op_0xEE = Op2_0x0F;
} // namespace Ops


namespace details
{
    std::unique_ptr<uint8_t[]> alloc_data(size_t rowsize, uint16_t height)
    {
        size_t sz = rowsize * height;
        if (sz == 0) {
            throw Error(ERR_RECORDER_EMPTY_IMAGE);
        }
        return std::unique_ptr<uint8_t[]>(new uint8_t[sz]{});
    }
} // namespace details

namespace
{
    using u8 = uint8_t;
    using u16 = uint16_t;
    using P = u8 *;
    using cP = u8 const *;

    template<DepthColor BppIn>
    struct PrivateDrawableImplRef
    {
        DrawableImpl<BppIn>& dimpl;

        using color_t = typename DrawableImpl<BppIn>::color_t;
        using traits = typename DrawableImpl<BppIn>::traits;

        void resize(unsigned width, unsigned height)
        {
            if (width == dimpl.width_ && height == dimpl.height_) {
                return;
            }

            uint16_t const saved_height_  = dimpl.height_;
            size_t   const saved_rowsize_ = dimpl.rowsize_;

            dimpl.width_   = width;
            dimpl.height_  = height;
            dimpl.rowsize_ = dimpl.width_ * dimpl.nbbytes_color();

            auto old_data  = std::exchange(dimpl.data_, details::alloc_data(dimpl.rowsize_, dimpl.height_));
            P    src       = old_data.get();
            P    dest      = dimpl.data_.get();

            size_t min_rowsize = std::min(saved_rowsize_, dimpl.rowsize_);
            for (uint16_t row_index = 0, row_count = std::min(saved_height_, dimpl.height_);
                row_index < row_count; ++row_index) {
                memcpy(dest, src, min_rowsize);

                src  += saved_rowsize_;
                dest += dimpl.rowsize_;
            }
        }

        struct Assign
        {
            color_t color;

            P operator()(P dest) const
            { return traits::assign(dest, color); }
        };

        template<class Op>
        struct AssignOp
        {
            color_t color;

            P operator()(P dest) const
            { return traits::assign(dest, color, Op()); }
        };

        struct Invert
        {
            P operator()(P dest) const
            { *dest ^= 0xff; return ++dest; }
        };


        void opaque_rect(Rect rect, const color_t color)
        {
            P const base = dimpl.first_pixel(rect);

            apply_for_line(base, rect.cx, Assign{color});

            P target = base;
            const size_t line_size = dimpl.rowsize();
            cP pe = target + line_size * rect.cy;
            while ((target += line_size) < pe) {
                memcpy(target, base, rect.cx * dimpl.nbbytes_color());
            }
        }

        void draw_pixel(int16_t x, int16_t y, const color_t color)
        {
            traits::assign(dimpl.first_pixel(x, y), color);
        }

        template<class Op, class... Col>
        void mem_blt(Rect rect, const ConstImageDataView & bmp, const uint16_t srcx, const uint16_t srcy, Op op, Col... c)
        {
            // TODO implements with ConstImageDataView::Storage::TopToBottom
            assert(bmp.storage_type() == ConstImageDataView::Storage::BottomToTop && "other is unimplemented");

            P dest = dimpl.first_pixel(rect);
            cP src = bmp.data(srcx, bmp.height() - srcy - 1);
            const size_t n = rect.cx * dimpl.nbbytes_color();
            const uint8_t bmp_bpp = safe_int(bmp.bits_per_pixel());
            const size_t bmp_line_size = bmp.line_size();

            if (bmp_bpp == dimpl.bpp()) {
                const size_t line_size = dimpl.rowsize();
                for (cP ep = dest + line_size * rect.cy; dest < ep; dest += line_size, src -= bmp_line_size) {
                    this->copy(dest, src, n, op, c...);
                }
            }
            else {
                switch (bmp_bpp) {
                    case 8: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                        safe_int(bmp.bytes_per_pixel()), bmp_line_size, op, typename traits::fromColor8{bmp.palette()}, c...); break;
                    case 15: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                        safe_int(bmp.bytes_per_pixel()), bmp_line_size, op, typename traits::fromColor15{}, c...); break;
                    case 16: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                        safe_int(bmp.bytes_per_pixel()), bmp_line_size, op, typename traits::fromColor16{}, c...); break;
                    case 24: this->spe_mem_blt(dest, src, rect.cx, rect.cy,
                        safe_int(bmp.bytes_per_pixel()), bmp_line_size, op, typename traits::fromColor24{}, c...); break;
                    default: ;
                }
            }
        }

    private:
        template<class Op, class ToColor, class... Col>
        void spe_mem_blt(
            P dest, cP src, u16 cx, u16 cy, size_t bmp_Bpp, size_t bmp_line_size, Op op, ToColor to_color, Col... c)
        {
            const size_t line_size = dimpl.rowsize();
            const size_t destn = cx * dimpl.nbbytes_color();
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
            P p = dimpl.first_pixel(rect);
            const size_t step = dimpl.rowsize();
            const size_t n = rect.cx * dimpl.nbbytes_color();
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
                x < dimpl.width() &&
                y < dimpl.height())) {
                return;
            }
            P p = dimpl.first_pixel(x, y);
            traits::assign(p, color, op2);
        }

        template <typename Op2>
        void colorline(int x, int y, int l, color_t color, Op2 /*unused*/)
        {
            if (!(y >= 0 &&
                y < dimpl.height())) {
                    return;
            }
            if (x < 0) {
                l += x;
                x = 0;
            }
            else if ((x + l) >= dimpl.width()) {
                l = dimpl.width() - x;
            }
            P p = dimpl.first_pixel(x, y);
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

            P const base = dimpl.first_pixel(rect);
            P       p    = base;

            int const origin_x = rect.x/* - org_x*/;
            int const origin_y = rect.y/* - org_y*/;

            for (size_t y = 0, cy = rect.cy; y < cy; ++y) {
                p = base + dimpl.rowsize() * y;
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
                        P dest = dimpl.first_pixel(rect.x, rect.bottom());
                        cP src = dimpl.first_pixel(srcx, srcy + rect.cy);
                        cP esrc = src - dimpl.rowsize() * (srcy + rect.cy - overlap.y);
                        while (src > esrc) {
                            dest -= dimpl.rowsize();
                            src -= dimpl.rowsize();
                            this->copy(dest, src, rect.cx * dimpl.Bpp, Op());
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
        void scr_blt_op_overlap(Rect const rect_dest, size_t srcx, size_t srcy, Ops::CopySrc /*unused*/)
        {
            this->scr_blt_impl(rect_dest, srcx, srcy, [](P dest, cP src, size_t n) {
                memmove(dest, src, n);
            });
        }

        template <typename Op>
        void scr_blt_op_overlap(Rect const rect_dest, size_t srcx, size_t srcy, Op op)
        {
            P dest = dimpl.first_pixel(rect_dest);
            cP src = dimpl.first_pixel(srcx, srcy);
            const size_t n = rect_dest.cx * dimpl.nbbytes_color();
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
            this->scr_blt_impl(dimpl.first_pixel(rect_dest), dimpl.first_pixel(srcx, srcy), rect_dest.cx * dimpl.nbbytes_color(), rect_dest.cy, f);
        }

        template <typename F>
        void scr_blt_impl(P dest, cP src, size_t n, size_t cy, F f)
        {
            for (P e = dest + dimpl.rowsize() * cy; e != dest; ) {
                f(dest, src, n);
                dest += dimpl.rowsize();
                src += dimpl.rowsize();
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
                traits::assign(dimpl.first_pixel(x, y), color, op);

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

            P p = dimpl.first_pixel(x, y);
            P pe = p + (endy - y + 1) * dimpl.rowsize();
            for (; p != pe; p += dimpl.rowsize()) {
                traits::assign(p, color, op);
            }
        }

        template<class Op>
        void horizontal_line(uint16_t x, uint16_t y, uint16_t endx, color_t color, Op /*unused*/)
        {
            assert(x <= endx);

            this->apply_for_line(dimpl.first_pixel(x, y), endx - x + 1, AssignOp<Op>{color});
        }

        template <typename Op>
        void patblt_op(Rect rect, color_t color, Op /*unused*/)
        {
            this->apply_for_rect(rect, AssignOp<Op>{color});
        }

        void patblt_op(Rect rect, color_t color, Ops::InvertSrc /*unused*/)
        {
            this->apply_for_rect(rect, Assign{~color});
        }

        void patblt_op(Rect rect, color_t color, Ops::CopySrc /*unused*/)
        {
            this->apply_for_rect(rect, Assign{color});
        }

        void invert_color(Rect rect)
        {
            this->apply_for_rect(rect, Invert{});
        }

    private:
        template<class Op>
        void copy(uint8_t * dest, const uint8_t * src, size_t n, Op op)
        {
            const uint8_t * e = dest + n;
            for (; dest != e; ++dest, ++src) {
                *dest = op(*dest, *src);
            }
        }

        void copy(uint8_t * dest, const uint8_t * src, size_t n, Ops::CopySrc /*unused*/)
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
            for (cP pe = p + n * dimpl.nbbytes_color(); p != pe; ) {
                p = f(p);
            }
            return p;
        }

        template<class F>
        void apply_for_rect(Rect rect, F f)
        {
            P p = dimpl.first_pixel(rect);
            const size_t line_size = dimpl.rowsize();
            const size_t cx = rect.cx * dimpl.nbbytes_color();
            const size_t inc_line = line_size - cx;
            for (cP pe = p + rect.cy * line_size; p != pe; p += inc_line) {
                for (cP pe2 = p + cx; p != pe2; ) {
                    p = f(p);
                }
            }
        }
    };
} // namespace


namespace
{
    template<DepthColor BppIn>
    PrivateDrawableImplRef<BppIn> privimpl(DrawableImpl<BppIn>& dimpl) noexcept
    {
        return {dimpl};
    }
}

void Drawable::resize(int width, int height)
{
    privimpl(this->impl_).resize(width, height);

    this->tracked_area         = Rect(0, 0, 0, 0);
    this->tracked_area_changed = false;

    this->logical_frame_ended = true;
}

template <typename Op, class... Color_>
void Drawable::mem_blt_op( Rect rect
                , const ConstImageDataView & bmp
                , const uint16_t srcx
                , const uint16_t srcy
                , Color_... c) {
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

    privimpl(this->impl_).mem_blt(trect, bmp, srcx, srcy, Op(), c...);
}

/*
    * The name doesn't say it : mem_blt COPIES a decoded bitmap from
    * a cache (data) and insert a subpart (srcx, srcy) to the local
    * image cache (dimpl.impl().first_pixel()) a the given position (rect).
    */
void Drawable::mem_blt(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy)
{
    this->mem_blt_op<Ops::CopySrc>(rect, bmp, srcx, srcy);
}

void Drawable::mem_blt_invert(Rect rect, ConstImageDataView bmp, const uint16_t srcx, const uint16_t srcy)
{
    this->mem_blt_op<Ops::InvertSrc>(rect, bmp, srcx, srcy);
}

void Drawable::mem_blt_ex( Rect rect
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

void Drawable::draw_bitmap(Rect rect, const ConstImageDataView & bmp)
{
    this->mem_blt_op<Ops::CopySrc>(rect, bmp, 0, 0);
}

void Drawable::mem_3_blt( Rect rect
                , const ConstImageDataView & bmp
                , const uint16_t srcx
                , const uint16_t srcy
                , uint8_t rop
                , const Color pattern_color)
{
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

void Drawable::black_color(Rect rect)
{
    const Rect trect = rect.intersect(this->width(), this->height());

    if (this->tracked_area.has_intersection(trect)) {
        this->tracked_area_changed = true;
    }

    privimpl(this->impl_).component_rect(trect, 0);
}

void Drawable::white_color(Rect rect)
{
    const Rect trect = rect.intersect(this->width(), this->impl_.height());

    if (this->tracked_area.has_intersection(rect)) {
        this->tracked_area_changed = true;
    }

    privimpl(this->impl_).component_rect(trect, 0xFF);
}

void Drawable::invert_color(Rect rect)
{
    const Rect trect = rect.intersect(this->width(), this->impl_.height());

    if (this->tracked_area.has_intersection(trect)) {
        this->tracked_area_changed = true;
    }

    privimpl(this->impl_).invert_color(trect);
}

// 2.2.2.2.1.1.1.6 Binary Raster Operation (ROP2_OPERATION)
//  The ROP2_OPERATION structure is used to define how the bits in a destination bitmap and a
//  selected brush or pen are combined by using Boolean operators.
// rop2Operation (1 byte): An 8-bit, unsigned integer. A raster-operation code that describes a
//  Boolean operation, in Reverse Polish Notation, to perform on the bits in a destination
//  bitmap (D) and selected brush or pen (P). This operation is a combination of the AND (a), OR
//  (o), NOT (n), and XOR (x) Boolean operators.

void Drawable::ellipse(const Ellipse & el, const uint8_t rop, const uint8_t fill, const Color color)
{
    if (this->tracked_area.has_intersection(el.get_rect())) {
        this->tracked_area_changed = true;
    }
    switch (rop) {
    case 0x01: // R2_BLACK
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x01>(el, fill, color);
        break;
    case 0x02: // R2_NOTMERGEPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x02>(el, fill, color);
        break;
    case 0x03: // R2_MASKNOTPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x03>(el, fill, color);
        break;
    case 0x04: // R2_NOTCOPYPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x04>(el, fill, color);
        break;
    case 0x05: // R2_MASKPENNOT
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x05>(el, fill, color);
        break;
    case 0x06:  // R2_NOT
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x06>(el, fill, color);
        break;
    case 0x07:  // R2_XORPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x07>(el, fill, color);
        break;
    case 0x08:  // R2_NOTMASKPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x08>(el, fill, color);
        break;
    case 0x09:  // R2_MASKPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x09>(el, fill, color);
        break;
    case 0x0A:  // R2_NOTXORPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0A>(el, fill, color);
        break;
    case 0x0B:  // R2_NOP
        break;
    case 0x0C:  // R2_MERGENOTPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0C>(el, fill, color);
        break;
    case 0x0D:  // R2_COPYPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0D>(el, fill, color);
        break;
    case 0x0E:  // R2_MERGEPENNOT
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0E>(el, fill, color);
        break;
    case 0x0F:  // R2_MERGEPEN
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0F>(el, fill, color);
        break;
    case 0x10: // R2_WHITE
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x10>(el, fill, color);
        break;
    default:
        privimpl(this->impl_).draw_ellipse<Ops::Op2_0x0D>(el, fill, color);
        break;
    }
}

// low level opaquerect,
// mostly avoid clipping because we already took care of it
// also we already swapped color if we are using BGR instead of RGB
void Drawable::opaquerect(Rect rect, const Color color)
{
    if (this->tracked_area.has_intersection(rect)) {
        this->tracked_area_changed = true;
    }
    privimpl(this->impl_).opaque_rect(rect, color);
}

void Drawable::draw_pixel(int16_t x, int16_t y, const Color color)
{
    if (this->tracked_area.has_intersection(x, y)) {
        this->tracked_area_changed = true;
    }
    privimpl(this->impl_).draw_pixel(x, y, color);
}

template <typename Op>
void Drawable::patblt_op(Rect rect, const Color color)
{
    if (this->tracked_area.has_intersection(rect)) {
        this->tracked_area_changed = true;
    }
    privimpl(this->impl_).patblt_op(rect, color, Op());
}

// low level patblt,
// mostly avoid clipping because we already took care of it
void Drawable::patblt(Rect rect, const uint8_t rop, const Color color)
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

template <typename Op>
void Drawable::patblt_op_ex(Rect rect, const uint8_t * brush_data, int8_t org_x, int8_t org_y,
    const Color back_color, const Color fore_color)
{
    if (this->tracked_area.has_intersection(rect)) {
        this->tracked_area_changed = true;
    }

    privimpl(this->impl_).patblt_op_ex<Op>(rect, brush_data, org_x, org_y, back_color, fore_color);
}

void Drawable::patblt_ex(Rect rect, const uint8_t rop,
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
void Drawable::destblt(Rect rect, const uint8_t rop)
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
void Drawable::scr_blt_op(uint16_t srcx, uint16_t srcy, Rect drect)
{
    if (this->tracked_area.has_intersection(drect)) {
        this->tracked_area_changed = true;
    }

    privimpl(this->impl_).scr_blt_op<Op>(drect, srcx, srcy);
}

// low level scrblt, mostly avoid considering clipping
// because we already took care of it
void Drawable::scrblt(unsigned srcx, unsigned srcy, Rect drect, uint8_t rop)
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

void Drawable::draw_line(
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
void Drawable::diagonal_line(int mix_mode, int x, int y, int endx, int endy, uint8_t rop, Color color)
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
        privimpl(this->impl_).diagonal_line(x, y, endx, endy, color, Ops::InvertTarget());
    }
    else {
        privimpl(this->impl_).diagonal_line(x, y, endx, endy, color, Ops::CopySrc());
    }
}

void Drawable::vertical_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endy, uint8_t rop, Color color)
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
        privimpl(this->impl_).vertical_line(x, y, endy, color, Ops::InvertTarget());
    }
    else {
        privimpl(this->impl_).vertical_line(x, y, endy, color, Ops::CopySrc());
    }
}

void Drawable::horizontal_line(uint8_t mix_mode, uint16_t x, uint16_t y, uint16_t endx, uint8_t rop, Color color)
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
        privimpl(this->impl_).horizontal_line(x, y, endx, color, Ops::InvertTarget());
    }
    else {
        privimpl(this->impl_).horizontal_line(x, y, endx, color, Ops::CopySrc());
    }
}

void Drawable::set_row(size_t rownum, const uint8_t * data, size_t data_length)
{
    memcpy(this->impl_.row_data(rownum), data, std::min(this->rowsize(), data_length));
}

void Drawable::trace_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave)
{
    for (DrawablePointer::ContiguousPixels const & contiguous_pixels : current_pointer->contiguous_pixels_view()) {
        if (contiguous_pixels.x + x < 0 || contiguous_pixels.y + y < 0) {
            continue;
        }
        uint8_t * pixel_begin = this->impl_.first_pixel(contiguous_pixels.x + x, contiguous_pixels.y + y);
        uint8_t * pixel_end = pixel_begin + contiguous_pixels.data_size;
        uint8_t * line_begin = this->impl_.row_data(contiguous_pixels.y+y);
        uint8_t * line_end = line_begin + this->impl_.rowsize();
        if (pixel_end > line_end) {
            pixel_end = line_end;
        }
        if (line_end >= this->impl_.last_pixel()) {
            continue;
        }
        if (line_begin < this->impl_.first_pixel()) {
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

void Drawable::clear_mouse(const DrawablePointer * current_pointer, const int x, const int y, uint8_t * psave)
{
    for (DrawablePointer::ContiguousPixels const & contiguous_pixels : current_pointer->contiguous_pixels_view()) {
        if (contiguous_pixels.x + x < 0 || contiguous_pixels.y + y < 0) {
            continue;
        }
        uint8_t * pixel_begin = this->impl_.first_pixel(contiguous_pixels.x + x, contiguous_pixels.y + y);
        uint8_t * pixel_end = pixel_begin + contiguous_pixels.data_size;
        uint8_t * line_begin = this->impl_.row_data(contiguous_pixels.y+y);
        uint8_t * line_end = line_begin + this->impl_.rowsize();
        if (pixel_end > line_end) {
            pixel_end = line_end;
        }
        if (line_end >= this->impl_.last_pixel()) {
            continue;
        }
        if (line_begin < this->impl_.first_pixel()) {
            continue;
        }
        size_t offset = (pixel_begin < line_begin) ? line_begin - pixel_begin : 0;
        if (pixel_end > pixel_begin+offset){
            memcpy(pixel_begin+offset, psave+offset, pixel_end-pixel_begin-offset);
        }
        psave += contiguous_pixels.data_size;
    }
}
