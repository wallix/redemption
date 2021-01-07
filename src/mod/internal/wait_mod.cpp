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

#include "configs/config.hpp"
#include "mod/internal/wait_mod.hpp"


WaitMod::WaitMod(
    WaitModVariables vars,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    ClientExecute & rail_client_execute, Font const& font, Theme const& theme,
    bool showform, uint32_t flag
)
    : RailModBase(
        events, drawable, front,
        width, height, rail_client_execute, font, theme)
    , language_button(vars.get<cfg::client::keyboard_layout_proposals>(), this->wait_widget,
        drawable, front, font, theme)
    , wait_widget(drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this, caption, message, 0, &this->language_button,
        font, theme, language(vars), showform, flag, vars.get<cfg::context::duration_max>())
    , vars(vars)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    , events_guard(events)
{
    this->screen.add_widget(&this->wait_widget);
    if (this->wait_widget.hasform) {
        this->wait_widget.set_widget_focus(&this->wait_widget.form, Widget::focus_reason_tabkey);
    }
    else {
        this->wait_widget.set_widget_focus(&this->wait_widget.goselector, Widget::focus_reason_tabkey);
    }
    this->screen.set_widget_focus(&this->wait_widget, Widget::focus_reason_tabkey);
    this->screen.rdp_input_invalidate(this->screen.get_rect());

    this->events_guard.create_event_timeout("Wait Mod Timeout",
        600s, [this](Event&)
        {
            this->refused();
        }
    );
}

WaitMod::~WaitMod() = default;

void WaitMod::init()
{
    RailModBase::init();
    this->copy_paste.ready(this->front);
}

void WaitMod::notify(Widget * sender, notify_event_t event)
{
    switch (event) {
        case NOTIFY_SUBMIT: this->accepted(); break;
        case NOTIFY_CANCEL: this->refused(); break;
        case NOTIFY_TEXT_CHANGED: this->confirm(); break;
        case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
            if (this->copy_paste) {
                copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit *>(sender), event); /*NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)*/
            }
            break;
        default:;
    }
}

void WaitMod::confirm()
{
    this->vars.set_acl<cfg::context::waitinforeturn>("confirm");
    this->vars.set_acl<cfg::context::comment>(this->wait_widget.form.comment_edit.get_text());
    this->vars.set_acl<cfg::context::ticket>(this->wait_widget.form.ticket_edit.get_text());
    this->vars.set_acl<cfg::context::duration>(this->wait_widget.form.duration_edit.get_text());
    this->set_mod_signal(BACK_EVENT_NEXT);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void WaitMod::accepted()
{
    this->vars.set_acl<cfg::context::waitinforeturn>("backselector");
    this->set_mod_signal(BACK_EVENT_NEXT);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void WaitMod::refused()
{
    this->vars.set_acl<cfg::context::waitinforeturn>("exit");
    this->set_mod_signal(BACK_EVENT_NEXT);
}

void WaitMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
