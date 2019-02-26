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

#include "configs/autogen/enums.hpp"
#include "mod/rdp/channels/sespro_channel_params.hpp"

#include <chrono>
#include <string>
#include <cstdint>


struct ModRdpSessionProbeParams
{
    bool is_public_session = false;

    bool start_launch_timeout_timer_only_after_logon = true;

    bool enable_session_probe = false;
    bool enable_launch_mask = true;


    bool use_to_launch_remote_program = true;
    bool use_clipboard_based_launcher = false;

    bool customize_executable_name = false;

    SessionProbeClipboardBasedLauncherParams clipboard_based_launcher {};

    std::string exe_or_file {};
    std::string arguments {};

    SessionProbeVirtualChannelParams vc;
};
