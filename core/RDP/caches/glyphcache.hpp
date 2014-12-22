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
   Copyright (C) Wallix 2012-2013.
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_CACHES_GLYPHCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_GLYPHCACHE_HPP_

#include "font.hpp"
#include "noncopyable.hpp"
#include "RDP/capabilities/glyphcache.hpp"

/* difference caches */
class GlyphCache : noncopyable {
    class char_item {
        friend struct GlyphCache;

        int stamp = 0;

    public:
        FontChar font_item;

        char_item() = default;
    };

    /* font */
    int char_stamp = 0;

public:
    char_item char_items[NUMBER_OF_GLYPH_CACHES][NUMBER_OF_GLYPH_CACHE_ENTRIES];

private:
    std::array<uint8_t, NUMBER_OF_GLYPH_CACHES> number_of_entries_in_cache = { {
          NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES
    } };

public:
    GlyphCache() {
        reset_internal();
    }

    int reset(std::array<uint8_t, NUMBER_OF_GLYPH_CACHES> & number_of_entries_in_glyph_cache) {
        /* free all the cached font items */
        reset_internal();

        this->number_of_entries_in_cache = number_of_entries_in_glyph_cache;

        return 0;
    }

private:
    void reset_internal()
    {
        /* free all the cached font items */
        for (auto & items : this->char_items) {
            for (char_item & item : items) {
                if (item.font_item) {
                    item.font_item = FontChar();
                }
            }
        }
        this->char_stamp = 0;
    }

public:
    enum t_glyph_cache_result {
          GLYPH_FOUND_IN_CACHE
        , GLYPH_ADDED_TO_CACHE
    };

    t_glyph_cache_result add_glyph(FontChar & font_item, int cacheid, int & cacheidx) {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->char_items[cacheid][cacheidx].font_item = FontChar(font_item);
        }
        return ret;
    }

    t_glyph_cache_result add_glyph(FontChar && font_item, int cacheid, int & cacheidx) {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->char_items[cacheid][cacheidx].font_item = std::move(font_item);
        }
        return ret;
    }

private:
    t_glyph_cache_result priv_add_glyph(FontChar & font_item, int cacheid, int & cacheidx) {
        this->char_stamp++;

        /* look for match */
        int ci     = 0;
        int oldest = 0x7fffffff;
        for (uint8_t cacheIndex = 0; cacheIndex < this->number_of_entries_in_cache[cacheid]; ++ cacheIndex) {
            char_item & item = this->char_items[cacheid][cacheIndex];
            if (item.font_item && item.font_item.item_compare(font_item))
            {
                item.stamp = this->char_stamp;
                cacheidx   = &item - std::begin(this->char_items[cacheid]);

                return GLYPH_FOUND_IN_CACHE;
            }

            /* look for oldest */
            if (item.stamp < oldest)
            {
                oldest = item.stamp;
                ci     = cacheIndex;
            }
        }

        this->char_items[cacheid][ci].stamp = this->char_stamp;

        cacheidx = ci;

        return GLYPH_ADDED_TO_CACHE;
    }

public:
    void set_glyph(FontChar && fc, size_t cacheid, size_t cacheidx)
    {
        this->char_stamp++;
        this->char_items[cacheid][cacheidx].font_item = std::move(fc);
        this->char_items[cacheid][cacheidx].stamp     = this->char_stamp;
    }

    int find_glyph(FontChar & font_item, int cacheid)
    {
        /* look for match */
        for (uint8_t cacheIndex = 0; cacheIndex < this->number_of_entries_in_cache[cacheid]; ++cacheIndex) {
            char_item & item = this->char_items[cacheid][cacheIndex];
            if (item.font_item && item.font_item.item_compare(font_item)) {
                return cacheIndex;
            }
        }

        return -1;
    }
};  // class GlyphCache

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_GLYPHCACHE_HPP_
