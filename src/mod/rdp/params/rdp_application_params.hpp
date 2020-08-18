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

#include <string>

struct ApplicationParams
{
    const char * primary_user_id = "";
    const char * target_application = "";

    // Application Bastion
    const char * alternate_shell = "";
    const char * shell_arguments = "";
    const char * shell_working_dir = "";

    bool use_client_provided_alternate_shell = false;

    const char * target_application_account = "";
    const char * target_application_password = "";
};

