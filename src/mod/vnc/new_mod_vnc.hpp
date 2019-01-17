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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include "mod/mod_api.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "mod/vnc/vnc_params.hpp"

#include <memory>

class ClientExecute;
class FrontAPI;
class ReportMessageApi;
class SessionReactor;
class Transport;
class VNCMetrics;

std::unique_ptr<mod_api> new_mod_vnc(
    Transport& t,
    SessionReactor& session_reactor,
    const char* username,
    const char* password,
    FrontAPI& front,
    uint16_t front_width,
    uint16_t front_height,
    int keylayout,
    int key_flags,
    bool clipboard_up,
    bool clipboard_down,
    const char * encodings,
    ReportMessageApi& report_message,
    bool server_is_apple,
    bool send_alt_ksym, 
    ClientExecute* client_execute,
    ModVncVariables vars,
    VNCVerbose verbose,
    VNCMetrics * metrics
);
