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
#include "RAIL/client_execute.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"


void WaitMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void WaitMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
{
    if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
        x = this->old_mouse_x;
        y = this->old_mouse_y;
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }

    if (!this->rail_enabled) {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
        return;
    }
    bool out_mouse_captured = false;
    if (!this->rail_client_execute.input_mouse(device_flags, x, y, keymap, out_mouse_captured)) {
        this->mouse_state.chained_input_mouse = [this] (int device_flags, int x, int y, Keymap2 * keymap, bool & out_mouse_captured){
            return this->rail_client_execute.input_mouse(device_flags, x, y, keymap, out_mouse_captured);
        };
        this->mouse_state.input_mouse(device_flags, x, y, keymap);

        if (out_mouse_captured) {
            this->allow_mouse_pointer_change(false);
            this->current_mouse_owner = MouseOwner::ClientExecute;
        }
        else {
            if (MouseOwner::WidgetModule != this->current_mouse_owner) {
                this->redo_mouse_pointer_change(x, y);
            }

            this->current_mouse_owner = MouseOwner::WidgetModule;
        }
    }

    this->screen.rdp_input_mouse(device_flags, x, y, keymap);

    if (out_mouse_captured) {
        this->allow_mouse_pointer_change(true);
    }
}

void WaitMod::rdp_input_scancode(
    long param1, long param2, long param3, long param4, Keymap2 * keymap)
{
    this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);

    if (this->rail_enabled) {
        if (!this->alt_key_pressed) {
            if ((param1 == 56) && !(param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = true;
            }
        }
        else {
//            if ((param1 == 56) && (param3 == (SlowPath::KBDFLAGS_DOWN | SlowPath::KBDFLAGS_RELEASE))) {
            if ((param1 == 56) && (param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = false;
            }
            else if ((param1 == 62) && !param3) {
                LOG(LOG_INFO, "WaitMod::rdp_input_scancode: Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }
}

void WaitMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

bool WaitMod::is_resizing_hosted_desktop_allowed() const
{
    assert(this->rail_enabled);

    return false;
}

WaitMod::WaitMod(
    WaitModVariables vars,
    TimeBase& time_base,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, const char * caption, const char * message,
    ClientExecute & rail_client_execute, Font const& font, Theme const& theme,
    bool showform, uint32_t flag
)
    : front_width(width)
    , front_height(height)
    , front(front)
    , screen(drawable, width, height, font, nullptr, theme)
    , rail_client_execute(rail_client_execute)
    , dvc_manager(false)
    , mouse_state(time_base, events)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , current_mouse_owner(MouseOwner::WidgetModule)
    , time_base(time_base)
    , events(events)
    , language_button(vars.get<cfg::client::keyboard_layout_proposals>(), this->wait_widget,
        drawable, front, font, theme)
    , wait_widget(drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this, caption, message, 0, &this->language_button,
        font, theme, language(vars), showform, flag, vars.get<cfg::context::duration_max>())
    , vars(vars)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
{
    this->screen.set_wh(front_width, front_height);

    this->screen.add_widget(&this->wait_widget);
    if (this->wait_widget.hasform) {
        this->wait_widget.set_widget_focus(&this->wait_widget.form, Widget::focus_reason_tabkey);
    }
    else {
        this->wait_widget.set_widget_focus(&this->wait_widget.goselector, Widget::focus_reason_tabkey);
    }
    this->screen.set_widget_focus(&this->wait_widget, Widget::focus_reason_tabkey);
    this->screen.rdp_input_invalidate(this->screen.get_rect());

    this->events.create_event_timeout("Wait Mod Timeout", this,
        this->time_base.get_current_time()+std::chrono::seconds(600),
        [this](Event&)
        {
            this->refused();
        }
    );
}

void WaitMod::init()
{
    if (this->rail_enabled && !this->rail_client_execute.is_ready()) {
        this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height,
                    this->font(), this->is_resizing_hosted_desktop_allowed());
        this->dvc_manager.ready(this->front);
    }
    this->copy_paste.ready(this->front);
}

WaitMod::~WaitMod()
{
    this->events.end_of_lifespan(this);
    this->rail_client_execute.reset(true);
    this->screen.clear();
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
    if (this->rail_enabled && this->rail_client_execute.is_ready()){
        if (front_channel_name == CHANNELS::channel_names::rail) {
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
        else if (front_channel_name == CHANNELS::channel_names::drdynvc) {
            this->dvc_manager.send_to_mod_drdynvc_channel(length, chunk, flags);
        }
    }

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
