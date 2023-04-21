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
   Copyright (C) Wallix 2010-2017
   Author(s): Christophe Grosjean, Raphael Zhou,
*/

#pragma once

#include "utils/ascii.hpp"

#define DUMMY_REMOTEAPP "||WABRemoteApp"

inline bool is_dummy_remote_app(chars_view exe_or_file)
{
    auto dummy_remoteapp_upper = DUMMY_REMOTEAPP ""_ascii_upper;

    return exe_or_file.size() >= dummy_remoteapp_upper.size()
        && insensitive_eq(exe_or_file.first(dummy_remoteapp_upper.size()), dummy_remoteapp_upper)
        && (exe_or_file.size() == dummy_remoteapp_upper.size() || exe_or_file[dummy_remoteapp_upper.size()] == ':');
}
