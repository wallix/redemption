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

class Drawable {
public:
    const Rect screen;
    const uint8_t bpp;
    const BGRPalette & palette;
    const size_t rowsize;
    uint8_t * data;
    bool bgr;

    Drawable(const uint16_t width, const uint16_t height, const uint8_t bpp, const BGRPalette & palette, bool bgr=true)
      : screen(Rect(0, 0, width, height)),
        bpp(bpp), palette(palette),
        rowsize(this->screen.cx * ::nbbytes(this->bpp)),
        data(new uint8_t [this->rowsize * this->screen.cy]),
        bgr(bgr)
    {
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = screen.intersect(clip).intersect(cmd.rect);
        uint32_t color = color_decode(cmd.color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->opaquerect(trect, color);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        const Rect trect = screen.intersect(clip).intersect(cmd.rect);
        this->destblt(trect, cmd.rop);
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect drect = screen.intersect(clip).intersect(cmd.rect);
        if (drect.isempty()){ return; }
        // adding delta move dest to source
        const signed int deltax = cmd.srcx - cmd.rect.x;
        const signed int deltay = cmd.srcy - cmd.rect.y;
        this->scrblt(drect.x + deltax, drect.y + deltay, drect);
    }


    uint8_t * first_pixel(const Rect & rect){
        return this->data + (rect.y * this->screen.cx + rect.x) * ::nbbytes(this->bpp);
    }

    uint8_t * beginning_of_last_line(const Rect & rect){
        return this->data + ((rect.y + rect.cy - 1) * this->screen.cx + rect.x) * ::nbbytes(this->bpp);
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
    void scrblt(unsigned srcx, unsigned srcy, const Rect drect)
    {
        const signed int deltax = srcx - drect.x;
        const signed int deltay = srcy - drect.y;
        const Rect srect = drect.offset(deltax, deltay);
        if (!srect.equal(drect)){
            const Rect & overlap = srect.intersect(drect);
            if ((deltay > 0)||(overlap.isempty())){
                uint8_t * target = this->first_pixel(drect);
                uint8_t * source = this->first_pixel(srect);
                int offset = 0;
                size_t width_in_bytes = this->screen.cx * ::nbbytes(this->bpp);
                for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                    memcpy(target + offset,
                           source + offset,
                           drect.cx * ::nbbytes(this->bpp));
                    offset += width_in_bytes;
                }
            }
            else if (deltay < 0){
                uint8_t * target = this->beginning_of_last_line(drect);
                uint8_t * source = this->beginning_of_last_line(srect);
                int offset = 0;
                size_t width_in_bytes = this->screen.cx * ::nbbytes(this->bpp);
                for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                    memcpy(target + offset,
                           source + offset,
                           drect.cx * ::nbbytes(this->bpp));
                    offset += - width_in_bytes;
                 }
            }
            else {
                uint8_t * target = this->first_pixel(drect);
                uint8_t * source = this->first_pixel(srect);
                int offset = 0;
                size_t width_in_bytes = this->screen.cx * ::nbbytes(this->bpp);
                for (size_t j = 0; j < (size_t)drect.cy ; j++) {
                    memmove(target + offset,
                           source + offset,
                           drect.cx * ::nbbytes(this->bpp));
                    offset += width_in_bytes;
                }
            }
        }
    }


};

#endif
