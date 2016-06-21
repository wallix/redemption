/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#pragma once

#include "../mod/mod_api.hpp"
#include "widget2/screen.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"

struct InternalMod : public mod_api {
public:
    uint16_t front_width;
    uint16_t front_height;
    FrontAPI & front;

    WidgetScreen screen;

    InternalMod(FrontAPI & front, uint16_t front_width, uint16_t front_height, Font const & font,
                Theme const & theme = Theme())
        : front_width(front_width)
        , front_height(front_height)
        , front(front)
        , screen(front, front_width, front_height, font, nullptr, theme)
    {
        this->front.server_resize(front_width, front_height, 24);
    }

    Font const & font() const {
        return this->screen.font;
    }

    Theme const & theme() const {
        return this->screen.theme;
    }

    ~InternalMod() override {}

    const Rect & get_screen_rect() const
    {
        return this->screen.rect;
    }


    TODO("implementation of the server_draw_text function below is a small subset of possibilities text can be packed (detecting duplicated strings). See MS-RDPEGDI 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)")
    virtual void server_draw_text(Font const & font, int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) {
        static GlyphCache mod_glyph_cache;

        UTF8toUnicodeIterator unicode_iter(text);
        bool is_first_char = true;
        int offset_first_char = 0;
        while (*unicode_iter) {
            int total_width = 0;
            int total_height = 0;
            uint8_t data[256];
            auto data_begin = std::begin(data);
            const auto data_end = std::end(data)-2;

            const int cacheId = 7;
            int distance_from_previous_fragment = 0;
            while (data_begin != data_end) {
                const uint32_t charnum = *unicode_iter;
                if (!charnum) {
                    break ;
                }
                ++unicode_iter;

                int cacheIndex = 0;
                FontChar const & font_item = font.glyph_defined(charnum) && font.font_items[charnum]
                ? font.font_items[charnum]
                : [&]() {
                    LOG(LOG_WARNING, "mod_api::server_draw_text() - character not defined >0x%02x<", charnum);
                    return std::ref(font.font_items[static_cast<unsigned>('?')]);
                }().get();
                if (is_first_char) {
                    is_first_char = false;
                    offset_first_char = font_item.offset;
                }
                TODO(" avoid passing parameters by reference to get results")
                const GlyphCache::t_glyph_cache_result cache_result =
                    mod_glyph_cache.add_glyph(font_item, cacheId, cacheIndex);
                (void)cache_result; // supress warning

                *data_begin = cacheIndex;
                ++data_begin;
                *data_begin = distance_from_previous_fragment;
                ++data_begin;
                distance_from_previous_fragment = font_item.incby;
                total_width += font_item.incby;
                total_height = std::max(total_height, font_item.height);
            }

            const Rect bk(x, y, total_width + 1, total_height + 1);

            RDPGlyphIndex glyphindex(
                cacheId,            // cache_id
                0x03,               // fl_accel
                0x0,                // ui_charinc
                1,                  // f_op_redundant,
                fgcolor,            // BackColor (text color)
                bgcolor,            // ForeColor (color of the opaque rectangle)
                bk,                 // bk
                bk,                 // op
                // brush
                RDPBrush(0, 0, 3, 0xaa,
                    reinterpret_cast<const uint8_t *>("\xaa\x55\xaa\x55\xaa\x55\xaa\x55")),
                x - offset_first_char, // glyph_x
                y + total_height,   // glyph_y
                data_begin - data,  // data_len in bytes
                data                // data
            );

            x += total_width;

            this->front.draw(glyphindex, clip, mod_glyph_cache);
        }
    }

    void send_to_front_channel(const char * const mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size,
        int flags) override {
        const CHANNELS::ChannelDef * front_channel =
            this->front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
        else {
            LOG(LOG_ERR, "Channel \"%s\" is not fonud!", mod_channel_name);
        }
    }

    void rdp_input_invalidate(const Rect& r) override {
        this->screen.rdp_input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags,
                               int16_t param1, int16_t param2) override {}
};

