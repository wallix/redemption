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

#include "drawable.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "RDP/caches/pointercache.hpp"

#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "font.hpp"
#include "png.hpp"

// orders provided to RDPDrawable *MUST* be 24 bits
// drawable also only support 24 bits orders
class RDPDrawable : public RDPGraphicDevice {
public:
    Drawable drawable;

    DrawablePointerCache ptr_cache;
    GlyphCache           gly_cache;

    RDPDrawable(const uint16_t width, const uint16_t height)
    : drawable(width, height)
    {
        Pointer pointer0(Pointer::POINTER_CURSOR0);
        this->ptr_cache.add_pointer_static(pointer0, 0);

        Pointer pointer1(Pointer::POINTER_CURSOR1);
        this->ptr_cache.add_pointer_static(pointer1, 1);
    }

    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        memcpy(this->drawable.data + this->drawable.rowsize * rownum, data, this->drawable.rowsize);
    }

    virtual uint8_t * get_row(size_t rownum)
    {
        return this->drawable.data + this->drawable.rowsize * rownum;
    }

    virtual size_t get_rowsize()
    {
        return this->drawable.rowsize;
    }

    uint32_t RGBtoBGR(uint32_t color)
    {
        return ((color << 16) | (color & 0xFF00)| (color >> 16)) & 0xFFFFFF;
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        const uint32_t color = this->RGBtoBGR(cmd.color);
        this->drawable.opaquerect(trect, color);
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        uint32_t bgrcolor = this->RGBtoBGR(cmd.color);
        this->drawable.ellipse(cmd.el, cmd.bRop2, cmd.fillMode, bgrcolor);
    }

    TODO("This will draw a standard ellipse without brush style");
    void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        uint32_t bgrcolor = this->RGBtoBGR(cmd.back_color);
        this->drawable.ellipse(cmd.el, cmd.brop2, cmd.fill_mode, bgrcolor);
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect drect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        if (drect.isempty()){ return; }
        // adding delta move dest to source
        const signed int deltax = cmd.srcx - cmd.rect.x;
        const signed int deltay = cmd.srcy - cmd.rect.y;
        this->drawable.scrblt(drect.x + deltax, drect.y + deltay, drect, cmd.rop);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        this->drawable.destblt(trect, cmd.rop);
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        const Rect clip_drawable_cmd_intersect = clip.intersect(this->drawable.width, this->drawable.height).intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight));

        Rect cmd_rect(0, 0, 0, 0);

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            cmd_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            cmd_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            cmd_rect.cx =  cmd.deltaEncodedRectangles[i].width;
            cmd_rect.cy =  cmd.deltaEncodedRectangles[i].height;
            const Rect trect = clip_drawable_cmd_intersect.intersect(cmd_rect);
            this->drawable.destblt(trect, cmd.bRop);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        TODO(" PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.");
        if ((cmd.rop == 0xF0) && (cmd.brush.style == 0x03))
        {
            uint8_t brush_data[8];
            memcpy(brush_data, cmd.brush.extra, 7);
            brush_data[7] = cmd.brush.hatch;

            const uint32_t back_color = this->RGBtoBGR(cmd.back_color);
            const uint32_t fore_color = this->RGBtoBGR(cmd.fore_color);
            this->drawable.patblt_ex(trect, cmd.rop, back_color, fore_color, brush_data);
        }
        else
        {
            const uint32_t color = this->RGBtoBGR(cmd.back_color);
            this->drawable.patblt(trect, cmd.rop, color);
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
            this->drawable.mem_blt(rect, bmp
                , cmd.srcx + (rect.x  - cmd.rect.x)
                , cmd.srcy + (rect.y  - cmd.rect.y)
                , 0xFFFFFF, false);
        break;
        case 0xCC:
            this->drawable.mem_blt(rect, bmp
                , cmd.srcx + (rect.x  - cmd.rect.x)
                , cmd.srcy + (rect.y  - cmd.rect.y)
                , 0, false);
        break;
        case 0x22:  // dest = dest AND (NOT source)
        case 0x66:  // dest = source XOR dest (SRCINVERT)
        case 0x88:  // dest = source AND dest (SRCAND)
        case 0xBB:  // dest = (NOT source) OR dest (MERGEPAINT)
        case 0xEE:  // dest = source OR dest (SRCPAINT)
            this->drawable.mem_blt_ex(rect, bmp
                , cmd.srcx + (rect.x - cmd.rect.x)
                , cmd.srcy + (rect.y - cmd.rect.y)
                , cmd.rop, false);
            break;
        default:
            // should not happen
            // LOG(LOG_INFO, "Unsupported Rop=0x%02X", cmd.rop);
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
            , cmd.rop, cmd.fore_color, false);
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
        drew_line(lineto.back_mode, lineto.startx, lineto.starty, lineto.endx, lineto.endy, lineto.rop2,
            lineto.pen.color, clip);
    }

    void drew_line(uint16_t BackMode, int16_t nXStart, int16_t nYStart,
                   int16_t nXEnd, int16_t nYEnd, uint8_t bRop2,
                   uint32_t PenColor, const Rect & clip) {
        // Color handling
        uint32_t color = RGBtoBGR(PenColor);

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
                                         (starty <= endy)?starty:endy,
                                         (starty <= endy)?endy:starty,
                                         bRop2,
                                         color);
        }
        else if (starty == endy){
            this->drawable.horizontal_line(BackMode,
                                           (startx <= endx)?startx:endx,
                                           starty,
                                           (startx <= endx)?endx:startx,
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

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->gly_cache.set_glyph(cmd);
    }

    void draw_glyph(Bitmap & bmp, FontChar * fc, size_t offset_x, uint32_t color)
    {
        uint8_t * bmp_data    = bmp.data_bitmap.get() + (offset_x + 1) * 3 + bmp.line_size * (fc->height - 1);
        uint8_t * fc_data     = fc->data;
        uint8_t   fc_bit_mask = 128;

        for (int y = 0; y < fc->height; y++)
        {
            for (int x = 0; x < fc->width; x++)
            {
                if (fc_bit_mask & (*fc_data))
                {
                    bmp_data[x * 3    ] = color;
                    bmp_data[x * 3 + 1] = color >> 8;
                    bmp_data[x * 3 + 2] = color >> 16;
//                  printf("X");
                }
//              else
//              {
//                  printf(".");
//              }

                fc_bit_mask >>= 1;
                if (!fc_bit_mask)
                {
                    fc_data++;
                    fc_bit_mask = 128;
                }
            }

            bmp_data -= bmp.line_size;
//          printf("\n");
        }
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip,
        const GlyphCache * gly_cache)
    {
        Bitmap glyph_fragments(24, NULL, cmd.bk.cx, cmd.bk.cy);

        {
            const uint32_t color = this->RGBtoBGR(cmd.fore_color);

            uint8_t * base = glyph_fragments.data_bitmap.get();
            uint8_t * p    = base;

            for (size_t x = 0; x < glyph_fragments.cx; x++)
            {
                p[0] = color;
                p[1] = color >> 8;
                p[2] = color >> 16;

                p += 3;
            }

            uint8_t * target = base;

            for (size_t y = 1; y < glyph_fragments.cy; y++)
            {
                target += glyph_fragments.line_size;
                memcpy(target, base, glyph_fragments.line_size);
            }
        }

        {
            bool has_delta_byte = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));

            StaticStream aj(cmd.data, cmd.data_len);

            uint16_t draw_pos = 0;

            while (aj.in_remain())
            {
                uint8_t  data     = aj.in_uint8();
                if (data <= 0xFD)
                {
                    FontChar * fc = this->gly_cache.char_items[cmd.cache_id][data].font_item;
                    if (!fc)
                    {
                        LOG(LOG_INFO, "RDPDrawable::draw(RDPGlyphIndex, ...): Unknown glyph=%u", data);
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

                    if (fc)
                    {
                        this->draw_glyph(glyph_fragments, fc, draw_pos, this->RGBtoBGR(cmd.back_color));
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

        this->drawable.draw_bitmap(
            Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy), glyph_fragments,
            false);
    }

    virtual void draw(const RDPBrushCache & cmd) {}
    virtual void draw(const RDPColCache & cmd) {}

    virtual void set_row(uint16_t r, uint8_t * row){
        memcpy(this->drawable.data + this->drawable.rowsize * r, row, this->drawable.rowsize);
    }
    virtual void flush() {}

    static FontChar * get_font(Font& font, uint32_t c)
    {
        FontChar *font_item = font.glyph_defined(c) ? font.font_items[c] : 0;
        if (!font_item) {
            LOG(LOG_WARNING, "RDPDrawable::get_font() - character not defined >0x%02x<", c);
            font_item = font.font_items['?'];
        }
        return font_item;
    }

    void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip, Font& font)
    {
        TODO("Merge common code with Front::server_draw_text()");
        if (text[0] != 0) {
            Rect screen_rect = clip.intersect(this->drawable.width, this->drawable.height);
            if (screen_rect.isempty()){
                return ;
            }

            fgcolor = RGBtoBGR(fgcolor);
            bgcolor = RGBtoBGR(bgcolor);
            uint32_t uni[128];
            size_t part_len = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));

            size_t index = 0;
            FontChar *font_item = 0;
            for (; index < part_len && x < screen_rect.x; index++) {
                font_item = this->get_font(font, uni[index]);
                if (x + font_item->width > screen_rect.x) {
                    break ;
                }
                x += font_item->width + 2;
            }

            for (; index < part_len && x < screen_rect.right(); index++) {
                font_item = this->get_font(font, uni[index]);
                int16_t cy = std::min<int16_t>(y + font_item->height, screen_rect.bottom()) - y;
                int i = 0;
                for (int yy = 0 ; yy < cy; yy++) {
                    unsigned char oc = 1<<7;
                    for (int xx = 0; xx < font_item->width; xx++) {
                        if (!oc) {
                            oc = 1 << 7;
                            ++i;
                        }
                        if (yy + y >= screen_rect.y && xx + x >= screen_rect.x && xx + x < screen_rect.right() && font_item->data[i + yy] & oc) {
                            this->drawable.opaquerect(Rect(x + xx, y + yy, 1, 1), fgcolor);
                        }
                        oc >>= 1;
                    }
                }
                x += font_item->width + 2;
            }
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) {
        int16_t startx = cmd.xStart;
        int16_t starty = cmd.yStart;

        int16_t endx;
        int16_t endy;

        for (uint8_t i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = startx + cmd.deltaEncodedPoints[i].xDelta;
            endy = starty + cmd.deltaEncodedPoints[i].yDelta;

            drew_line(0x0001, startx, starty, endx, endy, cmd.bRop2, cmd.PenColor, clip);

            startx = endx;
            starty = endy;
        }
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
            size_t size, const Bitmap & bmp) {
        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                    , bitmap_data.dest_right - bitmap_data.dest_left + 1
                    , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        const Rect & trect = rectBmp.intersect(this->drawable.width, this->drawable.height);

        this->drawable.draw_bitmap(trect, bmp, false);
    }

    virtual void send_pointer(int cache_idx, const Pointer & cursor)
    {
        this->ptr_cache.add_pointer_static(cursor, cache_idx);

        drawable_Pointer & dcursor = this->ptr_cache.Pointers[cache_idx];
        this->drawable.set_mouse_cursor(
            dcursor.contiguous_mouse_pixels, dcursor.mouse_cursor,
            dcursor.x, dcursor.y);
    }

    virtual void set_pointer(int cache_idx) {
        drawable_Pointer & Pointer = this->ptr_cache.Pointers[cache_idx];
        this->drawable.set_mouse_cursor(
            Pointer.contiguous_mouse_pixels, Pointer.mouse_cursor,
            Pointer.x, Pointer.y);
    }

    virtual void dump_png24(Transport * trans, bool bgr) {
        ::transport_dump_png24(trans, this->drawable.data,
            this->drawable.width, this->drawable.height,
            this->drawable.rowsize,
            bgr);
    }
};

#endif
