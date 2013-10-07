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

struct char_item
{
    int               stamp;
    struct FontChar * font_item;

    char_item() : font_item(0)
    {
        this->stamp = 0;
    }
};

/* difference caches */
struct GlyphCache
{
    /* font */
    int              char_stamp;
    struct char_item char_items[12][256];

    GlyphCache()
    {
        this->char_stamp = 0;
    }

    ~GlyphCache()
    {
        /* free all the cached font items */
        reset_internal();
    }

    int reset(struct ClientInfo & client_info)
    {
        /* free all the cached font items */
        reset_internal();

        return 0;
    }

protected:
    void reset_internal()
    {
        /* free all the cached font items */
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 256; j++)
            {
                if (this->char_items[i][j].font_item)
                {
                    delete this->char_items[i][j].font_item;

                    this->char_items[i][j].font_item = NULL;
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

    /*****************************************************************************/
    t_glyph_cache_result add_glyph(FontChar * font_item, int cacheid,
        int & cacheidx)
    {
        this->char_stamp++;

        /* look for match */
        for (size_t j = 0; j < 250; j++)
        {
            if (this->char_items[cacheid][j].font_item)
            {
                if (this->char_items[cacheid][j].font_item->item_compare(font_item))
                {
                    this->char_items[cacheid][j].stamp = this->char_stamp;
                    cacheidx                           = j;

                    return GLYPH_FOUND_IN_CACHE;
                }
            }
        }

        /* look for oldest */
        int c      = 0;
        int oldest = 0x7fffffff;
        for (size_t j = 0; j < 250; j++)
        {
            if (this->char_items[cacheid][j].stamp < oldest)
            {
                oldest = this->char_items[cacheid][j].stamp;
                c      = j;
            }
        }
        /* set, send char and return */
        if (this->char_items[cacheid][c].font_item)
        {
            delete this->char_items[cacheid][c].font_item;
        }

        this->char_items[cacheid][c].font_item = new FontChar(*font_item);
        this->char_items[cacheid][c].stamp     = this->char_stamp;

        cacheidx = c;

        return GLYPH_ADDED_TO_CACHE;
    }

    void set_glyph(const RDPGlyphCache & cmd)
    {
        int cacheidx = cmd.cacheId;
        int c        = cmd.glyphData_cacheIndex;

        this->char_stamp++;

        if (this->char_items[cacheidx][c].font_item)
        {
            delete this->char_items[cacheidx][c].font_item;
        }

        FontChar * fi = new FontChar(cmd.glyphData_x, cmd.glyphData_y,
            cmd.glyphData_cx, cmd.glyphData_cy, -1);
        memcpy(fi->data, cmd.glyphData_aj, fi->datasize());

        this->char_items[cacheidx][c].font_item = fi;
        this->char_items[cacheidx][c].stamp     = this->char_stamp;
    }

    int find_glyph(FontChar * font_item, int cacheid)
    {
        /* look for match */
        for (size_t j = 0; j < 250; j++)
        {
            if (this->char_items[cacheid][j].font_item)
            {
                if (this->char_items[cacheid][j].font_item->item_compare(font_item))
                {
                    return static_cast<int>(j);
                }
            }
        }

        return -1;
    }
};

#endif
