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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan, Jennifer Inthavong
 */


#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/widget/selector.hpp"
#include "mod/internal/widget/language_button.hpp"

using SelectorModVariables = vcfg::variables<
    vcfg::var<cfg::globals::auth_user,                  vcfg::accessmode::ask | vcfg::accessmode::set | vcfg::accessmode::get>,
    vcfg::var<cfg::context::selector,                   vcfg::accessmode::ask | vcfg::accessmode::set>,
    vcfg::var<cfg::context::target_protocol,            vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_device,              vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_user,                vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::context::password,                   vcfg::accessmode::ask>,
    vcfg::var<cfg::context::selector_current_page,      vcfg::accessmode::is_asked | vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_number_of_pages,   vcfg::accessmode::is_asked | vcfg::accessmode::get>,
    vcfg::var<cfg::context::selector_lines_per_page,    vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_group_filter,      vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_device_filter,     vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_proto_filter,      vcfg::accessmode::set>,
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::globals::host,                       vcfg::accessmode::get>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class SelectorMod : public RailModBase, public NotifyApi
{
public:
    SelectorMod(
        SelectorModVariables ini,
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute & rail_client_execute,
        Font const& font, Theme const& theme);

    void init() override;

    void acl_update(AclFieldMask const& acl_fields) override;

    void notify(Widget* widget, notify_event_t event) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                            long int param4, Keymap2* keymap) override;

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override;

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;

private:
    void refresh_device();
    void ask_page();

    SelectorModVariables ini;

    LanguageButton language_button;

    WidgetSelectorParams selector_params;
    WidgetSelector selector;

    int current_page;
    int number_page;

    CopyPaste copy_paste;

    int selector_lines_per_page_saved = 0;
};
