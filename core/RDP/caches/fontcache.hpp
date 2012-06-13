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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

*/

#if !defined(__RDP_CACHES_FONTCACHE_HPP__)
#define __RDP_CACHES_FONTCACHE_HPP__

#include "font.hpp"
struct char_item {
    int stamp;
    struct FontChar * font_item;
    char_item() : font_item(0) {
        this->stamp = 0;
    }
};

/* difference caches */
struct GlyphCache {

    /* font */
    int char_stamp;
    struct char_item char_items[12][256];

    GlyphCache() {
        this->char_stamp = 0;
    }

    ~GlyphCache()
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
        memset(this, 0, sizeof(struct GlyphCache));
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
};


#endif
