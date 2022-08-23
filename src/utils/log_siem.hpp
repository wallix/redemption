/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Jonathan Poelen
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   log file including syslog
*/

#pragma once

#include <string_view>


namespace log_siem
{
    void set_psid(std::string_view psid) noexcept;
    void incoming_connection(std::string_view source_ip, int source_port) noexcept;
    void set_user(std::string_view username) noexcept;
    void target_connection(char const* target_user, char const* session_id,
                           char const* target_host, unsigned target_port) noexcept;
    void target_connection_failed(char const* target_user, char const* session_id,
                                  char const* target_host, unsigned target_port,
                                  char const* reason) noexcept;
    void target_disconnection(char const* reason, char const* session_id) noexcept;
    void disconnection(char const* reason) noexcept;
} // namespace log_siem
