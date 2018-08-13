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


#pragma once

#include "core/font.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"

#include <array>


/* difference caches */
class GlyphCache : noncopyable {
    class Glyph {
        friend class GlyphCache;

        int stamp = 0;

        bool cached = false;

    public:
        FontChar font_item;
    };

    /* font */
    int glyph_stamp = 0;

public:
    using number_of_entries_t = std::array<uint8_t, NUMBER_OF_GLYPH_CACHES>;

private:
    number_of_entries_t number_of_entries_in_cache = { {
          NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES, NUMBER_OF_GLYPH_CACHE_ENTRIES
        , NUMBER_OF_GLYPH_CACHE_ENTRIES
    } };

public:
    Glyph glyphs[NUMBER_OF_GLYPH_CACHES][NUMBER_OF_GLYPH_CACHE_ENTRIES];

    GlyphCache() = default;

    explicit GlyphCache(number_of_entries_t const & number_of_entries_in_glyph_cache)
    : number_of_entries_in_cache(number_of_entries_in_glyph_cache)
    {}

    void reset() {
        this->glyph_stamp = 0;

        for (auto & array : this->glyphs) {
            for (auto & glyph : array) {
                glyph = Glyph();
            }
        }
    }

    enum t_glyph_cache_result {
          GLYPH_FOUND_IN_CACHE
        , GLYPH_ADDED_TO_CACHE
    };

    t_glyph_cache_result add_glyph(FontChar const & font_item, int cacheid, int & cacheidx) {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->glyphs[cacheid][cacheidx].font_item = font_item.clone();
        }
        return ret;
    }

    t_glyph_cache_result add_glyph(FontChar && font_item, int cacheid, int & cacheidx) {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->glyphs[cacheid][cacheidx].font_item = std::move(font_item);
        }
        return ret;
    }

private:
    t_glyph_cache_result priv_add_glyph(FontChar const & font_item, int cacheid, int & cacheidx) {
        this->glyph_stamp++;

        /* look for match */
        int ci     = 0;
        int oldest = 0x7fffffff;
        for (uint8_t cacheIndex = 0; cacheIndex < this->number_of_entries_in_cache[cacheid]; ++ cacheIndex) {
            Glyph & item = this->glyphs[cacheid][cacheIndex];
            if (item.font_item && item.font_item.item_compare(font_item)) {
                item.stamp = this->glyph_stamp;
                cacheidx   = &item - std::begin(this->glyphs[cacheid]);

                return GLYPH_FOUND_IN_CACHE;
            }

            /* look for oldest */
            if (item.stamp < oldest) {
                oldest = item.stamp;
                ci     = cacheIndex;
            }
        }

        cacheidx = ci;
        this->glyphs[cacheid][ci].stamp = this->glyph_stamp;
        this->glyphs[cacheid][ci].cached = true;

        return GLYPH_ADDED_TO_CACHE;
    }

public:
    void set_glyph(FontChar && fc, size_t cacheid, size_t cacheidx) {
        this->glyph_stamp++;
        this->glyphs[cacheid][cacheidx].font_item = std::move(fc);
        this->glyphs[cacheid][cacheidx].stamp     = this->glyph_stamp;
    }

/*
    int find_glyph(FontChar & font_item, int cacheid) {
        // look for match
        for (uint8_t cacheIndex = 0; cacheIndex < this->number_of_entries_in_cache[cacheid]; ++cacheIndex) {
            Glyph & item = this->glyphs[cacheid][cacheIndex];
            if (item.font_item && item.font_item.item_compare(font_item)) {
                return cacheIndex;
            }
        }

        return -1;
    }
*/

    bool is_cached(uint8_t cacheId, uint8_t cacheIndex) const {
        return this->glyphs[cacheId][cacheIndex].cached;
    }

    void set_cached(uint8_t cacheId, uint8_t cacheIndex, bool cached) {
        this->glyphs[cacheId][cacheIndex].cached = cached;
    }
};  // class GlyphCache

