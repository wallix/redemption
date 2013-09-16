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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
              Poelen Jonathan
*/

#ifndef _REDEMPTION_UTILS_DRAWABLE_HPP_
#define _REDEMPTION_UTILS_DRAWABLE_HPP_

#include <algorithm>
#include "bitmap.hpp"

#include "colors.hpp"
#include "rect.hpp"

struct Drawable {
    static const std::size_t Bpp = 3;

    uint16_t width;
    uint16_t height;
    const size_t rowsize;
    unsigned long pix_len;
    uint8_t * data;

    enum {
        char_width  = 7,
        char_height = 12
    };

    enum {
        ts_max_length = 32
    };

    enum {
        ts_width = ts_max_length * char_width,
        ts_height = char_height,
        size_str_timestamp = ts_max_length + 1
    };

    uint8_t timestamp_save[ts_width * ts_height * 3];
    uint8_t timestamp_data[ts_width * ts_height * 3];
    char previous_timestamp[size_str_timestamp];
    uint8_t previous_timestamp_length;

    struct Mouse_t {
        int      y;
        int      x;
        int      lg;
        const char * line;
    };

    size_t          contiguous_mouse_pixels;
    const Mouse_t * mouse_cursor;
    uint8_t         mouse_hotspot_x;
    uint8_t         mouse_hotspot_y;
    uint8_t         save_mouse[1024];
    uint16_t        save_mouse_x;
    uint16_t        save_mouse_y;

    Drawable(int width, int height)
    : width(width)
    , height(height)
    , rowsize(width * Bpp)
    , pix_len(this->rowsize * height)
    {
        static const Mouse_t default_mouse_cursor[] =
        {
            {0,  0, 3 * 1,  "\x00\x00\x00"},
            {1,  0, 3 * 2,  "\x00\x00\x00\x00\x00\x00"},
            {2,  0, 3 * 3,  "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},
            {3,  0, 3 * 4,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {4,  0, 3 * 5,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {5,  0, 3 * 6,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {6,  0, 3 * 7,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {7,  0, 3 * 8,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {8,  0, 3 * 9,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {9,  0, 3 * 10, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {10, 0, 3 * 11, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {11, 0, 3 * 12, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {12, 0, 3 * 12, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"},
            {13, 0, 3 * 8,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {14, 0, 3 * 4,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {14, 5, 3 * 4,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {15, 0, 3 * 3,  "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},
            {15, 5, 3 * 4,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {16, 1, 3 * 1,  "\x00\x00\x00"},
            {16, 6, 3 * 4,  "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"}
        };

        this->contiguous_mouse_pixels = 20;
        this->mouse_cursor            = default_mouse_cursor;
        this->mouse_hotspot_x         = 0;
        this->mouse_hotspot_y         = 0;

        if (!this->pix_len) {
            throw Error(ERR_RECORDER_EMPTY_IMAGE);
        }
        this->data = new (std::nothrow) uint8_t[this->pix_len];
        if (this->data == 0){
            throw Error(ERR_RECORDER_FRAME_ALLOCATION_FAILED);
        }
        std::fill<>(this->data, this->data + this->pix_len, 0);

        memset(this->timestamp_data, 0xFF, sizeof(this->timestamp_data));
        memset(this->previous_timestamp, 0x07, sizeof(this->previous_timestamp));
        this->previous_timestamp_length = 0;
    }

    ~Drawable()
    {
        delete[] this->data;
    }

    void bgr2rgb()
    {
        const uint32_t * s = reinterpret_cast<const uint32_t *>(this->data);
        uint32_t * t = reinterpret_cast<uint32_t *>(this->data);
        for (size_t y = 0; y < this->height ; y++){
            for (size_t n = 0; n < (this->width / 4) ; n++){
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
    }

    uint8_t * first_pixel()
    {
        return this->data;
    }

    const uint8_t * first_pixel() const
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

    const uint8_t * first_pixel(int x, int y) const
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

    int _posch_12x7(char ch)
    {
        return char_width * char_height *
        (isdigit(ch)  ? ch-'0'
        : isupper(ch) ? ch - 'A' + 14
        : ch == '-'   ? 10
        : ch == ':'   ? 11
        : ch == 0x07  ? 13
        :               12);
    }

    void draw_12x7_digits(uint8_t * rgbpixbuf, unsigned width, unsigned lg_message,
            const char * message, const char * old_message) {
        const char * digits =
        "       "
        "       "
        "  XXX  "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        "  XXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        "  XX   "
        " X X   "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        " XXXXX "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXX   "
        "    X  "
        "    X  "
        "   X   "
        "  X    "
        " X     "
        " XXXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXX  "
        "    X  "
        "    X  "
        "  XX   "
        "    X  "
        "    X  "
        " XXX   "
        "       "
        "       "
        "       "

        "       "
        "       "
        "    X  "
        "   XX  "
        "  X X  "
        " X  X  "
        " XXXXX "
        "    X  "
        "    X  "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXX  "
        " X     "
        " X     "
        " XXX   "
        "    X  "
        "    X  "
        " XXX   "
        "       "
        "       "
        "       "

        "       "
        "       "
        "   XXX "
        "  X    "
        " X     "
        " X XX  "
        " XX  X "
        " X   X "
        "  XXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXXX "
        "    X  "
        "    X  "
        "   X   "
        "  X    "
        "  X    "
        " X     "
        "       "
        "       "
        "       "

        "       "
        "       "
        "  XXX  "
        " X   X "
        " X  X  "
        "  XXX  "
        " X   X "
        " X   X "
        "  XXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        "  XXX  "
        " X   X "
        " X   X "
        "  XXXX "
        "     X "
        "    X  "
        " XXX   "
        "       "
        "       "
        "       "

        "       "
        "       "
        "       "
        "       "
        "       "
        " XXXXX "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "

        "       "
        "       "
        "       "
        "  XX   "
        "  XX   "
        "       "
        "  XX   "
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
        "XXXXXXX"

        "       "
        "       "
        "   X   "
        "  X X  "
        "  X X  "
        " X   X "
        " XXXXX "
        " X   X "
        "X     X"
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXX  "
        " X   X "
        " X   X "
        " XXXX  "
        " X   X "
        " X   X "
        " XXXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        "  XXXX "
        " X     "
        "X      "
        "X      "
        "X      "
        " X     "
        "  XXXX "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXX  "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        " XXXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXXX "
        " X     "
        " X     "
        " XXXX  "
        " X     "
        " X     "
        " XXXXX "
        "       "
        "       "
        "       "

        "       "
        "       "
        " XXXXX "
        " X     "
        " X     "
        " XXXX  "
        " X     "
        " X     "
        " X     "
        "       "
        "       "
        "       "

        "       "   // G
        "       "
        "  XXXX "
        " X     "
        "X      "
        "X   XX "
        "X    X "
        " X   X "
        "  XXXX "
        "       "
        "       "
        "       "

        "       "   // H
        "       "
        " X   X "
        " X   X "
        " X   X "
        " XXXXX "
        " X   X "
        " X   X "
        " X   X "
        "       "
        "       "
        "       "

        "       "   // I
        "       "
        " XXXXX "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        " XXXXX "
        "       "
        "       "
        "       "

        "       "   // J
        "       "
        "  XXX  "
        "    X  "
        "    X  "
        "    X  "
        "    X  "
        "    X  "
        " XXX   "
        "       "
        "       "
        "       "

        "       "   // K
        "       "
        " X   X "
        " X  X  "
        " X X   "
        " XX    "
        " X X   "
        " X  X  "
        " X   X "
        "       "
        "       "
        "       "

        "       "   // L
        "       "
        " X     "
        " X     "
        " X     "
        " X     "
        " X     "
        " X     "
        " XXXXX "
        "       "
        "       "
        "       "

        "       "   // M
        "       "
        "XX  XX "
        "XX  XX "
        "XX X X "
        "X XX X "
        "X XX X "
        "X X  X "
        "X    X "
        "       "
        "       "
        "       "

        "       "   // N
        "       "
        " X   X "
        " XX  X "
        " XXX X "
        " X X X "
        " X  XX "
        " X  XX "
        " X   X "
        "       "
        "       "
        "       "

        "       "   // O
        "       "
        " XXXX  "
        "X    X "
        "X    X "
        "X    X "
        "X    X "
        "X    X "
        " XXXX  "
        "       "
        "       "
        "       "

        "       "   // P
        "       "
        " XXXX  "
        " X   X "
        " X   X "
        " XXXX  "
        " X     "
        " X     "
        " X     "
        "       "
        "       "
        "       "

        "       "   // Q
        "       "
        " XXXX  "
        "X    X "
        "X    X "
        "X    X "
        "X    X "
        "X    X "
        " XXXX  "
        "    XX "
        "     XX"
        "       "

        "       "   // R
        "       "
        "XXXX   "
        "X   X  "
        "X   X  "
        "XXXX   "
        "X  X   "
        "X   X  "
        "X    X "
        "       "
        "       "
        "       "

        "       "   // S
        "       "
        "  XXXX "
        " X     "
        " X     "
        "  XXX  "
        "     X "
        "     X "
        " XXXX  "
        "       "
        "       "
        "       "

        "       "   // T
        "       "
        "XXXXXXX"
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        "       "
        "       "
        "       "

        "       "
        "       "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        " X   X "
        "  XXX  "
        "       "
        "       "
        "       "

        "       "
        "       "
        "X     X"
        " X   X "
        " X   X "
        " X   X "
        "  X X  "
        "  X X  "
        "   X   "
        "       "
        "       "
        "       "

        "       "
        "       "
        "X     X"
        "X  X  X"
        "X  X  X"
        " XX X X"
        " XX XX "
        " X  XX "
        " X   X "
        "       "
        "       "
        "       "

        "       "
        "       "
        "X     X"
        " X   X "
        "  X X  "
        "   X   "
        "  X X  "
        " X   X "
        "X     X"
        "       "
        "       "
        "       "

        "       "
        "       "
        "X     X"
        " X   X "
        "  X X  "
        "   X   "
        "   X   "
        "   X   "
        "   X   "
        "       "
        "       "
        "       "

        "       "
        "       "
        "XXXXXX "
        "     X "
        "    X  "
        "   X   "
        "  X    "
        " X     "
        "XXXXXX "
        "       "
        "       "
        "       "
        ;
        for (size_t i = 0 ; i < lg_message ; ++i){
            char newch = message[i];
            char oldch = old_message[i];

            if (newch != oldch){
                const char * pnewch = digits + _posch_12x7(newch);
                const char * poldch = digits + _posch_12x7(oldch);

                unsigned br_pix = 0;
                unsigned br_pixindex = i * (char_width * 3);

                for (size_t y = 0 ; y < char_height ; ++y, br_pix += char_width, br_pixindex += width*3){
                    for (size_t x = 0 ; x <  char_width ; ++x){
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

    /*
     * The name doesn't say it : mem_blt COPIES a decoded bitmap from
     * a cache (data) and insert a subpart (srcx, srcy) to the local
     * image cache (this->data) a the given position (rect).
     */
    void mem_blt(const Rect& rect, const Bitmap & bmp, const uint16_t srcx, const uint16_t srcy, const uint32_t xormask, const bool bgr)
    {
        if (bmp.cx < srcx || bmp.cy < srcy){
            return ;
        }

        const int16_t mincx = std::min<int16_t>(bmp.cx - srcx, std::min<int16_t>(this->width - rect.x, rect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.cy - srcy, std::min<int16_t>(this->height - rect.y, rect.cy));

        if (mincx <= 0 || mincy <= 0){
            return;
        }
        const Rect & trect = Rect(rect.x, rect.y, mincx, mincy);

        const uint8_t Bpp = ::nbbytes(bmp.original_bpp);
        uint8_t * target = this->first_pixel(trect);
        const uint8_t * source = bmp.data() + (bmp.cy - srcy - 1) * (bmp.bmp_size / bmp.cy) + srcx * Bpp;
        int steptarget = (this->width - trect.cx) * 3;
        int stepsource = (bmp.bmp_size / bmp.cy) + trect.cx * Bpp;

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

    template <typename Op>
    void memblt_op( const Rect & rect
                  , const Bitmap & bmp
                  , const uint16_t srcx
                  , const uint16_t srcy
                  , const bool bgr) {
        Op op;

        if (bmp.cx < srcx || bmp.cy < srcy){
            return ;
        }

        const int16_t mincx = std::min<int16_t>(bmp.cx - srcx,
            std::min<int16_t>(this->width - rect.x, rect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.cy - srcy,
            std::min<int16_t>(this->height - rect.y, rect.cy));

        if (mincx <= 0 || mincy <= 0){
            return;
        }
        const Rect & trect = Rect(rect.x, rect.y, mincx, mincy);

        const uint8_t   Bpp = ::nbbytes(bmp.original_bpp);
        uint8_t       * target = this->first_pixel(trect);
        const uint8_t * source = bmp.data() + (bmp.cy - srcy - 1) * (bmp.bmp_size / bmp.cy) +
            srcx * Bpp;

        int steptarget = (this->width - trect.cx) * 3;
        int stepsource = (bmp.bmp_size / bmp.cy) + trect.cx * Bpp;

        uint8_t s0, s1, s2;

        for (int y = 0; y < trect.cy ; y++, target += steptarget, source -= stepsource){
            for (int x = 0; x < trect.cx ; x++, target += 3, source += Bpp){
                uint32_t px = source[Bpp-1];
                for (int b = 1 ; b < Bpp ; b++){
                    px = (px << 8) + source[Bpp-1-b];
                }
                uint32_t color = /* xormask ^ */color_decode(px, bmp.original_bpp, bmp.original_palette);
                if (bgr){
                    color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
                }

                s0 = color         & 0xFF;
                s1 = (color >> 8 ) & 0xFF;
                s2 = (color >> 16) & 0xFF;

                target[0] = op(target[0], s0);
                target[1] = op(target[1], s1);
                target[2] = op(target[2], s2);
            }
        }
    }

    void mem_blt_ex( const Rect & rect
                   , const Bitmap & bmp
                   , const uint16_t srcx
                   , const uint16_t srcy
                   , uint8_t rop
                   , const bool bgr) {
        switch (rop) {
            // +------+-------------------------------+
            // | 0x22 | ROP: 0x00220326               |
            // |      | RPN: DSna                     |
            // +------+-------------------------------+
            case 0x22:
                this->memblt_op<Op_0x22>(rect, bmp, srcx, srcy, bgr);
            break;
            // +------+-------------------------------+
            // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
            // |      | RPN: DSx                      |
            // +------+-------------------------------+
            case 0x66:
                this->memblt_op<Op_0x66>(rect, bmp, srcx, srcy, bgr);
            break;
            // +------+-------------------------------+
            // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
            // |      | RPN: DSo                      |
            // +------+-------------------------------+
            case 0xEE:
                this->memblt_op<Op_0x66>(rect, bmp, srcx, srcy, bgr);
            break;

            default:
                LOG(LOG_INFO, "Drawable::mem_blt_ex(): unimplemented rop=%X", rop);
            break;
        }
    }


    void draw_bitmap(const Rect & rect, const Bitmap & bmp, bool bgr) {
        const int16_t mincx =
            std::min<int16_t>(bmp.cx, std::min<int16_t>(this->width  - rect.x, rect.cx));
        const int16_t mincy =
            std::min<int16_t>(bmp.cy, std::min<int16_t>(this->height - rect.y, rect.cy));

        if (mincx <= 0 || mincy <= 0) {
            return;
        }
        const Rect & trect = Rect(rect.x, rect.y, mincx, mincy);

        const uint8_t   Bpp    = ::nbbytes(bmp.original_bpp);
        uint8_t       * target = this->first_pixel(trect);
        const uint8_t * source = bmp.data() + (bmp.cy - 1) * (bmp.bmp_size / bmp.cy);

        int steptarget = (this->width - trect.cx) * 3;
        int stepsource = (bmp.bmp_size / bmp.cy) + trect.cx * Bpp;

        for (int y = 0; y < trect.cy; y++, target += steptarget, source -= stepsource) {
            for (int x = 0; x < trect.cx; x++, target += 3, source += Bpp) {
                uint32_t px = source[Bpp - 1];
                for (int b = 1; b < Bpp; b++) {
                    px = (px << 8) + source[Bpp - 1 - b];
                }
                uint32_t color = color_decode(px, bmp.original_bpp, bmp.original_palette);
                if (bgr){
                    color = ((color << 16) & 0xFF0000) | (color & 0xFF00) |((color >> 16) & 0xFF);
                }
                target[0] = color      ;
                target[1] = color >> 8 ;
                target[2] = color >> 16;
            }
        }
    }

    struct Op_0xB8
    {
        uint8_t operator()(uint8_t target, uint8_t source, uint8_t pattern)
        {
            return ((target ^ pattern) & source) ^ pattern;
        }
    };

    template <typename Op>
    void mem3blt_op( const Rect & rect
                   , const Bitmap & bmp
                   , const uint16_t srcx
                   , const uint16_t srcy
                   , const uint32_t pattern_color
                   , const bool bgr) {
        Op op;

        if (bmp.cx < srcx || bmp.cy < srcy) {
            return;
        }

        const int16_t mincx = std::min<int16_t>(bmp.cx - srcx,
            std::min<int16_t>(this->width - rect.x, rect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.cy - srcy,
            std::min<int16_t>(this->height - rect.y, rect.cy));

        if (mincx <= 0 || mincy <= 0) {
            return;
        }
        const Rect & trect = Rect(rect.x, rect.y, mincx, mincy);

        const uint8_t   Bpp    = ::nbbytes(bmp.original_bpp);
        uint8_t *       target = this->first_pixel(trect);
        const uint8_t * source = bmp.data() + (bmp.cy - srcy - 1) * (bmp.bmp_size / bmp.cy) +
            srcx * Bpp;

        int steptarget = (this->width - trect.cx) * 3;
        int stepsource = (bmp.bmp_size / bmp.cy) + trect.cx * Bpp;

        uint8_t s0, s1, s2;
        uint8_t p0, p1, p2;

        for (int y = 0; y < trect.cy ; y++, target += steptarget, source -= stepsource){
            for (int x = 0; x < trect.cx ; x++, target += 3, source += Bpp){
                uint32_t px = source[Bpp-1];
                for (int b = 1 ; b < Bpp ; b++){
                    px = (px << 8) + source[Bpp-1-b];
                }
                uint32_t color = color_decode(px, bmp.original_bpp, bmp.original_palette);
                if (bgr){
                    color =   ((color << 16) & 0xFF0000)
                            | ( color        & 0xFF00)
                            | ((color >> 16) & 0xFF);
                }

                s0 = color         & 0xFF;
                s1 = (color >> 8 ) & 0xFF;
                s2 = (color >> 16) & 0xFF;

                p0 = pattern_color         & 0xFF;
                p1 = (pattern_color >> 8 ) & 0xFF;
                p2 = (pattern_color >> 16) & 0xFF;

                target[0] = op(target[0], s0, p0);
                target[1] = op(target[1], s1, p1);
                target[2] = op(target[2], s2, p2);
            }
        }
    }

    void mem_3_blt( const Rect & rect
                  , const Bitmap & bmp
                  , const uint16_t srcx
                  , const uint16_t srcy
                  , uint8_t rop
                  , const uint32_t pattern_color
                  , const bool bgr) {
        switch (rop) {
            // +------+-------------------------------+
            // | 0xB8 | ROP: 0x00B8074A               |
            // |      | RPN: PSDPxax                  |
            // +------+-------------------------------+
            case 0xB8:
                this->mem3blt_op<Op_0xB8>(rect, bmp, srcx, srcy, pattern_color, bgr);
            break;

            default:
                LOG(LOG_INFO, "Drawable::mem_3_blt(): unimplemented rop=%X", rop);
            break;
        }
    }

    void black_color(const Rect & rect)
    {
        const Rect & trect = rect.intersect(Rect(0, 0, this->width, this->height));
        uint8_t * p = this->first_pixel(trect);
        const size_t step = this->rowsize;
        const size_t rect_rowsize = trect.cx * this->Bpp;
        for (int j = 0; j < trect.cy ; j++, p += step){
            bzero(p, rect_rowsize);
        }
    }

    void white_color(const Rect & rect)
    {
        uint8_t * p = this->first_pixel(rect);
        const size_t step = this->rowsize;
        const size_t rect_rowsize = rect.cx * this->Bpp;
        for (int j = 0; j < rect.cy ; j++, p += step){
            memset(p, 0xFF, rect_rowsize);
        }
    }

    void invert_color(const Rect & rect)
    {
        const Rect & trect = rect.intersect(Rect(0, 0, this->width, this->height));
        uint8_t * p = this->first_pixel(trect);
        const size_t rect_rowsize = trect.cx * this->Bpp;
        const size_t step = this->rowsize - rect_rowsize;
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
        uint8_t * const base = this->first_pixel(rect);
        uint8_t * p = base;

        for (size_t x = 0; x < (size_t)rect.cx ; x++){
            p[0] = color; p[1] = color >> 8; p[2] = color >> 16;
            p += 3;
        }
        uint8_t * target = base;
        size_t line_size = rect.cx * this->Bpp;
        for (size_t y = 1; y < (size_t)rect.cy ; y++){
            target += this->rowsize;
            memcpy(target, base, line_size);
        }
    }

    template <typename Op>
    void patblt_op(const Rect & rect, const uint32_t color)
    {
        Op op;
        uint8_t * const base = this->first_pixel(rect);
        uint8_t * p = base;
        uint8_t p0 = color & 0xFF;
        uint8_t p1 = (color >> 8) & 0xFF;
        uint8_t p2 = (color >> 16) & 0xFF;

        for (size_t y = 0; y < (size_t)rect.cy ; y++){
            p = base + this->rowsize * y;
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

    template <typename Op>
    void patblt_op_ex(const Rect & rect, const uint8_t * brush_data,
        const uint32_t back_color, const uint32_t fore_color)
    {
        Op op;
        uint8_t * const base = this->first_pixel(rect);
        uint8_t *       p    = base;

        uint8_t p0;
        uint8_t p1;
        uint8_t p2;

        for (size_t y = 0; y < (size_t)rect.cy ; y++)
        {
            p = base + this->rowsize * y;
            for (size_t x = 0; x < (size_t)rect.cx ; x++)
            {
                if (brush_data[y % 8] & (1 << (x % 8)))
                {
                    p0 = back_color         & 0xFF;
                    p1 = (back_color >> 8)  & 0xFF;
                    p2 = (back_color >> 16) & 0xFF;
                }
                else
                {
                    p0 = fore_color         & 0xFF;
                    p1 = (fore_color >> 8)  & 0xFF;
                    p2 = (fore_color >> 16) & 0xFF;
                }

                p[0] = op(p[0], p0);
                p[1] = op(p[1], p1);
                p[2] = op(p[2], p2);
                p += 3;
            }
        }
    }

    void patblt_ex(const Rect & rect, const uint8_t rop,
        const uint32_t back_color, const uint32_t fore_color,
        const uint8_t * brush_data)
    {
        if (rop != 0xF0)
        {
            LOG(LOG_INFO, "Unsupported parameters for PatBlt Primary Drawing Order!");
            this->patblt(rect, rop, back_color);
        }

        switch (rop)
        {
        // +------+-------------------------------+
        // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
        // |      | RPN: P                        |
        // +------+-------------------------------+
        case 0xF0:
            this->patblt_op_ex<Op_0xF0>(rect, brush_data, back_color, fore_color);
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
    void scr_blt_op(uint16_t srcx, uint16_t srcy, const Rect drect)
    {
        Op op;
        const int16_t deltax = (int16_t)(srcx - drect.x);
        const int16_t deltay = (int16_t)(srcy - drect.y);
        const Rect srect = drect.offset(deltax, deltay);
        const Rect & overlap = srect.intersect(drect);
        uint8_t * target = ((deltay >= 0)||overlap.isempty())
        ? this->first_pixel(drect)
        : this->beginning_of_last_line(drect);
        uint8_t * source = ((deltay >= 0)||overlap.isempty())
        ? this->first_pixel(srect)
        : this->beginning_of_last_line(srect);
        const signed int to_nextrow = (signed int)(((deltay >= 0)||overlap.isempty())
        ?  this->rowsize
        : -this->rowsize);
        const signed to_nextpixel = ((deltay != 0)||(deltax >= 0))?this->Bpp:-this->Bpp;
        const unsigned offset = (unsigned)(((deltay != 0)||(deltax >= 0))?0:this->Bpp*(drect.cx - 1));
        for (size_t y = 0; y < drect.cy ; y++) {
            uint8_t * linetarget = target + offset;
            uint8_t * linesource = source + offset;
            for (size_t x = 0; x < drect.cx ; x++) {
                for (uint8_t b = 0 ; b < this->Bpp; b++){
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
        uint8_t col[3] =
            { static_cast<uint8_t>(color)
            , static_cast<uint8_t>(color >> 8)
            , static_cast<uint8_t>(color >> 16)
            };

        const Rect line_clip = clip.intersect(Rect(0, 0, this->width, this->height));

        // Prep
        int x = startx;
        int y = starty;
        int dx = endx - startx;
        int dy = (endy >= starty)?(endy - starty):(starty - endy);
        int sy = (endy >= starty)?1:-1;
        int err = dx - dy;

        while (true){
            if (line_clip.contains_pt(x, y)){
                uint8_t * const p = this->data + (y * this->width + x) * this->Bpp;
                for (uint8_t b = 0 ; b < this->Bpp; b++){
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

    void vertical_line(const uint8_t mix_mode, const uint16_t x, const uint16_t starty, const uint16_t endy, const uint32_t color)
    {
        // Color handling
        uint8_t col[3] =
            { static_cast<uint8_t>(color)
            , static_cast<uint8_t>(color >> 8)
            , static_cast<uint8_t>(color >> 16)};

        uint8_t * p = this->data + (starty * this->width + x) * 3;
        for (int dy = starty; dy <= endy ; dy++) {
            p[0] = col[0];
            p[1] = col[1];
            p[2] = col[2];
            p += this->width * 3;
        }
    }

    void horizontal_line(const uint8_t mix_mode, const uint16_t startx, const uint16_t y, const uint16_t endx, const uint32_t color)
    {
        uint8_t col[3] =
            { static_cast<uint8_t>(color)
            , static_cast<uint8_t>(color >> 8)
            , static_cast<uint8_t>(color >> 16)
            };

        uint8_t * p = this->data + (y * this->width + startx) * 3;
        for (int dx = startx; dx <= endx ; dx++) {
            p[0] = col[0];
            p[1] = col[1];
            p[2] = col[2];
            p += 3;
        }
    }

    const Mouse_t & line_of_mouse(size_t i) {
        return this->mouse_cursor[i];
    }

    void set_mouse_cursor(int contiguous_mouse_pixels,
            const Mouse_t * mouse_cursor,
            uint8_t hotspot_x, uint8_t hotspot_y) {
        this->contiguous_mouse_pixels = contiguous_mouse_pixels;
        this->mouse_cursor            = mouse_cursor;
        this->mouse_hotspot_x         = hotspot_x;
        this->mouse_hotspot_y         = hotspot_y;
    }

    void trace_mouse(uint16_t ux, uint16_t uy) {
        this->save_mouse_x = ux;
        this->save_mouse_y = uy;

        uint8_t * psave = this->save_mouse;
        int       x     = ux - this->mouse_hotspot_x;
        int       y     = uy - this->mouse_hotspot_y;

        const uint8_t * data_end = this->data + this->height * this->width * 3;

        for (size_t i = 0; i < this->contiguous_mouse_pixels; i++) {
            uint8_t  * pixel_start = this->pixel_start_data(x, y, i);
            unsigned   lg          = this->line_of_mouse(i).lg;
            int offset = 0;
            if (pixel_start + lg <= this->data) continue;
            if (pixel_start < this->data) {
                offset = this->data - pixel_start;
                lg -= offset;
                pixel_start = this->data;
            }
            if (pixel_start > data_end) break;
            if (pixel_start + lg >= data_end) {
                lg = data_end - pixel_start;
            }
            memcpy(psave, pixel_start, lg);
            psave += lg;
            memcpy(pixel_start, this->line_of_mouse(i).line + offset, lg);
        }
    }

    void clear_mouse() {
        uint8_t * psave = this->save_mouse;
        int       x     = this->save_mouse_x - this->mouse_hotspot_x;
        int       y     = this->save_mouse_y - this->mouse_hotspot_y;

        const uint8_t * data_end = this->data + this->height * this->width * 3;

        for (size_t i = 0; i < this->contiguous_mouse_pixels; i++) {
            uint8_t  * pixel_start = this->pixel_start_data(x, y, i);
            unsigned   lg          = this->line_of_mouse(i).lg;
            if (pixel_start + lg <= this->data) continue;
            if (pixel_start < this->data) {
                lg -= this->data - pixel_start;
                pixel_start = this->data;
            }
            if (pixel_start > data_end) break;
            if (pixel_start + lg >= data_end) {
                lg = data_end - pixel_start;
            }
            memcpy(pixel_start, psave, lg);
            psave += lg;
        }
    }

    uint8_t * pixel_start_data(int x, int y, size_t i) {
        return this->data +
               ((this->line_of_mouse(i).y + y) * this->width + this->line_of_mouse(i).x + x) * 3;
    }

    void trace_timestamp(tm & now)
    {
        char    * timezone;
        uint8_t   timestamp_length;

        timezone = (daylight ? tzname[1] : tzname[0]);

        char rawdate[size_str_timestamp];
        memset(rawdate, 0, sizeof(rawdate));
        timestamp_length = 20 + strlen(timezone);
        snprintf(rawdate, timestamp_length + 1, "%4d-%02d-%02d %02d:%02d:%02d %s",
                 now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                 now.tm_hour, now.tm_min, now.tm_sec, timezone);

        this->draw_12x7_digits(this->timestamp_data, ts_width, size_str_timestamp-1, rawdate,
            this->previous_timestamp);
        memcpy(this->previous_timestamp, rawdate, size_str_timestamp);
        this->previous_timestamp_length = timestamp_length;

        uint8_t * tsave = this->timestamp_save;
        uint8_t* buf = this->data;
        int step = this->width * 3;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(tsave, buf, timestamp_length*char_width*3);
            tsave += timestamp_length*char_width*3;
            memcpy(buf, this->timestamp_data + y*ts_width*3, timestamp_length*char_width*3);
        }
    }

    void clear_timestamp()
    {
        const uint8_t * tsave = this->timestamp_save;
        int step = this->width * 3;
        uint8_t* buf = this->data;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(buf, tsave, this->previous_timestamp_length*char_width*3);
            tsave += this->previous_timestamp_length*char_width*3;
        }
    }

    TODO("Instead of copying the trace timestamp function (un clear timestamp) for pause, "
         "we could just parametrize the position of the timestamp on the screen");
    void trace_pausetimestamp(tm & now)
    {
        char    * timezone;
        uint8_t   timestamp_length;

        timezone = (daylight ? tzname[1] : tzname[0]);

        char rawdate[size_str_timestamp];
        memset(rawdate, 0, sizeof(rawdate));
        timestamp_length = 20 + strlen(timezone);
        snprintf(rawdate, timestamp_length + 1, "%4d-%02d-%02d %02d:%02d:%02d %s",
                 now.tm_year+1900, now.tm_mon+1, now.tm_mday,
                 now.tm_hour, now.tm_min, now.tm_sec, timezone);

        this->draw_12x7_digits(this->timestamp_data, ts_width, size_str_timestamp-1, rawdate,
            this->previous_timestamp);
        memcpy(this->previous_timestamp, rawdate, size_str_timestamp);
        this->previous_timestamp_length = timestamp_length;

        uint8_t * tsave = this->timestamp_save;
        uint8_t* buf = this->data
            + (this->width * 3) * (this->height / 2)
            + ((this->width - timestamp_length*char_width)*3) / 2 ;
        int step = this->width * 3;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(tsave, buf, timestamp_length*char_width*3);
            tsave += timestamp_length*char_width*3;
            memcpy(buf, this->timestamp_data + y*ts_width*3, timestamp_length*char_width*3);
        }
    }

    void clear_pausetimestamp()
    {
        const uint8_t * tsave = this->timestamp_save;
        int step = this->width * 3;
        uint8_t* buf = this->data
            + (this->width * 3) * (this->height / 2)
            + ((this->width - this->previous_timestamp_length*char_width)*3) / 2 ;
        for (size_t y = 0; y < ts_height ; ++y, buf += step){
            memcpy(buf, tsave, this->previous_timestamp_length*char_width*3);
            tsave += this->previous_timestamp_length*char_width*3;
        }
    }
};

#endif
