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
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#pragma once

#include "configs/config_access.hpp"

using ModRdpVariables = vcfg::variables<
    vcfg::var<cfg::context::auth_error_message,                 vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_channel_target,                vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify,                        vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_flags,        vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_exe_or_file,  vcfg::accessmode::set>,
    vcfg::var<cfg::mod_rdp::server_cert,                        vcfg::accessmode::set>,
    vcfg::var<cfg::mod_rdp::server_cert_response,               vcfg::accessmode::get | vcfg::accessmode::ask>,
    vcfg::var<cfg::context::auth_channel_answer,                vcfg::accessmode::get>,
    vcfg::var<cfg::context::pm_request,                         vcfg::accessmode::set>,
    vcfg::var<cfg::context::pm_response,                        vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command,                       vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_flags,       vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_original_exe_or_file, vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_exe_or_file, vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_working_dir, vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_arguments,   vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_exec_result, vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_account,     vcfg::accessmode::get>,
    vcfg::var<cfg::context::auth_command_rail_exec_password,    vcfg::accessmode::get>,
    vcfg::var<cfg::context::smartcard_login,                    vcfg::accessmode::set>,
    vcfg::var<cfg::context::native_session_id,                  vcfg::accessmode::set>,

    vcfg::var<cfg::context::rd_shadow_type,                     vcfg::accessmode::get>,
    vcfg::var<cfg::context::rd_shadow_invitation_error_code,    vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_error_message, vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_userdata,                 vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_id,            vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_addr,          vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_port,          vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_available,                vcfg::accessmode::set>
>;
