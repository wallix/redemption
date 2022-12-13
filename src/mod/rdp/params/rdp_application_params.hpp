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

#include "mod/rdp/channels/sespro_channel_params.hpp"
#include "utils/sugar/array_view.hpp"

#include <string>

struct ApplicationParams
{
    chars_view primary_user_id = ""_av;
    chars_view target_application = ""_av;

    // Application Bastion
    std::string alternate_shell;
    std::string shell_arguments;
    chars_view shell_working_dir = ""_av;

    bool use_client_provided_alternate_shell = false;

    chars_view target_application_account = ""_av;
    chars_view target_application_password = ""_av;

    time_t shadow_invite_time = 0;
};
