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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/widget/flat_dialog.hpp"
#include "mod/internal/widget/language_button.hpp"


using DialogModVariables = vcfg::variables<
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::context::accept_message,             vcfg::accessmode::set>,
    vcfg::var<cfg::context::display_message,            vcfg::accessmode::set>,
    vcfg::var<cfg::context::password,                   vcfg::accessmode::set>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class DialogMod : public RailModBase, public NotifyApi
{
public:
    DialogMod(
        DialogModVariables vars,
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, const char * caption, const char * message,
        const char * cancel_text, ClientExecute & rail_client_execute,
        Font const& font, Theme const& theme, ChallengeOpt has_challenge = NO_CHALLENGE); /*NOLINT*/

    ~DialogMod() override;

    void init() override;

    void notify(Widget* sender, notify_event_t event) override;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        this->dialog_widget.move_size_widget(left, top, width, height);
    }

private:
    void accepted();

    void refused();

    LanguageButton language_button;
    FlatDialog dialog_widget;

    DialogModVariables vars;

    CopyPaste copy_paste;
};
