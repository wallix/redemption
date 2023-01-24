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
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/dialog_mod.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/translation.hpp"


DialogMod::DialogMod(
    DialogModVariables vars,
    gdi::GraphicApi & drawable,
    FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    const char * cancel_text, ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme, CopyPaste& copy_paste,
    ChallengeOpt has_challenge
)
    : RailInternalModBase(drawable, width, height, rail_client_execute, font, theme, &copy_paste)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(), this->dialog_widget,
        drawable, front, font, theme)
    , dialog_widget(
        drawable, copy_paste, widget_rect,
        {
            .onsubmit = [this]{ this->accepted(); },
            .oncancel = [this]{ this->refused(); },
            .onctrl_shift = [this]{ this->language_button.next_layout(); },
        },
        caption, message, &this->language_button, theme, font,
        TR(trkeys::OK, language(vars)),
        cancel_text, has_challenge)
    , vars(vars)
{
    this->screen.add_widget(this->dialog_widget, WidgetComposite::HasFocus::Yes);
    this->screen.init_focus();
    this->screen.rdp_input_invalidate(this->screen.get_rect());
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void DialogMod::accepted()
{
    if (this->dialog_widget.challenge) {
        this->vars.set_acl<cfg::context::password>(this->dialog_widget.challenge->get_text());
    }
    else if (this->dialog_widget.cancel) {
        this->vars.set_acl<cfg::context::accept_message>(true);
    }
    else {
        this->vars.set_acl<cfg::context::display_message>(true);
    }
    this->set_mod_signal(BACK_EVENT_NEXT);
    // throw Error(ERR_BACK_EVENT_NEXT);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void DialogMod::refused()
{
    if (this->dialog_widget.challenge) {
        this->vars.set_acl<cfg::context::password>("");
    }
    else if (this->dialog_widget.cancel) {
        this->vars.set_acl<cfg::context::accept_message>(false);
    }
    else {
        this->vars.set_acl<cfg::context::display_message>(false);
    }
    this->set_mod_signal(BACK_EVENT_NEXT);
    // throw Error(ERR_BACK_EVENT_NEXT);
}
