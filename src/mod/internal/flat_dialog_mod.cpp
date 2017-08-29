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

#include "mod/internal/flat_dialog_mod.hpp"
#include "core/front_api.hpp"
#include "configs/config.hpp"
#include "utils/translation.hpp"


FlatDialogMod::FlatDialogMod(
    FlatDialogModVariables vars, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    const char * cancel_text, time_t now, ClientExecute & client_execute,
    ChallengeOpt has_challenge
)
    : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->dialog_widget,
        front, front, this->font(), this->theme())
    , dialog_widget(
        front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this, caption, message,
        &this->language_button,
        vars.get<cfg::theme>(), vars.get<cfg::font>(),
        TR(trkeys::OK, language(vars)),
        cancel_text, has_challenge)
    , vars(vars)
    , timeout(now, vars.get<cfg::debug::pass_dialog_box>())
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
{
    this->screen.add_widget(&this->dialog_widget);
    this->dialog_widget.set_widget_focus(&this->dialog_widget.ok, Widget::focus_reason_tabkey);
    this->screen.set_widget_focus(&this->dialog_widget, Widget::focus_reason_tabkey);
    this->screen.rdp_input_invalidate(this->screen.get_rect());

    if (this->dialog_widget.challenge) {
        this->dialog_widget.set_widget_focus(this->dialog_widget.challenge, Widget::focus_reason_tabkey);
        // this->vars.get<cfg::to_send_set::insert>()(AUTHID_AUTHENTICATION_CHALLENGE);
    }
}

FlatDialogMod::~FlatDialogMod()
{
    this->screen.clear();
}

void FlatDialogMod::notify(Widget* sender, notify_event_t event)
{
    (void)sender;
    switch (event) {
        case NOTIFY_SUBMIT: this->accepted(); break;
        case NOTIFY_CANCEL: this->refused(); break;
        case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event);
        }
        break;
        default:;
    }
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void FlatDialogMod::accepted()
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
    this->event.signal = BACK_EVENT_NEXT;
    this->event.set_trigger_time(wait_obj::NOW);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void FlatDialogMod::refused()
{
    if (!this->dialog_widget.challenge) {
        if (this->dialog_widget.cancel) {
            this->vars.set_acl<cfg::context::accept_message>(false);
        }
        else {
            this->vars.set_acl<cfg::context::display_message>(false);
        }
    }
    this->event.signal = BACK_EVENT_NEXT;
    this->event.set_trigger_time(wait_obj::NOW);
}

void FlatDialogMod::draw_event(time_t now, gdi::GraphicApi & gapi)
{
    LocallyIntegrableMod::draw_event(now, gapi);

    if (!this->copy_paste && this->event.is_waked_up_by_time()) {
        this->copy_paste.ready(this->front);
    }
    switch(this->timeout.check(now)) {
    case Timeout::TIMEOUT_REACHED:
        this->accepted();
        break;
    case Timeout::TIMEOUT_NOT_REACHED:
        this->event.set_trigger_time(1000000);
        break;
    case Timeout::TIMEOUT_INACTIVE:
        this->event.reset_trigger_time();
        break;
    }
}


void FlatDialogMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
