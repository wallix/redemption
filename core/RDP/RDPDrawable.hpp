/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier, Poelen Jonathan
*/

#ifndef _REDEMPTION_CORE_RDP_RDPDRAWABLE_HPP_
#define _REDEMPTION_CORE_RDP_RDPDRAWABLE_HPP_

#include "drawable.hpp"
#include "RDP/caches/bmpcache.hpp"

#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "font.hpp"


struct RDPDrawableConfig {
    bool bgr;

    RDPDrawableConfig(bool bgr = true)
    : bgr(bgr)
    {
    }
};

// orders provided to RDPDrawable *MUST* be 24 bits
// drawable also only support 24 bits orders
class RDPDrawable : public RDPGraphicDevice {
public:
    RDPDrawableConfig conf;
    Drawable drawable;

    RDPDrawable(const uint16_t width, const uint16_t height, const RDPDrawableConfig & conf)
    : conf(conf)
    , drawable(width, height)
    {
    }

    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        if (this->conf.bgr){
            uint32_t bgrtmp[8192];
            const uint32_t * s = reinterpret_cast<const uint32_t*>(data);
            uint32_t * t = bgrtmp;
            for (size_t n = 0; n < (this->drawable.width / 4) ; n++){
                unsigned bRGB = *s++;
                unsigned GBrg = *s++;
                unsigned rgbR = *s++;
                *t++ = ((GBrg << 16) & 0xFF000000)
                   | ((bRGB << 16) & 0x00FF0000)
                   | (bRGB         & 0x0000FF00)
                   | ((bRGB >> 16) & 0x000000FF) ;
                *t++ = (GBrg         & 0xFF000000)
                   | ((rgbR << 16) & 0x00FF0000)
                   | ((bRGB >> 16) & 0x0000FF00)
                   | ( GBrg        & 0x000000FF) ;
                *t++ = ((rgbR << 16) & 0xFF000000)
                   | (rgbR         & 0x00FF0000)
                   | ((rgbR >> 16) & 0x0000FF00)
                   | ((GBrg >> 16) & 0x000000FF) ;
            }
        }
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

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        uint32_t color = cmd.color;
        if (!this->conf.bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->drawable.opaquerect(trect, color);
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

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        TODO(" PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.")
        uint32_t color = cmd.back_color;
        if (!this->conf.bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->drawable.patblt(trect, cmd.rop, color);
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
                , 0xFFFFFF, this->conf.bgr);
        break;
        case 0xCC:
            this->drawable.mem_blt(rect, bmp
                , cmd.srcx + (rect.x  - cmd.rect.x)
                , cmd.srcy + (rect.y  - cmd.rect.y)
                , 0, this->conf.bgr);
        break;
        default:
            // should not happen
        break;
        }
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
        //        LOG(LOG_INFO, "back_mode=%d (%d,%d) -> (%d, %d) rop2=%d bg_color=%d clip=(%u, %u, %u, %u)",
        //            lineto.back_mode, lineto.startx, lineto.starty, lineto.endx, lineto.endy,
        //            lineto.rop2, lineto.back_color, clip.x, clip.y, clip.cx, clip.cy);

        TODO("We should perform a true intersection between line and clip rectangle, not cheat as below.")
        // enlarge_to compute a new rect including old rect and added point
        const Rect & line_rect = Rect(lineto.startx, lineto.starty, 1, 1).enlarge_to(lineto.endx, lineto.endy);
        if (line_rect.intersect(clip).isempty()){
            //            LOG(LOG_INFO, "line_rect(%u, %u, %u, %u)", line_rect.x, line_rect.y, line_rect.cx, line_rect.cy);
            return;
        }

        // Color handling
        uint32_t color = lineto.pen.color;
        if (!this->conf.bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }


        int startx = (lineto.startx >= clip.x + clip.cx)?clip.x + clip.cx-1:lineto.startx;
        int endx = (lineto.endx >= clip.x + clip.cx)?clip.x + clip.cx-1:lineto.endx;
        startx = (startx & 0x8000)?0:startx;
        endx = (endx & 0x8000)?0:endx;

        int starty = (lineto.starty >= clip.y + clip.cy)?clip.y + clip.cy - 1:lineto.starty;
        int endy = (lineto.endy >= clip.y + clip.cy)?clip.y + clip.cy - 1:lineto.endy;
        starty = (starty & 0x8000)?0:starty;
        endy = (endy & 0x8000)?0:endy;

        if (startx == endx){
            this->drawable.vertical_line(lineto.back_mode,
                                lineto.startx,
                                (starty <= endy)?starty:endy,
                                (starty <= endy)?endy:starty,
                                color);
        }
        else if (starty == endy){
            this->drawable.horizontal_line(lineto.back_mode,
                                  (startx <= endx)?startx:endx,
                                  starty,
                                  (startx <= endx)?endx:startx,
                                  color);

        }
        else if (lineto.startx <= lineto.endx){
            this->drawable.line(lineto.back_mode,
                       lineto.startx, lineto.starty, lineto.endx, lineto.endy,
                       color, clip);
        }
        else {
            this->drawable.line(lineto.back_mode,
                       lineto.endx, lineto.endy, lineto.startx, lineto.starty,
                       color, clip);
        }
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {}
    virtual void draw(const RDPBrushCache & cmd) {}
    virtual void draw(const RDPColCache & cmd) {}
    virtual void draw(const RDPGlyphCache & cmd) {}

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
        if (text[0] != 0) {
            uint32_t uni[128];
            size_t part_len = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
            int dx = 0;
            if (!this->conf.bgr){
                bgcolor = ((bgcolor << 16) & 0xFF0000) | (bgcolor & 0xFF00) |((bgcolor >> 16) & 0xFF);
                fgcolor = ((fgcolor << 16) & 0xFF0000) | (fgcolor & 0xFF00) |((fgcolor >> 16) & 0xFF);
            }
            for (size_t index = 0; index < part_len && x < clip.x + clip.cx; index++) {
                FontChar *font_item = this->get_font(font, uni[index]);
                this->drawable.opaquerect(clip.intersect(Rect(x-dx, y, font_item->width+dx, font_item->height)), bgcolor);
                int i = 0;
                for (int yy = 0 ; yy < font_item->height && yy + y < clip.y + clip.cy; yy++){
                    unsigned char oc = 1<<7;
                    for (int xx = 0; xx < font_item->width; xx++){
                        if (!oc) {
                            oc = 1 << 7;
                            ++i;
                        }
                        if (font_item->data[i + yy] & oc) {
                            this->drawable.opaquerect(Rect(x + xx, y + yy, 1, 1), fgcolor);
                        }
                        oc >>= 1;
                    }
                }
                x += font_item->width + 2;
                dx = 2;
            }
        }
    }
};

#endif
