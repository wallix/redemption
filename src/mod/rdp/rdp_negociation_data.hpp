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
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#pragma once

#include <cstdint>

struct RdpNegociationResult
{
    uint16_t front_width = 0;
    uint16_t front_height = 0;
    bool use_rdp5 = true;
    uint16_t userid = 0;
    int encryptionLevel = 0;
    int encryptionMethod = 0;
};

struct RdpLogonInfo
{
    RdpLogonInfo(char const* hostname, bool hide_client_name, char const* target_user) noexcept;

    char const* username()  const noexcept { return this->_username; }
    char const* domain()    const noexcept { return this->_domain; }
    char const* hostname()  const noexcept { return this->_hostname; }

private:
    char _username[128] = {};
    char _domain[256] = {};
    char _hostname[HOST_NAME_MAX + 1] = {};
};
