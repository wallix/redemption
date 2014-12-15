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

#ifndef _REDEMPTION_CORE_RDP_CACHES_FONTCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_FONTCACHE_HPP_

#include "font.hpp"
#include "noncopyable.hpp"

class ClientInfo;

/* difference caches */
class GlyphCache : noncopyable
{
    class char_item
    {
        friend struct GlyphCache;

        int stamp = 0;

    public:
        FontChar font_item;

        char_item() = default;
    };

    /* font */
    int char_stamp = 0;

public:
    char_item char_items[12][256];

    GlyphCache() = default;

    int reset(ClientInfo const & client_info)
    {
        /* free all the cached font items */
        reset_internal();

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
    enum t_glyph_cache_result
    {
        GLYPH_FOUND_IN_CACHE,
        GLYPH_ADDED_TO_CACHE
    };

    t_glyph_cache_result add_glyph(FontChar & font_item, int cacheid, int & cacheidx)
    {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->char_items[cacheid][cacheidx].font_item = FontChar(font_item);
        }
        return ret;
    }

    t_glyph_cache_result add_glyph(FontChar && font_item, int cacheid, int & cacheidx)
    {
        const t_glyph_cache_result ret = priv_add_glyph(font_item, cacheid, cacheidx);
        if (ret == GLYPH_ADDED_TO_CACHE) {
            this->char_items[cacheid][cacheidx].font_item = std::move(font_item);
        }
        return ret;
    }

private:
    t_glyph_cache_result priv_add_glyph(FontChar & font_item, int cacheid, int & cacheidx)
    {
        this->char_stamp++;

        /* look for match */
        for (char_item & item : this->char_items[cacheid])
        {
            if (item.font_item && item.font_item.item_compare(font_item))
            {
                item.stamp = this->char_stamp;
                cacheidx   = &item - std::begin(this->char_items[cacheid]);

                return GLYPH_FOUND_IN_CACHE;
            }
        }

        /* look for oldest */
        int c      = 0;
        int oldest = 0x7fffffff;
        for (char_item & item : this->char_items[cacheid])
        {
            if (item.stamp < oldest)
            {
                oldest = item.stamp;
                c      = &item - std::begin(this->char_items[cacheid]);
            }
        }

        this->char_items[cacheid][c].stamp = this->char_stamp;

        cacheidx = c;

        return GLYPH_ADDED_TO_CACHE;
    }

public:
    void set_glyph(FontChar && fc, size_t cacheid, size_t cacheidx)
    {
        this->char_stamp++;
        this->char_items[cacheid][cacheidx].font_item = std::move(fc);
        this->char_items[cacheid][cacheidx].stamp = this->char_stamp;
    }

    int find_glyph(FontChar & font_item, int cacheid)
    {
        /* look for match */
        for (char_item & item : this->char_items[cacheid])
        {
            if (item.font_item && item.font_item.item_compare(font_item))
            {
                return &item - std::begin(this->char_items[cacheid]);
            }
        }

        return -1;
    }
};

#endif
