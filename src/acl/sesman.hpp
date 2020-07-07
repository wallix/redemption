/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
               Dominique Lafages, Raphael Zhou, Meng Tan,
               Jennifer Inthavong
    Based on xrdp Copyright (C) Jay Sorg 2004-2010

    Front object (server), used to communicate with RDP client
*/

#pragma once

#include <string>

#include "utils/pattutils.hpp"
#include "configs/config.hpp"
#include "gdi/screen_info.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "acl/auth_api.hpp"
#include "acl/authentifier.hpp"

struct SesmanInterface : public AuthApi
{
    Inifile & ini;
private:
    AuthApi & authentifier;

public:
    bool screen_info_sent = false;
    ScreenInfo screen_info;

    bool auth_info_sent = false;
    std::string username;
    std::string domain;
    std::string password;

    bool server_cert_sent = false;
    std::string blob_server_cert;

    SesmanInterface(Inifile & ini, AuthApi & authentifier)
    : ini(ini), authentifier(authentifier)
    {
    }


    void set_auth_channel_target(const char * target) override
    {
        this->authentifier.set_auth_channel_target(target);
    }

    void set_auth_error_message(const char * error_message) override
    {
        this->authentifier.set_auth_error_message(error_message);
    }

    void disconnect_target() override
    {
        this->authentifier.disconnect_target();
    }

    void set_pm_request(const char * request) override
    {
        this->authentifier.set_pm_request(request);
    }

    void set_native_session_id(unsigned int session_id) override
    {
        this->authentifier.set_native_session_id(session_id);
    }

    void rd_shadow_available() override
    {
        this->authentifier.rd_shadow_available();
    }

    void rd_shadow_invitation(uint32_t error_code, const char * error_message, const char * request, const char * id, const char * addr, uint16_t port) override
    {
        this->authentifier.rd_shadow_invitation(error_code, error_message, request, id, addr, port);
    }

    void set_smartcard_login(const char * login) override
    {
        this->authentifier.set_smartcard_login(login);
    }

    void set_screen_info(ScreenInfo screen_info)
    {
        this->screen_info_sent = false;
        this->screen_info = screen_info;
    }

    void set_acl_screen_info()
    {
        if (!this->screen_info_sent) {
            this->ini.set_acl<cfg::context::opt_width>(this->screen_info.width);
            this->ini.set_acl<cfg::context::opt_height>(this->screen_info.height);
            this->ini.set_acl<cfg::context::opt_bpp>(safe_int(screen_info.bpp));
            this->screen_info_sent = true;
        }
    }

    void set_auth_info(std::string const& username, std::string const& domain, std::string const& password)
    {
        this->auth_info_sent = false;
        this->username = username;
        this->domain = domain;
        this->password = password;
    }

    void set_acl_auth_info()
    {
        if (!this->auth_info_sent) {
            std::string username = this->username;
            if (not domain.empty()
             && (username.find('@') == std::string::npos)
             && (username.find('\\') == std::string::npos)) {
                username = username + std::string("@") + domain;
            }

            LOG(LOG_INFO, "set_acl_auth_info: auth_user=%s", username);
            this->ini.set_acl<cfg::globals::auth_user>(username);
            this->ini.ask<cfg::context::selector>();
            this->ini.ask<cfg::globals::target_user>();
            this->ini.ask<cfg::globals::target_device>();
            this->ini.ask<cfg::context::target_protocol>();
            if (!password.empty()) {
                this->ini.set_acl<cfg::context::password>(password);
            }
            this->auth_info_sent = true;
        }
    }

    void set_server_cert(std::string const& blob_str)
    {
        this->blob_server_cert = blob_str;
        this->server_cert_sent = false;
    }

    void set_acl_server_cert()
    {
        if (!this->server_cert_sent) {
            this->ini.set_acl<cfg::mod_rdp::server_cert>(this->blob_server_cert);
            this->ini.get_mutable_ref<cfg::mod_rdp::server_cert_response>() = "";
            this->ini.ask<cfg::mod_rdp::server_cert_response>();
            this->server_cert_sent = true;
        }
    }

    void set_acl_recording_started()
    {
        this->ini.set_acl<cfg::context::recording_started>(true);
    }

    void set_acl_rt_ready()
    {
        if (!this->ini.get<cfg::context::rt_ready>()) {
            this->ini.set_acl<cfg::context::rt_ready>(true);
        }
    }

    bool has_ocr_pattern_check()
    {
        return ::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
            || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str());
    }

    bool has_kbd_pattern_check()
    {
        return ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
            || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str());
    }

    bool is_capture_necessary()
    {
        return (ini.get<cfg::video::allow_rt_without_recording>()
            || ini.get<cfg::globals::is_rec>()
            || !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
            || ::contains_kbd_or_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
            || ::contains_kbd_or_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str()));
    }

    void show_session_config()
    {
        LOG(LOG_INFO, "record_filebase    = %s", ini.get<cfg::capture::record_filebase>());
        LOG(LOG_INFO, "auth_user     = %s", ini.get<cfg::globals::auth_user>());
        LOG(LOG_INFO, "host          = %s", ini.get<cfg::globals::host>());
        LOG(LOG_INFO, "target_device = %s", ini.get<cfg::globals::target_device>());
        LOG(LOG_INFO, "target_user   = %s", ini.get<cfg::globals::target_user>());
    }

    BitsPerPixel wrm_color_depth()
    {
        return (ini.get<cfg::video::wrm_color_depth_selection_strategy>() == ColorDepthSelectionStrategy::depth16)
               ? BitsPerPixel{16}
               : BitsPerPixel{24};
    }
};

