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

struct TestDraw : mod_api
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h) : gd(w, h) {}

    void draw_event(time_t now, gdi::GraphicApi&) override { (void)now; }
    void rdp_input_invalidate(Rect) override {}
    void rdp_input_mouse(int, int, int, Keymap2*) override {}
    void rdp_input_scancode(long, long, long, long, Keymap2*) override {}
    void rdp_input_synchronize(uint32_t, uint16_t, int16_t, int16_t) override {}
    void send_to_front_channel(CHANNELS::ChannelNameId, const uint8_t*, size_t, size_t, int) override {}

    void refresh(Rect) override {}

    void save_to_png(const char * filename)
    {
        std::FILE * file = std::fopen(filename, "w+");
        dump_png24(file, this->gd.data(), this->gd.width(),
                   this->gd.height(), this->gd.rowsize(), true);
        std::fclose(file);
    }

private:
    RDPDrawable & get_graphic_proxy() { return this->gd; }
};

#endif
