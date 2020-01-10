/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2019
 *   Author(s): Meng Tan
 *
 *   Show a waiting message in modules transitions,
 *   also allow to quit with escape key
 */


#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget/tooltip.hpp"

using TransitionModVariables = vcfg::variables<
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class TransitionMod : public LocallyIntegrableMod
{

    TimerPtr timeout_timer;
    GraphicEventPtr started_copy_past_event;

    WidgetTooltip ttmessage;

    TransitionModVariables vars;

public:
    TransitionMod(
        TransitionModVariables vars,
        SessionReactor& session_reactor,
        TimerContainer& timer_events_,
        GraphicEventContainer& graphic_events_,
        gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
        Theme const& theme
    );

    ~TransitionMod() override;

    std::string module_name() override {return "Transition Mod";}

    [[nodiscard]] bool is_up_and_running() const override { return true; }

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                            long int param4, Keymap2* keymap) override;

};
