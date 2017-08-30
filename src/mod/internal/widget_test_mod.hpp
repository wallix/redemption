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

#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget/widget_test.hpp"
#include "mod/mod_api.hpp"

#include "configs/config_access.hpp"

using WidgetTestModVariables = vcfg::variables<
    vcfg::var<cfg::font,  vcfg::accessmode::get>,
    vcfg::var<cfg::theme, vcfg::accessmode::get>
>;

namespace GCC
{
    namespace UserData
    {
        class CSMonitor;
    }
}

class WidgetTestMod : public LocallyIntegrableMod, public NotifyApi
{
public:
    WidgetTestMod(
        WidgetTestModVariables vars,
        FrontAPI& front, uint16_t width, uint16_t height,
        Rect const widget_rect, std::unique_ptr<mod_api> managed_mod,
        ClientExecute& client_execute,
        const GCC::UserData::CSMonitor& cs_monitor);

    ~WidgetTestMod() override;

    void notify(Widget*, notify_event_t) override
    {}

    // RdpInput

    void rdp_input_invalidate(Rect r) override;

    void rdp_input_up_and_running() override;

    // Callback

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
                             InStream& chunk, size_t length,
                             uint32_t flags) override;

    // mod_api

    void draw_event(time_t now, gdi::GraphicApi& gapi) override;

    void get_event_handlers(std::vector<EventHandler>& out_event_handlers) override;

    bool is_up_and_running() override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width,
                          uint16_t height) override;

private:
    WidgetTest widget_test;

    class ManagedModEventHandler : public EventHandler::CB
    {
        WidgetTestMod& mod_;

    public:
        ManagedModEventHandler(WidgetTestMod& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& /*event*/, gdi::GraphicApi& drawable) override;
    } managed_mod_event_handler;
};
