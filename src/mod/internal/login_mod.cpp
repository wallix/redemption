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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan,
 *              Jennifer Inthavong
 */


#include "mod/internal/login_mod.hpp"
#include "main/version.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"


LoginMod::LoginMod(
    LoginModVariables vars,
    TimeBase& time_base,
    TimerContainer& timer_events_,
    GraphicEventContainer& graphic_events_,
    char const * username, char const * password,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
    Theme const& theme
)
    : front_width(width)
    , front_height(height)
    , front(front)
    , screen(drawable, width, height, font, nullptr, theme)
    , rail_client_execute(rail_client_execute)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , dvc_manager(false)
    , dc_state(DCState::Wait)
    , current_mouse_owner(MouseOwner::WidgetModule)
    , time_base(time_base)
    , timer_events_(timer_events_)
    , graphic_events_(graphic_events_)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(),
        this->login, drawable, front, font, theme)
    , login(
        drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this, "Redemption " VERSION,
        nullptr, nullptr,
        TR(trkeys::login, language(vars)),
        TR(trkeys::password, language(vars)),
        vars.get<cfg::context::opt_message>().c_str(),
        vars.get<cfg::context::login_message>().c_str(),
        &this->language_button, font, Translator(language(vars)), theme)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    , vars(vars)
{
    this->screen.set_wh(front_width, front_height);
    if (this->rail_enabled) {
        this->graphic_event = graphic_events_.create_action_executor(time_base)
        .on_action(jln::one_shot([this](gdi::GraphicApi&){
            if (!this->rail_client_execute) {
                this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height, this->font(),
                    this->is_resizing_hosted_desktop_allowed());

                this->dvc_manager.ready(this->front);
            }
        }));
    }
    if (vars.get<cfg::globals::authentication_timeout>().count()) {
        LOG(LOG_INFO, "LoginMod: Ending session in %u seconds",
            static_cast<unsigned>(vars.get<cfg::globals::authentication_timeout>().count()));
    }
    this->screen.add_widget(&this->login);

    this->login.login_edit.set_text(username);
    this->login.password_edit.set_text(password);

    this->screen.set_widget_focus(&this->login, Widget::focus_reason_tabkey);

    this->login.set_widget_focus(&this->login.login_edit, Widget::focus_reason_tabkey);
    if (username[0] != 0){
        this->login.set_widget_focus(&this->login.password_edit, Widget::focus_reason_tabkey);
    }

    this->screen.rdp_input_invalidate(this->screen.get_rect());

    if (vars.get<cfg::globals::authentication_timeout>().count()) {
        this->timeout_timer = timer_events_
        .create_timer_executor(time_base)
        .set_delay(vars.get<cfg::globals::authentication_timeout>())
        .on_action([this](JLN_TIMER_CTX ctx){
            this->set_mod_signal(BACK_EVENT_STOP);
            return ctx.terminate();
        });
    }

    this->started_copy_past_event = graphic_events_.create_action_executor(time_base)
    .on_action(jln::one_shot([this](gdi::GraphicApi&){
        this->copy_paste.ready(this->front);
    }));
}

LoginMod::~LoginMod()
{
    this->rail_client_execute.reset(true);
    this->screen.clear();
}

void LoginMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void LoginMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
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
    if (!this->rail_client_execute.input_mouse(device_flags, x, y, out_mouse_captured)) {
        switch (this->dc_state) {
            case DCState::Wait:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = DCState::FirstClickDown;

                    if (this->first_click_down_timer) {
                        this->first_click_down_timer->set_delay(std::chrono::seconds(1));
                    }
                    else {
                        this->first_click_down_timer = timer_events_
                        .create_timer_executor(this->time_base)
                        .set_delay(std::chrono::seconds(1))
                        .on_action(jln::one_shot([this]{
                            this->dc_state = DCState::Wait;
                        }));
                    }
                }
            break;

            case DCState::FirstClickDown:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = DCState::FirstClickRelease;
                }
                else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case DCState::FirstClickRelease:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = DCState::SecondClickDown;
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case DCState::SecondClickDown:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = DCState::Wait;

                    bool out_mouse_captured_2 = false;

                    this->rail_client_execute.input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y, out_mouse_captured_2);

                    this->cancel_double_click_detection();
                }
                else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            default:
                assert(false);

                this->cancel_double_click_detection();
            break;
        }

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

void LoginMod::rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
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
                LOG(LOG_INFO, "LoginMod::rdp_input_scancode: Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }
}

void LoginMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void LoginMod::cancel_double_click_detection()
{
    assert(this->rail_enabled);

    this->first_click_down_timer.reset();

    this->dc_state = DCState::Wait;
}

bool LoginMod::is_resizing_hosted_desktop_allowed() const
{
    assert(this->rail_enabled);

    return false;
}

void LoginMod::notify(Widget* sender, notify_event_t event)
{
    switch (event) {
    case NOTIFY_SUBMIT: {
        this->vars.set_acl<cfg::globals::auth_user>(this->login.login_edit.get_text());
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->vars.set_acl<cfg::context::password>(this->login.password_edit.get_text());
        this->set_mod_signal(BACK_EVENT_NEXT);
        break;
    }
    case NOTIFY_CANCEL:
        this->set_mod_signal(BACK_EVENT_STOP);
        break;
    case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event); /*NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)*/
        }
        break;
    default:;
    }
}

void LoginMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    if (this->rail_enabled && this->rail_client_execute){
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
