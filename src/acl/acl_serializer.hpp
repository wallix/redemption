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


#pragma once

#include "acl/acl_field_mask.hpp"
#include "utils/verbose_flags.hpp"

class Inifile;
class Transport;

class AclSerializer final
{
public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        variable = 0x0002,
        buffer   = 0x0040,
        dump     = 0x1000,
    };

    AclSerializer(Inifile & ini, Transport & auth_trans);
    ~AclSerializer();

    AclFieldMask incoming();
    std::size_t send_acl_data();

private:
    Inifile & ini;
    Transport & auth_trans;
};
