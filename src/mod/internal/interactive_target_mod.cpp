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
#include "mod/internal/interactive_target_mod.hpp"
#include "RAIL/client_execute.hpp"
#include "core/front_api.hpp"
#include "utils/translation.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"
#include "keyboard/mouse.hpp"

void InteractiveTargetMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void InteractiveTargetMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
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

void InteractiveTargetMod::rdp_input_scancode(
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
                LOG(LOG_INFO, "InteractiveTargetMod::rdp_input_scancode: Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }
}

void InteractiveTargetMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

bool InteractiveTargetMod::is_resizing_hosted_desktop_allowed() const
{
    assert(this->rail_enabled);

    return false;
}

InteractiveTargetMod::InteractiveTargetMod(
    InteractiveTargetModVariables vars,
    TimeBase& time_base,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front,
    uint16_t width, uint16_t height, Rect const widget_rect,
    ClientExecute & rail_client_execute, Font const& font, Theme const& theme)
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
    , ask_device(vars.is_asked<cfg::context::target_host>())
    , ask_login(vars.is_asked<cfg::globals::target_user>())
    , ask_password((this->ask_login || vars.is_asked<cfg::context::target_password>()))
    , language_button(vars.get<cfg::client::keyboard_layout_proposals>(), this->challenge,
        drawable, front, font, theme)
    , challenge(
        drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this,
        this->ask_device, this->ask_login, this->ask_password,
        theme,
        TR(trkeys::target_info_required, language(vars)),
        TR(trkeys::device, language(vars)), vars.get<cfg::globals::target_device>().c_str(),
        TR(trkeys::login, language(vars)), vars.get<cfg::globals::target_user>().c_str(),
        TR(trkeys::password, language(vars)),
        font, &this->language_button)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    , vars(vars)
{
    this->screen.set_wh(width, height);
    this->screen.add_widget(&this->challenge);
    this->challenge.password_edit.set_text("");
    this->screen.set_widget_focus(&this->challenge, Widget::focus_reason_tabkey);
    if (this->ask_device) {
        this->challenge.set_widget_focus(&this->challenge.device_edit, Widget::focus_reason_tabkey);
    }
    else if (this->ask_login) {
        this->challenge.set_widget_focus(&this->challenge.login_edit, Widget::focus_reason_tabkey);
    }
    else {
        this->challenge.set_widget_focus(&this->challenge.password_edit, Widget::focus_reason_tabkey);
    }
    this->screen.rdp_input_invalidate(this->screen.get_rect());
}

InteractiveTargetMod::~InteractiveTargetMod()
{
    this->rail_client_execute.reset(true);
    this->screen.clear();
}

void InteractiveTargetMod::init()
{
    if (this->rail_enabled && !this->rail_client_execute.is_ready()) {
        this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height,
                    this->font(), this->is_resizing_hosted_desktop_allowed());
        this->dvc_manager.ready(this->front);
    }
    this->copy_paste.ready(this->front);
}

void InteractiveTargetMod::notify(Widget* sender, notify_event_t event)
{
    (void)sender;
    switch (event) {
        case NOTIFY_SUBMIT: this->accepted(); break;
        case NOTIFY_CANCEL: this->refused(); break;
        case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
            if (this->copy_paste) {
                copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event); /*NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)*/
            }
            break;
        default: ;
    }
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void InteractiveTargetMod::accepted()
{
    if (this->ask_device) {
        this->vars.set_acl<cfg::context::target_host>(this->challenge.device_edit.get_text());
    }
    if (this->ask_login) {
        this->vars.set_acl<cfg::globals::target_user>(this->challenge.login_edit.get_text());
    }
    if (this->ask_password) {
        this->vars.set_acl<cfg::context::target_password>(this->challenge.password_edit.get_text());
    }
    this->vars.set_acl<cfg::context::display_message>(true);
    this->set_mod_signal(BACK_EVENT_NEXT);
}

// TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
void InteractiveTargetMod::refused()
{
    this->vars.set_acl<cfg::context::target_password>("");
    this->vars.set_acl<cfg::context::display_message>(false);
    this->set_mod_signal(BACK_EVENT_NEXT);
}

void InteractiveTargetMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
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
