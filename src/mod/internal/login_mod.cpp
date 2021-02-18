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

#include "configs/config.hpp"
#include "mod/internal/login_mod.hpp"
#include "main/version.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"
#include "utils/sugar/algostring.hpp"

#include <string>
#include <utility>


namespace
{
    inline std::pair<std::string, std::string>
    rpartition(std::string_view text, const char* seps)
    {
        size_t sep_pos = text.find_last_of(seps);
        if (sep_pos != std::string::npos) {
            return std::make_pair(
                std::string(text.substr(0, sep_pos)),
                std::string(text.substr(sep_pos + 1))
            );
        }

        return std::make_pair(std::string(), std::string(text));
    }

    inline std::string
    concat_target_login(std::string_view login, std::string_view target)
    {
        if (target.empty()) {
            return std::string(login);
        }

        size_t sep_pos = target.find_last_of("+:");
        if (sep_pos != std::string::npos) {
            return str_concat(target, target[sep_pos], login);
        }

        return str_concat(target, ':', login);
    }
}

LoginMod::LoginMod(
    LoginModVariables vars,
    EventContainer & events,
    char const * username, char const * password,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
    Theme const& theme
)
    : RailModBase(
        events, drawable, front,
        width, height, rail_client_execute, font, theme)
    , events_guard(events)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(),
        this->login, drawable, front, font, theme)
    , login(
        drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this, "Redemption " VERSION,
        nullptr, nullptr, nullptr,
        TR(trkeys::login, login_language(vars)),
        TR(trkeys::password, login_language(vars)),
        TR(trkeys::target, login_language(vars)),
        vars.get<cfg::context::opt_message>().c_str(),
        vars.get<cfg::context::login_message>().c_str(),
        &this->language_button,
        vars.get<cfg::internal_mod::enable_target_field>(),
        font, Translator(login_language(vars)), theme)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    , vars(vars)
{
    if (vars.get<cfg::globals::authentication_timeout>().count()) {
        LOG(LOG_INFO, "LoginMod: Ending session in %u seconds",
            static_cast<unsigned>(vars.get<cfg::globals::authentication_timeout>().count()));
    }
    this->screen.add_widget(&this->login);


    if (vars.get<cfg::internal_mod::enable_target_field>()) {
        auto [target, login] = rpartition(username, ":+");
        this->login.login_edit.set_text(login.c_str());
        this->login.target_edit.set_text(target.c_str());
    } else {
        this->login.login_edit.set_text(username);
        this->login.target_edit.set_text(nullptr);
    }
    this->login.password_edit.set_text(password);

    this->screen.set_widget_focus(&this->login, Widget::focus_reason_tabkey);

    this->login.set_widget_focus(&this->login.login_edit, Widget::focus_reason_tabkey);
    if (username[0] != 0){
        this->login.set_widget_focus(&this->login.password_edit, Widget::focus_reason_tabkey);
    }

    this->screen.rdp_input_invalidate(this->screen.get_rect());

    if (vars.get<cfg::globals::authentication_timeout>().count()) {
        this->events_guard.create_event_timeout(
            "Log Box Timeout",
            vars.get<cfg::globals::authentication_timeout>(),
            [this](Event&e)
            {
                e.garbage = true;
                this->set_mod_signal(BACK_EVENT_STOP);
            });
    }
}

LoginMod::~LoginMod() = default;

void LoginMod::init()
{
    RailModBase::init();
    this->copy_paste.ready(this->front);
}

void LoginMod::notify(Widget* sender, notify_event_t event)
{
    switch (event) {
    case NOTIFY_SUBMIT: {
        std::string auth_user = concat_target_login(
            this->login.login_edit.get_text(),
            this->login.target_edit.get_text()
        );
        this->vars.set_acl<cfg::globals::auth_user>(auth_user);
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->vars.set_acl<cfg::context::password>(this->login.password_edit.get_text());
        this->set_mod_signal(BACK_EVENT_NEXT);
        // throw Error(ERR_BACK_EVENT_NEXT);
        break;
    }
    case NOTIFY_CANCEL:
        this->set_mod_signal(BACK_EVENT_STOP);
        break;
    case NOTIFY_PASTE:
    case NOTIFY_COPY:
    case NOTIFY_CUT:
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *sender, event);
        }
        break;
    default:;
    }
}

void LoginMod::send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}
