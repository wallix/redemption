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

void ModuleManager::create_mod_vnc(
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, FrontAPI& front, ClientInfo const& client_info, ClientExecute& client_execute,
    Keymap2::KeyFlags key_flags)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'\n");

    unique_fd client_sck = this->connect_to_target_host(
        report_message, trkeys::authentification_vnc_fail);

    try {
        const char * const name = "VNC Target";

        std::unique_ptr<mod_api> managed_mod(new ModWithSocket<mod_vnc>(
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
            client_info.width,
            client_info.height,
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
            (client_info.remote_program ? &client_execute : nullptr),
            to_verbose_flags(ini.get<cfg::debug::mod_vnc>())
        ));

        if (client_info.remote_program) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            Rect adjusted_client_execute_rect =
                client_execute.adjust_rect(get_widget_rect(
                    client_info.width,
                    client_info.height,
                    client_info.cs_monitor
                ));

            std::string target_info
              = ini.get<cfg::context::target_str>()
              + ":"
              + ini.get<cfg::globals::primary_user_id>();

            client_execute.set_target_info(target_info.c_str());

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                front,
                client_info.width,
                client_info.height,
                adjusted_client_execute_rect,
                std::move(managed_mod),
                client_execute,
                client_info.cs_monitor,
                false
            );
            this->set_mod(host_mod);
            this->rail_module_host_mod_ptr = host_mod;
            LOG(LOG_INFO, "ModuleManager::internal module 'RailModuleHostMod' ready");
        }
        else {
            this->set_mod(managed_mod.release());
        }
    }
    catch (...) {
        report_message.log5("type=\"SESSION_CREATION_FAILED\"");

        throw;
    }

    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded");
    ini.get_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}
