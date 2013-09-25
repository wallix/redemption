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
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_

#include <math.h>

#include "RDP/pointer.hpp"
#include "drawable.hpp"
#include "client_info.hpp"


enum {
    POINTER_TO_SEND         = 0,
    POINTER_ALLREADY_SENT
};

/* difference caches */
struct PointerCache {
    int pointer_cache_entries;

    /* pointer */
    int pointer_stamp;
    struct Pointer Pointers[32];
    int stamps[32];

    PointerCache() {
        this->pointer_cache_entries = 0;
        this->pointer_stamp = 0;
    }

    ~PointerCache() {}

    TODO(" much duplicated code with constructor and destructor  create some intermediate functions or object")
    int reset(struct ClientInfo & client_info) {
        memset(this, 0, sizeof(struct PointerCache));
        this->pointer_cache_entries = client_info.pointer_cache_entries;
        return 0;
    }

    void add_pointer_static(const Pointer & cursor, int index) {
        this->Pointers[index].x = cursor.x;
        this->Pointers[index].y = cursor.y;
        memcpy(this->Pointers[index].data, cursor.data, cursor.data_size());
        memcpy(this->Pointers[index].mask, cursor.mask, cursor.mask_size());
        this->stamps[index] = this->pointer_stamp;
    }

    /* check if the pointer is in the cache or not and if it should be sent      */
    int add_pointer(const Pointer & cursor, int & cache_idx)
    {
        int i;
        int oldest = 0x7fffffff;
        int index = 2;

        this->pointer_stamp++;
        /* look for match */
        for (i = 2; i < this->pointer_cache_entries; i++) {
            if (this->Pointers[i].x == cursor.x 
            &&  this->Pointers[i].y == cursor.y 
            &&  this->Pointers[i].width == cursor.width 
            &&  this->Pointers[i].height == cursor.height 
            &&  this->Pointers[i].bpp == cursor.bpp 
            &&  (memcmp(this->Pointers[i].data, cursor.data, cursor.data_size()) == 0) 
            &&  (memcmp(this->Pointers[i].mask, cursor.mask, cursor.mask_size()) == 0)) {
                this->stamps[i] = this->pointer_stamp;
                cache_idx = i;
                return POINTER_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        for (i = 2; i < this->pointer_cache_entries; i++) {
            if (this->stamps[i] < oldest) {
                oldest = this->stamps[i];
                index  = i;
            }
        }
        
        this->stamps[index] = this->pointer_stamp;
        cache_idx = index;
        this->add_pointer_static(cursor, index);
        return POINTER_TO_SEND;
    }

};  // struct PointerCache

struct drawable_Pointer {
    Drawable::Mouse_t mouse_cursor[16 * 32];
    int               contiguous_mouse_pixels;
    uint8_t           data[32 * 32 * 3];

    int x; /* hotspot */
    int y;
};

struct DrawablePointerCache {
    struct drawable_Pointer Pointers[32];

    DrawablePointerCache() {
        for (int i = 0,
                 c = sizeof(this->Pointers) / sizeof(drawable_Pointer);
             i < c; i++) {
            memset(&this->Pointers[i], 0, sizeof(this->Pointers[i]));
        }
    }

    void add_pointer_static(const Pointer & cursor, int index) {
        drawable_Pointer & dcursor = this->Pointers[index];

        this->make_drawable_mouse_cursor(cursor.data, cursor.mask, dcursor);

        dcursor.x = cursor.x;
        dcursor.y = cursor.y;

    }

protected:
    void make_drawable_mouse_cursor(const uint8_t * data,
        const uint8_t * mask, drawable_Pointer & Pointer) {
        memset(&Pointer, 0, sizeof(Pointer));

        bool                draw_pixel;
        int                 current_contiguous_mouse_pixels = 0;
        Drawable::Mouse_t * mouse_cursor                    = &Pointer.mouse_cursor[-1];
        bool                in_contiguous_mouse_pixels      = false;
        uint8_t           * line_data                       = Pointer.data;

        for (unsigned line = 0; line < 32; line++) {
            in_contiguous_mouse_pixels = false;

            for (unsigned column = 0; column < 32; column++) {
                div_t res = div(column, 8);
                unsigned rem = 7 - res.rem;

                draw_pixel = !(((*(mask + 128 - (line + 1) * 32 / 8 + res.quot)) & (1 << rem)) >> rem);
//                printf("%c", (draw_pixel ? 'X' : '.'));

                const uint8_t * pixel = data + 32 * 32 * 3 - (line + 1) * 32 * 3 + column * 3;
//                printf("%02X%02X%02X", *pixel, *(pixel + 1), *(pixel+2));

                if (draw_pixel && !in_contiguous_mouse_pixels) {
                    current_contiguous_mouse_pixels++;
                    mouse_cursor++;

                    mouse_cursor->x    = column;
                    mouse_cursor->y    = line;
                    mouse_cursor->lg   = 0;
                    mouse_cursor->line = reinterpret_cast<const char *>(line_data);

                    in_contiguous_mouse_pixels = true;
                }
                else if (!draw_pixel && in_contiguous_mouse_pixels) {
                    in_contiguous_mouse_pixels = false;
                }

                if (in_contiguous_mouse_pixels) {
                    memcpy(line_data, pixel, 3);
                    line_data        += 3;
                    mouse_cursor->lg += 3;
                }
            }
//            printf("\n");
        }

        Pointer.contiguous_mouse_pixels = current_contiguous_mouse_pixels;
    }
};

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_
