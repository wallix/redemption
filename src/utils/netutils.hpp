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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Network related utility functions

*/

#pragma once

#include "utils/sugar/unique_fd.hpp"

#include <cstdint>


class in_addr;

bool try_again(int errnum);

/// std::expected
/// \return nullptr if ok, view string if error
char const* resolve_ipv4_address(const char* ip, in_addr & s4_sin_addr);

unique_fd ip_connect(const char* ip, int port, int nbretry /* 3 */, int retry_delai_ms /*1000*/, char const** error_result = nullptr);

// TODO int retry_delai_ms -> std::milliseconds
unique_fd local_connect(const char* sck_name, int nbretry = 3, int retry_delai_ms = 1000);

int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport,
                       char * transparent_dest, size_t sz_transparent_dest, uint32_t verbose = 0);
