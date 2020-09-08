
#pragma once

#include "gdi/graphic_api.hpp"
#include "utils/raster_ops.hpp"
#include "utils/bitmap.hpp"

#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/rdp_draw_glyphs.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
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

#include <QtGui/QPainter>
#include <QtGui/QPixmap>


namespace qtclient
{

class Graphics : gdi::GraphicApi
{
public:
    Graphics()
    {}

    void resize(int width, int height)
    {
        if (this->painter.isActive()) {
            this->painter.end();
        }

        this->cache = QPixmap(width, height);

        if (!this->cache.isNull()) {
            this->painter.begin(&this->cache);
        }
    }

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        const Rect rect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
        // this->setClip(rect.x, rect.y, rect.cx, rect.cy);

        const QColor backColor = qcolor(cmd.back_color, color_ctx);
        const QColor foreColor = qcolor(cmd.fore_color, color_ctx);

        auto drawModeWithBrush = [&](Qt::BrushStyle style, QPainter::CompositionMode mode){
            this->painter.setBrush(QBrush(backColor, style));
            this->painter.setCompositionMode(mode);
            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->painter.setCompositionMode(Rop::Reset);
            this->painter.setBrush(Qt::SolidPattern);
        };

        auto drawMode = [&](QPainter::CompositionMode mode){
            this->painter.setCompositionMode(mode);
            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->painter.setCompositionMode(Rop::Reset);
        };

        if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {
            switch (cmd.rop) {
                case 0x5A: drawModeWithBrush(Qt::Dense4Pattern, Rop::DPx); break;

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
                case 0xF0:
                    this->painter.setPen(Qt::NoPen);
                    this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                    this->painter.setBrush(QBrush(foreColor, Qt::Dense4Pattern));
                    this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    this->painter.setBrush(Qt::SolidPattern);
                    break;
                default: LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", cmd.rop);
                    break;
            }
        }
        else {
            switch (cmd.rop) {
                case 0x00: // blackness
                    this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::black);
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
                    this->painter.setPen(Qt::NoPen);
                    this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                    this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    break;

                case 0xF5: drawMode(Rop::PDno); break;
                case 0xFA: drawMode(Rop::DPo); break;

                case 0xFF: // whiteness
                    this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::white);
                    break;

                default: LOG(LOG_WARNING, "RDPPatBlt rop = %x", cmd.rop);
                    break;
            }
        }
    }


    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        Rect rect(cmd.rect.intersect(clip));
        // this->setClip(rect.x, rect.y, rect.cx, rect.cy);

        this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, qcolor(cmd.color, color_ctx));
    }

    void draw(const RDPBitmapData & cmd, Bitmap const& bmp) override
    {
        Rect rectBmp(
            cmd.dest_left,
            cmd.dest_top,
            (cmd.dest_right - cmd.dest_left + 1),
            (cmd.dest_bottom - cmd.dest_top + 1));
        Rect clip(0, 0, this->cache.width(), this->cache.height());

        this->drawImage(bmp, rectBmp, clip, 0, 0);
    }

    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void) clip;

        // TODO clipping
        this->painter.setPen(qcolor(cmd.back_color, color_ctx));
        // this->setClip(clip.x, clip.y, clip.cx, clip.cy);
        this->painter.drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override
    {
        const Rect drect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
        if (drect.isempty()) {
            return;
        }
        // // this->setClip(drect.x, drect.y, drect.cx, drect.cy);

        int srcx(drect.x + cmd.srcx - cmd.rect.x);
        int srcy(drect.y + cmd.srcy - cmd.rect.y);

        switch (cmd.rop) {

            case 0x00:
                this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                break;

            case 0x55: {
                QImage img(this->cache.toImage().copy(srcx, srcy, drect.cx, drect.cy));
                img.invertPixels();
                this->painter.drawImage(QRect(drect.x, drect.y, drect.cx, drect.cy), img);
                break;
            }

            case 0xAA: // nothing to change
                break;

            case 0xCC: {
                QImage img(this->cache.toImage().copy(srcx, srcy, drect.cx, drect.cy));
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

    void draw(const RDPMemBlt & cmd, Rect clip, Bitmap const& bmp) override
    {
        // this->setClip(drect.x, drect.y, drect.cx, drect.cy);

        ImagePos p(bmp, cmd.rect, clip, cmd.srcx, cmd.srcy);

        if (p.isEmpty()) {
            return;
        }

        auto drawWithMode = [&](QPainter::CompositionMode mode){
            this->painter.setCompositionMode(mode);
            this->_drawImage(p, bmp);
            this->painter.setCompositionMode(Rop::Reset);
        };

        switch (cmd.rop) {
            case 0x00: this->_fillRect(p, Qt::black); break;
            case 0x22: drawWithMode(Rop::DSna); break;
            case 0x33: drawWithMode(Rop::Sn); break;
            case 0x55: drawWithMode(Rop::Dn); break;
            case 0x66: drawWithMode(Rop::DSx); break;
            case 0x88: drawWithMode(Rop::DSa); break;
            case 0x99: drawWithMode(Rop::DSxn); break;
            case 0xAA: break;
            case 0xBB: drawWithMode(Rop::DSno); break;
            case 0xCC: this->_drawImage(p, bmp); break;
            case 0xEE: drawWithMode(Rop::DSo); break;
            case 0xFF: this->_fillRect(p, Qt::white); break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
                break;
        }
    }

    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const& bmp) override
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
                this->_drawRect(p);
                this->painter.setCompositionMode(Rop::DSa);
                this->_drawImage(p, bmp);
                // this->painter.setBrush(brush);
                this->painter.setCompositionMode(Rop::DSx);
                this->_drawRect(p);
                this->painter.setCompositionMode(Rop::Reset);
                break;
            }

            default:
                LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
                break;
        }
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override
    {
        ImagePos p(cmd.rect.cx, cmd.rect.cy, cmd.rect, clip, cmd.rect.x, cmd.rect.y);

        if (p.isEmpty()) {
            return;
        }

        switch (cmd.rop) {
            case 0x00: this->_fillRect(p, Qt::black); break;

            case 0x55: {
                this->painter.setCompositionMode(Rop::Dn);
                this->_drawRect(p);
                this->painter.setCompositionMode(Rop::Reset);
                break;
            }

            case 0xAA: break;
            case 0xFF: this->_fillRect(p, Qt::white); break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPDestBlt rop = %x", cmd.rop);
                break;
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override
    {
        const Rect viewport = computeClip(clip).intersect(to_rect(cmd));

        if (viewport.isempty()) {
            return ;
        }

        Rects rects(cmd, viewport);

        switch (cmd.bRop) {
            case 0x00:
                this->painter.setBrush(Qt::black);
                this->painter.drawRects(rects.rects, rects.length);
                break;

            case 0x55: {
                this->painter.setCompositionMode(Rop::Dn);
                this->painter.drawRects(rects.rects, rects.length);
                this->painter.setCompositionMode(Rop::Reset);
                break;
            }

            case 0xAA:
                break;

            case 0xFF:
                this->painter.setBrush(Qt::white);
                this->painter.drawRects(rects.rects, rects.length);
                break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMultiDstBlt rop = %x", cmd.bRop);
                break;
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void) cmd;
        (void) clip;
        (void) color_ctx;

        LOG(LOG_WARNING, "DEFAULT: RDPMultiOpaqueRect");
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override
    {
        (void) color_ctx;
        (void) cmd;
        (void) clip;
        LOG(LOG_WARNING, "DEFAULT: RDPMultiPatBlt");
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override
    {
        (void) cmd;
        (void) clip;

        LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt");
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {

        Rect screen_rect = clip.intersect(this->cache.width(), this->cache.height());
        if (screen_rect.isempty()){
            return ;
        }
        // this->setClip(screen_rect.x, screen_rect.y, screen_rect.cx, screen_rect.cy);

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

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");

        /*RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolygonCB");

        /*RDPPolygonCB new_cmd24 = cmd;
        new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
        new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) clip;
        (void) cmd;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
        /*RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");

        /*RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPEllipseCB");
    /*
        RDPEllipseCB new_cmd24 = cmd;
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDP::FrameMarker & order) override {
        (void) order;
    }

//     using ClientOutputGraphicAPI::draw;
    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {
    }

    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override {
        LOG(LOG_INFO, "RDPSetSurfaceCommand(x=%d y=%d width=%d height=%d)", cmd.destRect.x, cmd.destRect.y,
                cmd.width, cmd.height);
        QImage img(content.data, cmd.width, cmd.height, QImage::Format_RGBX8888);
        this->painter.drawImage(QPoint(cmd.destRect.x, cmd.destRect.y), img);
    }

    void drawLine(int x1, int y1, int x2, int y2, RDPColor rdpColor, gdi::ColorCtx color_ctx)
    {
        this->painter.setPen(qcolor(rdpColor, color_ctx));
        this->painter.drawLine(x1, y1, x2, y2);
    }


private:
    static QColor qcolor(RDPColor rdpColor, gdi::ColorCtx const& color_ctx)
    {
        BGRColor bgr = color_decode(rdpColor, color_ctx);
        return {bgr.red(), bgr.green(), bgr.blue()};
    }

    Rect computeClip(Rect const& clip)
    {
        return clip.intersect(this->cache.width(), this->cache.height());
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

        QRect getQRect() const { return QRect(x, y, w, h); }

        bool isEmpty() const noexcept { return w <= 0 || h <= 0; }

        int x;
        int y;
        int w = 0;
        int h = 0;

        int srcx;
        int srcy;
    };

    void drawImage(Bitmap const& bmp, Rect drect, Rect clip, int sx, int sy)
    {
        ImagePos p(bmp, drect, clip, sx, sy);

        if (p.isEmpty()) {
            return;
        }

        this->_drawImage(p, bmp);
    }

    void _drawImage(ImagePos const& p, Bitmap const& bmp)
    {
        QImage::Format format/* = QImage::Format_Invalid*/;
        switch (bmp.bpp()) {
            case BitsPerPixel{15}: format = QImage::Format_RGB555; break;
            case BitsPerPixel{16}: format = QImage::Format_RGB16;  break;
            case BitsPerPixel{24}: format = QImage::Format_RGB888; break;
            case BitsPerPixel{32}: format = QImage::Format_RGB32;  break;
            default: return;
        }

        QImage qbitmap(bmp.data() + bmp.line_size() * p.srcy + p.srcx, p.w, p.h, bmp.line_size(), format);

        if (format == QImage::Format_RGB888) {
            // inplace transformation
            std::move(qbitmap).rgbSwapped();
        }

        // inplace transformation
        std::move(qbitmap).mirrored(false, true);

        this->painter.drawImage(QRect(p.x, p.y, p.w, p.h), qbitmap);
    }

    void _drawRect(ImagePos const& p)
    {
        this->painter.drawRect(p.x, p.y, p.w, p.w);
    }

    void _fillRect(ImagePos const& p, Qt::GlobalColor color)
    {
        this->painter.fillRect(p.x, p.y, p.w, p.h, color);
    }

    // TODO removed when RDPMultiDstBlt and RDPMultiOpaqueRect contains a rect member
    //@{
    static Rect to_rect(RDPMultiDstBlt const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    static Rect to_rect(RDPMultiOpaqueRect const & cmd)
    { return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight); }

    static Rect to_rect(RDP::RDPMultiPatBlt const & cmd)
    { return cmd.rect; }
    //@}

    template<class RDPMulti, class FRect>
    static void draw_multi(const RDPMulti & cmd, Rect clip, FRect f)
    {
        Rect cmd_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
            f(clip.intersect(cmd_rect));
        }
    }

    struct Rects
    {
        template<class RDPMulti>
        Rects(const RDPMulti & cmd, Rect clip)
        {
            QRect* prect = rects;
            draw_multi(cmd, clip, [&](Rect const& rect) {
                *prect = QRect(rect.x, rect.y, rect.cx, rect.cy);
            });
            length = prect - std::begin(rects);
        }

        QRect rects[256];
        int length;
    };

    struct Rop
    {
        using Mode = QPainter::CompositionMode;

        static constexpr Mode DSna = Mode::RasterOp_SourceXorDestination;
        static constexpr Mode DPx = Mode::RasterOp_SourceXorDestination;
        static constexpr Mode DPon = Mode::RasterOp_NotSourceOrNotDestination;
        static constexpr Mode Pn = Mode::RasterOp_NotSource;
        static constexpr Mode PDna = Mode::RasterOp_SourceAndNotDestination;
        static constexpr Mode Dn = Mode::RasterOp_NotDestination;
        static constexpr Mode DPan = Mode::RasterOp_NotSourceAndNotDestination;
        static constexpr Mode DPa = Mode::RasterOp_SourceAndDestination;
        static constexpr Mode PDxn = Mode::RasterOp_NotSourceXorDestination;
        static constexpr Mode DPno = Mode::RasterOp_NotSourceOrDestination;
        static constexpr Mode PDno = Mode::RasterOp_SourceOrNotDestination;
        static constexpr Mode DPo = Mode::RasterOp_SourceOrDestination;
        static constexpr Mode Sn = Mode::RasterOp_NotSource;
        static constexpr Mode DSx = Mode::RasterOp_SourceXorDestination;
        static constexpr Mode DSa = Mode::RasterOp_SourceAndDestination;
        static constexpr Mode DSxn = Mode::RasterOp_NotSourceXorDestination;
        static constexpr Mode DSno = Mode::RasterOp_NotSourceOrDestination;
        static constexpr Mode DSo = Mode::RasterOp_SourceOrDestination;

        static constexpr Mode Reset = Mode::CompositionMode_SourceOver;
    };

    QPixmap cache;
    QPainter painter;
};

}
