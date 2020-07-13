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
#include "core/report_message_api.hpp"
#include <functional>

struct Sesman : public AuthApi, public ReportMessageApi
{
    Inifile & ini;

    struct LogParam
    {
        LogId id;
        struct KV
        {
            std::string key;
            std::string value;
        };
        std::vector<KV> kv_list;

        LogParam(LogId id, KVList list)
        : id(id)
        {
            kv_list.reserve(list.size());
            for (auto const& kv : list) {
                this->kv_list.emplace_back(KV{
                    std::string(kv.key.data(), kv.key.size()),
                    std::string(kv.value.data(), kv.value.size()),
                });
            }
        }
    };

    bool log6_sent = false;
    bool session_log_is_open = false;
    std::vector<LogParam> buffered_log_params;

    bool screen_info_sent = true;
    ScreenInfo screen_info;

    bool auth_info_sent = true;
    std::string username;
    std::string domain;
    std::string password;

    bool server_cert_sent = true;
    std::string blob_server_cert;

    bool recording_started_sent = true;

    bool rt_ready_sent = true;

    bool smartcard_login_sent = true;
    std::string smartcard_login;

    bool rd_shadow_invitation_sent = true;
    uint32_t error_code;
    std::string shadow_error_message;
    std::string userdata;
    std::string id;
    std::string addr;
    uint16_t port;

    bool rd_shadow_available_sent = true;

    bool native_session_id_sent = true;
    uint32_t native_session_id;

    bool pm_request_sent = true;
    std::string request;

    bool disconnect_target_sent = true;

    bool auth_error_message_sent = true;
    std::string auth_error_message;

    bool auth_channel_target_sent = true;
    std::string auth_channel_target;

    bool report_sent = true;
    struct Report {
        std::string reason;
        std::string message;
    };

    std::vector<Report> reports;

    Sesman(Inifile & ini) : ini(ini) {}

    void log6(LogId id, KVList kv_list) override
    {
        this->log6_sent = false;
        this->buffered_log_params.emplace_back(id, kv_list);
    }

    void set_connect_target()
    {
        this->session_log_is_open = true;
    }

    void set_server_cert(std::string const& blob_str) override
    {
        this->server_cert_sent = false;
        this->blob_server_cert = blob_str;
    }

    void set_screen_info(ScreenInfo screen_info) override
    {
        this->screen_info_sent = false;
        this->screen_info = screen_info;
    }

    void set_auth_info(std::string const& username, std::string const& domain, std::string const& password) override
    {
        this->auth_info_sent = false;
        this->username = username;
        this->domain = domain;
        this->password = password;
    }

    void set_recording_started() override
    {
        this->recording_started_sent = false;
    }

    void set_rt_ready() override
    {
        this->rt_ready_sent = false;
    }

    void set_smartcard_login(const char * login) override
    {
        this->smartcard_login_sent = false;
        if (!login){
            this->smartcard_login = "";
        }
        else {
            this->smartcard_login = std::string(login);
        }
    }

    void set_rd_shadow_invitation(uint32_t error_code, const char * error_message, const char * userdata, const char * id, const char * addr, uint16_t port) override
    {
        this->rd_shadow_invitation_sent = false;
        this->error_code = error_code;
        this->shadow_error_message = error_message;
        this->userdata = userdata;
        this->id = id;
        this->addr = addr;
        this->port = port;
    }

    void set_rd_shadow_available() override
    {
        this->rd_shadow_available_sent = true;
    }

    void set_native_session_id(unsigned int session_id) override
    {
        this->native_session_id_sent = false;
        this->native_session_id = session_id;
    }

    void set_pm_request(std::string_view request) override
    {
        this->pm_request_sent = false;
        this->request = request;
    }

    void set_disconnect_target() override
    {
        this->disconnect_target_sent = false;
    }

    std::string get_auth_error_message(const char * error_message) override
    {
        return this->auth_error_message;
    }


    void set_auth_error_message(const char * error_message) override
    {
        this->auth_error_message = error_message;
        this->auth_error_message_sent = false;
    }

    void set_auth_channel_target(const char * target) override
    {
        this->auth_channel_target_sent = false;
        this->auth_channel_target = target;
    }

    void report(const char * reason, const char * message) override
    {
        this->report_sent = false;
        Report report{reason, message};
        this->reports.push_back(report);
    }


    // flush_acl is only called from session and is no part of AuthApi interface
    void flush_acl()
    {
        this->flush_acl_server_cert();
        this->flush_acl_screen_info();
        this->flush_acl_auth_info();
        this->flush_acl_recording_started();
        this->flush_acl_rt_ready();
        this->flush_acl_smartcard_login();
        this->flush_acl_rd_shadow_invitation();
        this->flush_acl_native_session_id();
        this->flush_acl_pm_request();
        this->flush_acl_auth_error_message();
        this->flush_acl_auth_channel_target();
    }

    void flush_acl_report(std::function<void(std::string,std::string)> fn)
    {
        if (!this->report_sent) {
            for(auto & report: this->reports){
                fn(report.reason, report.message);
            }
            this->reports.clear();
            this->report_sent = true;
        }
    }

    void flush_acl_disconnect_target(std::function<void(void)> close_log)
    {
        if (!this->disconnect_target_sent)
        {
            if (this->session_log_is_open){
                close_log();
                this->session_log_is_open = false;
            }
            this->ini.set_acl<cfg::context::module>("close");
            this->disconnect_target_sent = true;
        }
    }


    void flush_acl_log6(std::function<void(LogId id, KVList kv_list)> log6)
    {
        if (!this->log6_sent) {
            if (!this->buffered_log_params.empty()) {
                std::vector<KVLog> v;
                for (LogParam const & log_param : this->buffered_log_params) {
                    v.reserve(log_param.kv_list.size());
                    for (auto const& kv : log_param.kv_list) {
                        v.emplace_back(KVLog{kv.key, kv.value});
                    }
                    //serializer.log6(log_param.id, {v});
                    log6(log_param.id, {v});
                    v.clear();
                }
                this->buffered_log_params.clear();
                this->buffered_log_params.shrink_to_fit();
            }
            this->log6_sent = true;
        }
    }

private:

    void flush_acl_server_cert()
    {
        if (!this->server_cert_sent) {
            this->ini.set_acl<cfg::mod_rdp::server_cert>(this->blob_server_cert);
            this->ini.get_mutable_ref<cfg::mod_rdp::server_cert_response>() = "";
            this->ini.ask<cfg::mod_rdp::server_cert_response>();
            this->server_cert_sent = true;
        }
    }

    void flush_acl_screen_info()
    {
        if (!this->screen_info_sent) {
            this->ini.set_acl<cfg::context::opt_width>(this->screen_info.width);
            this->ini.set_acl<cfg::context::opt_height>(this->screen_info.height);
            this->ini.set_acl<cfg::context::opt_bpp>(safe_int(screen_info.bpp));
            this->screen_info_sent = true;
        }
    }

    void flush_acl_auth_info()
    {
        if (!this->auth_info_sent) {
            std::string username = this->username;
            if (not domain.empty()
             && (username.find('@') == std::string::npos)
             && (username.find('\\') == std::string::npos)) {
                username = username + std::string("@") + domain;
            }

            LOG(LOG_INFO, "flush_acl_auth_info: auth_user=%s", username);
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

    void flush_acl_recording_started()
    {
        if (!this->recording_started_sent){
            this->ini.set_acl<cfg::context::recording_started>(true);
            this->recording_started_sent = true;
        }
    }


    void flush_acl_rt_ready()
    {
        if (!this->rt_ready_sent){
            if (!this->ini.get<cfg::context::rt_ready>()) {
                this->ini.set_acl<cfg::context::rt_ready>(true);
            }
            this->rt_ready_sent= true;
        }
    }

    void flush_acl_smartcard_login()
    {
        if (!this->smartcard_login_sent){
            this->ini.set_acl<cfg::context::smartcard_login>(this->smartcard_login);
            this->smartcard_login_sent = true;
        }
    }

    void flush_acl_rd_shadow_invitation()
    {
        if (!this->rd_shadow_invitation_sent){
            this->ini.set_acl<cfg::context::rd_shadow_invitation_error_code>(this->error_code);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_error_message>(this->shadow_error_message);
            this->ini.set_acl<cfg::context::rd_shadow_userdata>(this->userdata);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_id>(this->id);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_addr>(this->addr);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_port>(this->port);
            this->rd_shadow_invitation_sent = true;
        }
    }

    void flush_acl_rd_shadow_available()
    {
        if(!this->rd_shadow_available_sent){
            this->ini.set_acl<cfg::context::rd_shadow_available>(true);
            this->rd_shadow_available_sent = true;
        }
    }

    void flush_acl_native_session_id()
    {
        if (!this->native_session_id_sent){
            this->ini.set_acl<cfg::context::native_session_id>(this->native_session_id);
            this->native_session_id_sent = true;
        }
    }

    void flush_acl_pm_request()
    {
        if (!this->pm_request_sent){
            this->ini.set_acl<cfg::context::pm_request>(request);
            this->pm_request_sent = true;
        }
    }

    void flush_acl_auth_error_message()
    {
        if (!this->auth_error_message_sent){
            this->ini.set<cfg::context::auth_error_message>(this->auth_error_message);
            this->auth_error_message_sent = true;
        }
    }

    void flush_acl_auth_channel_target()
    {
        if (!this->auth_channel_target_sent){
            this->ini.set_acl<cfg::context::auth_channel_target>(this->auth_channel_target);
            this->auth_channel_target_sent = true;
        }
    }

};
