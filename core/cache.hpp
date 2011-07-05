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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   cache header file

*/

#if !defined(__CACHE_HPP__)
#define __CACHE_HPP__

#include "font.hpp"
struct char_item {
    int stamp;
    struct FontChar * font_item;
    char_item() : font_item(0) {
        this->stamp = 0;
    }
};

struct brush_item {
    int stamp;
    /* expand this to a structure to handle more complicated brushes
       for now its 8x8 1bpp brushes only */
    #warning use RDPBrush
    uint8_t pattern[8];
    brush_item() {
        this->stamp = 0;
        memset(this->pattern, 0, 8);
    }
};

struct pointer_item {
    int stamp;
    int x; /* hotspot */
    int y;
    uint8_t data[32 * 32 * 3];
    uint8_t mask[32 * 32 / 8];
    pointer_item() {
        this->stamp = 0;
        this->x = 0; /* hotspot */
        this->y = 0;
        memset(this->data, 0, 32 * 32 * 3);
        memset(this->mask, 0, 32 * 32 / 8);
    }
    ~pointer_item() {
    }
};

/* difference caches */
struct Cache {
    int pointer_cache_entries;

    /* font */
    int char_stamp;
    struct char_item char_items[12][256];

    /* pointer */
    int pointer_stamp;
    struct pointer_item pointer_items[32];

    /* brush */
    int brush_stamp;
    struct brush_item brush_items[64];

    Cache() {
        this->pointer_cache_entries = 0;

        /* font */
        this->char_stamp = 0;
        /* pointer */
        this->pointer_stamp = 0;
        /* brush */
        this->brush_stamp = 0;
    }

    ~Cache()
    {
        /* free all the cached font items */
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 256; j++) {
                if (this->char_items[i][j].font_item){
                    delete this->char_items[i][j].font_item;
                }
            }
        }
    }

    int load_static_pointers();

    #warning much duplicated code with constructor and destructor, create some intermediate functions or object
    int reset(struct ClientInfo & client_info)
    {
        /* free all the cached font items */
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 256; j++) {
                if (this->char_items[i][j].font_item){
                    delete this->char_items[i][j].font_item;
                }
            }
        }

        #warning why not just clear what should be (inner structs
        /* set whole struct to zero */
        memset(this, 0, sizeof(struct Cache));
        /* set some stuff back */
        this->pointer_cache_entries = client_info.pointer_cache_entries;
        return 0;
    }

    enum  t_glyph_cache_result {
        GLYPH_FOUND_IN_CACHE,
        GLYPH_ADDED_TO_CACHE
    };

    /*****************************************************************************/
    t_glyph_cache_result add_glyph(FontChar* font_item, int & cacheid, int & cacheidx)
    {
        this->char_stamp++;
        /* look for match */
        for (size_t i = 7; i < 12; i++) {
            for (size_t j = 0; j < 250; j++) {
                if (this->char_items[i][j].font_item){
                    if (this->char_items[i][j].font_item->item_compare(font_item)) {
                        this->char_items[i][j].stamp = this->char_stamp;
                        cacheidx = j;
                        cacheid = i;
                        return GLYPH_FOUND_IN_CACHE;
                    }
                }
            }
        }
        /* look for oldest */
        int f = 0;
        int c = 0;
        int oldest = 0x7fffffff;
        for (size_t i = 7; i < 12; i++) {
            for (size_t j = 0; j < 250; j++) {
                if (this->char_items[i][j].stamp < oldest) {
                    oldest = this->char_items[i][j].stamp;
                    f = i;
                    c = j;
                }
            }
        }
        /* set, send char and return */
        #warning define a copy constructor
        FontChar * fi = new FontChar(font_item->offset, font_item->baseline, font_item->width, font_item->height, font_item->incby);
        memcpy(fi->data, font_item->data, font_item->datasize());
        this->char_items[f][c].font_item = fi;
        this->char_items[f][c].stamp = this->char_stamp;
        cacheidx = c;
        cacheid = f;
        return GLYPH_ADDED_TO_CACHE;
    }

    int add_pointer(uint8_t* data, uint8_t* mask, int x, int y, int & cache_idx){
    #warning see code below to avoid useless copy
        struct pointer_item pointer_item;

        pointer_item.x = x;
        pointer_item.y = y;
        memcpy(pointer_item.data, data, 32 * 32 * 3);
        memcpy(pointer_item.mask, mask, 32 * 32 / 8);
        return this->add_pointer(&pointer_item, cache_idx);
    }


    /* check if the pointer is in the cache or not and if it should be sent      */
    int add_pointer(struct pointer_item* pointer_item, int & cache_idx)
    {
        int i;
        int oldest;
        int index;

        this->pointer_stamp++;
        /* look for match */
        for (i = 2; i < this->pointer_cache_entries; i++) {
            if (this->pointer_items[i].x == pointer_item->x &&
                    this->pointer_items[i].y == pointer_item->y &&
                    memcmp(this->pointer_items[i].data,
                             pointer_item->data, 32 * 32 * 3) == 0 &&
                    memcmp(this->pointer_items[i].mask,
                             pointer_item->mask, 32 * 32 / 8) == 0) {
                this->pointer_items[i].stamp = this->pointer_stamp;
                cache_idx = i;
                return POINTER_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        index = 2;
        oldest = 0x7fffffff;
        for (i = 2; i < this->pointer_cache_entries; i++) {
            if (this->pointer_items[i].stamp < oldest) {
                oldest = this->pointer_items[i].stamp;
                index = i;
            }
        }
        this->pointer_items[index].x = pointer_item->x;
        this->pointer_items[index].y = pointer_item->y;
        memcpy(this->pointer_items[index].data, pointer_item->data, 32 * 32 * 3);
        memcpy(this->pointer_items[index].mask, pointer_item->mask, 32 * 32 / 8);
        this->pointer_items[index].stamp = this->pointer_stamp;
        cache_idx = index;
        return POINTER_TO_SEND;
    }

    void add_pointer_static(struct pointer_item* pointer_item, int index)
    {
        this->pointer_items[index].x = pointer_item->x;
        this->pointer_items[index].y = pointer_item->y;
        memcpy(this->pointer_items[index].data, pointer_item->data, 32 * 32 * 3);
        memcpy(this->pointer_items[index].mask, pointer_item->mask, 32 * 32 / 8);
        this->pointer_items[index].stamp = this->pointer_stamp;
    }

    /*****************************************************************************/
    /* this does not take owership of brush_item_data, it makes a copy */
    int add_brush(uint8_t* brush_item_data, int & cache_idx)
    {
        int i;
        int oldest;
        int index;

        if (this == 0) {
            return 0;
        }
        this->brush_stamp++;
        /* look for match */
        for (i = 0; i < 64; i++) {
            if (memcmp(this->brush_items[i].pattern, brush_item_data, 8) == 0) {
                this->brush_items[i].stamp = this->brush_stamp;
                cache_idx = i;
                return BRUSH_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        index = 0;
        oldest = 0x7fffffff;
        for (i = 0; i < 64; i++) {
            if (this->brush_items[i].stamp < oldest) {
                oldest = this->brush_items[i].stamp;
                index = i;
            }
        }
        memcpy(this->brush_items[index].pattern, brush_item_data, 8);
        this->brush_items[index].stamp = this->brush_stamp;
        cache_idx = index;
        return BRUSH_TO_SEND;
    }

};


#endif
