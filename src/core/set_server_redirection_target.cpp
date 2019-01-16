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
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#include "configs/config.hpp"
#include "core/report_message_api.hpp"
#include "core/set_server_redirection_target.hpp"
#include "utils/log.hpp"
#include "utils/sugar/cast.hpp"


void set_server_redirection_target(Inifile& ini, ReportMessageApi& reporter)
{
    // SET new target in ini
    RedirectionInfo const& redir_info = ini.get<cfg::mod_rdp::redir_info>();
    const char * host = char_ptr_cast(redir_info.host);
    const char * password = char_ptr_cast(redir_info.password);
    const char * username = char_ptr_cast(redir_info.username);
    const char * change_user = "";
    if (redir_info.dont_store_username && username[0] != 0) {
        LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
        ini.set_acl<cfg::globals::target_user>(username);
        change_user = username;
    }
    if (password[0] != 0) {
        LOG(LOG_INFO, "SrvRedir: Change target password");
        ini.set_acl<cfg::context::target_password>(password);
    }
    LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
    ini.set_acl<cfg::context::target_host>(host);
    char message[770];
    sprintf(message, "%s@%s", change_user, host);
    reporter.report("SERVER_REDIRECTION", message);
}
