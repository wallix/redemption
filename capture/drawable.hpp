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
#include <time.h>

class Drawable {
public:
    uint8_t bpp;
    const BGRPalette & palette;
    bool bgr;
    BmpCache & bmpcache;

    struct CaptureBuf
    {
        static const std::size_t Bpp = 3;

        int width;
        int height;
        const size_t rowsize;
        unsigned long pix_len;
        uint8_t * data;

        CaptureBuf(int width, int height)
            : width(width)
            , height(height)
            , rowsize(width * Bpp)
            , pix_len(this->rowsize * height)
        {
            if (!this->pix_len) {
                throw Error(ERR_RECORDER_EMPTY_IMAGE);
            }
            this->data = new (std::nothrow) uint8_t[this->pix_len];
            if (this->data == 0){
                throw Error(ERR_RECORDER_FRAME_ALLOCATION_FAILED);
            }
            std::fill<>(this->data, this->data + this->pix_len, 0);
        }

        ~CaptureBuf()
        {
            delete[] this->data;
        }

        uint8_t * first_pixel()
        {
            return this->data;
        }

        uint8_t * first_pixel(const Rect & rect)
        {
            return this->data + (rect.y * this->width + rect.x) * Bpp;
        }

        uint8_t * first_pixel(int y)
        {
            return this->data + y * this->width * Bpp;
        }

        uint8_t * first_pixel(int x, int y)
        {
            return this->data + (y * this->width + x) * Bpp;
        }

        uint8_t * after_last_pixel()
        {
            return this->data + this->pix_len;
        }

        uint8_t * beginning_of_last_line(const Rect & rect)
        {
            return this->data + ((rect.y + rect.cy - 1) * this->width + rect.x) * Bpp;
        }

        int size() const
        {
            return this->width * this->height;
        }

        struct Mouse_t{
            uint8_t y;
            uint8_t x;
            uint8_t lg;
            const char * line;
        };

        enum { contiguous_mouse_pixels = 20 };

        const Mouse_t & line_of_mouse(size_t i){
            static const Mouse_t mouse_cursor[contiguous_mouse_pixels] =
            {
                {0,  0, 3*1, "\x00\x00\x00"},
                {1,  0, 3*2, "\x00\x00\x00\x00\x00\x00"},
                {2,  0, 3*3, "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},
                {3,  0, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {4,  0, 3*5, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {5,  0, 3*6, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {6,  0, 3*7, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {7,  0, 3*8, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {8,  0, 3*9, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {9,  0, 3*10, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {10, 0, 3*11, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {11, 0, 3*12, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {12, 0, 3*12, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"},
                {13, 0, 3*8, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {14, 0, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {14, 5, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {15, 0, 3*3, "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},
                {15, 5, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
                {16, 1, 3*1, "\x00\x00\x00"},
                {16, 6, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"}
            };
            return mouse_cursor[i];
        }
        
        enum { mouse_height = 17 };

        uint8_t save_mouse[1024];
        uint16_t save_mouse_x;
        uint16_t save_mouse_y;

        char * pixel_start_data(int x, int y, size_t i)
        {
            return (char*)this->data + ((this->line_of_mouse(i).y + y) * this->width + this->line_of_mouse(i).x + x) * 3;
        }

        int _posch(char ch)
        {
            return 7 * 11 *
            (isdigit(ch) ? ch-'0'
            : ch == '-'  ?    10
            : ch == ':'  ?    11
            : ch == 'X'  ?    13
            :                 12);
        }

        void draw_11x7_digits(uint8_t * rgbpixbuf, unsigned width, unsigned lg_message, const char * message, const char * old_message)
        {
            const char * digits =
            "       "
            "  XX   "
            " X  X  "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            " X  X  "
            "  XX   "
            "       "

            "       "
            "  XX   "
            " XXX   "
            "X XX   "
            "  XX   "
            "  XX   "
            "  XX   "
            "  XX   "
            "  XX   "
            "XXXXXX "
            "       "

            "       "
            " XXXX  "
            "XX  XX "
            "XX  XX "
            "    XX "
            "  XXX  "
            " XX    "
            "XX     "
            "XX     "
            "XXXXXX "
            "       "


            "       "
            "XXXXXX "
            "    XX "
            "   XX  "
            "  XX   "
            " XXXX  "
            "    XX "
            "    XX "
            "XX  XX "
            " XXXX  "
            "       "


            "       "
            "    XX "
            "   XXX "
            "  XXXX "
            " XX XX "
            "XX  XX "
            "XX  XX "
            "XXXXXX "
            "    XX "
            "    XX "
            "       "

            "       "
            "XXXXXX "
            "XX     "
            "XX     "
            "XXXXX  "
            "XX  XX "
            "    XX "
            "    XX "
            "XX  XX "
            " XXXX  "
            "       "

            "       "
            " XXXX  "
            "XX  XX "
            "XX     "
            "XX     "
            "XXXXX  "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            " XXXX  "
            "       "

            "       "
            "XXXXXX "
            "    XX "
            "    XX "
            "   XX  "
            "   XX  "
            "  XX   "
            "  XX   "
            " XX    "
            " XX    "
            "       "

            "       "
            " XXXX  "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            " XXXX  "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            " XXXX  "
            "       "

            "       "
            " XXXX  "
            "XX  XX "
            "XX  XX "
            "XX  XX "
            " XXXXX "
            "    XX "
            "    XX "
            "XX  XX "
            " XXXX  "
            "       "

            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "XXXXXX "
            "       "
            "       "
            "       "
            "       "

            "       "
            "       "
            "  XX   "
            " XXXX  "
            "  XX   "
            "       "
            "       "
            "  XX   "
            " XXXX  "
            "  XX   "
            "       "

            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "       "
            "       "

            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            "XXXXXXX"
            ;
            for (size_t i = 0 ; i < lg_message ; ++i){
                char newch = message[i];
                char oldch = old_message[i];

                
                if (newch != oldch){
                    const char * pnewch = digits + _posch(newch);
                    const char * poldch = digits + _posch(oldch);

                    unsigned br_pix = 0;
                    unsigned br_pixindex = i * (7 * 3);

                    for (size_t y = 0 ; y < 11 ; ++y, br_pix += 7, br_pixindex += width*3){
                        for (size_t x = 0 ; x <  7 ; ++x){
                            unsigned pix = br_pix + x;
                            if (pnewch[pix] != poldch[pix]){
                                uint8_t pixcolorcomponent = (pnewch[pix] == 'X') ? 0xFF : 0;
                                unsigned pixindex = br_pixindex + x*3;
                                rgbpixbuf[pixindex] =
                                    rgbpixbuf[pixindex+1] =
                                        rgbpixbuf[pixindex+2] = pixcolorcomponent;
                            }
                        }
                    }
                }
            }
        }

    } data;


    Drawable(const uint16_t width, const uint16_t height,
             const uint8_t bpp, const BGRPalette & palette,
             BmpCache & bmpcache, bool bgr=true)
      : bpp(bpp),
        palette(palette), // source palette for RDP primitives
        bgr(bgr),
        bmpcache(bmpcache),
        data(width, height)
    {
        memset(this->timestamp_data, 0xFF, sizeof(this->timestamp_data));
        memset(this->previous_timestamp, 'X', sizeof(this->previous_timestamp));
//        memset(this->timestamp_data, 0x00, sizeof(this->timestamp_data));
//        memset(this->previous_timestamp, ' ', len_str_timestamp);

    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect & trect = Rect(0, 0, this->data.width, this->data.height).intersect(clip).intersect(cmd.rect);
        uint32_t color = color_decode(cmd.color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->opaquerect(trect, color);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        const Rect trect = Rect(0, 0, this->data.width, this->data.height).intersect(clip).intersect(cmd.rect);
        this->destblt(trect, cmd.rop);
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        const Rect trect = Rect(0, 0, this->data.width, this->data.height).intersect(clip).intersect(cmd.rect);
        TODO(" PatBlt is not yet fully implemented. It is awkward to do because computing actual brush pattern is quite tricky (brushes are defined in a so complex way  with stripes  etc.) and also there is quite a lot of possible ternary operators  and how they are encoded inside rop3 bits is not obvious at first. We should begin by writing a pseudo patblt always using back_color for pattern. Then  work on correct computation of pattern and fix it.")
        uint32_t color = color_decode(cmd.back_color, this->bpp, this->palette);
        if (this->bgr){
            color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
        }
        this->patblt(trect, cmd.rop, color);
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect drect = Rect(0, 0, this->data.width, this->data.height).intersect(clip).intersect(cmd.rect);
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
        // nothing to do, cache management is performed outside Drawable
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip)
    {
        const Rect& rect = clip.intersect(cmd.rect);
        if (rect.isempty()){
            return ;
        }

        Bitmap* bmp = this->bmpcache.get(cmd.cache_id & 0x3, cmd.cache_idx);
        if (!bmp){
            LOG(LOG_ERR,
                "bitmap not found in cache (%p) (id = %u, idx = %u)",
                &this->bmpcache, cmd.cache_id, cmd.cache_idx);
            return;
        }

        switch (cmd.rop) {
            case 0x00:
                this->black_color(rect); break;
            case 0xFF:
                this->white_color(rect); break;
            case 0x55:
                this->mem_blt(rect, *bmp, cmd.srcx, cmd.srcy, 0xFFFFFF, this->bgr);
            break;
            case 0xCC:
                this->mem_blt(rect, *bmp, cmd.srcx, cmd.srcy, 0, this->bgr);
            break;
            default:
                // should not happen
            break;
        }
    }


    /*
     * The name doesn't say it : mem_blt COPIES a decoded bitmap from
     * a cache (data) and insert a subpart (srcx, srcy) to the local
     * image cache (this->data) a the given position (rect).
     */
    void mem_blt(const Rect& rect, Bitmap & bmp, const unsigned int srcx, const unsigned int srcy, const uint32_t xormask, const bool bgr)
    {
        if (bmp.cx < srcx || bmp.cy < srcy){
            return ;
        }

        const Rect & trect = Rect(
            rect.x, rect.y,
            std::min<int>(bmp.cx - srcx,
                          std::min(this->data.width - rect.x, rect.cx)),
            std::min<int>(bmp.cy - srcy,
                          std::min(this->data.height - rect.y, rect.cy)));
        if (trect.isempty()){
            return ;
        }
        const uint8_t Bpp = ::nbbytes(bmp.original_bpp);
        uint8_t * target = this->data.first_pixel(trect);
        uint8_t * source = bmp.data_co(bmp.original_bpp) + ((bmp.cy - srcy - 1) * bmp.cx + srcx) * Bpp;
        int steptarget = (this->data.width - trect.cx) * 3;
        int stepsource = (bmp.cx + trect.cx) * Bpp;

        for (int y = 0; y < trect.cy ; y++, target += steptarget, source -= stepsource){
            for (int x = 0; x < trect.cx ; x++, target += 3, source += Bpp){
                uint32_t px = source[Bpp-1];
                for (int b = 1 ; b < Bpp ; b++){
                    px = (px << 8) + source[Bpp-1-b];
                }
                uint32_t color = xormask ^ color_decode(px, bmp.original_bpp, bmp.original_palette);
                if (bgr){
                    color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
                }
                target[0] = color;
                target[1] = color >> 8;
                target[2] = color >> 16;
            }
        }
    }


    void black_color(const Rect & rect)
    {
        const Rect & trect = rect.intersect(this->data.width, this->data.height);
        uint8_t * p = this->data.first_pixel(trect);
        const size_t step = this->data.rowsize;
        const size_t rect_rowsize = trect.cx * this->data.Bpp;
        for (int j = 0; j < trect.cy ; j++, p += step){
            bzero(p, rect_rowsize);
        }
    }

    void white_color(const Rect & rect)
    {
        uint8_t * p = this->data.first_pixel(rect);
        const size_t step = this->data.rowsize;
        const size_t rect_rowsize = rect.cx * this->data.Bpp;
        for (int j = 0; j < rect.cy ; j++, p += step){
            memset(p, 0xFF, rect_rowsize);
        }
    }

    void invert_color(const Rect & rect)
    {
        const Rect & trect = rect.intersect(this->data.width, this->data.height);
        uint8_t * p = this->data.first_pixel(trect);
        const size_t rect_rowsize = trect.cx * this->data.Bpp;
        const size_t step = this->data.rowsize - rect_rowsize;
        for (int j = 0; j < trect.cy ; j++, p += step){
            for (int i = 0; i < trect.cx ; i++, p += 3){
                TODO("Applying inversion on blocks of 32 bits instead of bytes should be faster")
                p[0] ^= 0xFF; p[1] ^= 0xFF; p[2] ^= 0xFF;
            }
        }
    }

    // low level opaquerect,
    // mostly avoid clipping because we already took care of it
    // also we already swapped color if we are using BGR instead of RGB
    void opaquerect(const Rect & rect, const uint32_t color)
    {
        uint8_t * const base = this->data.first_pixel(rect);
        uint8_t * p = base;

        for (size_t x = 0; x < (size_t)rect.cx ; x++){
            p[0] = color; p[1] = color >> 8; p[2] = color >> 16;
            p += 3;
        }
        uint8_t * target = base;
        size_t line_size = rect.cx * ::nbbytes(this->bpp);
        for (size_t y = 1; y < (size_t)rect.cy ; y++){
            target += this->data.rowsize;
            memcpy(target, base, line_size);
        }
    }

    template <typename Op>
        void patblt_op(const Rect & rect, const uint32_t color)
        {
            Op op;
            uint8_t * const base = this->data.first_pixel(rect);
            uint8_t * p = base;
            uint8_t p0 = color & 0xFF;
            uint8_t p1 = (color >> 8) & 0xFF;
            uint8_t p2 = (color >> 16) & 0xFF;

            for (size_t y = 0; y < (size_t)rect.cy ; y++){
                p = base + this->data.rowsize * y;
                for (size_t x = 0; x < (size_t)rect.cx ; x++){
                    p[0] = op(p[0], p0);
                    p[1] = op(p[1], p1);
                    p[2] = op(p[2], p2);
                    p += 3;
                }
            }
        }

    struct Op_0x05
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target | source);
        }
    };

    struct Op_0x0F
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~source;
        }
    };

    struct Op_0x50
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~target & source;
        }
    };

    struct Op_0x5A
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target ^ source;
        }
    };

    struct Op_0x5F
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target & source);
        }
    };

    struct Op_0xA0
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target & source;
        }
    };

    struct Op_0xA5
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target ^ source);
        }
    };

    struct Op_0xAF
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target | ~source;
        }
    };

    TODO("This one is a memset and should be simplified")
    struct Op_0xF0
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return source;
        }
    };

    struct Op_0xF5
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~target | source;
        }
    };

    struct Op_0xFA
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target | source;
        }
    };

    // low level patblt,
    // mostly avoid clipping because we already took care of it
    void patblt(const Rect & rect, const uint8_t rop, const uint32_t color)
    {


        TODO(" this switch contains much duplicated code  to merge it we should use a function template with a parameter that would be a function (the inner operator). Even if templates are often more of a problem than a solution  in this particular case I see no obvious better way.")
        switch (rop){
// +------+-------------------------------+
// | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
// |      | RPN: 0                        |
// +------+-------------------------------+
        case 0x00: // blackness
            this->black_color(rect);
        break;
// +------+-------------------------------+
// | 0x05 | ROP: 0x000500A9               |
// |      | RPN: DPon                     |
// +------+-------------------------------+
        case 0x05:
            this->patblt_op<Op_0x05>(rect, color);
        break;
// +------+-------------------------------+
// | 0x0F | ROP: 0x000F0001               |
// |      | RPN: Pn                       |
// +------+-------------------------------+
        case 0x0F:
            this->patblt_op<Op_0x0F>(rect, color);
        break;
// +------+-------------------------------+
// | 0x50 | ROP: 0x00500325               |
// |      | RPN: PDna                     |
// +------+-------------------------------+
        case 0x50:
            this->patblt_op<Op_0x50>(rect, color);
        break;
// +------+-------------------------------+
// | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
// |      | RPN: Dn                       |
// +------+-------------------------------+
        case 0x55: // inversion
            this->invert_color(rect);
        break;
// +------+-------------------------------+
// | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
// |      | RPN: DPx                      |
// +------+-------------------------------+
        case 0x5A:
            this->patblt_op<Op_0x5A>(rect, color);
        break;
// +------+-------------------------------+
// | 0x5F | ROP: 0x005F00E9               |
// |      | RPN: DPan                     |
// +------+-------------------------------+
        case 0x5F:
            this->patblt_op<Op_0x5F>(rect, color);
        break;
// +------+-------------------------------+
// | 0xA0 | ROP: 0x00A000C9               |
// |      | RPN: DPa                      |
// +------+-------------------------------+
        case 0xA0:
            this->patblt_op<Op_0xA0>(rect, color);
        break;
// +------+-------------------------------+
// | 0xA5 | ROP: 0x00A50065               |
// |      | RPN: PDxn                     |
// +------+-------------------------------+
        case 0xA5:
            this->patblt_op<Op_0xA5>(rect, color);
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
            this->patblt_op<Op_0xAF>(rect, color);
        break;
// +------+-------------------------------+
// | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
// |      | RPN: P                        |
// +------+-------------------------------+
        case 0xF0:
            this->patblt_op<Op_0xF0>(rect, color);
        break;
// +------+-------------------------------+
// | 0xF5 | ROP: 0x00F50225               |
// |      | RPN: PDno                     |
// +------+-------------------------------+
        case 0xF5:
            this->patblt_op<Op_0xF5>(rect, color);
        break;
// +------+-------------------------------+
// | 0xFA | ROP: 0x00FA0089               |
// |      | RPN: DPo                      |
// +------+-------------------------------+
        case 0xFA:
            this->patblt_op<Op_0xFA>(rect, color);
        break;
// +------+-------------------------------+
// | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
// |      | RPN: 1                        |
// +------+-------------------------------+
        case 0xFF: // whiteness
            this->white_color(rect);
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
        switch (rop){
        case 0x00: // blackness
            this->black_color(rect);
        break;
        case 0x55: // inversion
            this->invert_color(rect);
        break;
        case 0xAA: // change nothing
        break;
        case 0xFF: // whiteness
            this->white_color(rect);
        break;
        default:
            // should not happen
        break;
        }
    }

    struct Op_0x11
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target | ~source);
        }
    };

    struct Op_0x22
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target & ~source;
        }
    };

    struct Op_0x33
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            TODO("The templated function can be optimize in the case the target is not read.")
            (void)target;
            return ~source;
        }
    };

    struct Op_0x44
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~target & source;
        }
    };

    struct Op_0x66
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target ^ source;
        }
    };

    struct Op_0x77
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target & source);
        }
    };

    struct Op_0x88
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target & source;
        }
    };

    struct Op_0x99
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~(target ^ source);
        }
    };

    struct Op_0xBB
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target | ~source;
        }
    };

    struct Op_0xCC
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            TODO("The templated function can be optimized because in the case the target is not read. See commented code in src_blt (and add performance benchmark)")
            (void)target;
            return source;
        }
    };

    struct Op_0xDD
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return ~target | source;
        }
    };

    struct Op_0xEE
    {
        uint8_t operator()(uint8_t target, uint8_t source)
        {
            return target | source;
        }
    };


    template <typename Op>
        void scr_blt_op(unsigned srcx, unsigned srcy, const Rect drect)
        {
            Op op;
            const signed int deltax = srcx - drect.x;
            const signed int deltay = srcy - drect.y;
            const uint8_t Bpp = ::nbbytes(this->bpp);
            const Rect srect = drect.offset(deltax, deltay);
            const Rect & overlap = srect.intersect(drect);
            uint8_t * target = ((deltay >= 0)||overlap.isempty())
                             ? this->data.first_pixel(drect)
                             : this->data.beginning_of_last_line(drect);
            uint8_t * source = ((deltay >= 0)||overlap.isempty())
                             ? this->data.first_pixel(srect)
                             : this->data.beginning_of_last_line(srect);
            const signed int to_nextrow = ((deltay >= 0)||overlap.isempty())
                                     ?  this->data.rowsize
                                     : -this->data.rowsize;
            signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?Bpp:-Bpp;
            const unsigned offset = ((deltay != 0)||(deltax >= 0))?0:Bpp*(drect.cx - 1);
            for (size_t y = 0; y < (size_t)drect.cy ; y++) {
                uint8_t * linetarget = target + offset;
                uint8_t * linesource = source + offset;
                for (size_t x = 0; x < (size_t)drect.cx ; x++) {
                    for (uint8_t b = 0 ; b < Bpp; b++){
                        linetarget[b] = op(linetarget[b], linesource[b]);
                    }
                    linetarget += to_nextpixel;
                    linesource += to_nextpixel;
                }
                target += to_nextrow;
                source += to_nextrow;
            }
        };

    // low level scrblt, mostly avoid considering clipping
    // because we already took care of it
    void scrblt(unsigned srcx, unsigned srcy, const Rect drect, uint8_t rop)
    {
        TODO(" this switch contains much duplicated code  to merge it we should use a function template with a parameter that would be a function (the inner operator). Even if templates are often more of a problem than a solution  in this particular case I see no obvious better way.")
        switch (rop){
        // +------+-------------------------------+
        // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
        // |      | RPN: 0                        |
        // +------+-------------------------------+
        case 0x00:
            this->black_color(drect);
        break;
        // +------+-------------------------------+
        // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
        // |      | RPN: DSon                     |
        // +------+-------------------------------+
        case 0x11:
            this->scr_blt_op<Op_0x11>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x22 | ROP: 0x00220326               |
        // |      | RPN: DSna                     |
        // +------+-------------------------------+
        case 0x22:
            this->scr_blt_op<Op_0x22>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
        // |      | RPN: Sn                       |
        // +------+-------------------------------+
        case 0x33:
            this->scr_blt_op<Op_0x33>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
        // |      | RPN: SDna                     |
        // +------+-------------------------------+
        case 0x44:
            this->scr_blt_op<Op_0x44>(srcx, srcy, drect);
        break;

        // +------+-------------------------------+
        // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
        // |      | RPN: Dn                       |
        // +------+-------------------------------+
        case 0x55:
            this->invert_color(drect);
        break;
        // +------+-------------------------------+
        // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
        // |      | RPN: DSx                      |
        // +------+-------------------------------+
        case 0x66:
            this->scr_blt_op<Op_0x66>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x77 | ROP: 0x007700E6               |
        // |      | RPN: DSan                     |
        // +------+-------------------------------+
        case 0x77:
            this->scr_blt_op<Op_0x77>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
        // |      | RPN: DSa                      |
        // +------+-------------------------------+
        case 0x88:
            this->scr_blt_op<Op_0x88>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0x99 | ROP: 0x00990066               |
        // |      | RPN: DSxn                     |
        // +------+-------------------------------+
        case 0x99:
            this->scr_blt_op<Op_0x99>(srcx, srcy, drect);
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
            this->scr_blt_op<Op_0xBB>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
        // |      | RPN: S                        |
        // +------+-------------------------------+
        case 0xCC:
            this->scr_blt_op<Op_0xCC>(srcx, srcy, drect);
//        {
//            const signed int deltax = srcx - drect.x;
//            const signed int deltay = srcy - drect.y;
//            const Rect srect = drect.offset(deltax, deltay);
//            if (!srect.equal(drect)){
//                const Rect & overlap = srect.intersect(drect);
//                if ((deltay >= 0)||(overlap.isempty())){
//                    uint8_t * target = this->first_pixel(drect);
//                    uint8_t * source = this->first_pixel(srect);
//                    for (size_t j = 0; j < (size_t)drect.cy ; j++) {
//                        memcpy(target, source, drect.cx * ::nbbytes(this->bpp));
//                        target += this->data.rowsize;
//                        source += this->data.rowsize;
//                    }
//                }
//                else if (deltay < 0){
//                    uint8_t * target = this->beginning_of_last_line(drect);
//                    uint8_t * source = this->beginning_of_last_line(srect);
//                    for (size_t j = 0; j < (size_t)drect.cy ; j++) {
//                        memcpy(target, source, drect.cx * ::nbbytes(this->bpp));
//                        target -= this->data.rowsize;
//                        source -= this->data.rowsize;
//                     }
//                }
//            }
//        }
        break;
        // +------+-------------------------------+
        // | 0xDD | ROP: 0x00DD0228               |
        // |      | RPN: SDno                     |
        // +------+-------------------------------+
        case 0xDD:
            this->scr_blt_op<Op_0xDD>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
        // |      | RPN: DSo                      |
        // +------+-------------------------------+
        case 0xEE:
            this->scr_blt_op<Op_0xEE>(srcx, srcy, drect);
        break;
        // +------+-------------------------------+
        // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
        // |      | RPN: 1                        |
        // +------+-------------------------------+
        case 0xFF:
            this->white_color(drect);
        break;
        default:
            // should not happen
        break;
        }
    }

    // nor horizontal nor vertical, use Bresenham
    void line(const int mix_mode, const int startx, const int starty, const int endx, const int endy, const uint32_t color, const Rect & clip)
    {
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
                uint8_t * const p = this->data.data + (y * Rect(0, 0, this->data.width, this->data.height).cx + x) * Bpp;
                for (uint8_t b = 0 ; b < Bpp; b++){
                    p[b] = col[b];
                }
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
        // Color handling
        uint8_t col[3] = { color, color >> 8, color >> 16};

        // also base of the new coordinate system
        const unsigned y0 = std::max(starty, clip.y);
        const unsigned y1 = std::min(endy, clip.y + clip.cy - 1);
        const uint16_t & height = Rect(0, 0, this->data.width, this->data.height).cx;
        const uint8_t Bpp = ::nbbytes(this->bpp);

        // these tests are probably unnecessary if calling code is ok
        if (y0 >= height){ return; }
        if (y1 >= height){ return; }

        if (y0 < y1){ // this test is probably unnecessary if calling code is ok
            uint8_t * const base = this->data.data + (y0 * height + x) * 3;

            for (unsigned dy = 0; dy <= (y1 - y0) ; dy++) {
                uint8_t * const p = base + dy * height * 3;
                for (uint8_t b = 0 ; b < Bpp; b++){
                    p[b] = col[b];
                }
            }
        }
    }

    void horizontal_line(const int mix_mode, const int startx, const int y, const int endx, const uint32_t color, const Rect & clip)
    {
        const unsigned x0 = std::max(startx, clip.x);
        const unsigned x1 = std::min(endx, clip.x + clip.cx - 1);
        uint8_t col[3] = { color, color >> 8, color >> 16};
        const uint16_t & height = Rect(0, 0, this->data.width, this->data.height).cx;
        const uint8_t Bpp = ::nbbytes(this->bpp);

        // this tests is probably unnecessary if calling code is ok
        if (y >= height){ return; }

        // base adress (*3 because 3 bytes per pixel)
        uint8_t * const base = this->data.data + (y * height) * 3;

        for (unsigned x = x0; x <= x1 ; x++) {
            // Pixel position
            uint8_t * const p = base + x * 3;
            for (uint8_t b = 0 ; b < Bpp; b++){
                p[b] = col[b];
            }
        }
    }

    void trace_mouse(uint16_t x, uint16_t y)
    {
        uint8_t * psave = this->data.save_mouse;
        for (size_t i = 0 ; i < CaptureBuf::contiguous_mouse_pixels ; i++){
            char * pixel_start = this->data.pixel_start_data(x,y, i);
            unsigned lg = this->data.line_of_mouse(i).lg;
            memcpy(psave, pixel_start, lg);
            psave += lg;
            memcpy(pixel_start, this->data.line_of_mouse(i).line, lg);
        }
        this->data.save_mouse_x = x;
        this->data.save_mouse_y = y;
    }

    void clear_mouse()
    {
        uint8_t * psave = this->data.save_mouse;
        uint16_t x = this->data.save_mouse_x;
        uint16_t y = this->data.save_mouse_y;
        for (size_t i = 0 ; i < CaptureBuf::contiguous_mouse_pixels ; i++){
            char * pixel_start = this->data.pixel_start_data(x,y, i);
            unsigned lg = this->data.line_of_mouse(i).lg;
            memcpy(pixel_start, psave, lg);
            psave += lg;
        }
    }


    enum {
        ts_width = 133,
        ts_height = 11,
        size_str_timestamp = 20
    };

    uint8_t timestamp_save[ts_width * ts_height * 3];
    uint8_t timestamp_data[ts_width * ts_height * 3];
    char previous_timestamp[size_str_timestamp];

    void trace_timestamp(tm & now)
    {
        char rawdate[size_str_timestamp];
        snprintf(rawdate, size_str_timestamp, "%4d-%02d-%02d %02d:%02d:%02d",
                now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min, now.tm_sec);

        this->data.draw_11x7_digits(this->timestamp_data, ts_width, size_str_timestamp-1, rawdate, this->previous_timestamp);
        memcpy(this->previous_timestamp, rawdate, size_str_timestamp);
        
        uint8_t * tsave = this->timestamp_save;
        uint8_t* buf = this->data.data;
        int step = this->data.width * 3;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(tsave, buf, ts_width*3);
            tsave += ts_width*3;
            memcpy(buf, this->timestamp_data + y*ts_width*3, ts_width*3);
        }
    }

    void clear_timestamp()
    {
        const uint8_t * tsave = this->timestamp_save;
        int step = this->data.width * 3;
        uint8_t* buf = this->data.data;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(buf, tsave, ts_width*3);
            tsave += ts_width*3;
        }

    }

};

#endif
