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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */


#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/widget/notify_api.hpp"
#include "mod/internal/widget/flat_wait.hpp"
#include "mod/internal/widget/language_button.hpp"


using WaitModVariables = vcfg::variables<
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::context::comment,                    vcfg::accessmode::set>,
    vcfg::var<cfg::context::duration,                   vcfg::accessmode::set>,
    vcfg::var<cfg::context::ticket,                     vcfg::accessmode::set>,
    vcfg::var<cfg::context::waitinforeturn,             vcfg::accessmode::set>,
    vcfg::var<cfg::context::duration_max,               vcfg::accessmode::get>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class WaitMod : public RailModBase, public NotifyApi
{
public:
    WaitMod(
        WaitModVariables vars,
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front,
        uint16_t width, uint16_t height, Rect const widget_rect, const char * caption,
        const char * message, ClientExecute & rail_client_execute, Font const& font,
        Theme const& theme, bool showform = false, uint32_t flag = 0);

    ~WaitMod() override;

    void init() override;

    void notify(Widget * sender, notify_event_t event) override;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        this->wait_widget.move_size_widget(left, top, width, height);
    }

private:
    LanguageButton language_button;
    FlatWait wait_widget;

    WaitModVariables vars;

    CopyPaste copy_paste;

    EventsGuard events_guard;

    void confirm();
    void accepted();
    void refused();
};
