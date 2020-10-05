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

#include "utils/verbose_flags.hpp"
#include "transport/transport.hpp"

#include <string>
#include <functional>


class Inifile;
class Transport;

class AclSerializer final
{
public:
    enum class State
    {
        not_yet_connected,
        connected,
        connection_failed,
        disconnected_by_redemption,
        disconnected_by_authentifier
    };

    std::function<void()> on_inactivity_timeout = []{};

    Inifile & ini;
    Transport * auth_trans;
    std::string acl_manager_disconnect_reason;
    State acl_status = State::not_yet_connected;

private:
    char session_id[256];

public:
    // TODO not used
    bool remote_answer;       // false initialy, set to true once response is
                              // received from acl and asked_remote_answer is
                              // set to false

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        variable = 0x02,
        buffer   = 0x40,
    };

    AclSerializer(Inifile & ini);
    ~AclSerializer();

    void disconnect()
    {
        if (this->acl_status == State::connected) {
            this->acl_status = State::disconnected_by_redemption;
        }

        // If connexion was cut by authentifier, we also want to call disconnect on transport
        if (this->auth_trans) {
            this->auth_trans->disconnect();
            this->auth_trans = nullptr;
        }
    }

    // Set an already opened Transport to ACL : after calling this method AclSerializer is connected
    void set_auth_trans(Transport * auth_trans)
    {
        this->auth_trans = auth_trans;
        this->acl_status = State::connected;
    }

    void set_failed_auth_trans()
    {
        this->acl_status = State::connection_failed;
    }

    bool is_connexion_failed() const
    {
        return this->acl_status == State::connection_failed;
    }

    bool is_before_connexion() const
    {
        return this->acl_status == State::not_yet_connected;
    }

    bool is_after_connexion() const
    {
        return this->acl_status == State::disconnected_by_authentifier
            || this->acl_status == State::disconnected_by_redemption;
    }

    bool is_connected() const
    {
        return acl_status == State::connected;
    }

    void incoming();
    void send_acl_data();

private:
    void in_items();
};
