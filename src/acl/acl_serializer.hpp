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

class KeepAlive
{
    // Keep alive Variables
    int  grace_delay;
    long timeout;
    long renew_time;
    bool wait_answer;     // true when we are waiting for a positive response
                          // false when positive response has been received and
                          // timers have been set to new timers.
    bool connected;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state    = 0x10,
        arcsight = 0x20,

        // SocketTransport (see 'socket_transport.hpp')
        sock_basic = 1u << 29,
        sock_dump  = 1u << 30,
        sock_watch = 1u << 31
    };

    KeepAlive(std::chrono::seconds grace_delay_, Verbose verbose);

    ~KeepAlive();

    bool is_started();

    void start(time_t now);

    void stop();

    bool check(time_t now, Inifile & ini);
};


class Inactivity
{
    // Inactivity management
    // let t be the timeout of the blocking select in session loop,
    // the effective inactivity timeout detection will be between
    // inactivity_timeout and inactivity_timeout + t.
    // hence we should have t << inactivity_timeout.
    time_t inactivity_timeout;
    time_t last_activity_time;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    Inactivity(std::chrono::seconds timeout, time_t start, Verbose verbose);

    ~Inactivity();

    bool check_user_activity(time_t now, bool & has_user_activity);

    void update_inactivity_timeout(time_t inactivity_timeout);

    time_t get_inactivity_timeout();
};


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

