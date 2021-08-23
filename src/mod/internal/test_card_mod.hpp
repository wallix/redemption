/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, JOnathan Poelen, Raphael Zhou

   Use (implemented) basic RDP orders to draw some known test pattern
*/

#pragma once

#include "mod/mod_api.hpp"

class BGRPalette;
class Font;
class FrontAPI;

namespace gdi
{
    class GraphicApi;
}

class TestCardMod : public mod_api
{
    uint16_t front_width;
    uint16_t front_height;

    Font const & font;

    bool unit_test;

    gdi::GraphicApi & gd;

    [[nodiscard]] Rect get_screen_rect() const;

public:
    TestCardMod(
        gdi::GraphicApi & gd, uint16_t width, uint16_t height,
        Font const & font, bool unit_test = true); /*NOLINT*/

    void init() override;

    void rdp_input_invalidate(Rect /*rect*/) override
    {}

    void rdp_input_mouse(int /*device_flags*/, int /*x*/, int /*y*/) override {}

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        (void)unicode;
        (void)flag;
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        (void)locks;
    }

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    [[nodiscard]] Dimension get_dim() const override;

    [[nodiscard]] bool is_up_and_running() const override
    {
        return true;
    }

    bool server_error_encountered() const override { return false; }

    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}

    void acl_update(AclFieldMask const&/* acl_fields*/) override {}

private:
    void draw_event();
};
