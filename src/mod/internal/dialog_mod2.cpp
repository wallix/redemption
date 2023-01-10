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

#include "configs/config.hpp"
#include "mod/internal/dialog_mod2.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/translation.hpp"


DialogMod2::DialogMod2(
    DialogMod2Variables vars,
    gdi::GraphicApi & drawable,
    FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    const char * link_value, const char * link_label,
    ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme
)
    : RailModBase(drawable, width, height, rail_client_execute, font, theme)
    , front(front)
    , dialog_widget(
        drawable, widget_rect, this->screen,
        {.onsubmit = [this]{ this->accepted(); },
         .oncancel = [this]{ this->refused(); }},
        caption, message, link_value, link_label,
        this->copy_paste, theme, font, TR(trkeys::OK, language(vars)))
    , vars(vars)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
{
    this->screen.add_widget(&this->dialog_widget);
    this->dialog_widget.set_widget_focus(&this->dialog_widget.ok, Widget::focus_reason_tabkey);
    this->screen.set_widget_focus(&this->dialog_widget, Widget::focus_reason_tabkey);
    this->screen.rdp_input_invalidate(this->screen.get_rect());
}

DialogMod2::~DialogMod2() = default;

void DialogMod2::init()
{
    RailModBase::init();
    this->copy_paste.ready(this->front);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void DialogMod2::accepted()
{
    this->vars.set_acl<cfg::context::display_message>(true);
    this->set_mod_signal(BACK_EVENT_NEXT);
    // throw Error(ERR_BACK_EVENT_NEXT);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void DialogMod2::refused()
{
    this->vars.set_acl<cfg::context::display_message>(false);
    this->set_mod_signal(BACK_EVENT_NEXT);
    // throw Error(ERR_BACK_EVENT_NEXT);
}

void DialogMod2::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
