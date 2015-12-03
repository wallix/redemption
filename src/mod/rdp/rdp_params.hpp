/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_MOD_RDP_RDP_PARAMS_HPP_
#define _REDEMPTION_MOD_RDP_RDP_PARAMS_HPP_

#include "log.hpp"
#include "translation.hpp"

#include <string>

class Transport;
class auth_api;

struct ModRDPParams {
    const char * target_user;
    const char * target_password;
    const char * target_host;
    const char * client_address;

    const char * client_name;

    bool enable_tls;
    bool enable_nla;
    bool enable_krb;
    bool enable_fastpath;           // If true, fast-path must be supported.
    bool enable_mem3blt;
    bool enable_bitmap_update;
    bool enable_new_pointer;
    bool enable_glyph_cache;
    bool enable_session_probe;
    bool enable_session_probe_loading_mask;

    bool disable_clipboard_log_syslog;
    bool disable_clipboard_log_wrm;
    bool disable_file_system_log_syslog;
    bool disable_file_system_log_wrm;

    unsigned     session_probe_launch_timeout;
    unsigned     session_probe_on_launch_failure;
    unsigned     session_probe_keepalive_timeout;
    const char * session_probe_alternate_shell;

    bool         enable_transparent_mode;
    const char * output_filename;
    Transport  * persistent_key_list_transport;
    Transport  * transparent_recorder_transport;

    int key_flags;

    auth_api * acl;

    const char * outbound_connection_blocking_rules;

    bool         ignore_auth_channel;
    const char * auth_channel;

    const char * alternate_shell;
    const char * shell_working_directory;
    const char * target_application_account;
    const char * target_application_password;

    int rdp_compression;

    std::string * error_message;
    bool          disconnect_on_logon_user_change;
    uint32_t      open_session_timeout;

    bool                        server_cert_store;
    configs::ServerCertCheck    server_cert_check;
    configs::ServerNotification server_access_allowed_message;
    configs::ServerNotification server_cert_create_message;
    configs::ServerNotification server_cert_success_message;
    configs::ServerNotification server_cert_failure_message;
    configs::ServerNotification server_cert_error_message;

    const char * device_id;

    const char * extra_orders;

    bool enable_persistent_disk_bitmap_cache;
    bool enable_cache_waiting_list;
    bool persist_bitmap_cache_on_disk;

    uint32_t password_printing_mode;

    const std::string * allow_channels;
    const std::string * deny_channels;

    bool remote_program;
    bool server_redirection_support;

    bool bogus_sc_net_size;

    unsigned client_device_announce_timeout;

    const char * proxy_managed_drives;

    Translation::language_t lang;

    uint32_t verbose;
    uint32_t cache_verbose;

    ModRDPParams( const char * target_user
                , const char * target_password
                , const char * target_host
                , const char * client_address
                , int key_flags
                , uint32_t verbose = 0
                )
        : target_user(target_user)
        , target_password(target_password)
        , target_host(target_host)
        , client_address(client_address)

        , client_name(nullptr)

        , enable_tls(true)
        , enable_nla(true)
        , enable_krb(false)
        , enable_fastpath(true)
        , enable_mem3blt(true)
        , enable_bitmap_update(false)
        , enable_new_pointer(true)
        , enable_glyph_cache(false)
        , enable_session_probe(false)
        , enable_session_probe_loading_mask(true)

        , disable_clipboard_log_syslog(false)
        , disable_clipboard_log_wrm(false)

        , disable_file_system_log_syslog(false)
        , disable_file_system_log_wrm(false)

        , session_probe_launch_timeout(0)
        , session_probe_on_launch_failure(0)
        , session_probe_keepalive_timeout(0)
        , session_probe_alternate_shell("")

        , enable_transparent_mode(false)
        , output_filename("")
        , persistent_key_list_transport(nullptr)
        , transparent_recorder_transport(nullptr)

        , key_flags(key_flags)

        , acl(nullptr)

        , outbound_connection_blocking_rules(nullptr)

        , ignore_auth_channel(false)
        , auth_channel("")

        , alternate_shell("")
        , shell_working_directory("")
        , target_application_account("")
        , target_application_password("")

        , rdp_compression(0)

        , error_message(nullptr)
        , disconnect_on_logon_user_change(false)
        , open_session_timeout(0)

        , server_cert_check(configs::ServerCertCheck::fails_if_no_match_and_succeed_if_no_know)
        , server_access_allowed_message(configs::ServerNotification::syslog)
        , server_cert_create_message(configs::ServerNotification::syslog)
        , server_cert_success_message(configs::ServerNotification::syslog)
        , server_cert_failure_message(configs::ServerNotification::syslog)
        , server_cert_error_message(configs::ServerNotification::syslog)

        , device_id("")
        , extra_orders("")

        , enable_persistent_disk_bitmap_cache(false)
        , enable_cache_waiting_list(false)
        , persist_bitmap_cache_on_disk(false)

        , password_printing_mode(0)

        , allow_channels(nullptr)
        , deny_channels(nullptr)

        , remote_program(false)
        , server_redirection_support(false)

        , bogus_sc_net_size(true)

        , client_device_announce_timeout(1000)

        , proxy_managed_drives("")

        , lang(Translation::EN)

        , verbose(verbose)
        , cache_verbose(0)
    {}

    ~ModRDPParams()
    {
        TODO("Making it a unique_ptr would avoid need for delete");
    }

    void log() const {
        LOG(LOG_INFO,
            "ModRDPParams target_user=\"%s\"",                     this->target_user);
        LOG(LOG_INFO,
            "ModRDPParams target_password=\"%s\"",                 (this->target_password ? "<hidden>" : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams target_host=\"%s\"",                     this->target_host);
        LOG(LOG_INFO,
            "ModRDPParams client_address=\"%s\"",                  this->client_address);

        LOG(LOG_INFO,
            "ModRDPParams client_name=\"%s\"",                     (this->client_name ? this->client_name : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams enable_tls=%s",                          (this->enable_tls ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_nla=%s",                          (this->enable_nla ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_krb=%s",                          (this->enable_krb ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_fastpath=%s",                     (this->enable_fastpath ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_mem3blt=%s",                      (this->enable_mem3blt ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_bitmap_update=%s",                (this->enable_bitmap_update ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_new_pointer=%s",                  (this->enable_new_pointer ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_glyph_cache=%s",                  (this->enable_glyph_cache ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_session_probe=%s",                (this->enable_session_probe ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_session_probe_loading_mask=%s",   (this->enable_session_probe_loading_mask ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams session_probe_launch_timeout=%u",        this->session_probe_launch_timeout);
        LOG(LOG_INFO,
            "ModRDPParams session_probe_on_launch_failure=%u",     this->session_probe_on_launch_failure);
        LOG(LOG_INFO,
            "ModRDPParams session_probe_keepalive_timeout=%u",     this->session_probe_keepalive_timeout);

        LOG(LOG_INFO,
            "ModRDPParams dsiable_clipboard_log_syslog=%s",        this->disable_clipboard_log_syslog ? "yes" : "no");
        LOG(LOG_INFO,
            "ModRDPParams dsiable_clipboard_log_wrm=%s",           this->disable_clipboard_log_wrm ? "yes" : "no");

        LOG(LOG_INFO,
            "ModRDPParams dsiable_file_system_log_syslog=%s",      this->disable_file_system_log_syslog ? "yes" : "no");
        LOG(LOG_INFO,
            "ModRDPParams dsiable_file_system_log_wrm=%s",         this->disable_file_system_log_wrm ? "yes" : "no");

        LOG(LOG_INFO,
            "ModRDPParams enable_transparent_mode=%s",             (this->enable_transparent_mode ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams output_filename=\"%s\"",                 (this->output_filename ? this->output_filename : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams persistent_key_list_transport=<%p>",     static_cast<void*>(this->persistent_key_list_transport));
        LOG(LOG_INFO,
            "ModRDPParams transparent_recorder_transport=<%p>",    static_cast<void*>(this->transparent_recorder_transport));

        LOG(LOG_INFO,
            "ModRDPParams key_flags=%d",                           this->key_flags);

        LOG(LOG_INFO,
            "ModRDPParams acl=<%p>",                               static_cast<void*>(this->acl));

        LOG(LOG_INFO,
            "ModRDPParams outbound_connection_blocking_rules=\"%s\"",
                                                                   (this->outbound_connection_blocking_rules ?
                                                                    this->outbound_connection_blocking_rules : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams ignore_auth_channel=%s",                 (this->ignore_auth_channel ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams auth_channel=\"%s\"",                    (this->auth_channel ? this->auth_channel : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams alternate_shell=\"%s\"",                 (this->alternate_shell ? this->alternate_shell : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams shell_working_directory=\"%s\"",         (this->shell_working_directory ? this->shell_working_directory : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams target_application_account=\"%s\"",      (this->target_application_account ? this->target_application_account : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams target_application_password=\"%s\"",     (this->target_application_password ? "<hidden>" : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams rdp_compression=%d",                     this->rdp_compression);

        LOG(LOG_INFO,
            "ModRDPParams error_message=<%p>",                     static_cast<void*>(this->error_message));
        LOG(LOG_INFO,
            "ModRDPParams disconnect_on_logon_user_change=%s",     (this->disconnect_on_logon_user_change ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams open_session_timeout=%d",                this->open_session_timeout);

        LOG(LOG_INFO,
            "ModRDPParams server_cert_check=%d",                   static_cast<unsigned>(this->server_cert_check));
        LOG(LOG_INFO,
            "ModRDPParams server_access_allowed_message=%d",       static_cast<unsigned>(this->server_access_allowed_message));
        LOG(LOG_INFO,
            "ModRDPParams server_cert_create_message=%d",          static_cast<unsigned>(this->server_cert_create_message));
        LOG(LOG_INFO,
            "ModRDPParams server_cert_success_message=%d",         static_cast<unsigned>(this->server_cert_success_message));
        LOG(LOG_INFO,
            "ModRDPParams server_cert_failure_message=%d",         static_cast<unsigned>(this->server_cert_failure_message));
        LOG(LOG_INFO,
            "ModRDPParams server_cert_error_message=%d",           static_cast<unsigned>(this->server_cert_error_message));

        LOG(LOG_INFO, "ModRDPParams extra_orders=%s",              (this->extra_orders ? this->extra_orders : "<none>"));

        LOG(LOG_INFO,
            "ModRDPParams enable_persistent_disk_bitmap_cache=%s", (this->enable_persistent_disk_bitmap_cache ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams enable_cache_waiting_list=%s",           (this->enable_cache_waiting_list ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams persist_bitmap_cache_on_disk=%s",        (this->persist_bitmap_cache_on_disk ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams password_printing_mode=%u",              this->password_printing_mode);

        LOG(LOG_INFO,
            "ModRDPParams allow_channels=%s",                      (this->allow_channels ? this->allow_channels->c_str() : "<none>"));
        LOG(LOG_INFO,
            "ModRDPParams deny_channels=%s",                       (this->deny_channels ? this->deny_channels->c_str() : "<none>"));

        LOG(LOG_INFO,
            "ModRDPParams remote_program=%s",                      (this->remote_program ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams server_redirection_support=%s",          (this->server_redirection_support ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams bogus_sc_net_size=%s",                   (this->bogus_sc_net_size ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams client_device_announce_timeout=%u",      this->client_device_announce_timeout);

        LOG(LOG_INFO, "ModRDPParams proxy_managed_drives=%s",      (this->proxy_managed_drives ? this->proxy_managed_drives : "<none>"));

        LOG(LOG_INFO, "ModRDPParams lang=%s",                      ((this->lang == Translation::EN) ? "EN" : ((this->lang == Translation::FR) ? "FR" : "<unknown>")));

        LOG(LOG_INFO,
            "ModRDPParams verbose=0x%08X",                         this->verbose);
        LOG(LOG_INFO,
            "ModRDPParams cache_verbose=0x%08X",                   this->cache_verbose);
    }   // void log() const
};  // struct ModRDPParams

#endif  // #ifndef _REDEMPTION_MOD_RDP_RDP_PARAMS_HPP_
