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

#include "font.hpp"

#include "bitmapupdate.hpp"
#include "pointer.hpp"

#include "caches/glyphcache.hpp"

#include "capabilities/glyphcache.hpp"

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
#include "orders/AlternateSecondaryWindowing.hpp"

#include "drawable.hpp"
#include "png.hpp"
#include "text_metrics.hpp"

#include "gdi/graphic_api.hpp"

// orders provided to RDPDrawable *MUST* be 24 bits
// drawable also only support 24 bits orders
class RDPDrawable
: public gdi::GraphicApi
{
    using Color = Drawable::Color;

    Drawable drawable;
    int frame_start_count;
    BGRPalette mod_palette_rgb;

    uint8_t fragment_cache[MAXIMUM_NUMBER_OF_FRAGMENT_CACHE_ENTRIES][1 /* size */ + MAXIMUM_SIZE_OF_FRAGMENT_CACHE_ENTRIE];

public:
    RDPDrawable(const uint16_t width, const uint16_t height, int order_bpp)
    : GraphicApi(gdi::GraphicDepth::from_bpp(order_bpp))
    , drawable(width, height)
    , frame_start_count(0)
    , mod_palette_rgb(BGRPalette::classic_332())
    {
        REDASSERT(this->order_depth().is_defined());
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
    Color u32_to_color(uint32_t color) const {
        return this->drawable.u32bgr_to_color(color);
    }

    Color u32rgb_to_color(BGRColor color) const {
        using Depths = gdi::GraphicDepth;

        if (!this->order_depth().is_depth24()) {
            switch (this->order_depth()){
                case Depths::depth8():  color = decode_color8_opaquerect()(color, this->mod_palette_rgb); break;
                case Depths::depth15(): color = decode_color15_opaquerect()(color); break;
                case Depths::depth16(): color = decode_color16_opaquerect()(color); break;
                default: REDASSERT(false);
            }
        }

        return this->u32_to_color(color);
    }

    std::pair<Color, Color> u32rgb_to_color(BGRColor color1, BGRColor color2) const {
        using Depths = gdi::GraphicDepth;

        if (!this->order_depth().is_depth24()) {
            switch (this->order_depth()) {
                case Depths::depth8():
                    color1 = decode_color8_opaquerect()(color1, this->mod_palette_rgb);
                    color2 = decode_color8_opaquerect()(color2, this->mod_palette_rgb);
                    break;
                case Depths::depth15():
                    color1 = decode_color15_opaquerect()(color1);
                    color2 = decode_color15_opaquerect()(color2);
                    break;
                case Depths::depth16():
                    color1 = decode_color16_opaquerect()(color1);
                    color2 = decode_color16_opaquerect()(color2);
                    break;
                default: REDASSERT(false);
            }
        }

        return std::pair<Color, Color>{this->u32_to_color(color1), this->u32_to_color(color2)};
    }

public:
    void set_row(size_t rownum, const uint8_t * data) override {
        this->drawable.set_row(rownum, data);
    }

    void draw(RDPColCache   const &) override {
    }

    void draw(RDPBrushCache const &) override {
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        const Rect trect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        this->drawable.opaquerect(trect, this->u32rgb_to_color(cmd.color));
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        this->drawable.ellipse(cmd.el, cmd.bRop2, cmd.fillMode, this->u32rgb_to_color(cmd.color));
    }

    TODO("This will draw a standard ellipse without brush style")
    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        this->drawable.ellipse(cmd.el, cmd.brop2, cmd.fill_mode, this->u32rgb_to_color(cmd.back_color));
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        // Destination rectangle : drect
        const Rect drect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        if (drect.isempty()){ return; }
        // adding delta move dest to source
        const signed int deltax = cmd.srcx - cmd.rect.x;
        const signed int deltay = cmd.srcy - cmd.rect.y;
        this->drawable.scrblt(drect.x + deltax, drect.y + deltay, drect, cmd.rop);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip) override {
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
    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        this->draw_multi(cmd, clip, [&](const Rect & trect) {
            this->drawable.destblt(trect, cmd.bRop);
        });
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        const Color color = this->u32rgb_to_color(cmd._Color);
        this->draw_multi(cmd, clip, [color, this](const Rect & trect) {
            this->drawable.opaquerect(trect, color);
        });
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        TODO(" PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.");
        if (cmd.brush.style == 0x03 && (cmd.bRop == 0xF0 || cmd.bRop == 0x5A)) {
            enum { BackColor, ForeColor };
            auto colors = this->u32rgb_to_color(cmd.BackColor, cmd.ForeColor);
            uint8_t brush_data[8];
            memcpy(brush_data, cmd.BrushExtra, 7);
            brush_data[7] = cmd.BrushHatch;
            this->draw_multi(cmd, clip, [&](const Rect & trect) {
                this->drawable.patblt_ex(
                    trect, cmd.bRop,
                    std::get<BackColor>(colors), std::get<ForeColor>(colors),
                    brush_data, cmd.brush.org_x, cmd.brush.org_y
                );
            });
        }
        else {
            const Color color = this->u32rgb_to_color(cmd.BackColor);
            this->draw_multi(cmd, clip, [&](const Rect & trect) {
                this->drawable.patblt(trect, cmd.bRop, color);
            });
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        const signed int deltax = cmd.nXSrc - cmd.rect.x;
        const signed int deltay = cmd.nYSrc - cmd.rect.y;
        this->draw_multi(cmd, clip, [&](const Rect & trect) {
            this->drawable.scrblt(trect.x + deltax, trect.y + deltay, trect, cmd.bRop);
        });
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        const Rect trect = clip.intersect(this->drawable.width(), this->drawable.height()).intersect(cmd.rect);
        TODO("PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.");

        if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) {
            enum { BackColor, ForeColor };
            auto colors = this->u32rgb_to_color(cmd.back_color, cmd.fore_color);
            uint8_t brush_data[8];
            memcpy(brush_data, cmd.brush.extra, 7);
            brush_data[7] = cmd.brush.hatch;

            this->drawable.patblt_ex(
                trect, cmd.rop,
                std::get<BackColor>(colors), std::get<ForeColor>(colors),
                brush_data, cmd.brush.org_x, cmd.brush.org_y
            );
        }
        else {
            this->drawable.patblt(trect, cmd.rop, this->u32rgb_to_color(cmd.back_color));
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
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

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
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
    void draw(const RDPLineTo & lineto, const Rect & clip) override {
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

private:
    void draw_glyph( FontChar const & fc, size_t draw_pos, int16_t offset_y, Color color
                   , int16_t bmp_pos_x, int16_t bmp_pos_y, Rect const & clip)
    {
        const int16_t   local_offset_x     = draw_pos + fc.offset;
        const int16_t   local_offset_y     = offset_y + fc.baseline;

              uint8_t   fc_bit_mask        = 128;
        const uint8_t * fc_data            = fc.data.get();
        const bool      skip_padding_pixel = (fc.width % 8);

        for (int y = 0; y < fc.height; y++)
        {
            const int pt_y = bmp_pos_y + local_offset_y + y;
//            if (!(clip.y <= pt_y && pt_y < clip.bottom())) {
//                break;
//            }

            for (int x = 0; x < fc.width; x++)
            {
                if (fc_bit_mask & (*fc_data))
                {
                    const int pt_x = bmp_pos_x + local_offset_x + x;
                    if (clip.x <= pt_x && pt_x < clip.right() &&
                        clip.y <= pt_y && pt_y < clip.bottom()) {
                        this->drawable.draw_pixel(pt_x, pt_y, color);
                    }
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
            //printf("\n");
        }
        //printf("\n");
    }

public:
    void draw_VariableBytes(uint8_t const * data, uint16_t size, bool has_delta_bytes,
            uint16_t & draw_pos_ref, int16_t offset_y, Color color,
            int16_t bmp_pos_x, int16_t bmp_pos_y, Rect const & clip,
            uint8_t cache_id, const GlyphCache & gly_cache) {
        InStream variable_bytes(data, size);

        uint8_t const * fragment_begin_position = variable_bytes.get_current();

        while (variable_bytes.in_remain())
        {
            uint8_t data = variable_bytes.in_uint8();
            if (data <= 0xFD)
            {
                FontChar const & fc = gly_cache.glyphs[cache_id][data].font_item;
                if (!fc)
                {
                    LOG( LOG_INFO
                       , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
                       , cache_id, data);
                    REDASSERT(fc);
                }

                if (has_delta_bytes)
                {
                    data = variable_bytes.in_uint8();
                    if (data == 0x80)
                    {
                        draw_pos_ref += variable_bytes.in_uint16_le();
                    }
                    else
                    {
                        draw_pos_ref += data;
                    }
                }

                if (fc)
                {
                    this->draw_glyph( fc, draw_pos_ref, offset_y, color, bmp_pos_x, bmp_pos_y
                                    , clip);
                }
            }
            else if (data == 0xFE)
            {
                const uint8_t fragment_index = variable_bytes.in_uint8();

                uint16_t delta = 0;
                if (has_delta_bytes)
                {
                    delta = variable_bytes.in_uint8();
                    if (delta == 0x80)
                    {
                        delta = variable_bytes.in_uint16_le();
                    }
                }
                REDASSERT(!delta);  // Fragment's position delta is not yet supported.

                LOG(LOG_WARNING,
                    "RDPDrawable::draw_VariableBytes: "
                        "Experimental support of USE (0xFE) operation byte in "
                        "GlyphIndex Primary Drawing Order. "
                        "fragment_index=%u fragment_size=%u delta=%u",
                    fragment_index, this->fragment_cache[fragment_index][0], delta);

                fragment_begin_position = variable_bytes.get_current();

                this->draw_VariableBytes(&this->fragment_cache[fragment_index][1],
                    this->fragment_cache[fragment_index][0], has_delta_bytes,
                    draw_pos_ref, offset_y, color, bmp_pos_x, bmp_pos_y, clip,
                    cache_id, gly_cache);
            }
            else if (data == 0xFF)
            {
                const uint8_t fragment_index = variable_bytes.in_uint8();
                const uint8_t fragment_size  = variable_bytes.in_uint8();

                LOG(LOG_WARNING,
                    "RDPDrawable::draw_VariableBytes: "
                        "Experimental support of ADD (0xFF) operation byte in "
                        "GlyphIndex Primary Drawing Order. "
                        "fragment_index=%u fragment_size=%u",
                    fragment_index, fragment_size);

                REDASSERT(!variable_bytes.in_remain());

                REDASSERT(fragment_begin_position + fragment_size + 3 == variable_bytes.get_current());

                this->fragment_cache[fragment_index][0] = fragment_size;
                ::memcpy(&this->fragment_cache[fragment_index][1],
                         fragment_begin_position,
                         fragment_size
                        );

                fragment_begin_position = variable_bytes.get_current();
            }
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override {
        if (!cmd.bk.has_intersection(clip)) {
            return;
        }

        // set a background color
        {
            Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
            if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
                ajusted.cy--;
                this->drawable.opaquerect(ajusted.intersect(clip), this->u32rgb_to_color(cmd.fore_color));
            }
        }

        bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));
        const Color color = this->u32rgb_to_color(cmd.back_color);
        const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
        const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

        uint16_t draw_pos = 0;

        Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(clip);

        this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
            draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
            clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
    }

    static const FontChar & get_font(const Font& font, uint32_t c)
    {
        if (!font.glyph_defined(c) || !font.font_items[c]) {
            LOG(LOG_WARNING, "RDPDrawable::get_font() - character not defined >0x%02x<", c);
            return font.font_items[unsigned('?')];
        }
        return font.font_items[c];
    }

    // for testing purposes
    void text_metrics(Font const & font, const char * text, int & width, int & height)
    {
        ::text_metrics(font, text, width, height);
    }

    // for testing purposes
    void server_draw_text(Font const & font, int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor,
                          const Rect& clip)
    {
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
                x += font_item.incby;
            }
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
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
    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
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
    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
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

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        const Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                          , bitmap_data.dest_right - bitmap_data.dest_left + 1
                          , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        const Rect trect = rectBmp.intersect(this->drawable.width(), this->drawable.height());

        this->drawable.draw_bitmap(trect, bmp);
    }

    void draw(const RDP::FrameMarker & order) override {
        this->frame_start_count += ((order.action == RDP::FrameMarker::FrameStart) ? 1 : -1);
        REDASSERT(this->frame_start_count >= 0);
        this->drawable.logical_frame_ended = (this->frame_start_count == 0);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {}
    void draw(const RDP::RAIL::WindowIcon          & order) override {}
    void draw(const RDP::RAIL::CachedIcon          & order) override {}
    void draw(const RDP::RAIL::DeletedWindow       & order) override {}

    void set_pointer(const Pointer & cursor) override {
        this->drawable.use_pointer(cursor.x, cursor.y, cursor.data, cursor.mask);
    }

    void set_palette(const BGRPalette & palette) override {
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
