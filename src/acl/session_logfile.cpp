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
  Author(s): Christophe Grosjean, Meng Tauth_rail_exec_an, Jennifer Inthavong

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/

#include "acl/acl_serializer.hpp"
#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/log_id.hpp"
#include "core/report_message_api.hpp"
#include "main/version.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "std17/charconv.hpp"
#include "utils/fileutils.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/log.hpp"
#include "utils/log_siem.hpp"
#include "utils/stream.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "core/set_server_redirection_target.hpp"

#include <string>
#include <algorithm>

#include <ctime>
#include <cstdio>
#include <cassert>

