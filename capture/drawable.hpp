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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__DRAWABLE_HPP__)
#define __DRAWABLE_HPP__

#include "bitmap.hpp"

#include "colors.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"

#include "bmpcache.hpp"

class Drawable {
public:
    const Rect full;
    const uint8_t bpp;
    const BGRPalette & palette;
    const size_t rowsize;
    uint8_t * data;
    bool bgr;
    BmpCache & bmpcache;

    Drawable(const uint16_t width, const uint16_t height,
             const uint8_t bpp, const BGRPalette & palette,
             BmpCache & bmpcache, bool bgr=true)
      : full(Rect(0, 0, width, height)),
        bpp(bpp), palette(palette),
        rowsize(this->full.cx * ::nbbytes(this->bpp)),
        data(new uint8_t [this->rowsize * this->full.cy]),
        bgr(bgr),
        bmpcache(bmpcache)
    {
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = this->full.intersect(clip).intersect(cmd.rect);
        uint32_t color = color_decode(cmd.color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->opaquerect(trect, color);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        const Rect trect = this->full.intersect(clip).intersect(cmd.rect);
        this->destblt(trect, cmd.rop);
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        const Rect trect = this->full.intersect(clip).intersect(cmd.rect);
        #warning PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way, with stripes, etc.) and also there is quite a lot of possible ternary operators, and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then, work on correct computation of pattern and fix it.
        uint32_t color = color_decode(cmd.back_color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->patblt(trect, cmd.rop, color);
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect drect = this->full.intersect(clip).intersect(cmd.rect);
        if (drect.isempty()){ return; }
        // adding delta move dest to source
        const signed int deltax = cmd.srcx - cmd.rect.x;
        const signed int deltay = cmd.srcy - cmd.rect.y;
        this->scrblt(drect.x + deltax, drect.y + deltay, drect, cmd.rop);
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

        // enlarge_to compute a new rect including old rect and added point
        const Rect & line_rect = Rect(lineto.startx, lineto.starty, 1, 1).enlarge_to(lineto.endx, lineto.endy);
        if (line_rect.intersect(clip).isempty()){
//            LOG(LOG_INFO, "line_rect(%u, %u, %u, %u)", line_rect.x, line_rect.y, line_rect.cx, line_rect.cy);
            return;
        }
        // Color handling
        uint32_t color = color_decode(lineto.pen.color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }

        if (lineto.startx == lineto.endx){
            if (lineto.starty <= lineto.endy){
                this->vertical_line(lineto.back_mode,
                     lineto.startx, lineto.starty, lineto.endy,
                     color, clip);
            }
            else {
                this->vertical_line(lineto.back_mode,
                     lineto.startx, lineto.endy, lineto.starty,
                     color, clip);
            }
        }
        else if (lineto.starty == lineto.endy){
            this->horizontal_line(lineto.back_mode,
                 lineto.startx, lineto.starty, lineto.endx,
                 color, clip);

        }
        else if (lineto.startx <= lineto.endx){
            this->line(lineto.back_mode,
                 lineto.startx, lineto.starty, lineto.endx, lineto.endy,
                 color, clip);
        }
        else {
            this->line(lineto.back_mode,
                 lineto.endx, lineto.endy, lineto.startx, lineto.starty,
                 color, clip);
        }
    }

    void draw(const RDPBmpCache & cmd)
    {
        this->bmpcache.put(cmd.id, cmd.idx, cmd.bmp);
    }

    void draw(const RDPMemBlt & memblt, const Rect & clip)
    {
        #warning we should use rop parameter to change mem_blt behavior and palette_id part of cache_id
        const uint8_t id = memblt.cache_id & 0xFF;
        const Rect & rect = memblt.rect;
        const uint16_t srcx = memblt.srcx;
        const uint16_t srcy = memblt.srcy;
        const uint16_t idx = memblt.cache_idx;
        Bitmap * pbmp =  this->bmpcache.get(id, idx);
        const uint8_t * const bmp_data = pbmp->data_co(this->bpp);

        // Where we draw -> target
        uint32_t px = 0;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t * target = this->data + (rect.y * this->full.cx + rect.x) * 3;
        for (int j = 0; j < rect.cy ; j++){
            for (int i = 0; i < rect.cx ; i++){
                #warning: it would be nicer to manage clipping earlier and not test every pixel
                if (!(clip.contains_pt(i + rect.x, j + rect.y))) {
                  continue;
                }
                #warning this should not be done here, implement bitmap color conversion and use it here
                uint32_t src_px_offset = ((rect.cy - j - srcy - 1) * align4(rect.cx) + i + srcx) * nbbytes(this->bpp);
                switch (this->bpp){
                    default:
                    case 32:
                        assert(false);
                    break;
                    case 24:
                        {
                            px = (bmp_data[src_px_offset+2]<<16)
                               + (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = (px >> 16) & 0xFF;
                            g = (px >> 8)  & 0xFF;
                            b =  px        & 0xFF;
                        }
                        break;
                    case 16:
                        {
                            px = (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = (((px >> 8) & 0xf8) | ((px >> 13) & 0x7));
                            g = (((px >> 3) & 0xfc) | ((px >> 9) & 0x3));
                            b = (((px << 3) & 0xf8) | ((px >> 2) & 0x7));
                        }
                        break;
                    case 15:
                        {
                            px = (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = ((px >> 7) & 0xf8) | ((px >> 12) & 0x7);
                            g = ((px >> 2) & 0xf8) | ((px >> 8) & 0x7);
                            b = ((px << 3) & 0xf8) | ((px >> 2) & 0x7);
                        }
                        break;
                    case 8:
                        {
                            px = bmp_data[src_px_offset+0];

                            r = px & 7;
                            r = (r << 5) | (r << 2) | (r >> 1);
                            g = (px >> 3) & 7;
                            g = (g << 5) | (g << 2) | (g >> 1);
                            b =  (px >> 6) & 3;
                            b = (b << 6) | (b << 4) | (b << 2) | b;
                        }
                        break;
                }
                // Pixel assignment (!)
                uint8_t * pt = target + (j * this->full.cx + i) * 3;
                pt[0] = b;
                pt[1] = g;
                pt[2] = r;
            }
        }
    }

    uint8_t * first_pixel(const Rect & rect){
        return this->data + (rect.y * this->full.cx + rect.x) * ::nbbytes(this->bpp);
    }

    uint8_t * beginning_of_last_line(const Rect & rect){
        return this->data + ((rect.y + rect.cy - 1) * this->full.cx + rect.x) * ::nbbytes(this->bpp);
    }




    // low level opaquerect,
    // mostly avoid clipping because we already took care of it
    // also we already swapped color if we are using BGR instead of RGB
    void opaquerect(const Rect & rect, const uint32_t color)
    {
        uint8_t * const base = first_pixel(rect);
        uint8_t * p = base;

        for (size_t x = 0; x < (size_t)rect.cx ; x++){
            p[0] = color; p[1] = color >> 8; p[2] = color >> 16;
            p += 3;
        }
        uint8_t * target = base;
        size_t line_size = rect.cx * ::nbbytes(this->bpp);
        for (size_t y = 1; y < (size_t)rect.cy ; y++){
            target += this->rowsize;
            memcpy(target, base, line_size);
        }
    }

    // low level patblt,
    // mostly avoid clipping because we already took care of it
    void patblt(const Rect & rect, const uint8_t rop, const uint32_t color)
    {
        uint8_t * const base = first_pixel(rect);
        uint8_t * p = base;
        uint8_t p0 = color & 0xFF;
        uint8_t p1 = (color >> 8) & 0xFF;
        uint8_t p2 = (color >> 16) & 0xFF;

        switch (rop){
// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
        case 0x00: // blackness
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                memset(p, 0, rect.cx * ::nbbytes(this->bpp));
                p += this->rowsize;
            }
        break;
// +------+-------------------------------+
// | 0x05 | ROP: 0x000500A9               |
// |      | RPN: DPon                     |
// +------+-------------------------------+
        case 0x05:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~(p[0] | p0);
                    p[1] = ~(p[1] | p1);
                    p[2] = ~(p[2] | p2);
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0x0F | ROP: 0x000F0001               |
// |      | RPN: Pn                       |
// +------+-------------------------------+
        case 0x0F:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~p0;
                    p[1] = ~p1;
                    p[2] = ~p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0x50 | ROP: 0x00500325               |
// |      | RPN: PDna                     |
// +------+-------------------------------+
        case 0x50:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~p[0] & p0;
                    p[1] = ~p[1] & p1;
                    p[2] = ~p[2] & p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
        case 0x55: // inversion
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] ^= 0xFF; p[1] ^= 0xFF; p[2] ^= 0xFF;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
// |      | RPN: DPx                      |
// +------+-------------------------------+
        case 0x5A:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = p[0] ^ p0;
                    p[1] = p[1] ^ p1;
                    p[2] = p[2] ^ p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0x5F | ROP: 0x005F00E9               |
// |      | RPN: DPan                     |
// +------+-------------------------------+
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~(p[0] & p0);
                    p[1] = ~(p[1] & p1);
                    p[2] = ~(p[2] & p2);
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xA0 | ROP: 0x00A000C9               |
// |      | RPN: DPa                      |
// +------+-------------------------------+
        case 0xA0:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = p[0] | p0;
                    p[1] = p[1] | p1;
                    p[2] = p[2] | p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xA5 | ROP: 0x00A50065               |
// |      | RPN: PDxn                     |
// +------+-------------------------------+
        case 0xA5:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~(p[0] ^ p0);
                    p[1] = ~(p[1] ^ p1);
                    p[2] = ~(p[2] ^ p2);
                    p += 3;
                }
            }
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
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = p[0] | ~p0;
                    p[1] = p[1] | ~p1;
                    p[2] = p[2] | ~p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
// |      | RPN: P                        |
// +------+-------------------------------+
        case 0xF0:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = p0;
                    p[1] = p1;
                    p[2] = p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xF5 | ROP: 0x00F50225               |
// |      | RPN: PDno                     |
// +------+-------------------------------+
        case 0xF5:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = ~p[0] | p0;
                    p[1] = ~p[1] | p1;
                    p[2] = ~p[2] | p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xFA | ROP: 0x00FA0089               |
// |      | RPN: DPo                      |
// +------+-------------------------------+
        case 0xFA:
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = p[0] | p0;
                    p[1] = p[1] | p1;
                    p[2] = p[2] | p2;
                    p += 3;
                }
            }
        break;
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+
        case 0xFF: // whiteness
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                memset(p, 0, rect.cx * ::nbbytes(this->bpp));
                p += this->rowsize;
            }
        break;
        default:
            // should not happen, do nothing
        break;
        }
    }

    // low level destblt,
    // mostly avoid clipping because we already took care of it
    void destblt(const Rect & rect, const uint8_t rop)
    {
        uint8_t * const base = first_pixel(rect);
        uint8_t * p = base;

        switch (rop){
        case 0x00: // blackness
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                memset(p, 0, rect.cx * ::nbbytes(this->bpp));
                p += this->rowsize;
            }
        break;
        case 0x55: // inversion
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] ^= 0xFF; p[1] ^= 0xFF; p[2] ^= 0xFF;
                    p += 3;
                }
            }
        break;
        case 0xAA: // change nothing
        break;
        case 0xFF: // whiteness
            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                memset(p, 0, rect.cx * ::nbbytes(this->bpp));
                p += this->rowsize;
            }
        break;
        default:
            // should not happen
        break;
        }
    }

    // low level scrblt, mostly avoid considering clipping
    // because we already took care of it
    void scrblt(unsigned srcx, unsigned srcy, const Rect drect, uint8_t rop)
    {
        switch (rop){
        // +------+-------------------------------+
        // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
        // |      | RPN: 0                        |
        // +------+-------------------------------+
        case 0x00:
            this->destblt(drect, rop);
        break;
        // +------+-------------------------------+
        // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
        // |      | RPN: DSon                     |
        // +------+-------------------------------+
        case 0x11:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~(linetarget[b] | ~linesource[b]);
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x22 | ROP: 0x00220326               |
        // |      | RPN: DSna                     |
        // +------+-------------------------------+
        case 0x22:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = linetarget[b] & ~linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
        // |      | RPN: Sn                       |
        // +------+-------------------------------+
        case 0x33:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
        // |      | RPN: SDna                     |
        // +------+-------------------------------+
        case 0x44:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~linetarget[b] & linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }

        break;
        // +------+-------------------------------+
        // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
        // |      | RPN: Dn                       |
        // +------+-------------------------------+
        case 0x55:
            this->destblt(drect, rop);
        break;
        // +------+-------------------------------+
        // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
        // |      | RPN: DSx                      |
        // +------+-------------------------------+
        case 0x66:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = linetarget[b] ^ linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x77 | ROP: 0x007700E6               |
        // |      | RPN: DSan                     |
        // +------+-------------------------------+
        case 0x77:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~(linetarget[b] & linesource[b]);
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
        // |      | RPN: DSa                      |
        // +------+-------------------------------+
        case 0x88:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = linetarget[b] & linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0x99 | ROP: 0x00990066               |
        // |      | RPN: DSxn                     |
        // +------+-------------------------------+
        case 0x99:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~(linetarget[b] ^ linesource[b]);
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }

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
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = linetarget[b] | ~linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
        // |      | RPN: S                        |
        // +------+-------------------------------+
        case 0xCC:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const Rect srect = drect.offset(deltax, deltay);
            if (!srect.equal(drect)){
                const Rect & overlap = srect.intersect(drect);
                if ((deltay > 0)||(overlap.isempty())){
                    uint8_t * target = this->first_pixel(drect);
                    uint8_t * source = this->first_pixel(srect);
                    for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                        memcpy(target, source, drect.cx * ::nbbytes(this->bpp));
                        target += this->rowsize;
                        source += this->rowsize;
                    }
                }
                else if (deltay < 0){
                    uint8_t * target = this->beginning_of_last_line(drect);
                    uint8_t * source = this->beginning_of_last_line(srect);
                    for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                        memcpy(target, source, drect.cx * ::nbbytes(this->bpp));
                        target -= this->rowsize;
                        source -= this->rowsize;
                     }
                }
                else {
                    uint8_t * target = this->first_pixel(drect);
                    uint8_t * source = this->first_pixel(srect);
                    for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                        memmove(target, source, drect.cx * ::nbbytes(this->bpp));
                        target += this->rowsize;
                        source += this->rowsize;
                    }
                }
            }
        }
        break;
        // +------+-------------------------------+
        // | 0xDD | ROP: 0x00DD0228               |
        // |      | RPN: SDno                     |
        // +------+-------------------------------+
        case 0xDD:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = ~linetarget[b] | linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
        // |      | RPN: DSo                      |
        // +------+-------------------------------+
        case 0xEE:
        {
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(drect)
                             : this->beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->first_pixel(srect)
                             : this->beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->rowsize
                                     : -this->rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = linetarget[b] | linesource[b];
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        }
        break;
        // +------+-------------------------------+
        // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
        // |      | RPN: 1                        |
        // +------+-------------------------------+
        case 0xFF:
            this->destblt(drect, rop);
        break;
        default:
            // should not happen
        break;
        }
    }

    void line(const int mix_mode, const int startx, const int starty, const int endx, const int endy, const uint32_t color, const Rect & clip)
    {
//        printf("diagonal_line\n");

        // Color handling
        uint8_t col[3] = { color, color >> 8, color >> 16};

        // Prep
        int x = startx;
        int y = starty;
        int dx = endx - startx;
        int dy = (endy >= starty)?(endy - starty):(starty - endy);
        int sy = (endy >= starty)?1:-1;
        int err = dx - dy;

        while (true){
            if (clip.contains_pt(x, y)){
                const uint8_t Bpp = ::nbbytes(this->bpp);
                uint8_t * const p = this->data + (y * this->full.cx + x) * Bpp;
                for (uint8_t b = 0 ; b < Bpp; b++){
                    p[b] = col[b];
                }
//                printf("clip contains(%u, %u) Bpp=%u\n", x, y, Bpp);
            }

            if ((x >= endx) && (y == endy)){
                break;
            }

            // Calculating pixel position
            int e2 = err * 2; //prevents use of floating point
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

    void vertical_line(const int mix_mode, const int x, const int starty, const int endy, const uint32_t color, const Rect & clip)
    {
//        printf("vertical_line\n");
        // Color handling
        uint8_t col[3] = { color, color >> 8, color >> 16};

        // base adress (*3 because it has 3 color components)
        // also base of the new coordinate system
        uint8_t * const base = this->data + (starty * this->full.cx + x) * 3;
        const unsigned y0 = std::max(starty, clip.y);
        const unsigned y1 = std::min(endy, clip.y + clip.cy - 1);

        for (unsigned y = y0; y <= y1 ; y++) {
            // Pixel position
            uint8_t * const p = base + y * this->full.cx * 3;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            for (uint8_t b = 0 ; b < Bpp; b++){
                p[b] = col[b];
            }
        }
    }

    void horizontal_line(const int mix_mode, const int startx, const int y, const int endx, const uint32_t color, const Rect & clip)
    {
//        printf("horizontal_line\n");
        const unsigned x0 = std::max(startx, clip.x);
        const unsigned x1 = std::min(endx, clip.x + clip.cx - 1);
        uint8_t col[3] = { color, color >> 8, color >> 16};
        // base adress (*3 because it has 3 color components) also base of the new coordinate system
        uint8_t * const base = this->data + (y * this->full.cx + startx) * 3;

        // Prep
        for (unsigned x = x0; x <= x1 ; x++) {
            // Pixel position
            uint8_t * const p = base + x * 3;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            for (uint8_t b = 0 ; b < Bpp; b++){
                p[b] = col[b];
            }
        }
    }

};

#endif
