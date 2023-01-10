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

#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/copy_paste.hpp"
#include "RAIL/client_execute.hpp"
#include "gdi/graphic_api.hpp"
#include "keyboard/keymap.hpp"


RailModBase::RailModBase(
    gdi::GraphicApi & gd,
    uint16_t width, uint16_t height,
    ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme)
    : screen(gd, width, height, font, theme)
    , rail_client_execute(rail_client_execute)
    , rail_enabled(rail_client_execute.is_rail_enabled())
{
    this->screen.set_wh(width, height);

    gd.set_palette(BGRPalette::classic_332());
}

RailModBase::~RailModBase()
{
    this->screen.clear();
    this->rail_client_execute.reset(true);
}

void RailModBase::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void RailModBase::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
        x = this->old_mouse_x;
        y = this->old_mouse_y;
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }

    if (!this->rail_enabled) {
        this->screen.rdp_input_mouse(device_flags, x, y);
        return;
    }

    bool mouse_is_captured
        = this->rail_client_execute.input_mouse(device_flags, x, y);

    if (mouse_is_captured) {
        this->screen.allow_mouse_pointer_change(false);
        this->current_mouse_owner = MouseOwner::ClientExecute;
    }
    else {
        if (MouseOwner::WidgetModule != this->current_mouse_owner) {
            this->screen.redo_mouse_pointer_change(x, y);
        }

        this->current_mouse_owner = MouseOwner::WidgetModule;
    }

    this->screen.rdp_input_mouse(device_flags, x, y);

    if (mouse_is_captured) {
        this->screen.allow_mouse_pointer_change(true);
    }
}

void RailModBase::rdp_input_scancode(
    KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    this->check_alt_f4(keymap);
    this->screen.rdp_input_scancode(flags, scancode, event_time, keymap);
}

void RailModBase::rdp_input_unicode(KbdFlags flag, uint16_t unicode)
{
    this->screen.rdp_input_unicode(flag, unicode);
}

void RailModBase::check_alt_f4(Keymap const& keymap) const
{
    if (this->rail_enabled
     && keymap.last_kevent() == Keymap::KEvent::F4
     && keymap.is_alt_pressed()
    ) {
        LOG(LOG_INFO, "RailModBase::rdp_input_scancode: Close by user (Alt+F4)");
        throw Error(ERR_WIDGET);
    }
}

void RailModBase::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length, uint32_t flags)
{
    if (front_channel_name == CHANNELS::channel_names::rail) {
        if (this->rail_enabled && this->rail_client_execute.is_ready()) {
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
    }
}

RailInternalModBase::RailInternalModBase(
    gdi::GraphicApi & gd,
    uint16_t width, uint16_t height,
    ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme,
    CopyPaste* copy_paste
)
    : RailModBase(gd, width, height, rail_client_execute, font, theme)
    , front_width(width)
    , front_height(height)
    , copy_paste(copy_paste)
{
    gd.set_palette(BGRPalette::classic_332());
}

void RailInternalModBase::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (front_channel_name == CHANNELS::channel_names::cliprdr) {
        if (this->copy_paste && *this->copy_paste) {
            this->copy_paste->send_to_mod_channel(chunk, flags);
        }
    }
}
