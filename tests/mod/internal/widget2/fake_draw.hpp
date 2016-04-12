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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TESTS_MOD_INTERNAL_WIDGET2_FAKE_DRAW_HPP
#define REDEMPTION_TESTS_MOD_INTERNAL_WIDGET2_FAKE_DRAW_HPP

#include <cstdio>

#include "core/font.hpp"
#include "mod/mod_api.hpp"
#include "core/RDP/RDPDrawable.hpp"

#ifdef IN_IDE_PARSER
#define FIXTURES_PATH
#endif

struct TestDraw : mod_api
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h) : mod_api(w, h), gd(w, h, 24) {}

    virtual void draw_event(time_t now) override {}
    virtual void rdp_input_invalidate(const Rect& r) override {}
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {}
    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {}
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {}
    virtual void send_to_front_channel(const char*const mod_channel_name, const uint8_t* data, size_t length, size_t chunk_size, int flags) override {}


    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) override
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) override
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) override
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) override
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip) override
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override
    {
        this->gd.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPBrushCache & cmd) override
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPColCache & cmd) override
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip) override {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::FrameMarker & order) override
    {
        this->gd.draw(order);
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override
    {
        this->gd.draw(bitmap_data, bmp);
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::WindowIcon & order) override {
        this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::CachedIcon & order) override {
        this->gd.draw(order);
    }

    virtual void draw(const RDP::RAIL::DeletedWindow & order) override {
        this->gd.draw(order);
    }

    virtual void set_pointer(const Pointer & cursor) override {
        this->gd.set_pointer(cursor);
    }

    virtual void begin_update() override {}

    virtual void end_update() override {}

    virtual void server_draw_text(Font const & font, int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) override
    {
        this->gd.server_draw_text(font, x, y, text, fgcolor, bgcolor, clip);
    }

    virtual void text_metrics(Font const & font, const char * text, int & width, int & height) override
    {
        this->gd.text_metrics(font, text, width, height);
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.data(), this->gd.width(),
                   this->gd.height(), this->gd.rowsize(), true);
        fclose(file);
    }
};

#endif
