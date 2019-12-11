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
#include "mod/metrics_hmac.hpp"
#include "utils/sugar/unique_fd.hpp"

#include "acl/module_manager.hpp"


void ModuleManager::create_mod_vnc(
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo const& client_info,
    ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'");

    unique_fd client_sck = this->connect_to_target_host(
        report_message, trkeys::authentification_vnc_fail);

    try {
        const char * const name = "VNC Target";

        struct ModVNCWithMetrics : public mod_vnc
        {
            struct ModMetrics : Metrics
            {
                using Metrics::Metrics;

                VNCMetrics protocol_metrics{*this};
            };

            std::unique_ptr<ModMetrics> metrics;
            SessionReactor::TimerPtr metrics_timer;
            using mod_vnc::mod_vnc;
        };

        bool const enable_metrics = (ini.get<cfg::metrics::enable_vnc_metrics>()
            && create_metrics_directory(ini.get<cfg::metrics::log_dir_path>().as_string()));

        std::unique_ptr<ModVNCWithMetrics::ModMetrics> metrics;

        if (enable_metrics) {
            metrics = std::make_unique<ModVNCWithMetrics::ModMetrics>(
                ini.get<cfg::metrics::log_dir_path>().as_string(),
                ini.get<cfg::context::session_id>(),
                hmac_user(
                    ini.get<cfg::globals::auth_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_account(
                    ini.get<cfg::globals::target_user>(),
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
        }

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
            ini.get<cfg::mod_vnc::support_cursor_pseudo_encoding>(),
            (client_info.remote_program ? &rail_client_execute : nullptr),
            to_verbose_flags(ini.get<cfg::debug::mod_vnc>()),
            enable_metrics ? &metrics->protocol_metrics : nullptr
        );

        if (enable_metrics) {
            new_mod->metrics = std::move(metrics);
            new_mod->metrics_timer = session_reactor.create_timer()
                .set_delay(std::chrono::seconds(ini.get<cfg::metrics::log_interval>()))
                .on_action([metrics = new_mod->metrics.get()](JLN_TIMER_CTX ctx){
                    metrics->log(ctx.get_current_time());
                    return ctx.ready();
                })
            ;
        }

        if (client_info.remote_program) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            Rect adjusted_client_execute_rect =
                rail_client_execute.adjust_rect(get_widget_rect(
                    client_info.screen_info.width,
                    client_info.screen_info.height,
                    client_info.cs_monitor
                ));

            std::string target_info = str_concat(
              ini.get<cfg::context::target_str>(),
              ':',
              ini.get<cfg::globals::primary_user_id>());

            rail_client_execute.set_target_info(target_info);

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                drawable,
                front,
                client_info.screen_info.width,
                client_info.screen_info.height,
                adjusted_client_execute_rect,
                std::move(new_mod),
                rail_client_execute,
                this->_font,
                this->_theme,
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
        report_message.log6(LogId::SESSION_CREATION_FAILED, this->session_reactor.get_current_time(), {});

        throw;
    }

    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded");
    ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}
