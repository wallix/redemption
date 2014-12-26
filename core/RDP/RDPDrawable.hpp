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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Poelen Jonathan, Raphael Zhou, Meng Tan
*/

#ifndef _REDEMPTION_CORE_RDP_RDPDRAWABLE_HPP_
#define _REDEMPTION_CORE_RDP_RDPDRAWABLE_HPP_

#include <utility>

#include "drawable.hpp"
#include "font.hpp"

#include "RDPGraphicDevice.hpp"
#include "orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "orders/RDPOrdersPrimaryScrBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "orders/RDPOrdersPrimaryDestBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "orders/RDPOrdersPrimaryPatBlt.hpp"
#include "orders/RDPOrdersPrimaryMemBlt.hpp"
#include "orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "orders/RDPOrdersPrimaryLineTo.hpp"
#include "orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "orders/RDPOrdersPrimaryPolyline.hpp"
#include "orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "orders/RDPOrdersSecondaryGlyphCache.hpp"

#include "pointer.hpp"
#include "bitmapupdate.hpp"
#include "caches/glyphcache.hpp"
#include "png.hpp"
#include "text_metrics.hpp"

#include "CaptureDevice.hpp"

// orders provided to RDPDrawable *MUST* be 24 bits
// drawable also only support 24 bits orders
class RDPDrawable : public RDPGraphicDevice, public RDPCaptureDevice
{
    using Color = Drawable::Color;

    Drawable drawable;
//    GlyphCache gly_cache;
    int frame_start_count;
    int order_bpp;
    BGRPalette mod_palette_rgb;

public:
    RDPDrawable(const uint16_t width, const uint16_t height, int order_bpp)
    : drawable(width, height)
    , frame_start_count(0)
    , order_bpp(order_bpp)
    , mod_palette_rgb(BGRPalette::classic_332())
    {
        REDASSERT(order_bpp);
    }

    const uint8_t * data() const noexcept {
        return this->drawable.data();
    }

    uint16_t width() const noexcept {
        return this->drawable.width();
    }

    uint16_t height() const noexcept {
        return this->drawable.height();
    }

    unsigned size() const noexcept {
        return this->drawable.size();
    }

    size_t rowsize() const noexcept {
        return this->drawable.rowsize();
    }

    size_t pix_len() const noexcept {
        return this->drawable.pix_len();
    }

    void set_mouse_cursor_pos(int x, int y) {
        this->drawable.set_mouse_cursor_pos(x, y);
    }

    void show_mouse_cursor(bool x) {
        this->drawable.dont_show_mouse_cursor = !x;
    }

    // TODO FIXME temporary
    //@{
    Drawable & impl() noexcept {
        return this->drawable;
    }

    const Drawable & impl() const noexcept {
        return this->drawable;
    }
    //@}

private:
    Color u32_to_color(uint32_t color) const
    {
        return this->drawable.u32bgr_to_color(color);
    }

    Color u32rgb_to_color(BGRColor color) const
    {
        return this->u32_to_color((this->order_bpp == 24)
            ? color
            : ::color_decode_opaquerect(color, this->order_bpp, this->mod_palette_rgb)
        );
    }

    std::pair<Color, Color> u32rgb_to_color(BGRColor color1, BGRColor color2) const
    {
        if (this->order_bpp == 24) {
            return std::pair<Color, Color>{this->u32_to_color(color1), this->u32_to_color(color2)};
        }
        return std::pair<Color, Color>{
            this->u32_to_color(::color_decode_opaquerect(color1, this->order_bpp, this->mod_palette_rgb)),
            this->u32_to_color(::color_decode_opaquerect(color2, this->order_bpp, this->mod_palette_rgb))
        };
    }

public:
    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        this->drawable.set_row(rownum, data);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        this->drawable.opaquerect(trect, this->u32rgb_to_color(cmd.color));
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        this->drawable.ellipse(cmd.el, cmd.bRop2, cmd.fillMode, this->u32rgb_to_color(cmd.color));
    }

    TODO("This will draw a standard ellipse without brush style")
    void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        this->drawable.ellipse(cmd.el, cmd.brop2, cmd.fill_mode, this->u32rgb_to_color(cmd.back_color));
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect drect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        if (drect.isempty()){ return; }
        // adding delta move dest to source
        const signed int deltax = cmd.srcx - cmd.rect.x;
        const signed int deltay = cmd.srcy - cmd.rect.y;
        this->drawable.scrblt(drect.x + deltax, drect.y + deltay, drect, cmd.rop);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        this->drawable.destblt(trect, cmd.rop);
    }

private:
    template<class RDPMulti, class FRect>
    void draw_multi(const RDPMulti & cmd, const Rect & clip, FRect f)
    {
        const Rect clip_drawable_cmd_intersect
          = clip.intersect(this->drawable.width(), this->drawable.height())
          .intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight));

        Rect cmd_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
            f(clip_drawable_cmd_intersect.intersect(cmd_rect));
        }
    }

public:
    void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        this->draw_multi(cmd, clip, [&](const Rect & trect) {
            this->drawable.destblt(trect, cmd.bRop);
        });
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip)
    {
        const Color color = this->u32rgb_to_color(cmd._Color);
        this->draw_multi(cmd, clip, [color, this](const Rect & trect) {
            this->drawable.opaquerect(trect, color);
        });
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip)
    {
        TODO(" PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.");
        if ((cmd.bRop == 0xF0) && (cmd.BrushStyle == 0x03)) {
            enum { BackColor, ForeColor };
            auto colors = this->u32rgb_to_color(cmd.BackColor, cmd.ForeColor);
            uint8_t brush_data[8];
            memcpy(brush_data, cmd.BrushExtra, 7);
            brush_data[7] = cmd.BrushHatch;
            this->draw_multi(cmd, clip, [&](const Rect & trect) {
                this->drawable.patblt_ex(trect, cmd.bRop, std::get<BackColor>(colors), std::get<ForeColor>(colors), brush_data);
            });
        }
        else {
            const Color color = this->u32rgb_to_color(cmd.BackColor);
            this->draw_multi(cmd, clip, [&](const Rect & trect) {
                this->drawable.patblt(trect, cmd.bRop, color);
            });
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip)
    {
        const signed int deltax = cmd.nXSrc - cmd.rect.x;
        const signed int deltay = cmd.nYSrc - cmd.rect.y;
        this->draw_multi(cmd, clip, [&](const Rect & trect) {
            this->drawable.scrblt(trect.x + deltax, trect.y + deltay, trect, cmd.bRop);
        });
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        TODO("PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.");

        if ((cmd.rop == 0xF0) && (cmd.brush.style == 0x03)) {
            enum { BackColor, ForeColor };
            auto colors = this->u32rgb_to_color(cmd.back_color, cmd.fore_color);
            uint8_t brush_data[8];
            memcpy(brush_data, cmd.brush.extra, 7);
            brush_data[7] = cmd.brush.hatch;

            this->drawable.patblt_ex(trect, cmd.rop, std::get<BackColor>(colors), std::get<ForeColor>(colors), brush_data);
        }
        else {
            this->drawable.patblt(trect, cmd.rop, this->u32rgb_to_color(cmd.back_color));
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        const Rect& rect = clip.intersect(cmd.rect);
        if (rect.isempty()){
            return ;
        }

        switch (cmd.rop) {
        case 0x00:
            this->drawable.black_color(rect);
        break;
        case 0xFF:
            this->drawable.white_color(rect);
        break;
        case 0x55:
            this->drawable.mem_blt_invert(rect, bmp
                , cmd.srcx + (rect.x - cmd.rect.x)
                , cmd.srcy + (rect.y - cmd.rect.y));
        break;
        case 0xCC:
            this->drawable.mem_blt(rect, bmp
                , cmd.srcx + (rect.x - cmd.rect.x)
                , cmd.srcy + (rect.y - cmd.rect.y));
        break;
        case 0x22:  // dest = dest AND (NOT source)
        case 0x66:  // dest = source XOR dest (SRCINVERT)
        case 0x88:  // dest = source AND dest (SRCAND)
        case 0xBB:  // dest = (NOT source) OR dest (MERGEPAINT)
        case 0xEE:  // dest = source OR dest (SRCPAINT)
            this->drawable.mem_blt_ex(rect, bmp
                , cmd.srcx + (rect.x - cmd.rect.x)
                , cmd.srcy + (rect.y - cmd.rect.y)
                , cmd.rop);
            break;
        default:
            // should not happen
            //LOG(LOG_INFO, "Unsupported Rop=0x%02X", cmd.rop);
        break;
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        const Rect& rect = clip.intersect(cmd.rect);
        if (rect.isempty()){
            return ;
        }

        this->drawable.mem_3_blt(rect, bmp
            , cmd.srcx + (rect.x  - cmd.rect.x)
            , cmd.srcy + (rect.y  - cmd.rect.y)
            , cmd.rop
            , this->u32rgb_to_color(cmd.fore_color)
        );
    }

    /*
     *
     *            +----+----+
     *            |\   |   /|  4 cases.
     *            | \  |  / |  > Case 1 is the normal case
     *            |  \ | /  |  > Case 2 has a negative coeff
     *            | 3 \|/ 2 |  > Case 3 and 4 are the same as
     *            +----0---->x    Case 1 and 2 but one needs to
     *            | 4 /|\ 1 |     exchange begin and end.
     *            |  / | \  |
     *            | /  |  \ |
     *            |/   |   \|
     *            +----v----+
     *                 y
     *  Anyway, we base the line drawing on bresenham's algorithm
     */
    void draw(const RDPLineTo & lineto, const Rect & clip)
    {
        this->draw_line(lineto.back_mode, lineto.startx, lineto.starty, lineto.endx, lineto.endy, lineto.rop2,
                        this->u32rgb_to_color(lineto.pen.color), clip);
    }

private:
    void draw_line(uint16_t BackMode, int16_t nXStart, int16_t nYStart,
                   int16_t nXEnd, int16_t nYEnd, uint8_t bRop2,
                   Color color, const Rect & clip) {
        LineEquation equa(nXStart, nYStart, nXEnd, nYEnd);
        int startx = 0;
        int starty = 0;
        int endx = 0;
        int endy = 0;
        if (equa.resolve(clip)) {
            startx = equa.segin.a.x;
            starty = equa.segin.a.y;
            endx = equa.segin.b.x;
            endy = equa.segin.b.y;
        }
        else {
            return;
        }

        if (startx == endx){
            this->drawable.vertical_line(BackMode,
                                         startx,
                                         std::min(starty, endy),
                                         std::max(starty, endy),
                                         bRop2,
                                         color);
        }
        else if (starty == endy){
            this->drawable.horizontal_line(BackMode,
                                           std::min(startx, endx),
                                           starty,
                                           std::max(startx, endx),
                                           bRop2,
                                           color);

        }
        else if (startx <= endx){
            this->drawable.line(BackMode,
                                startx,
                                starty,
                                endx,
                                endy,
                                bRop2,
                                color);
        }
        else {
            this->drawable.line(BackMode,
                                endx,
                                endy,
                                startx,
                                starty,
                                bRop2,
                                color);
        }
    }

/*
public:
    virtual void draw(const RDPGlyphCache & cmd)
    {
        //LOG( LOG_INFO
        //   , "RDPDrawable::draw(RDPGlyphCache, ...): cacheId=%u cacheIndex=%u"
        //   , cmd.cacheId, cmd.cacheIndex);
        FontChar fc(cmd.x, cmd.y, cmd.cx, cmd.cy, -1);
        memcpy(fc.data.get(), cmd.aj, fc.datasize());
        this->gly_cache.set_glyph(std::move(fc), cmd.cacheId, cmd.cacheIndex);
    }
*/

private:
    void draw_glyph(Bitmap & bmp, FontChar const & fc, size_t draw_pos, int16_t offset_y, Color color) const
    {
//LOG(LOG_INFO, "offset_y=%d", offset_y);
//LOG(LOG_INFO, "Glyph.offset=%d Glyph.baseline=%d Glyph.width=%u Glyph.height=%u", fc.offset, fc.baseline, fc.width, fc.height);
              uint8_t * bmp_data           = const_cast<uint8_t *>( bmp.data())
                                                                  + (draw_pos/* + 1*/ + fc.offset) * 3
//                                                                  + bmp.line_size() * (fc.height - 1);
                                                                  + bmp.line_size() * (offset_y - fc.baseline);
//LOG(LOG_INFO, "old.y=%d new.y=%d", (fc.height - 1), (bmp.cy() - (offset_y + fc.baseline)));
              uint8_t   fc_bit_mask        = 128;
        const uint8_t * fc_data            = fc.data.get();
        const bool      skip_padding_pixel = (fc.width % 8);

        for (int y = 0; y < fc.height; y++)
        {
            for (int x = 0; x < fc.width; x++)
            {
                if (fc_bit_mask & (*fc_data))
                {
                    bmp_data[x * 3    ] = color.red();
                    bmp_data[x * 3 + 1] = color.green();
                    bmp_data[x * 3 + 2] = color.blue();
                    //printf("X");
                }
                //else
                //{
                //    printf(".");
                //}

                fc_bit_mask >>= 1;
                if (!fc_bit_mask)
                {
                    fc_data++;
                    fc_bit_mask = 128;
                }
            }

            if (skip_padding_pixel) {
                fc_data++;
                fc_bit_mask = 128;
                //printf("_");
            }

            bmp_data -= bmp.line_size();
            //printf("\n");
        }
        //printf("\n");
    }

public:
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
//cmd.log(LOG_INFO, clip);
//LOG(LOG_INFO, "bk.x=%d bk.y=%d, bk.cx=%u bk.cy=%u, x=%d, y=%d", cmd.bk.x, cmd.bk.y, cmd.bk.cx, cmd.bk.cy, cmd.glyph_x, cmd.glyph_y);

        if (!cmd.bk.has_intersection(clip)) {
            return;
        }

        Bitmap glyph_fragments(24, NULL, cmd.bk.cx, cmd.bk.cy);

        {
            const Color color = this->u32rgb_to_color(cmd.fore_color);

            uint8_t * base = const_cast<uint8_t *>(glyph_fragments.data());
            uint8_t * p    = base;

            for (size_t x = 0; x < glyph_fragments.cx(); x++)
            {
                p[0] = color.red();
                p[1] = color.green();
                p[2] = color.blue();
// p[0] = 0xFF;
// p[1] = 0;
// p[2] = 0;

                p += 3;
            }

            uint8_t * target = base;

            for (size_t y = 1; y < glyph_fragments.cy(); y++)
            {
                target += glyph_fragments.line_size();
                memcpy(target, base, glyph_fragments.line_size());
            }
        }

        {
            bool has_delta_byte = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
            const Color color = this->u32rgb_to_color(cmd.back_color);
            const int16_t offset_y = glyph_fragments.cy() - (cmd.glyph_y - cmd.bk.y + 1);

            StaticStream aj(cmd.data, cmd.data_len);

            uint16_t draw_pos = 0;

            while (aj.in_remain())
            {
                uint8_t  data     = aj.in_uint8();
                if (data <= 0xFD)
                {
//                    FontChar const & fc = this->gly_cache.glyphs[cmd.cache_id][data].font_item;
                    FontChar const & fc = gly_cache->glyphs[cmd.cache_id][data].font_item;
                    if (!fc)
                    {
                        LOG( LOG_INFO
                           , "RDPDrawable::draw(RDPGlyphIndex, ...): Unknown glyph, cacheId=%u cacheIndex=%u"
                           , cmd.cache_id, data);
                        REDASSERT(fc);
                    }

                    if (has_delta_byte)
                    {
                        data = aj.in_uint8();
                        if (data == 0x80)
                        {
                            draw_pos += aj.in_uint16_le();
                        }
                        else
                        {
                            draw_pos += data;
                        }
                    }
                    else
                    {
                        REDASSERT(cmd.ui_charinc);
//                        draw_pos += cmd.ui_charinc;
                    }

                    if (fc)
                    {
                        this->draw_glyph(glyph_fragments, fc, draw_pos, offset_y, color);
                    }
                }
                else if (data == 0xFE)
                {
                    LOG(LOG_INFO, "RDPDrawable::draw(RDPGlyphIndex, ...): Unsupported data");
                    throw Error(ERR_RDP_UNSUPPORTED);
                }
                else if (data == 0xFF)
                {
                    aj.in_skip_bytes(2);
                    REDASSERT(!aj.in_remain());
                }
            }
        }

        const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

        if (clip.contains(cmd.bk)) {
//            this->drawable.draw_bitmap(Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy), glyph_fragments);
            this->drawable.draw_bitmap(Rect(cmd.bk.x + offset_x, cmd.bk.y, cmd.bk.cx, cmd.bk.cy), glyph_fragments);
        }
        else {
            Rect clipped_glyph_fragment_rect = cmd.bk.intersect(clip).offset(offset_x, 0);

            Bitmap clipped_glyph_fragment_bmp( glyph_fragments
                                             , clipped_glyph_fragment_rect.offset( clipped_glyph_fragment_rect.x - cmd.bk.x
                                                                                 , clipped_glyph_fragment_rect.y - cmd.bk.y
                                                                                 ).wh()
                                             );

            this->drawable.draw_bitmap(clipped_glyph_fragment_rect, clipped_glyph_fragment_bmp);
        }
    }

    virtual void draw(const RDPBrushCache & cmd) {}
    virtual void draw(const RDPColCache & cmd) {}

    virtual void flush() {}

    static const FontChar & get_font(const Font& font, uint32_t c)
    {
        if (!font.glyph_defined(c) || !font.font_items[c]) {
            LOG(LOG_WARNING, "RDPDrawable::get_font() - character not defined >0x%02x<", c);
            return font.font_items[unsigned('?')];
        }
        return font.font_items[c];
    }

    // for testing purposes
    void text_metrics(const char * text, int & width, int & height, const Font & font)
    {
        ::text_metrics(font, text, width, height);
    }

    // for testing purposes
    void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip, Font& font)
    {
        TODO("Merge common code with Front::server_draw_text()");
        if (text[0] != 0) {
            Rect screen_rect = clip.intersect(this->drawable.width(), this->drawable.height());
            if (screen_rect.isempty()){
                return ;
            }

            const Color fg_color = this->u32_to_color(fgcolor);

            UTF8toUnicodeIterator unicode_iter(text);

            for (; *unicode_iter; ++unicode_iter) {
                const FontChar & font_item = this->get_font(font, *unicode_iter);
                if (x + font_item.width > screen_rect.x) {
                    break ;
                }
                x += font_item.incby;
            }

            for (; *unicode_iter && x < screen_rect.right(); ++unicode_iter) {
                const FontChar & font_item = this->get_font(font, *unicode_iter);
                int16_t cy = std::min<int16_t>(y + font_item.height, screen_rect.bottom()) - y;
                int i = 0;
                //x += font_item.offset;
                for (int yy = 0 ; yy < cy; yy++) {
                    unsigned char oc = 1<<7;
                    for (int xx = 0; xx < font_item.width; xx++) {
                        if (!oc) {
                            oc = 1 << 7;
                            ++i;
                        }
                        if (yy + y >= screen_rect.y && xx + x >= screen_rect.x && xx + x < screen_rect.right() && font_item.data[i + yy] & oc) {
                            this->drawable.draw_pixel(x + xx, y + yy, fg_color);
                        }
                        oc >>= 1;
                    }
                }
                //x += font_item.incby - font_item.offset;
                x += font_item.incby;
            }
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) {
        int16_t startx = cmd.xStart;
        int16_t starty = cmd.yStart;

        int16_t endx;
        int16_t endy;

        const Color color = this->u32rgb_to_color(cmd.PenColor);

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = startx + cmd.deltaEncodedPoints[i].xDelta;
            endy = starty + cmd.deltaEncodedPoints[i].yDelta;

            this->draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, color, clip);

            startx = endx;
            starty = endy;
        }
    }

    TODO("this functions only draw polygon borders but do not fill "
         "them with solid color.")
    void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        int16_t startx = cmd.xStart;
        int16_t starty = cmd.yStart;

        int16_t endx;
        int16_t endy;

        const Color BrushColor = this->u32rgb_to_color(cmd.BrushColor);

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = startx + cmd.deltaPoints[i].xDelta;
            endy = starty + cmd.deltaPoints[i].yDelta;

            this->draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);

            startx = endx;
            starty = endy;
        }
        endx = cmd.xStart;
        endy = cmd.yStart;

        this->draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, BrushColor, clip);
    }

    TODO("this functions only draw polygon borders but do not fill "
         "them with brush color.")
    void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        int16_t startx = cmd.xStart;
        int16_t starty = cmd.yStart;

        int16_t endx;
        int16_t endy;

        const Color foreColor = this->u32rgb_to_color(cmd.foreColor);

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = startx + cmd.deltaPoints[i].xDelta;
            endy = starty + cmd.deltaPoints[i].yDelta;

            this->draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);

            startx = endx;
            starty = endy;
        }
        endx = cmd.xStart;
        endy = cmd.yStart;

        this->draw_line(0x0001, startx, starty, endx, endy, cmd.bRop2, foreColor, clip);
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
            size_t size, const Bitmap & bmp) {
        const Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                          , bitmap_data.dest_right - bitmap_data.dest_left + 1
                          , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        const Rect trect = rectBmp.intersect(this->drawable.width(), this->drawable.height());

        this->drawable.draw_bitmap(trect, bmp);
    }

    virtual void draw(const RDP::FrameMarker & order) {
        this->frame_start_count += ((order.action == RDP::FrameMarker::FrameStart) ? 1 : -1);
        REDASSERT(this->frame_start_count >= 0);
        this->drawable.logical_frame_ended = (this->frame_start_count == 0);
    }

    virtual void server_set_pointer(const Pointer & cursor) {
        this->drawable.use_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
    }

    virtual void set_mod_palette(const BGRPalette & palette) {
        this->mod_palette_rgb = palette;
    }

    void dump_png24(Transport & trans, bool bgr) const {
        ::transport_dump_png24(trans, this->drawable.data(),
            this->drawable.width(), this->drawable.height(),
            this->drawable.rowsize(),
            bgr);
    }
};

#endif
