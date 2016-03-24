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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphael Zhou
*/

#ifndef _REDEMPTION_MOD_MOD_API_HPP_
#define _REDEMPTION_MOD_MOD_API_HPP_

#include <ctime>

#include "callback.hpp"
#include "font.hpp"
#include "utils/text_metrics.hpp"
#include "wait_obj.hpp"
#include "RDP/caches/glyphcache.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/RDPGraphicDevice.hpp"

class Inifile;

enum {
    BUTTON_STATE_UP   = 0,
    BUTTON_STATE_DOWN = 1
};

class mod_api : public Callback, public RDPGraphicDevice {
protected:
    wait_obj           event;
    RDPPen             pen;
    RDPGraphicDevice * gd;

    uint16_t front_width;
    uint16_t front_height;

public:
    mod_api(const uint16_t front_width, const uint16_t front_height)
    : gd(this)
    , front_width(front_width)
    , front_height(front_height) {
        this->event.set(0);
    }

    ~mod_api() override {}

    virtual wait_obj& get_event() { return this->event; }
    virtual wait_obj * get_secondary_event() { return nullptr; }

    virtual wait_obj * get_asynchronous_task_event(int & out_fd) { out_fd = -1; return nullptr; }
    virtual void process_asynchronous_task() {}

    virtual wait_obj * get_session_probe_launcher_event() { return nullptr; }
    virtual void process_session_probe_launcher() {}

    uint16_t get_front_width() const { return this->front_width; }
    uint16_t get_front_height() const { return this->front_height; }

    virtual void text_metrics(Font const & font, const char * text, int & width, int & height) {
        ::text_metrics(font, text, width, height,
            [](uint32_t charnum) {
                LOG(LOG_WARNING, "mod_api::text_metrics() - character not defined >0x%02x<", charnum);
            }
        );
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

            this->gd->draw(glyphindex, clip, &mod_glyph_cache);
        }
    }

protected:
    static RDPGraphicDevice * get_gd(mod_api const & mod)
    {
        return mod.gd;
    }

    static void set_gd(mod_api & mod, RDPGraphicDevice * gd)
    {
        mod.gd = gd;
    }

public:
    virtual void send_to_front_channel(const char * const mod_channel_name,
        uint8_t const * data, size_t length, size_t chunk_size, int flags) = 0;

    // draw_event is run when mod socket received some data (drawing order)
    // or auto-generated by modules, say to comply to some refresh.
    // draw event decodes incoming traffic from backend and eventually calls front to draw things
    // may raise an exception (say if connection to server is closed), but returns nothings
    virtual void draw_event(time_t now) = 0;

    // used when context changed to avoid creating a new module
    // it usually perform some task identical to what constructor does
    // henceforth it should often be called by constructors
    virtual void refresh_context(Inifile & ini) {}

    virtual bool is_up_and_running() { return false; }

    virtual void disconnect() {}

    virtual void begin_update() = 0;
    virtual void end_update() = 0;

    virtual void flush() {}

    virtual void display_osd_message(std::string & message) {}
};

#endif
