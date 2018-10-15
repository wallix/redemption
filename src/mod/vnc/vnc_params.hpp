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
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "configs/config_access.hpp"

#include <chrono>
#include <string>

using ModVncVariables = vcfg::variables<
    vcfg::var<cfg::context::target_service,                    vcfg::accessmode::get>,
    vcfg::var<cfg::context::session_id,                        vcfg::accessmode::get>,
    vcfg::var<cfg::globals::auth_user,                         vcfg::accessmode::get>,
    vcfg::var<cfg::globals::host,                              vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_device,                     vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_dir_path,                  vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_file_turnover_interval,    vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::sign_key,                      vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::enable_vnc_metrics  ,          vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_interval,                  vcfg::accessmode::get>
>;
