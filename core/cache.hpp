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
    TODO(" use RDPBrush")
    uint8_t pattern[8];
    brush_item() {
        this->stamp = 0;
        memset(this->pattern, 0, 8);
    }
};

enum {
    POINTER_NULL,
    POINTER_CURSOR0,
    POINTER_CURSOR1
};

struct pointer_item {
    int stamp;
    int x; /* hotspot */
    int y;
    uint8_t data[32 * 32 * 3];
    uint8_t mask[32 * 32 / 8];
    pointer_item(uint8_t pointer_type = POINTER_NULL)
    {
        this->stamp = 0;
        switch (pointer_type){
            default:
            case POINTER_NULL:
            {
                this->x = 0; /* hotspot */
                this->y = 0;
                memset(this->data, 0, 32 * 32 * 3);
                memset(this->mask, 0, 32 * 32 / 8);
            }
            break;
            case POINTER_CURSOR0:
            {
                this->x = 0; /* hotspot */
                this->y = 0;
                const char * data_cursor0 =
                /* 0000 */ "................................"
                /* 0060 */ "................................"
                /* 00c0 */ "................................"
                /* 0120 */ "................................"
                /* 0180 */ "................................"
                /* 01e0 */ "................................"
                /* 0240 */ "................................"
                /* 02a0 */ "................................"
                /* 0300 */ "................................"
                /* 0360 */ "................................"
                /* 03c0 */ "................................"
                /* 0420 */ "................................"
                /* 0480 */ "................................"
                /* 04e0 */ ".......XX......................."
                /* 0540 */ "......X..X......................"
                /* 05a0 */ "......X..X......................"
                /* 0600 */ ".....X..X......................."
                /* 0660 */ "X....X..X......................."
                /* 06c0 */ "XX..X..X........................"
                /* 0720 */ "X.X.X..X........................"
                /* 0780 */ "X..X..X........................."
                /* 07e0 */ "X.....XXXXX....................."
                /* 0840 */ "X........X......................"
                /* 08a0 */ "X.......X......................."
                /* 0900 */ "X......X........................"
                /* 0960 */ "X.....X........................."
                /* 09c0 */ "X....X.........................."
                /* 0a20 */ "X...X..........................."
                /* 0a80 */ "X..X............................"
                /* 0ae0 */ "X.X............................."
                /* 0b40 */ "XX.............................."
                /* 0ba0 */ "X..............................."
                ;
                uint8_t * tmp = this->data;
                for (size_t i = 0 ; i < 32*32 ; i++){
                    uint8_t v = (data_cursor0[i] == 'X')?0xFF:0;
                    tmp[0] = tmp[1] = tmp[2] = v;
                    tmp += 3;
                }

                const char * mask_cursor0 =
                /* 0000 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0008 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0010 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0018 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0020 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0028 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0030 */ "\xff\xff\xff\xff"
                           "\xfe\x7f\xff\xff"
                /* 0038 */ "\xfc\x3f\xff\xff"
                           "\xfc\x3f\xff\xff"
                /* 0040 */ "\xf8\x7f\xff\xff"
                           "\x78\x7f\xff\xff"
                /* 0048 */ "\x30\xff\xff\xff"
                           "\x10\xff\xff\xff"
                /* 0050 */ "\x01\xff\xff\xff"
                           "\x00\x1f\xff\xff"
                /* 0058 */ "\x00\x3f\xff\xff"
                           "\x00\x7f\xff\xff"
                /* 0060 */ "\x00\xff\xff\xff"
                           "\x01\xff\xff\xff"
                /* 0068 */ "\x03\xff\xff\xff"
                           "\x07\xff\xff\xff"
                /* 0070 */ "\x0f\xff\xff\xff"
                           "\x1f\xff\xff\xff"
                /* 0078 */ "\x3f\xff\xff\xff"
                           "\x7f\xff\xff\xff"
                ;
                memcpy(this->mask, mask_cursor0, 32 * 32 / 8);
            }
            break;
            case POINTER_CURSOR1:
            {
                this->x = 15; /* hotspot */
                this->y = 16;
                const char * data_cursor1 =
                /* 0000 */ "................................"
                /* 0060 */ "................................"
                /* 00c0 */ "................................"
                /* 0120 */ "................................"
                /* 0180 */ "................................"
                /* 01e0 */ "................................"
                /* 0240 */ "................................"
                /* 02a0 */ "...........XXXX.XXXX............"
                /* 0300 */ "...........X...X...X............"
                /* 0360 */ "...........XXXX.XXXX............"
                /* 03c0 */ "..............X.X..............."
                /* 0420 */ "..............X.X..............."
                /* 0480 */ "..............X.X..............."
                /* 04e0 */ "..............X.X..............."
                /* 0540 */ "..............X.X..............."
                /* 05a0 */ "..............X.X..............."
                /* 0600 */ "..............X.X..............."
                /* 0660 */ "..............X.X..............."
                /* 06c0 */ "..............X.X..............."
                /* 0720 */ "..............X.X..............."
                /* 0780 */ "..............X.X..............."
                /* 07e0 */ "..............X.X..............."
                /* 0840 */ "...........XXXX.XXXX............"
                /* 08a0 */ "...........X...X...X............"
                /* 0900 */ "...........XXXX.XXXX............"
                /* 0960 */ "................................"
                /* 09c0 */ "................................"
                /* 0a20 */ "................................"
                /* 0a80 */ "................................"
                /* 0ae0 */ "................................"
                /* 0b40 */ "................................"
                /* 0ba0 */ "................................"
                ;
                uint8_t * tmp = this->data;
                for (size_t i = 0 ; i < 32*32 ; i++){
                    uint8_t v = (data_cursor1[i] == 'X')?0xFF:0;
                    tmp[0] = tmp[1] = tmp[2] = v;
                    tmp += 3;
                }
                const char * mask_cursor1 =
                /* 0000 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0008 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0010 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0018 */ "\xff\xff\xff\xff"
                           "\xff\xe1\x0f\xff"
                /* 0020 */ "\xff\xe0\x0f\xff"
                           "\xff\xe0\x0f\xff"
                /* 0028 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0030 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0038 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0040 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0048 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0050 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0058 */ "\xff\xe0\x0f\xff"
                           "\xff\xe0\x0f\xff"
                /* 0060 */ "\xff\xe1\x0f\xff"
                           "\xff\xff\xff\xff"
                /* 0068 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0070 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0078 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                ;
                memcpy(this->mask, mask_cursor1, 32 * 32 / 8);
            }
            break;
        }
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

    TODO(" much duplicated code with constructor and destructor  create some intermediate functions or object")
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

        TODO(" why not just clear what should be (inner structs")
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
        TODO(" define a copy constructor")

        FontChar * fi = new FontChar(font_item->offset, font_item->baseline, font_item->width, font_item->height, font_item->incby);
        memcpy(fi->data, font_item->data, font_item->datasize());
        this->char_items[f][c].font_item = fi;
        this->char_items[f][c].stamp = this->char_stamp;
        cacheidx = c;
        cacheid = f;
        return GLYPH_ADDED_TO_CACHE;
    }

    int add_pointer(uint8_t* data, uint8_t* mask, int x, int y, int & cache_idx){
    TODO(" see code below to avoid useless copy")
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
