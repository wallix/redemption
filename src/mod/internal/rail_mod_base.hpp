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
Copyright (C) Wallix 2020
Author(s): Proxy Team
*/

#pragma once

#include "mod/mod_api.hpp"
#include "mod/internal/widget/screen.hpp"

class ClientExecute;
class EventContainer;
class FrontAPI;
class CopyPaste;

class RailModBase : public mod_api
{
public:
    RailModBase(
        gdi::GraphicApi & gd,
        uint16_t width, uint16_t height,
        ClientExecute & rail_client_execute,
        Font const& font, Theme const& theme);

    ~RailModBase();

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void rdp_input_mouse_ex(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

protected:
    void check_alt_f4(Keymap const& keymap) const;

protected:
    WidgetScreen screen;

    ClientExecute & rail_client_execute;

private:
    const bool rail_enabled;

    enum class MouseOwner : bool
    {
        ClientExecute,
        WidgetModule,
    };

    MouseOwner current_mouse_owner;

    uint16_t old_mouse_x = 0;
    uint16_t old_mouse_y = 0;
};


class RailInternalModBase : public RailModBase
{
public:
    RailInternalModBase(
        gdi::GraphicApi & gd,
        uint16_t width, uint16_t height,
        ClientExecute & rail_client_execute,
        Font const& font, Theme const& theme,
        CopyPaste* copy_paste);

    [[nodiscard]] bool is_up_and_running() const override
    {
        return true;
    }

    [[nodiscard]] Rect get_screen_rect() const
    {
        return this->screen.get_rect();
    }

    bool server_error_encountered() const override
    {
        return false;
    }

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

    void rdp_input_synchronize(KeyLocks locks) override
    {
        (void)locks;
    }

    [[nodiscard]]
    Dimension get_dim() const override
    {
        return Dimension(this->front_width, this->front_height);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream& chunk, size_t length, uint32_t flags) override;

private:
    uint16_t front_width;
    uint16_t front_height;

private:
    CopyPaste* copy_paste;
};
