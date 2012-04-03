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

#if !defined(__RDPDRAWABLE_HPP__)
#define __RDPDRAWABLE_HPP__

#include "drawable.hpp"
#include "bmpcache.hpp"

#include "RDP/RDPGraphicDevice.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"


class RDPDrawable : public RDPGraphicDevice {
public:
    uint8_t bpp;
    BGRPalette palette;
    bool bgr;
    Drawable drawable;

    RDPDrawable(const uint16_t width, const uint16_t height,
                const uint8_t bpp, const BGRPalette & palette,
                bool bgr=true)
    : bpp(bpp),
    bgr(bgr),
    drawable(width, height)
    {
        memcpy(this->palette, palette, sizeof(palette));
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = clip.intersect(this->drawable.width, this->drawable.height).intersect(cmd.rect);
        uint32_t color = color_decode(cmd.color, this->bpp, this->palette);
        if (!this->bgr){
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
        uint32_t color = color_decode(cmd.back_color, this->bpp, this->palette);
        if (!this->bgr){
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
            {
                /*if (bmp.original_bpp != this->bpp){
                    uint8_t outbuf[65536];
                    bmp.convert_data_bitmap(this->bpp, outbuf);
                    const Bitmap newbmp(this->bpp, &bmp.original_palette,
                            bmp.cx, bmp.cy, outbuf,
                            bmp.cx * nbbytes(this->bpp) * bmp.cy, false, false);
                    this->drawable.mem_blt(rect, newbmp, cmd.srcx, cmd.srcy, 0xFFFFFF, this->bgr);
                }
                else*/ {
                    this->drawable.mem_blt(rect, bmp, cmd.srcx, cmd.srcy, 0xFFFFFF, this->bgr);
                }
            }
            break;
            case 0xCC:
            {
                /*if (bmp.original_bpp != this->bpp){
                    uint8_t outbuf[65536];
                    bmp.convert_data_bitmap(this->bpp, outbuf);
                    const Bitmap newbmp(this->bpp, &bmp.original_palette,
                            bmp.cx, bmp.cy, outbuf,
                            bmp.cx * nbbytes(this->bpp) * bmp.cy, false, false);
                    this->drawable.mem_blt(rect, newbmp, cmd.srcx, cmd.srcy, 0, this->bgr);
                }
                else*/ {
                    this->drawable.mem_blt(rect, bmp, cmd.srcx, cmd.srcy, 0, this->bgr);
                }
            }
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
        uint32_t color = color_decode(lineto.pen.color, this->bpp, this->palette);
        if (!this->bgr){
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
    virtual void flush() {}


};

#endif
