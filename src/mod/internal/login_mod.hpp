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


#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/flat_login.hpp"
#include "mod/internal/widget/language_button.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/dvc_manager.hpp"
#include "mod/internal/widget/screen.hpp"
#include "core/events.hpp"
#include "utils/timebase.hpp"
#include "keyboard/mouse.hpp"

class ClientExecute;

using LoginModVariables = vcfg::variables<
    vcfg::var<cfg::context::password,                   vcfg::accessmode::set>,
    vcfg::var<cfg::globals::auth_user,                  vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector,                   vcfg::accessmode::ask>,
    vcfg::var<cfg::context::target_protocol,            vcfg::accessmode::ask>,
    vcfg::var<cfg::globals::target_device,              vcfg::accessmode::ask>,
    vcfg::var<cfg::globals::target_user,                vcfg::accessmode::ask>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::context::opt_message,                vcfg::accessmode::get>,
    vcfg::var<cfg::context::login_message,              vcfg::accessmode::get>,
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::globals::authentication_timeout,     vcfg::accessmode::get>,
    vcfg::var<cfg::internal_mod::enable_target_field,   vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class LoginMod : public mod_api, public NotifyApi
{
    uint16_t front_width;
    uint16_t front_height;

    FrontAPI & front;

    WidgetScreen screen;

    ClientExecute & rail_client_execute;
    const bool rail_enabled;
    DVCManager dvc_manager;

    MouseState mouse_state;

    enum class MouseOwner
    {
        ClientExecute,
        WidgetModule,
    };

    MouseOwner current_mouse_owner;

    TimeBase& time_base;
    EventContainer& events;

    LanguageButton language_button;
    FlatLogin login;

    CopyPaste copy_paste;

    LoginModVariables vars;

    // Not initialized in constructor
    bool alt_key_pressed = false;

    int old_mouse_x = 0;
    int old_mouse_y = 0;

private:
    [[nodiscard]] virtual bool is_resizing_hosted_desktop_allowed() const;

public:
    LoginMod(
        LoginModVariables vars,
        TimeBase& time_base,
        EventContainer& events,
        char const * username, char const * password,
        gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
        Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
        Theme const& theme
    );

    ~LoginMod() override;

    void init() override;

    std::string module_name() override {return "LoginMod";}

    void notify(Widget* sender, notify_event_t event) override;

    [[nodiscard]] bool is_up_and_running() const override { return true; }

    bool server_error_encountered() const override { return false; }

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override;
    void create_shadow_session(const char * /*userdata*/, const char * /*type*/) override {}
    void send_auth_channel_data(const char * /*data*/) override {}
    void send_checkout_channel_data(const char * /*data*/) override {}

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        this->login.move_size_widget(left, top, width, height);
    }

public:
    [[nodiscard]] Font const & font() const { return this->screen.font; }

    [[nodiscard]] Theme const & theme() const { return this->screen.theme; }

    [[nodiscard]] Rect get_screen_rect() const { return this->screen.get_rect(); }

    void rdp_gdi_up_and_running() override {}
    void rdp_gdi_down() override {}

    void rdp_input_invalidate(Rect r) override;
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;
    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override;
    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override { this->screen.rdp_input_unicode(unicode, flag); }
    void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/, int16_t /*param1*/, int16_t /*param2*/) override {}

    void refresh(Rect r) override;

    [[nodiscard]] Dimension get_dim() const override { return Dimension(this->front_width, this->front_height); }

    void allow_mouse_pointer_change(bool allow) { this->screen.allow_mouse_pointer_change(allow); }

    void redo_mouse_pointer_change(int x, int y) { this->screen.redo_mouse_pointer_change(x, y); }
};
