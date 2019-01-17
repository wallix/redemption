/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/report_message_api.hpp"
#include "mod/vnc/vnc.hpp"
#include "utils/sugar/unique_fd.hpp"

#include "acl/module_manager.hpp"

#ifndef __EMSCRIPTEN__
# include "mod/metrics_hmac.hpp"
#endif

void ModuleManager::create_mod_vnc(
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, FrontAPI& front, ClientInfo const& client_info, ClientExecute& client_execute,
    Keymap2::KeyFlags key_flags)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'\n");

    unique_fd client_sck = this->connect_to_target_host(
        report_message, trkeys::authentification_vnc_fail, "VNC");

    try {
        const char * const name = "VNC Target";

        const char * target_user = ini.get<cfg::globals::target_user>().c_str();

#ifndef __EMSCRIPTEN__
        struct ModVNCWithMetrics : public mod_vnc
        {
            std::unique_ptr<Metrics> metrics = nullptr;
            std::unique_ptr<VNCMetrics> protocol_metrics = nullptr;
            SessionReactor::TimerPtr metrics_timer;

            using mod_vnc::mod_vnc;
        };

        bool const enable_metrics = (ini.get<cfg::metrics::enable_vnc_metrics>()
            && create_metrics_directory(ini.get<cfg::metrics::log_dir_path>().to_string()));

        std::unique_ptr<Metrics> metrics;
        std::unique_ptr<VNCMetrics> protocol_metrics;

        if (enable_metrics) {
            metrics = std::make_unique<Metrics>(
                ini.get<cfg::metrics::log_dir_path>().to_string(),
                ini.get<cfg::context::session_id>(),
                hmac_user(
                    ini.get<cfg::globals::auth_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_account(
                    {target_user, strlen(target_user)},
                    ini.get<cfg::metrics::sign_key>()),
                hmac_device_service(
                    ini.get<cfg::globals::target_device>(),
                    ini.get<cfg::context::target_service>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_client_info(
                    ini.get<cfg::globals::host>(),
                    client_info.screen_info,
                    ini.get<cfg::metrics::sign_key>()),
                this->timeobj.get_time(),
                ini.get<cfg::metrics::log_file_turnover_interval>(),
                ini.get<cfg::metrics::log_interval>());

            protocol_metrics = std::make_unique<VNCMetrics>(metrics.get());
        }
#else
        using ModVNCWithMetrics = mod_rdp;
#endif

        auto new_mod = std::make_unique<ModWithSocket<ModVNCWithMetrics>>(
            *this,
            authentifier,
            name,
            std::move(client_sck),
            ini.get<cfg::debug::mod_vnc>(),
            nullptr,
            sock_mod_barrier(),
            this->session_reactor,
            ini.get<cfg::globals::target_user>().c_str(),
            ini.get<cfg::context::target_password>().c_str(),
            front,
            client_info.screen_info.width,
            client_info.screen_info.height,
            client_info.keylayout,
            key_flags,
            ini.get<cfg::mod_vnc::clipboard_up>(),
            ini.get<cfg::mod_vnc::clipboard_down>(),
            ini.get<cfg::mod_vnc::encodings>().c_str(),
            ini.get<cfg::mod_vnc::server_clipboard_encoding_type>()
                != ClipboardEncodingType::latin1
                ? mod_vnc::ClipboardEncodingType::UTF8
                : mod_vnc::ClipboardEncodingType::Latin1,
            ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>(),
            report_message,
            ini.get<cfg::mod_vnc::server_is_apple>(),
            ini.get<cfg::mod_vnc::server_unix_alt>(),
            (client_info.remote_program ? &client_execute : nullptr),
            ini,
            to_verbose_flags(ini.get<cfg::debug::mod_vnc>()),
            enable_metrics ? protocol_metrics.get() : nullptr
        );

        if (enable_metrics) {
            new_mod->metrics = std::move(metrics);
            new_mod->protocol_metrics = std::move(protocol_metrics);
            new_mod->metrics_timer = session_reactor.create_timer()
                .set_delay(std::chrono::seconds(ini.get<cfg::metrics::log_interval>()))
                .on_action([mod = new_mod.get()](JLN_TIMER_CTX ctx){
                    mod->metrics->log(ctx.get_current_time());
                    return ctx.ready();
                })
            ;
        }

        if (client_info.remote_program) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            Rect adjusted_client_execute_rect =
                client_execute.adjust_rect(get_widget_rect(
                    client_info.screen_info.width,
                    client_info.screen_info.height,
                    client_info.cs_monitor
                ));

            std::string target_info = str_concat(
              ini.get<cfg::context::target_str>(),
              ':',
              ini.get<cfg::globals::primary_user_id>());

            client_execute.set_target_info(target_info);

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                front,
                client_info.screen_info.width,
                client_info.screen_info.height,
                adjusted_client_execute_rect,
                std::move(new_mod),
                client_execute,
                this->load_font(),
                this->load_theme(),
                client_info.cs_monitor,
                false
            );
            this->set_mod(host_mod);
            this->rail_module_host_mod_ptr = host_mod;
            LOG(LOG_INFO, "ModuleManager::internal module 'RailModuleHostMod' ready");
        }
        else {
            this->set_mod(new_mod.release());
        }
    }
    catch (...) {
        ArcsightLogInfo arc_info;
        arc_info.name = "SESSION_CREATION";
        arc_info.ApplicationProtocol = "vnc";
        arc_info.WallixBastionStatus = "FAIL";
        arc_info.signatureID = ArcsightLogInfo::SESSION_CREATION;
        report_message.log6("type=\"SESSION_CREATION_FAILED\"", arc_info, this->session_reactor.get_current_time());

        throw;
    }

    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded");
    ini.get_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}
