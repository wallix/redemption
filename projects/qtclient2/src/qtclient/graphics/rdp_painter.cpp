/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/graphics/rdp_painter.hpp"

#include "utils/bitmap.hpp"
#include "utils/line_equation.hpp"

#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"
#include "core/RDP/orders/for_each_delta_rect.hpp"
#include "gdi/clip_from_cmd.hpp"

#include <QtGui/QPixmap>

namespace
{
    inline QColor qcolor(RDPColor rdpColor, gdi::ColorCtx const& color_ctx)
    {
        BGRColor bgr = color_decode(rdpColor, color_ctx);
        return {bgr.red(), bgr.green(), bgr.blue()};
    }

    inline QRect qrect(Rect const& rect)
    {
        return QRect(rect.x, rect.y, rect.cx, rect.cy);
    }

    inline QRegion qregion(Rect const& rect)
    {
        return QRegion(rect.x, rect.y, rect.cx, rect.cy);
    }

    inline Rect intersect(Rect const& clip, QPixmap const& cache)
    {
        return clip.intersect(cache.width(), cache.height());
    }

    using CompositionMode = QPainter::CompositionMode;

    namespace Rop
    {
        constexpr auto DSna = CompositionMode::RasterOp_SourceXorDestination;
        constexpr auto DPx = CompositionMode::RasterOp_SourceXorDestination;
        constexpr auto DPon = CompositionMode::RasterOp_NotSourceOrNotDestination;
        constexpr auto Pn = CompositionMode::RasterOp_NotSource;
        constexpr auto PDna = CompositionMode::RasterOp_SourceAndNotDestination;
        constexpr auto Dn = CompositionMode::RasterOp_NotDestination;
        constexpr auto DPan = CompositionMode::RasterOp_NotSourceAndNotDestination;
        constexpr auto DPa = CompositionMode::RasterOp_SourceAndDestination;
        constexpr auto PDxn = CompositionMode::RasterOp_NotSourceXorDestination;
        constexpr auto DPno = CompositionMode::RasterOp_NotSourceOrDestination;
        constexpr auto PDno = CompositionMode::RasterOp_SourceOrNotDestination;
        constexpr auto DPo = CompositionMode::RasterOp_SourceOrDestination;
        constexpr auto Sn = CompositionMode::RasterOp_NotSource;
        constexpr auto DSx = CompositionMode::RasterOp_SourceXorDestination;
        constexpr auto DSa = CompositionMode::RasterOp_SourceAndDestination;
        constexpr auto DSxn = CompositionMode::RasterOp_NotSourceXorDestination;
        constexpr auto DSno = CompositionMode::RasterOp_NotSourceOrDestination;
        constexpr auto DSo = CompositionMode::RasterOp_SourceOrDestination;

        constexpr auto Reset = CompositionMode::CompositionMode_SourceOver;
    };

    void drawRectWithComposition(QPainter& painter, Rect rect, QPainter::CompositionMode mode)
    {
        painter.setCompositionMode(mode);
        painter.drawRect(qrect(rect));
        painter.setCompositionMode(Rop::Reset);
    }

    void drawPatBlt(
        QPainter& painter,
        QRect const* rects, int count_rect, RDPBrush const& brush, uint8_t rop,
        RDPColor back_color, RDPColor fore_color, gdi::ColorCtx color_ctx)
    {
        const QColor backColor = qcolor(back_color, color_ctx);
        const QColor foreColor = qcolor(fore_color, color_ctx);

        if (brush.style == 0x03 && (rop == 0xF0 || rop == 0x5A)) {
            auto drawModeWithBrush = [&](Qt::BrushStyle style, CompositionMode mode){
                painter.setBrush(QBrush(backColor, style));
                painter.setCompositionMode(mode);
                painter.drawRects(rects, count_rect);
                painter.setCompositionMode(Rop::Reset);
                painter.setBrush(Qt::SolidPattern);
            };

            switch (rop) {
                case 0x5A: drawModeWithBrush(Qt::Dense4Pattern, Rop::DPx); break;

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
                case 0xF0:
                    // painter.setPen(Qt::NoPen);
                    painter.setBrush(QBrush(foreColor, Qt::Dense4Pattern));
                    // painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                    painter.drawRects(rects, count_rect);
                    painter.setBrush(Qt::SolidPattern);
                    break;

                default:
                    LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", rop);
                    break;
            }
        }
        else {
            auto drawMode = [&](QPainter::CompositionMode mode){
                painter.setCompositionMode(mode);
                painter.drawRects(rects, count_rect);
                painter.setCompositionMode(Rop::Reset);
            };

            auto drawRect = [&](const QBrush& brush){
                painter.setBrush(brush);
                painter.drawRects(rects, count_rect);
            };

            switch (rop) {
                // blackness
                case 0x00: drawRect(Qt::black); break;

                case 0x05: drawMode(Rop::DPon); break;
                case 0x0F: drawMode(Rop::Pn); break;
                case 0x50: drawMode(Rop::PDna); break;
                case 0x55: drawMode(Rop::Dn); break;
                case 0x5A: drawMode(Rop::DPx); break;
                case 0x5F: drawMode(Rop::DPan); break;
                case 0xA0: drawMode(Rop::DPa); break;
                case 0xA5: drawMode(Rop::PDxn); break;
                case 0xAA: break;
                case 0xAF: drawMode(Rop::DPno); break;

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
                case 0xF0: drawRect(backColor); break;
                case 0xF5: drawMode(Rop::PDno); break;
                case 0xFA: drawMode(Rop::DPo); break;

                // whiteness
                case 0xFF: drawRect(Qt::white); break;

                default: LOG(LOG_WARNING, "RDPPatBlt rop = %x", rop);
                    break;
            }
        }
    }

    void drawDstBlt(QPainter& painter, QRect const* rects, int count_rect, uint8_t rop)
    {
        switch (rop) {
            case 0x00:
                painter.setBrush(Qt::black);
                painter.drawRects(rects, count_rect);
                break;

            case 0x55: {
                painter.setCompositionMode(Rop::Dn);
                painter.drawRects(rects, count_rect);
                painter.setCompositionMode(Rop::Reset);
                break;
            }

            case 0xAA:
                break;

            case 0xFF:
                painter.setBrush(Qt::white);
                painter.drawRects(rects, count_rect);
                break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPDstBlt rop = %x", rop);
                break;
        }
    }

    struct ImagePos
    {
        explicit ImagePos(Bitmap const& bmp, Rect drect, Rect clip, int sx, int sy)
        : ImagePos(bmp.cx(), bmp.cy(), drect, clip, sx, sy)
        {}

        explicit ImagePos(int bmp_cx, int bmp_cy, Rect drect, Rect clip, int sx, int sy)
        {
            const Rect rect = drect.intersect(clip);

            if (rect.isempty()) {
                return ;
            }

            x = drect.x;
            y = drect.y;
            w = std::min(bmp_cx - srcx, int(rect.cx));
            h = std::min(bmp_cy - srcy, int(rect.cy));
            srcx = sx + rect.x - drect.x;
            srcy = sy + rect.y - drect.y;
        }

        QRect qrect() const noexcept
        {
            return QRect(x, y, w, h);
        }

        bool isEmpty() const noexcept
        {
            return w <= 0 || h <= 0;
        }

        int x;
        int y;
        int w = 0;
        int h = 0;

        int srcx;
        int srcy;
    };

    void drawBitmap(QPainter& painter, ImagePos const& p, Bitmap const& bmp, QImage::Format format)
    {
        QImage qbitmap(bmp.data() + bmp.line_size() * p.srcy + p.srcx, p.w, p.h, bmp.line_size(), format);

        // inplace transformation
        std::move(qbitmap).mirrored(false, true);

        painter.drawImage(p.qrect(), qbitmap);
    }

    QImage::Format format(Bitmap const& bmp)
    {
        switch (safe_cast<uint8_t>(bmp.bpp())) {
            case 15: return QImage::Format_RGB555;
            case 16: return QImage::Format_RGB16;
            case 24: return QImage::Format_BGR888;
            case 32: return QImage::Format_RGB32;
            case 8:  return QImage::Format_Indexed8;
            default:
                REDEMPTION_UNREACHABLE();
        }
    }

    QImage bmp2img(int x, int y, int w, int h, Bitmap const& bmp)
    {
        auto line_size = checked_cast<int>(bmp.line_size());
        return QImage(bmp.data() + line_size * y + x, w, h, line_size, format(bmp));
    }

    void drawBitmap(QPainter& painter, ImagePos const& p, Bitmap const& bmp)
    {
        switch (safe_cast<uint8_t>(bmp.bpp())) {
            case 15: drawBitmap(painter, p, bmp, QImage::Format_RGB555); break;
            case 16: drawBitmap(painter, p, bmp, QImage::Format_RGB16);  break;
            case 24: drawBitmap(painter, p, bmp, QImage::Format_BGR888); break;
            case 32: drawBitmap(painter, p, bmp, QImage::Format_RGB32);  break;
            case 8:
                // Drawing into a QImage with QImage::Format_Indexed8 is not supported.
                drawBitmap(painter, p, Bitmap(BitsPerPixel(16), bmp), QImage::Format_RGB16);
                break;

            default:
                REDEMPTION_UNREACHABLE();
        }
    }

    void drawBitmap(QPainter& painter, Bitmap const& bmp, Rect drect, Rect clip, int sx, int sy)
    {
        ImagePos p(bmp, drect, clip, sx, sy);

        if (p.isEmpty()) {
            return;
        }

        drawBitmap(painter, p, bmp);
    }

    struct Rects
    {
        template<class RDPMulti>
        Rects(const RDPMulti & cmd, Rect clip)
        {
            QRect* prect = rects;
            for_each_delta_rect(cmd, [&](Rect const& drect) {
                const auto rect = clip.intersect(drect);
                *prect = QRect(rect.x, rect.y, rect.cx, rect.cy);
            });
            length = checked_int(prect - std::begin(rects));
        }

        QRect rects[256];
        int length;
    };

    template<class Cmd, class F>
    void drawMulti(Cmd const& cmd, F f)
    {
        const Rect viewport = clip_from_cmd(cmd);

        if (viewport.isempty()) {
            return ;
        }

        Rects rects(cmd, viewport);

        f(rects.rects, rects.length);
    }
} // anonymous namespace


using qtclient::RdpPainter;

void RdpPainter::resize(int width, int height)
{
    // if (isActive()) {
    //     end();
    // }
    //
    // this->cache = QPixmap(width, height);
    //
    // if (!this->cache.isNull()) {
    //     begin(&this->cache);
    // }
    //
    // fillRect(0, 0, width, height, Qt::black);
}

void RdpPainter::draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    setClipRegion(qregion(clip));
    const auto rect = qrect(cmd.rect);
    drawPatBlt(*this, &rect, 1, cmd.brush, cmd.rop, cmd.back_color, cmd.fore_color, color_ctx);
}

void RdpPainter::draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    setClipRegion(qregion(clip));
    fillRect(qrect(cmd.rect), qcolor(cmd.color, color_ctx));
}

void RdpPainter::draw(const RDPBitmapData & cmd, Bitmap const& bmp)
{
    int x = cmd.dest_left;
    int y = cmd.dest_top;
    int w = cmd.dest_right - cmd.dest_left + 1;
    int h = cmd.dest_bottom - cmd.dest_top + 1;
    setClipping(false);
    drawImage(QRect(x, y, w, h), bmp2img(x, y, w, h, bmp).mirrored(false, true));
}

void RdpPainter::draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    LineEquation equa(cmd.startx, cmd.starty, cmd.endx, cmd.endy);

    if (!equa.resolve(clip)) {
        return;
    }

    const int x1 = equa.segin.a.x;
    const int y1 = equa.segin.a.y;
    const int x2 = equa.segin.b.x;
    const int y2 = equa.segin.b.y;

    setClipRegion(qregion(clip));
    setPen(qcolor(cmd.back_color, color_ctx));
    drawLine(x1, y1, x2, y2);
}

void RdpPainter::draw(const RDPScrBlt & cmd, Rect clip)
{
    setClipRegion(qregion(clip));
    auto rect = qrect(cmd.rect);

    switch (cmd.rop) {
        case 0x00:
            fillRect(rect, Qt::black);
            break;

        case 0x55:
            save();
            setCompositionMode(Rop::Sn);
            setBrush(Qt::SolidPattern);
            drawRect(rect);
            translate(QPoint(cmd.srcx - cmd.rect.x, cmd.srcy - cmd.rect.y));
            restore();
            break;

        case 0xAA: // nothing to change
            break;

        case 0xCC:
            save();
            drawRect(rect);
            translate(QPoint(cmd.srcx - cmd.rect.x, cmd.srcy - cmd.rect.y));
            restore();
            break;

        case 0xFF:
            fillRect(rect, Qt::white);
            break;

        default: LOG(LOG_WARNING, "DEFAULT: RDPScrBlt rop = %x", cmd.rop);
            break;
    }
}

void RdpPainter::draw(const RDPMemBlt & cmd, Rect clip, Bitmap const& bmp)
{
    ImagePos p(bmp, cmd.rect, clip, cmd.srcx, cmd.srcy);

    if (p.isEmpty()) {
        return;
    }

    QPainter::CompositionMode mode;

    switch (cmd.rop) {
        case 0x00: fillRect(p.qrect(), Qt::black); return;
        case 0x22: mode = Rop::DSna; break;
        case 0x33: mode = Rop::Sn; break;
        case 0x55: mode = Rop::Dn; break;
        case 0x66: mode = Rop::DSx; break;
        case 0x88: mode = Rop::DSa; break;
        case 0x99: mode = Rop::DSxn; break;
        case 0xAA: return;
        case 0xBB: mode = Rop::DSno; break;
        case 0xCC: drawBitmap(*this, p, bmp); return;
        case 0xEE: mode = Rop::DSo; break;
        case 0xFF: fillRect(p.qrect(), Qt::white); return;

        default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
            return;
    }

    setCompositionMode(mode);
    drawBitmap(*this, p, bmp);
    setCompositionMode(Rop::Reset);
}

void RdpPainter::draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp)
{
    ImagePos p(bmp, cmd.rect, clip, cmd.srcx, cmd.srcy);

    if (p.isEmpty()) {
        return;
    }

    switch (cmd.rop) {
        // +------+-------------------------------+
        // | 0xB8 | ROP: 0x00B8074A               |
        // |      | RPN: PSDPxax                  |
        // +------+-------------------------------+
        case 0xB8: {
            QBrush brush(qcolor(cmd.fore_color, color_ctx));
            setBrush(brush);
            setCompositionMode(Rop::DSx);
            drawRect(p.qrect());
            setCompositionMode(Rop::DSa);
            drawBitmap(*this, p, bmp);
            // setBrush(brush);
            setCompositionMode(Rop::DSx);
            drawRect(p.qrect());
            setCompositionMode(Rop::Reset);
            break;
        }

        default:
            LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
            break;
    }
}

void RdpPainter::draw(const RDPDstBlt & cmd, Rect clip)
{
    setClipRegion(qregion(clip));
    const auto rect = qrect(cmd.rect);
    drawDstBlt(*this, &rect, 1, cmd.rop);
}

void RdpPainter::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
    setClipRegion(qregion(clip));
    drawMulti(cmd, [&](QRect const* rects, int count_rect){
        drawDstBlt(*this, rects, count_rect, cmd.bRop);
    });
}

void RdpPainter::draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    setClipRegion(qregion(clip));
    drawMulti(cmd, [&](QRect const* rects, int count_rect){
        setBrush(qcolor(cmd.color, color_ctx));
        drawRects(rects, count_rect);
    });
}

void RdpPainter::draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    setClipRegion(qregion(clip));
    drawMulti(cmd, [&](QRect const* rects, int count_rect){
        drawPatBlt(*this,
            rects, count_rect, cmd.brush, cmd.bRop,
            cmd.BackColor, cmd.ForeColor, color_ctx
        );
    });
}

void RdpPainter::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    setClipRegion(qregion(clip));
    const Rect viewport = clip_from_cmd(cmd);

    if (viewport.isempty()) {
        return ;
    }

    const int16_t deltax = cmd.nXSrc - cmd.rect.x;
    const int16_t deltay = cmd.nYSrc - cmd.rect.y;

    // TODO

    // auto compute_rects = [&](auto f){
    //     for_each_delta_rect(cmd, [&](Rect const& cmd_rect) {
    //         Rect drect = clip.intersect(cmd_rect);
    //         Rect src = intersect(cmd_rect.offset(deltax, deltay), this->cache);
    //         Rect trect(drect.x, drect.y, std::min(drect.cx, src.cx), std::min(drect.cy, src.cy));
    //         f(trect, src.x, src.y);
    //     });
    // };
    //
    // auto draw_blt = [&](auto const& invertPixels){
    //     compute_rects([&](Rect const& rect, int srcx, int srcy){
    //         QImage img(this->cache.copy(srcx, srcy, rect.cx, rect.cy).toImage());
    //         if (invertPixels) {
    //             img.invertPixels();
    //         }
    //         drawImage(qrect(rect), img);
    //     });
    // };
    //
    // auto draw_color = [&](Qt::GlobalColor color){
    //     QRect rects[256];
    //     auto* prect = rects;
    //     compute_rects([&](Rect const& rect, auto...){
    //         *prect++ = qrect(rect);
    //     });
    //     setBrush(color);
    //     drawRects(rects, prect - std::begin(rects));
    // };
    //
    // switch (cmd.bRop) {
    //     case 0x00: draw_color(Qt::black); break;
    //     case 0x55: draw_blt(std::true_type{}); break;
    //     case 0xAA: break;
    //     case 0xCC: draw_blt(std::false_type{}); break;
    //     case 0xFF: draw_color(Qt::white); break;
    //
    //     default: LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt rop = %x", cmd.bRop);
    //         break;
    // }
}

void RdpPainter::draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
    // TODO
    // const Rect viewport = intersect(clip, this->cache);
    // if (viewport.isempty()){
    //     return ;
    // }
    //
    // Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(viewport);
    // if (clipped_glyph_fragment_rect.isempty()) {
    //     return;
    // }
    //
    // // set a background color
    // {
    //     Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
    //     if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
    //         ajusted.cy--;
    //         ajusted = ajusted.intersect(viewport);
    //         fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, qcolor(cmd.fore_color, color_ctx));
    //     }
    // }
    //
    // bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
    //
    // const QColor color = qcolor(cmd.back_color, color_ctx);
    // const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    // const int16_t offset_x = cmd.glyph_x - cmd.bk.x;
    //
    // uint16_t draw_pos = 0;
    //
    // InStream variable_bytes({cmd.data, cmd.data_len});
    //
    // //uint8_t const * fragment_begin_position = variable_bytes.get_current();
    //
    // while (variable_bytes.in_remain()) {
    //     uint8_t data = variable_bytes.in_uint8();
    //
    //     if (data <= 0xFD) {
    //         FontChar const & fc = gly_cache.glyphs[cmd.cache_id][data].font_item;
    //         if (!fc)
    //         {
    //             LOG( LOG_INFO
    //                 , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
    //                 , cmd.cache_id, data);
    //             assert(fc);
    //         }
    //
    //         if (has_delta_bytes)
    //         {
    //             data = variable_bytes.in_uint8();
    //             if (data == 0x80)
    //             {
    //                 draw_pos += variable_bytes.in_uint16_le();
    //             }
    //             else
    //             {
    //                 draw_pos += data;
    //             }
    //         }
    //
    //         if (fc)
    //         {
    //             const int16_t x = draw_pos + cmd.bk.x + offset_x;
    //             const int16_t y = offset_y + cmd.bk.y;
    //             if (Rect(0,0,0,0) != clip.intersect(Rect(x, y, fc.incby, fc.height))){
    //
    //                 const uint8_t * fc_data            = fc.data.get();
    //                 for (int yy = 0 ; yy < fc.height; yy++)
    //                 {
    //                     uint8_t   fc_bit_mask        = 128;
    //                     for (int xx = 0 ; xx < fc.width; xx++)
    //                     {
    //                         if (!fc_bit_mask)
    //                         {
    //                             fc_data++;
    //                             fc_bit_mask = 128;
    //                         }
    //                         if (clip.contains_pt(x + fc.offsetx + xx, y + fc.offsety + yy)
    //                         && (fc_bit_mask & *fc_data))
    //                         {
    //                             fillRect(x + fc.offsetx + xx, y + fc.offsety + yy, 1, 1, color);
    //                         }
    //                         fc_bit_mask >>= 1;
    //                     }
    //                     fc_data++;
    //                 }
    //             }
    //         } else {
    //             LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache unknown FontChar");
    //         }
    //
    //         if (cmd.ui_charinc) {
    //             draw_pos += cmd.ui_charinc;
    //         }
    //
    //     } else {
    //         LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache 0xFD");
    //     }
    // }
    // //this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
    //     //draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
    //     //clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
}

void RdpPainter::draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");
}

void RdpPainter::draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonCB");
}

void RdpPainter::draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)clip;
    (void)cmd;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
}

void RdpPainter::draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx)
{

    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");
}

void RdpPainter::draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseCB");
}

void RdpPainter::draw(RDPSetSurfaceCommand const & cmd)
{
    (void)cmd;
}

void RdpPainter::draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content)
{
    // LOG(LOG_INFO,
    //     "RDPSetSurfaceCommand(x=%d y=%d width=%d height=%d)",
    //     cmd.destRect.x, cmd.destRect.y, cmd.width, cmd.height);
    QImage img(content.data, cmd.width, cmd.height, QImage::Format_RGBX8888);
    drawImage(QPoint(cmd.destRect.x, cmd.destRect.y), img);
}
