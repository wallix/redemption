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

#include "utils/log.hpp"
#include "utils/pattutils.hpp"
#include "configs/config.hpp"
#include "gdi/screen_info.hpp"
#include "gdi/capture_probe_api.hpp"
#include "core/log_id.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "acl/auth_api.hpp"
#include "utils/timebase.hpp"
#include "utils/sugar/algostring.hpp"

#include <functional>


struct Sesman : public AuthApi
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

    bool selector_page_sent = true;
    unsigned current_page = 0;
    std::string group_filter;
    std::string device_filter;
    std::string proto_filter;

    bool keyboard_layout_sent = true;
    unsigned keyboard_layout = 0;


    bool auth_channel_target_sent = true;
    std::string auth_channel_target;

    bool report_sent = true;
    struct Report {
        std::string reason;
        std::string message;
    };

    std::vector<Report> reports;

    bool dispatch_to_capture = false;
    gdi::CaptureProbeApi * front = nullptr;
    LogCategoryFlags dont_log;

    TimeBase & time_base;

    void begin_dispatch_to_capture() override
    {
        dispatch_to_capture = true;

        if (bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::wrm)) {
            this->dont_log |= LogCategoryId::Drive;
        }
        if (bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::wrm)) {
            this->dont_log |= LogCategoryId::Clipboard;
        }
    }
    void end_dispatch_to_capture() override
    {
        dispatch_to_capture = false;
        this->dont_log = ::LogCategoryFlags();
    }


    Sesman(Inifile & ini, TimeBase & time_base) : ini(ini), time_base(time_base) {}

    void set_front(gdi::CaptureProbeApi * front)
    {
        this->front = front;
    }

    void log6(LogId id, KVList kv_list) override
    {
        this->log6_sent = false;
        this->buffered_log_params.emplace_back(id, kv_list);

        if (!this->dispatch_to_capture) {
            return;
        }

        if (dont_log.test(detail::log_id_category_map[underlying_cast(id)])) {
            return ;
        }

        this->front->session_update(this->time_base.get_current_time(), id, kv_list);
    }

    void set_connect_target()
    {
        this->session_log_is_open = true;
    }


    void set_selector_page(unsigned current, std::string group, std::string device, std::string proto) override
    {
        this->selector_page_sent = false;
        this->current_page = current;
        this->group_filter = std::move(group);
        this->device_filter = std::move(device);
        this->proto_filter = std::move(proto);

    }

    void set_keyboard_layout(unsigned keyboard_layout) override
    {
        this->keyboard_layout_sent = false;
        this->keyboard_layout = keyboard_layout;
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
            this->smartcard_login = login;
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
        this->rd_shadow_available_sent = false;
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

    std::string get_auth_error_message() override
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
    void flush_acl(bool verbose)
    {
        this->flush_acl_server_cert(verbose);
        this->flush_acl_screen_info(verbose);
        this->flush_acl_auth_info(verbose);
        this->flush_acl_recording_started(verbose);
        this->flush_acl_rt_ready(verbose);
        this->flush_acl_smartcard_login(verbose);
        this->flush_acl_rd_shadow_available(verbose);
        this->flush_acl_rd_shadow_invitation(verbose);
        this->flush_acl_native_session_id(verbose);
        this->flush_acl_pm_request(verbose);
        this->flush_acl_auth_error_message(verbose);
        this->flush_acl_auth_channel_target(verbose);
        this->flush_acl_selector_page(verbose);
        this->flush_acl_keyboard_layout(verbose);
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

    void flush_acl_selector_page(bool verbose)
    {
        if (!this->selector_page_sent) {
            LOG_IF(verbose, LOG_INFO, "flush_acl_selector_page()");
            this->ini.set_acl<cfg::context::selector_current_page>(this->current_page);

            this->ini.set_acl<cfg::context::selector_group_filter>(this->group_filter);
            this->ini.set_acl<cfg::context::selector_device_filter>(this->device_filter);
            this->ini.set_acl<cfg::context::selector_proto_filter>(this->proto_filter);

            this->ini.ask<cfg::globals::target_user>();
            this->ini.ask<cfg::globals::target_device>();
            this->ini.ask<cfg::context::selector>();
            this->selector_page_sent = true;
        }
    }

    void flush_acl_keyboard_layout(bool verbose)
    {
        if (!this->keyboard_layout_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_keyboard_layout()");
            this->ini.set_acl<cfg::client::keyboard_layout>(this->keyboard_layout);
            this->keyboard_layout_sent = true;
        }
    }


    void flush_acl_server_cert(bool verbose)
    {
        if (!this->server_cert_sent) {
            LOG_IF(verbose, LOG_INFO, "flush_acl_server_cert()");
            this->ini.set_acl<cfg::mod_rdp::server_cert>(this->blob_server_cert);
            this->ini.get_mutable_ref<cfg::mod_rdp::server_cert_response>() = "";
            this->ini.ask<cfg::mod_rdp::server_cert_response>();
            this->server_cert_sent = true;
        }
    }

    void flush_acl_screen_info(bool verbose)
    {
        if (!this->screen_info_sent) {
            LOG_IF(verbose, LOG_INFO, "flush_acl_screen_info()");
            this->ini.set_acl<cfg::context::opt_width>(this->screen_info.width);
            this->ini.set_acl<cfg::context::opt_height>(this->screen_info.height);
            this->ini.set_acl<cfg::context::opt_bpp>(safe_int(screen_info.bpp));
            this->screen_info_sent = true;
        }
    }

    void flush_acl_auth_info(bool verbose)
    {
        if (!this->auth_info_sent) {
            std::string username = this->username;
            if (not domain.empty()
             && (username.find('@') == std::string::npos)
             && (username.find('\\') == std::string::npos)) {
                str_append(username, '@', domain);
            }

            LOG_IF(verbose, LOG_INFO, "flush_acl_auth_info(auth_user=%s)", username);

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

    void flush_acl_recording_started(bool verbose)
    {
        if (!this->recording_started_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_recording_started()");
            this->ini.set_acl<cfg::context::recording_started>(true);
            this->recording_started_sent = true;
        }
    }


    void flush_acl_rt_ready(bool verbose)
    {
        if (!this->rt_ready_sent){
            if (!this->ini.get<cfg::context::rt_ready>()) {
                LOG_IF(verbose, LOG_INFO, "flush_acl_rt_ready()");
                this->ini.set_acl<cfg::context::rt_ready>(true);
            }
            this->rt_ready_sent= true;
        }
    }

    void flush_acl_smartcard_login(bool verbose)
    {
        if (!this->smartcard_login_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_smartcard_login()");
            this->ini.set_acl<cfg::context::smartcard_login>(this->smartcard_login);
            this->smartcard_login_sent = true;
        }
    }

    void flush_acl_rd_shadow_invitation(bool verbose)
    {
        if (!this->rd_shadow_invitation_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_rd_shadow_invitation()");
            this->ini.set_acl<cfg::context::rd_shadow_invitation_error_code>(this->error_code);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_error_message>(this->shadow_error_message);
            this->ini.set_acl<cfg::context::rd_shadow_userdata>(this->userdata);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_id>(this->id);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_addr>(this->addr);
            this->ini.set_acl<cfg::context::rd_shadow_invitation_port>(this->port);
            this->rd_shadow_invitation_sent = true;
        }
    }

    void flush_acl_rd_shadow_available(bool verbose)
    {
        if(!this->rd_shadow_available_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_rd_shadow_available()");
            this->ini.set_acl<cfg::context::rd_shadow_available>(true);
            this->rd_shadow_available_sent = true;
        }
    }

    void flush_acl_native_session_id(bool verbose)
    {
        if (!this->native_session_id_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_native_session_id()");
            this->ini.set_acl<cfg::context::native_session_id>(this->native_session_id);
            this->native_session_id_sent = true;
        }
    }

    void flush_acl_pm_request(bool verbose)
    {
        if (!this->pm_request_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_pm_request()");
            this->ini.set_acl<cfg::context::pm_request>(request);
            this->pm_request_sent = true;
        }
    }

    void flush_acl_auth_error_message(bool verbose)
    {
        if (!this->auth_error_message_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_auth_error_message()");
            this->ini.set<cfg::context::auth_error_message>(this->auth_error_message);
            this->auth_error_message_sent = true;
        }
    }

    void flush_acl_auth_channel_target(bool verbose)
    {
        if (!this->auth_channel_target_sent){
            LOG_IF(verbose, LOG_INFO, "flush_acl_auth_channel_target()");
            this->ini.set_acl<cfg::context::auth_channel_target>(this->auth_channel_target);
            this->auth_channel_target_sent = true;
        }
    }
};
