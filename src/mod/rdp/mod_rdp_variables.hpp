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
    vcfg::var<cfg::context::auth_notify,                       vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_flags,       vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_exe_or_file, vcfg::accessmode::set>,
    vcfg::var<cfg::context::target_service,                    vcfg::accessmode::get>,
    vcfg::var<cfg::context::session_id,                        vcfg::accessmode::get>,
    vcfg::var<cfg::globals::auth_user,                         vcfg::accessmode::get>,
    vcfg::var<cfg::globals::host,                              vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_device,                     vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_dir_path,                  vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_file_turnover_interval,    vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::sign_key,                      vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::enable_rdp_metrics,            vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_interval,                  vcfg::accessmode::get>,
    vcfg::var<cfg::mod_rdp::server_cert,                   vcfg::accessmode::set>,
    vcfg::var<cfg::mod_rdp::server_cert_response,          vcfg::accessmode::ask | vcfg::accessmode::get | vcfg::accessmode::get_ref>
>;
