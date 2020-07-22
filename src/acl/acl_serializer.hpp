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

#include "core/back_event_t.hpp"
#include "acl/auth_api.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/timebase.hpp"
#include "acl/session_logfile.hpp"

#include <string>
#include <chrono>

#include <ctime>

class Inifile;
class ModuleManager;
class AuthApi;
class ModWrapper;

class AclSerializer final
{
public:
    Inifile & ini;
    Transport * auth_trans;

private:
    char session_id[256];

private:
public:
    std::string session_type;
    bool remote_answer;       // false initialy, set to true once response is
                              // received from acl and asked_remote_answer is
                              // set to false

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        variable = 0x2,
        buffer   = 0x40,
        state    = 0x10,
        arcsight  = 0x20,
    };

    AclSerializer(Inifile & ini);
    ~AclSerializer();

    void disconnect() {
        if (this->auth_trans){
            this->auth_trans->disconnect();
            this->auth_trans = nullptr;
        }
    }

    void set_auth_trans(Transport * auth_trans) { this->auth_trans = auth_trans; }
    void in_items();
    void incoming();
    void send_acl_data();
};

