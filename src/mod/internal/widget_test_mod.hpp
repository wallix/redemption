/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen, Raphael Zhou
*/

#pragma once

#include "mod/mod_api.hpp"
#include "core/events.hpp"
#include <memory>

class FrontAPI;
class Font;

namespace gdi
{
    class GraphicApi;
}

class WidgetTestMod : public mod_api
{
public:
    WidgetTestMod(
        gdi::GraphicApi & gd,
        EventContainer & events,
        FrontAPI & front, uint16_t width, uint16_t height,
        Font const & font);

    ~WidgetTestMod() override;

    void rdp_gdi_up_and_running() override {}

    void rdp_gdi_down() override {}

    void rdp_input_invalidate(Rect /*rect*/) override;

    void rdp_input_mouse(int /*device_flags*/, int x, int y, Keymap2 * /*keymap*/) override;

    void rdp_input_scancode(long /*param1*/, long /*param2*/, long /*param3*/,
                            long /*param4*/, Keymap2 * keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override;

    void refresh(Rect clip) override;

    [[nodiscard]] bool is_up_and_running() const override
    {
        return true;
    }

    bool server_error_encountered() const override { return false; }

    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}

    class WidgetTestModPrivate;
    friend WidgetTestModPrivate;
    std::unique_ptr<WidgetTestModPrivate> d;
};
