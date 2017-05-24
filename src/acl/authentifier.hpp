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
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan, Jennifer Inthavong

  Session related with ACL
  find out the next module to run from context reading
*/


#pragma once

#include "utils/log.hpp"
#include "configs/config.hpp"
#include "acl_serializer.hpp"

#include "utils/verbose_flags.hpp"


class Authentifier : public AuthApi, public ReportMessageApi
{
    struct LogParam
    {
        bool duplicate_with_pid;
        std::string type;
        std::string extra;
    };

    bool session_log_is_open = false;
    std::vector<LogParam> buffered_log_params;

    Inifile & ini;
    CryptoContext & cctx;

public:
    bool connected_to_acl;
    AclSerializer * acl_serial;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    Authentifier(Inifile & ini, CryptoContext & cctx, Verbose verbose)
        : ini(ini)
        , cctx(cctx)
        , connected_to_acl(false)
        , acl_serial(nullptr)
        , verbose(verbose)
    {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::Authentifier");
        }
    }

    void set_acl_serial(AclSerializer * acl_serial)
    {
        this->connected_to_acl = true;
        this->acl_serial = acl_serial;
    }

    ~Authentifier() override {
        if (bool(this->verbose & Verbose::state)) {
            LOG(LOG_INFO, "auth::~Authentifier");
        }
    }

    void receive() {
        if (this->connected_to_acl){
            this->acl_serial->receive();
        }
    }

    void set_auth_channel_target(const char * target) override {
        if (this->connected_to_acl){
            this->acl_serial->ini.set_acl<cfg::context::auth_channel_target>(target);
        }
    }

    void set_auth_error_message(const char * error_message) override {
        if (this->connected_to_acl){
            this->acl_serial->ini.set<cfg::context::auth_error_message>(error_message);
        }
    }

    void report(const char * reason, const char * message) override {
        if (this->connected_to_acl){
            this->acl_serial->report(reason, message);
        }
    }

    void disconnect_target() override {
        if (this->connected_to_acl){
            this->acl_serial->ini.set_acl<cfg::context::module>(STRMODULE_CLOSE);
        }
    }

    void log4(bool duplicate_with_pid, const char * type, const char * extra = nullptr) override
    {
        if (this->connected_to_acl && this->session_log_is_open) {
            this->acl_serial->log4(duplicate_with_pid, type, extra);
        }
        else {
            this->buffered_log_params.push_back({duplicate_with_pid, type, extra ? extra : ""});
        }
    }

    void start_mod() override
    {
        if (this->connected_to_acl){
            if (this->session_log_is_open) {
                this->acl_serial->close_session_log();
            }
            else {
                cctx.set_master_key(ini.get<cfg::crypto::key0>());
                cctx.set_hmac_key(ini.get<cfg::crypto::key1>());
            }

            for (LogParam const & log_param : this->buffered_log_params) {
                this->acl_serial->log4(
                    log_param.duplicate_with_pid,
                    log_param.type.c_str(),
                    log_param.extra.c_str()
                );
            }
            this->buffered_log_params.clear();
            this->session_log_is_open = true;
        }
    }

    void stop_mod() override
    {
        /*do nothing*/
    }
};
