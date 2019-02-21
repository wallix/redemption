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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include "core/front_api.hpp"
#include "core/RDP/channels/rdpdr_completion_id_manager.hpp"
#include "core/session_reactor.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "system/linux/system/ssl_sha256.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/strutils.hpp"

#include <deque>



