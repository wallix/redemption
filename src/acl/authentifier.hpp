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

#include "acl/acl_serializer.hpp"
#include "acl/auth_api.hpp"
#include "acl/module_manager/enums.hpp"
#include "configs/config.hpp"
#include "utils/log.hpp"
#include "utils/difftimeval.hpp"

#include "utils/verbose_flags.hpp"


class Authentifier : public AuthApi, public ReportMessageApi
{
    struct LogParam
    {
        LogId id;
        struct KV
        {
            std::string key;
            std::string value;
        };
        std::vector<KV> kv_list;

        LogParam(LogId id, KVList list)
        : id(id)
        {
            kv_list.reserve(list.size());
            for (auto const& kv : list) {
                this->kv_list.emplace_back(KV{
                    std::string(kv.key.data(), kv.key.size()),
                    std::string(kv.value.data(), kv.value.size()),
                });
            }
        }
    };

    bool session_log_is_open = false;
    std::vector<LogParam> buffered_log_params;

    Inifile & ini;
    CryptoContext & cctx;

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
        , acl_serial(nullptr)
        , verbose(verbose)
    {
        LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "auth::Authentifier");
    }

    void set_acl_serial(AclSerializer * acl_serial)
    {
        this->acl_serial = acl_serial;
    }

    ~Authentifier() override {
        LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "auth::~Authentifier");
    }

//    void receive() {
//        if (this->acl_serial){
//            this->acl_serial->receive();
//        }
//    }

    void set_auth_channel_target(const char * target) override {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::auth_channel_target>(target);
        }
    }

    void set_auth_error_message(const char * error_message) override {
        if (this->acl_serial){
            this->acl_serial->ini.set<cfg::context::auth_error_message>(error_message);
        }
    }

    void report(const char * reason, const char * message) override {
        if (this->acl_serial){
            this->acl_serial->report(reason, message);
        }
    }

    void disconnect_target() override {
        LOG(LOG_INFO, "!!!!!!!=======!!!!!!!!!!========== DISCONNECT TARGET ======!!!!!!!!============= acl_serial=%s close_box=%s",
            this->acl_serial?"true":"false",
            ((this->acl_serial!=nullptr)&&this->acl_serial->ini.get<cfg::globals::enable_close_box>())?"true":"false");
        if (this->acl_serial &&
            // Call disconnect_target >>> Show Close Box (with back to selector)
            this->acl_serial->ini.get<cfg::globals::enable_close_box>()) {
            // TODO: STRMODULE_CLOSE
            LOG(LOG_INFO, "=================== sending close to ACL ==========");
            this->acl_serial->ini.set_acl<cfg::context::module>("close");
        }
    }

    void log6(LogId id, const timeval time, KVList kv_list) override
    {
        // TODO: should we delay logs sent to SIEM ?
        if (this->acl_serial && this->session_log_is_open) {
            this->acl_serial->log6(id, time, kv_list);
        }
        else {
            this->buffered_log_params.emplace_back(id, kv_list);
        }
    }

    void new_remote_mod() override
    {
        if (this->acl_serial){
            this->cctx.set_master_key(ini.get<cfg::crypto::key0>());
            this->cctx.set_hmac_key(ini.get<cfg::crypto::key1>());
            this->cctx.set_trace_type(ini.get<cfg::globals::trace_type>());

            this->acl_serial->start_session_log();
            this->session_log_is_open = true;

            if (!this->buffered_log_params.empty()) {
                std::vector<KVLog> v;
                for (LogParam const & log_param : this->buffered_log_params) {
                    v.reserve(log_param.kv_list.size());
                    for (auto const& kv : log_param.kv_list) {
                        v.emplace_back(KVLog{kv.key, kv.value});
                    }
                    this->acl_serial->log6(log_param.id, tvtime(), {v});
                    v.clear();
                }
                this->buffered_log_params.clear();
                this->buffered_log_params.shrink_to_fit();
            }
        }
    }

    void delete_remote_mod() override
    {
        if (this->acl_serial && this->session_log_is_open) {
            this->acl_serial->close_session_log();
            this->session_log_is_open = false;
        }
    }

    void set_pm_request(const char * request) override {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::pm_request>(request);
        }
    }

    void set_native_session_id(unsigned int session_id) override {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::native_session_id>(session_id);
        }
    }

    void rd_shadow_available() override
    {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_available>(true);
        }
    }

    void rd_shadow_invitation(uint32_t error_code, const char * error_message, const char * userdata, const char * id, const char * addr, uint16_t port) override
    {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_invitation_error_code>(error_code);
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_invitation_error_message>(error_message);
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_userdata>(userdata);
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_invitation_id>(id);
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_invitation_addr>(addr);
            this->acl_serial->ini.set_acl<cfg::context::rd_shadow_invitation_port>(port);
        }
    }

    void set_smartcard_login(const char * login) override
    {
        if (this->acl_serial){
            this->acl_serial->ini.set_acl<cfg::context::smartcard_login>(login);
        }
    }
};
