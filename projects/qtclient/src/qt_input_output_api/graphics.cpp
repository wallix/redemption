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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#include "qt_input_output_api/graphics.hpp"

#include "utils/bitmap.hpp"

#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_draw_glyphs.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
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
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"
#include "core/RDP/orders/for_each_delta_rect.hpp"
#include "gdi/clip_from_cmd.hpp"

#include <QtGui/QPainter>
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

                default: LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", rop);
                    break;
            }
        }
        else {
            auto drawMode = [&](QPainter::CompositionMode mode){
                painter.setCompositionMode(mode);
                painter.drawRects(rects, count_rect);
                painter.setCompositionMode(Rop::Reset);
            };

            switch (rop) {
                case 0x00: // blackness
                    painter.setBrush(Qt::black);
                    painter.drawRects(rects, count_rect);
                    break;

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
                case 0xF0:
                    painter.setBrush(backColor);
                    painter.drawRects(rects, count_rect);
                    break;

                case 0xF5: drawMode(Rop::PDno); break;
                case 0xFA: drawMode(Rop::DPo); break;

                case 0xFF: // whiteness
                    painter.setBrush(Qt::white);
                    painter.drawRects(rects, count_rect);
                    break;

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

        QRect qrect() const noexcept { return QRect(x, y, w, h); }
        bool isEmpty() const noexcept { return w <= 0 || h <= 0; }

        int x;
        int y;
        int w = 0;
        int h = 0;

        int srcx;
        int srcy;

    };

    void drawImage(QPainter& painter, ImagePos const& p, Bitmap const& bmp)
    {
        QImage::Format format/* = QImage::Format_Invalid*/;
        REDEMPTION_DIAGNOSTIC_PUSH()
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
        switch (bmp.bpp()) {
            case BitsPerPixel{15}: format = QImage::Format_RGB555; break;
            case BitsPerPixel{16}: format = QImage::Format_RGB16;  break;
            case BitsPerPixel{24}: format = QImage::Format_RGB888; break;
            case BitsPerPixel{32}: format = QImage::Format_RGB32;  break;
            default: assert(!"unknown bpp");
        }
        REDEMPTION_DIAGNOSTIC_POP()

        QImage qbitmap(bmp.data() + bmp.line_size() * p.srcy + p.srcx, p.w, p.h, bmp.line_size(), format);

        if (format == QImage::Format_RGB888) {
            // inplace transformation
            std::move(qbitmap).rgbSwapped();
        }

        // inplace transformation
        std::move(qbitmap).mirrored(false, true);

        painter.drawImage(p.qrect(), qbitmap);
    }

    void drawImage(QPainter& painter, Bitmap const& bmp, Rect drect, Rect clip, int sx, int sy)
    {
        ImagePos p(bmp, drect, clip, sx, sy);

        if (p.isEmpty()) {
            return;
        }

        drawImage(painter, p, bmp);
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
            length = prect - std::begin(rects);
        }

        QRect rects[256];
        int length;
    };

    template<class Cmd, class F>
    void drawMulti(Cmd const& cmd, Rect clip, F f)
    {
        const Rect viewport = clip.intersect(clip_from_cmd(cmd));

        if (viewport.isempty()) {
            return ;
        }

        Rects rects(cmd, viewport);

        f(rects.rects, rects.length);
    }
} // anonymous namespace


using qtclient::Graphics;

void Graphics::resize(int width, int height)
{
    if (this->painter.isActive()) {
        this->painter.end();
    }

    this->cache = QPixmap(width, height);

    if (!this->cache.isNull()) {
        this->painter.begin(&this->cache);
    }

    this->painter.fillRect(0, 0, width, height, Qt::black);
}

void Graphics::draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    const auto rect = qrect(intersect(clip, this->cache).intersect(cmd.rect));
    drawPatBlt(this->painter,
        &rect, 1, cmd.brush, cmd.rop, cmd.back_color, cmd.fore_color, color_ctx);
}

void Graphics::draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    Rect rect(cmd.rect.intersect(clip));
    this->painter.fillRect(qrect(rect), qcolor(cmd.color, color_ctx));
}

void Graphics::draw(const RDPBitmapData & cmd, Bitmap const& bmp)
{
    Rect rectBmp(
        cmd.dest_left,
        cmd.dest_top,
        (cmd.dest_right - cmd.dest_left + 1),
        (cmd.dest_bottom - cmd.dest_top + 1));
    Rect clip(0, 0, this->cache.width(), this->cache.height());

    drawImage(this->painter, bmp, rectBmp, clip, 0, 0);
}

void Graphics::draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    LineEquation equa(cmd.startx, cmd.starty, cmd.endx, cmd.endy);

    if (!equa.resolve(clip)) {
        return;
    }

    const int x1 = equa.segin.a.x;
    const int y1 = equa.segin.a.y;
    const int x2 = equa.segin.b.x;
    const int y2 = equa.segin.b.y;

    this->painter.setPen(qcolor(cmd.back_color, color_ctx));
    this->painter.drawLine(x1, y1, x2, y2);
}

void Graphics::draw(const RDPScrBlt & cmd, Rect clip)
{
    const Rect drect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
    if (drect.isempty()) {
        return;
    }

    const int srcx = drect.x + cmd.srcx - cmd.rect.x;
    const int srcy = drect.y + cmd.srcy - cmd.rect.y;

    switch (cmd.rop) {

        case 0x00:
            this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;

        case 0x55: {
            QImage img(this->cache.copy(srcx, srcy, drect.cx, drect.cy).toImage());
            img.invertPixels();
            this->painter.drawImage(QRect(drect.x, drect.y, drect.cx, drect.cy), img);
            break;
        }

        case 0xAA: // nothing to change
            break;

        case 0xCC: {
            QImage img(this->cache.copy(srcx, srcy, drect.cx, drect.cy).toImage());
            this->painter.drawImage(QRect(drect.x, drect.y, drect.cx, drect.cy), img);
            break;
        }

        case 0xFF:
            this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;
        default: LOG(LOG_WARNING, "DEFAULT: RDPScrBlt rop = %x", cmd.rop);
            break;
    }
}

void Graphics::draw(const RDPMemBlt & cmd, Rect clip, Bitmap const& bmp)
{
    ImagePos p(bmp, cmd.rect, clip, cmd.srcx, cmd.srcy);

    if (p.isEmpty()) {
        return;
    }

    auto drawWithMode = [&](QPainter::CompositionMode mode){
        this->painter.setCompositionMode(mode);
        drawImage(this->painter, p, bmp);
        this->painter.setCompositionMode(Rop::Reset);
    };

    switch (cmd.rop) {
        case 0x00: this->painter.fillRect(p.qrect(), Qt::black); break;
        case 0x22: drawWithMode(Rop::DSna); break;
        case 0x33: drawWithMode(Rop::Sn); break;
        case 0x55: drawWithMode(Rop::Dn); break;
        case 0x66: drawWithMode(Rop::DSx); break;
        case 0x88: drawWithMode(Rop::DSa); break;
        case 0x99: drawWithMode(Rop::DSxn); break;
        case 0xAA: break;
        case 0xBB: drawWithMode(Rop::DSno); break;
        case 0xCC: drawImage(this->painter, p, bmp); break;
        case 0xEE: drawWithMode(Rop::DSo); break;
        case 0xFF: this->painter.fillRect(p.qrect(), Qt::white); break;

        default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
            break;
    }
}

void Graphics::draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp)
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
            this->painter.setBrush(brush);
            this->painter.setCompositionMode(Rop::DSx);
            this->painter.drawRect(p.qrect());
            this->painter.setCompositionMode(Rop::DSa);
            drawImage(this->painter, p, bmp);
            // this->painter.setBrush(brush);
            this->painter.setCompositionMode(Rop::DSx);
            this->painter.drawRect(p.qrect());
            this->painter.setCompositionMode(Rop::Reset);
            break;
        }

        default:
            LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
            break;
    }
}

void Graphics::draw(const RDPDstBlt & cmd, Rect clip)
{
    const auto rect = qrect(intersect(clip, this->cache).intersect(cmd.rect));
    drawDstBlt(this->painter, &rect, 1, cmd.rop);
}

void Graphics::draw(const RDPMultiDstBlt & cmd, Rect clip)
{
    drawMulti(cmd, intersect(clip, this->cache), [&](QRect const* rects, int count_rect){
        drawDstBlt(this->painter, rects, count_rect, cmd.bRop);
    });
}

void Graphics::draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    drawMulti(cmd, intersect(clip, this->cache), [&](QRect const* rects, int count_rect){
        this->painter.setBrush(qcolor(cmd._Color, color_ctx));
        this->painter.drawRects(rects, count_rect);
    });
}

void Graphics::draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    drawMulti(cmd, intersect(clip, this->cache), [&](QRect const* rects, int count_rect){
        drawPatBlt(this->painter,
            rects, count_rect, cmd.brush, cmd.bRop,
            cmd.BackColor, cmd.ForeColor, color_ctx
        );
    });
}

void Graphics::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip)
{
    const Rect viewport = intersect(clip, this->cache).intersect(clip_from_cmd(cmd));

    if (viewport.isempty()) {
        return ;
    }

    const int deltax = cmd.nXSrc - cmd.rect.x;
    const int deltay = cmd.nYSrc - cmd.rect.y;

    auto compute_rects = [&](auto f){
        for_each_delta_rect(cmd, [&](Rect const& cmd_rect) {
            Rect drect = clip.intersect(cmd_rect);
            Rect src = intersect(cmd_rect.offset(deltax, deltay), this->cache);
            Rect trect(drect.x, drect.y, std::min(drect.cx, src.cx), std::min(drect.cy, src.cy));
            f(trect, src.x, src.y);
        });
    };

    auto draw_blt = [&](auto const& invertPixels){
        compute_rects([&](Rect const& rect, int srcx, int srcy){
            QImage img(this->cache.copy(srcx, srcy, rect.cx, rect.cy).toImage());
            if (invertPixels) {
                img.invertPixels();
            }
            this->painter.drawImage(qrect(rect), img);
        });
    };

    auto draw_color = [&](Qt::GlobalColor color){
        QRect rects[256];
        auto* prect = rects;
        compute_rects([&](Rect const& rect, auto...){
            *prect = qrect(rect);
            ++prect;
        });
        this->painter.setBrush(color);
        this->painter.drawRects(rects, prect - std::begin(rects));
    };

    switch (cmd.bRop) {
        case 0x00: draw_color(Qt::black); break;
        case 0x55: draw_blt(std::true_type{}); break;
        case 0xAA: break;
        case 0xCC: draw_blt(std::false_type{}); break;
        case 0xFF: draw_color(Qt::white); break;

        default: LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt rop = %x", cmd.bRop);
            break;
    }
}

void Graphics::draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache)
{
    Rect screen_rect = clip.intersect(this->cache.width(), this->cache.height());
    if (screen_rect.isempty()){
        return ;
    }

    Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
    if (clipped_glyph_fragment_rect.isempty()) {
        return;
    }

    // set a background color
    {
        Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
        if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
            ajusted.cy--;
            ajusted = ajusted.intersect(screen_rect);
            this->painter.fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, qcolor(cmd.fore_color, color_ctx));
        }
    }

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));

    const QColor color = qcolor(cmd.back_color, color_ctx);
    const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

    uint16_t draw_pos = 0;

    InStream variable_bytes({cmd.data, cmd.data_len});

    //uint8_t const * fragment_begin_position = variable_bytes.get_current();

    while (variable_bytes.in_remain()) {
        uint8_t data = variable_bytes.in_uint8();

        if (data <= 0xFD) {
            FontChar const & fc = gly_cache.glyphs[cmd.cache_id][data].font_item;
            if (!fc)
            {
                LOG( LOG_INFO
                    , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
                    , cmd.cache_id, data);
                assert(fc);
            }

            if (has_delta_bytes)
            {
                data = variable_bytes.in_uint8();
                if (data == 0x80)
                {
                    draw_pos += variable_bytes.in_uint16_le();
                }
                else
                {
                    draw_pos += data;
                }
            }

            if (fc)
            {
                const int16_t x = draw_pos + cmd.bk.x + offset_x;
                const int16_t y = offset_y + cmd.bk.y;
                if (Rect(0,0,0,0) != clip.intersect(Rect(x, y, fc.incby, fc.height))){

                    const uint8_t * fc_data            = fc.data.get();
                    for (int yy = 0 ; yy < fc.height; yy++)
                    {
                        uint8_t   fc_bit_mask        = 128;
                        for (int xx = 0 ; xx < fc.width; xx++)
                        {
                            if (!fc_bit_mask)
                            {
                                fc_data++;
                                fc_bit_mask = 128;
                            }
                            if (clip.contains_pt(x + fc.offsetx + xx, y + fc.offsety + yy)
                            && (fc_bit_mask & *fc_data))
                            {
                                this->painter.fillRect(x + fc.offsetx + xx, y + fc.offsety + yy, 1, 1, color);
                            }
                            fc_bit_mask >>= 1;
                        }
                        fc_data++;
                    }
                }
            } else {
                LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache unknown FontChar");
            }

            if (cmd.ui_charinc) {
                draw_pos += cmd.ui_charinc;
            }

        } else {
            LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache 0xFD");
        }
    }
    //this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
        //draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
        //clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
}

void Graphics::draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");
}

void Graphics::draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx)
{

    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonCB");
}

void Graphics::draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx)
{
    (void)clip;
    (void)cmd;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
}

void Graphics::draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx)
{

    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");
}

void Graphics::draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx)
{

    (void)cmd;
    (void)clip;
    (void)color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseCB");
}

void Graphics::draw(const RDP::FrameMarker & order)
{
    (void)order;
}

void Graphics::draw(RDPSetSurfaceCommand const & cmd)
{
    (void)cmd;
}

void Graphics::draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content)
{
    // LOG(LOG_INFO,
    //     "RDPSetSurfaceCommand(x=%d y=%d width=%d height=%d)",
    //     cmd.destRect.x, cmd.destRect.y, cmd.width, cmd.height);
    QImage img(content.data, cmd.width, cmd.height, QImage::Format_RGBX8888);
    this->painter.drawImage(QPoint(cmd.destRect.x, cmd.destRect.y), img);
}

// void Graphics::drawLine(int x1, int y1, int x2, int y2, RDPColor rdpColor, gdi::ColorCtx color_ctx)
// {
//     this->painter.setPen(qcolor(rdpColor, color_ctx));
//     this->painter.drawLine(x1, y1, x2, y2);
// }
