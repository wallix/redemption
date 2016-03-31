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

#ifndef REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP

#include "../mod/mod_api.hpp"
#include "widget2/screen.hpp"
#include "configs/config.hpp"
#include "front_api.hpp"
#include "channel_list.hpp"

struct InternalMod : public mod_api {
public:
    FrontAPI & front;

    WidgetScreen screen;

    InternalMod(FrontAPI & front, uint16_t front_width, uint16_t front_height, Font const & font,
                Theme const & theme = Theme())
        : mod_api(front_width, front_height)
        , front(front)
        , screen(*this, front_width, front_height, font, nullptr, theme)
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

    void begin_update() override {
        this->front.begin_update();
    }

    void end_update() override {
        this->front.end_update();
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPScrBlt & cmd, const Rect &clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->front.draw(cmd, clip, bmp);
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->front.draw(cmd, clip, bmp);
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        this->front.draw(cmd, clip, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        this->front.draw(cmd, clip);
    }

    void draw(const RDP::FrameMarker & order) override {
        this->front.draw(order);
    }

    void draw(const RDPColCache & cmd) override {
        this->front.draw(cmd);
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data, size_t size, const Bitmap & bmp) override {
        this->front.draw(bitmap_data, data, size, bmp);
    }

    void draw(const RDPBrushCache& cmd) override {
        this->front.draw(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        this->front.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        this->front.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        this->front.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        this->front.draw(order);
    }

    void server_set_pointer(const Pointer & cursor) override {
        this->front.server_set_pointer(cursor);
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

#endif
