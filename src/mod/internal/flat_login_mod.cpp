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


#include "mod/internal/flat_login_mod.hpp"
#include "main/version.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"


FlatLoginMod::FlatLoginMod(
    FlatLoginModVariables vars, SessionReactor& session_reactor,
    char const * username, char const * password,
    FrontAPI & front, uint16_t width, uint16_t height, Rect const widget_rect, time_t /*now*/,
    ClientExecute & client_execute, Font const& font, Theme const& theme
)
    : LocallyIntegrableMod(session_reactor, front, width, height, font, client_execute, theme)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(),
        this->login, front, front, font, theme)
    , login(
        front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
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
        this->timeout_timer = session_reactor.create_timer()
        .set_delay(vars.get<cfg::globals::authentication_timeout>())
        .on_action([](JLN_TIMER_CTX ctx){
            ctx.get_reactor().set_next_event(BACK_EVENT_STOP);
            return ctx.terminate();
        });
    }

    this->started_copy_past_event = session_reactor.create_graphic_event()
    .on_action(jln::one_shot([this](gdi::GraphicApi&){
        this->copy_paste.ready(this->front);
    }));
}

FlatLoginMod::~FlatLoginMod()
{
    this->screen.clear();
}

void FlatLoginMod::notify(Widget* sender, notify_event_t event)
{
    switch (event) {
    case NOTIFY_SUBMIT: {
        char const* username = this->login.login_edit.get_text();
        this->vars.set_acl<cfg::globals::auth_user>(username);
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->vars.set_acl<cfg::context::password>(this->login.password_edit.get_text());
        this->session_reactor.set_next_event(BACK_EVENT_NEXT);
        break;
    }
    case NOTIFY_CANCEL:
        this->session_reactor.set_next_event(BACK_EVENT_STOP);
        break;
    case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event); /*NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)*/
        }
        break;
    default:;
    }
}

void FlatLoginMod::draw_event(time_t now, gdi::GraphicApi & gapi)
{
    LocallyIntegrableMod::draw_event(now, gapi);
}

void FlatLoginMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
