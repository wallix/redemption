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

struct ModRDPParams {
    const char * target_user;
    const char * target_password;
    const char * target_device;
    const char * client_address;

    bool enable_tls;
    bool enable_nla;
    bool enable_krb;
    bool enable_clipboard;
    bool enable_fastpath;           // If true, fast-path must be supported.
    bool enable_mem3blt;
    bool enable_bitmap_update;
    bool enable_new_pointer;

    bool         enable_transparent_mode;
    const char * output_filename;

    int      key_flags;

    auth_api * acl;

    const char * auth_channel;
    const char * alternate_shell;
    const char * shell_working_directory;

    int rdp_compression;

    redemption::string * error_message;
    bool                 disconnect_on_logon_user_change;
    uint32_t             open_session_timeout;

    unsigned certificate_change_action;

    const char * extra_orders;

    uint32_t verbose;

    ModRDPParams( const char * target_user
                , const char * target_password
                , const char * target_device
                , const char * client_address
                , int key_flags
                , uint32_t verbose = 0
                )
        : target_user(target_user)
        , target_password(target_password)
        , target_device(target_device)
        , client_address(client_address)

        , enable_tls(true)
        , enable_nla(true)
        , enable_krb(false)
        , enable_clipboard(true)
        , enable_fastpath(true)
        , enable_mem3blt(true)
        , enable_bitmap_update(false)
        , enable_new_pointer(true)

        , enable_transparent_mode(false)
        , output_filename("")

        , key_flags(key_flags)

        , acl(NULL)

        , auth_channel("")
        , alternate_shell("")
        , shell_working_directory("")

        , rdp_compression(0)

        , error_message(NULL)
        , disconnect_on_logon_user_change(false)
        , open_session_timeout(0)

        , certificate_change_action(0)

        , extra_orders("")

        , verbose(verbose)
    {}

    void log() {
        LOG(LOG_INFO,
            "ModRDPParams : target_user=\"%s\"",                 target_user);
        LOG(LOG_INFO,
            "ModRDPParams : target_password=\"%s\"",             target_password);
        LOG(LOG_INFO,
            "ModRDPParams : target_device=\"%s\"",               target_device);
        LOG(LOG_INFO,
            "ModRDPParams : client_address=\"%s\"",              client_address);

        LOG(LOG_INFO,
            "ModRDPParams : enable_tls=%s   ",                   (enable_tls ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_nla=%s   ",                   (enable_nla ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_krb=%s   ",                   (enable_krb ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_clipboard=%s   ",             (enable_clipboard ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_fastpath=%s   ",              (enable_fastpath ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_mem3blt=%s   ",               (enable_mem3blt ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_bitmap_update=%s   ",         (enable_bitmap_update ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : enable_new_pointer=%s   ",           (enable_new_pointer ? "yes" : "no"));

        LOG(LOG_INFO,
            "ModRDPParams : enable_transparent_mode=%s   ",      (enable_transparent_mode ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : output_filename=%s   ",              (output_filename ? output_filename : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams : key_flags=%d",                       key_flags);

        LOG(LOG_INFO,
            "ModRDPParams : acl=<0x%p>",                         acl);

        LOG(LOG_INFO,
            "ModRDPParams : auth_channel=\"%s\"",                (auth_channel ? auth_channel : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams : alternate_shell=\"%s\"",             (alternate_shell ? alternate_shell : "<null>"));
        LOG(LOG_INFO,
            "ModRDPParams : shell_working_directory=\"%s\"",     (shell_working_directory ? shell_working_directory : "<null>"));

        LOG(LOG_INFO,
            "ModRDPParams : rdp_compression=%d",                 rdp_compression);

        LOG(LOG_INFO,
            "ModRDPParams : error_message=<0x%p>",               error_message);
        LOG(LOG_INFO,
            "ModRDPParams : disconnect_on_logon_user_change=%s", (disconnect_on_logon_user_change ? "yes" : "no"));
        LOG(LOG_INFO,
            "ModRDPParams : open_session_timeout=%d",            open_session_timeout);

        LOG(LOG_INFO,
            "ModRDPParams : certificate_change_action=%d",       certificate_change_action);

        LOG(LOG_INFO,
            "ModRDPParams : extra_orders=%s",                    (extra_orders ? extra_orders : "<none>"));

        LOG(LOG_INFO,
            "ModRDPParams : verbose=0x%08X",                     verbose);
    }   // void log()
};  // struct ModRDPParams

#endif  // #ifndef _REDEMPTION_MOD_RDP_RDP_PARAMS_HPP_