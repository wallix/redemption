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
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#ifndef _REDEMPTION_ACL_MODULES_MANAGER_HPP_
#define _REDEMPTION_ACL_MODULES_MANAGER_HPP_

#include "sockettransport.hpp"
#include "config.hpp"
#include "netutils.hpp"
#include "mod_api.hpp"
#include "auth_api.hpp"
#include "null/null.hpp"
#include "rdp/rdp.hpp"
#include "vnc/vnc.hpp"
#include "xup/xup.hpp"
#include "internal/bouncer2_mod.hpp"
#include "internal/test_card_mod.hpp"
#include "internal/replay_mod.hpp"
#include "internal/rwl_mod.hpp"
#include "front.hpp"
#include "translation.hpp"

#include "internal/flat_login_mod.hpp"
#include "internal/flat_selector_mod.hpp"
#include "internal/flat_wab_close_mod.hpp"
#include "internal/flat_dialog_mod.hpp"

#define STRMODULE_LOGIN       "login"
#define STRMODULE_SELECTOR    "selector"
#define STRMODULE_CONFIRM     "confirm"
#define STRMODULE_CHALLENGE   "challenge"
#define STRMODULE_VALID       "valid"
#define STRMODULE_TRANSITORY  "transitory"
#define STRMODULE_CLOSE       "close"
#define STRMODULE_CONNECTION  "connection"
#define STRMODULE_MESSAGE     "message"
#define STRMODULE_RDP         "RDP"
#define STRMODULE_VNC         "VNC"
#define STRMODULE_INTERNAL    "INTERNAL"

enum {
    MODULE_EXIT,
    MODULE_WAITING,
    MODULE_RUNNING,
    MODULE_REFRESH,
    MODULE_VNC,
    MODULE_RDP,
    MODULE_XUP,
    MODULE_INTERNAL,
    MODULE_INTERNAL_CLOSE,
    MODULE_INTERNAL_WIDGET2_DIALOG,
    MODULE_INTERNAL_WIDGET2_MESSAGE,
    MODULE_INTERNAL_WIDGET2_LOGIN,
    MODULE_INTERNAL_WIDGET2_RWL,
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
    MODULE_INTERNAL_DIALOG_CHALLENGE,
    MODULE_INTERNAL_BOUNCER2,
    MODULE_INTERNAL_TEST,
    MODULE_INTERNAL_WIDGET2_SELECTOR,
    MODULE_EXIT_INTERNAL_CLOSE,
    MODULE_TRANSITORY,
    MODULE_AUTH,
    MODULE_CLI,
};

class MMApi
{
public:

    mod_api * mod;

    bool last_module;
    bool connected;

    MMApi() : mod(NULL)
        , last_module(false)
        , connected(false) {}
    virtual ~MMApi() {}
    virtual void remove_mod() = 0;
    virtual void new_mod(int target_module, time_t now, auth_api * acl) = 0;
    virtual int next_module() = 0;
    // virtual int get_mod_from_protocol() = 0;
    virtual void invoke_close_box(const char * auth_error_message,
                                  BackEvent_t & signal, time_t now) {
        this->last_module = true;
    };
    virtual bool is_last_module() {
        return this->last_module;
    }
    virtual bool is_connected() {
        return this->connected;
    }
    virtual bool is_up_and_running() {
        bool res = false;
        if (this->mod) {
            res = this->mod->is_up_and_running();
        }
        return res;
    }
    virtual void record(auth_api * acl) {}
};

class MMIni : public MMApi {
public:
    Inifile & ini;
    uint32_t verbose;
    MMIni(Inifile & _ini) : ini(_ini)
                          , verbose(ini.debug.auth) {}
    virtual ~MMIni() {}
    virtual void remove_mod() {};
    virtual void new_mod(int target_module, time_t now, auth_api * acl) {
        LOG(LOG_INFO, "new mod %d at time: %d\n", static_cast<int>(target_module), static_cast<int>(now));
        switch (target_module) {
        case MODULE_VNC:
        case MODULE_XUP:
        case MODULE_RDP:
            this->connected = true;
            break;
        default:
            break;
        };
    };

    virtual void invoke_close_box(const char * auth_error_message,
                                  BackEvent_t & signal, time_t now) {
        this->last_module = true;
        if (auth_error_message) {
            this->ini.context.auth_error_message.copy_c_str(auth_error_message);
        }
        this->remove_mod();
        if (this->ini.globals.enable_close_box) {
            this->new_mod(MODULE_INTERNAL_CLOSE, now, NULL);
            signal = BACK_EVENT_NONE;
        }
        else {
            signal = BACK_EVENT_STOP;
        }
    }

    int next_module() {
        LOG(LOG_INFO, "----------> ACL next_module <--------");
        const char * module_cstr = this->ini.context.module.get_cstr();

        if (this->connected &&
            (!strcmp(module_cstr, STRMODULE_RDP) ||
             !strcmp(module_cstr, STRMODULE_VNC))) {
            LOG(LOG_INFO, "===========> MODULE_CLOSE");
            if (this->ini.context.auth_error_message.is_empty()) {
                this->ini.context.auth_error_message.copy_c_str(TR("end_connection",
                                                                   this->ini));
            }
            return MODULE_INTERNAL_CLOSE;
        }
        if (!strcmp(module_cstr, STRMODULE_LOGIN)) {
            LOG(LOG_INFO, "===========> MODULE_LOGIN");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        else if (!strcmp(module_cstr, STRMODULE_SELECTOR)) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR");
            return MODULE_INTERNAL_WIDGET2_SELECTOR;
        }
        else if (!strcmp(module_cstr, STRMODULE_CONFIRM)) {
            LOG(LOG_INFO, "===============> MODULE_DIALOG_CONFIRM");
            return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
        }
        else if (!strcmp(module_cstr, STRMODULE_CHALLENGE)) {
            LOG(LOG_INFO, "===========> MODULE_DIALOG_CHALLENGE");
            return MODULE_INTERNAL_DIALOG_CHALLENGE;
        }
        else if (!strcmp(module_cstr, STRMODULE_VALID)) {
            LOG(LOG_INFO, "===========> MODULE_DIALOG_VALID");
            return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
        }
        else if (!strcmp(module_cstr, STRMODULE_TRANSITORY)) {
            LOG(LOG_INFO, "===============> WAIT WITH CURRENT MODULE");
            return MODULE_TRANSITORY;
        }
        else if (!strcmp(module_cstr, STRMODULE_CLOSE)) {
            LOG(LOG_INFO, "===========> MODULE_INTERNAL_CLOSE");
            return MODULE_INTERNAL_CLOSE;
        }
        else if (!strcmp(module_cstr, STRMODULE_RDP)) {
            LOG(LOG_INFO, "===========> MODULE_RDP");
            return MODULE_RDP;
        }
        else if (!strcmp(module_cstr, STRMODULE_VNC)) {
            LOG(LOG_INFO, "===========> MODULE_VNC");
            return MODULE_VNC;
        }
        else if (!strcmp(module_cstr, STRMODULE_INTERNAL)) {
            LOG(LOG_INFO, "===========> MODULE_INTERNAL");
            int res = MODULE_EXIT;
            const char * target = this->ini.globals.target_device.get_cstr();
            if (0 == strcmp(target, "bouncer2")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "==========> INTERNAL bouncer2");
                }
                res = MODULE_INTERNAL_BOUNCER2;
            }
            else if (0 == strncmp(target, "autotest", 8)) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "==========> INTERNAL test");
                }
                const char * user = this->ini.globals.target_user.get_cstr();
                size_t len_user = strlen(user);
                strncpy(this->ini.context.movie, user, sizeof(this->ini.context.movie));
                this->ini.context.movie[sizeof(this->ini.context.movie) - 1] = 0;
                if (0 != strcmp(".mwrm", user + len_user - 5)) {
                    strcpy(this->ini.context.movie + len_user, ".mwrm");
                }
                res = MODULE_INTERNAL_TEST;
            }
            else if (0 == strcmp(target, "widget2_message")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MODULE_INTERNAL_WIDGET2_MESSAGE;
            }
            else {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "==========> INTERNAL card");
                }
                res = MODULE_INTERNAL_CARD;
            }
            return res;
        }
        LOG(LOG_INFO, "===========> UNKNOWN MODULE");
        return MODULE_INTERNAL_CLOSE;
    }

};


class ModuleManager : public MMIni
{
public:
    Front & front;
    mod_api * no_mod;
    Transport * mod_transport;

    ModuleManager(Front & front, Inifile & ini)
        : MMIni(ini)
        , front(front)
        , mod_transport(NULL)
    {
        this->no_mod = new null_mod(this->front);
        this->no_mod->event.reset();
        this->mod = this->no_mod;
    }

    virtual void remove_mod()
    {
        if (this->mod != this->no_mod){
            delete this->mod;
            if (this->mod_transport) {
                delete this->mod_transport;
                this->mod_transport = NULL;
            }
            this->mod = this->no_mod;
        }
    }

    virtual ~ModuleManager()
    {
        this->remove_mod();
        delete this->no_mod;
    }

    virtual void new_mod(int target_module, time_t now, auth_api * acl)
    {
        LOG(LOG_INFO, "target_module=%u", target_module);
        if (this->last_module) this->front.stop_capture();
        switch (target_module)
            {
            case MODULE_INTERNAL_BOUNCER2:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'bouncer2_mod'");
                this->mod = new Bouncer2Mod(this->front,
                                            this->front.client_info.width,
                                            this->front.client_info.height
                                            );
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::internal module 'bouncer2_mod' ready");
                }
                break;
            case MODULE_INTERNAL_TEST:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test'");
                this->mod = new ReplayMod(
                                          this->front
                                          , this->ini.video.replay_path
                                          , this->ini.context.movie
                                          , this->front.client_info.width
                                          , this->front.client_info.height
                                          , this->ini.context.auth_error_message
                                          );
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::internal module 'test' ready");
                }
                break;
            case MODULE_INTERNAL_CARD:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test_card'");
                this->mod = new TestCardMod(this->front,
                                            this->front.client_info.width,
                                            this->front.client_info.height
                                            );
                LOG(LOG_INFO, "ModuleManager::internal module 'test_card' ready");
                break;
            case MODULE_INTERNAL_WIDGET2_SELECTOR:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'selector'");
                this->mod = new FlatSelectorMod(this->ini,
                            // new SelectorMod(this->ini,
                                            this->front,
                                            this->front.client_info.width,
                                            this->front.client_info.height
                                            );
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::internal module 'selector' ready");
                }
                break;
            case MODULE_INTERNAL_CLOSE:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'INTERNAL::Close'");
                    if (this->ini.context.auth_error_message.is_empty()) {
                        this->ini.context.auth_error_message.copy_c_str("Connection to server ended");
                    }
                    this->mod = new FlatWabCloseMod(this->ini,
                                // new WabCloseMod(this->ini,
                                                this->front,
                                                this->front.client_info.width,
                                                this->front.client_info.height,
                                                now,
                                                true
                                                );
                    this->front.init_pointers();
                }
                LOG(LOG_INFO, "ModuleManager::internal module Close ready");
                break;

            case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
            case MODULE_INTERNAL_WIDGET2_DIALOG:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Accept Message'");
                    const char * message = this->ini.context.message.get_cstr();
                    const char * button = this->ini.translation.button_refused.get_cstr();
                    const char * caption = "Information";
                    this->mod = new FlatDialogMod(
                                // new DialogMod(
                                              this->ini,
                                              this->front,
                                              this->front.client_info.width,
                                              this->front.client_info.height,
                                              caption,
                                              message,
                                              button,
                                              now);
                    LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Accept Message' ready");
                }
                break;
            case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
            case MODULE_INTERNAL_WIDGET2_MESSAGE:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Display Message'");
                    const char * message = this->ini.context.message.get_cstr();
                    const char * button = NULL;
                    const char * caption = "Information";
                    this->mod = new FlatDialogMod(
                                // new DialogMod(
                                              this->ini,
                                              this->front,
                                              this->front.client_info.width,
                                              this->front.client_info.height,
                                              caption,
                                              message,
                                              button,
                                              now);
                    LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Display Message' ready");
                }
                break;
            case MODULE_INTERNAL_DIALOG_CHALLENGE:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Challenge'");
                    const char * message = this->ini.context.message.get_cstr();
                    const char * button = NULL;
                    const char * caption = "Challenge";
                    ChallengeOpt challenge = CHALLENGE_HIDE;
                    if (this->ini.context_get_bool(AUTHID_AUTHENTICATION_CHALLENGE)) {
                        challenge = CHALLENGE_ECHO;
                    }
                    this->ini.context_ask(AUTHID_AUTHENTICATION_CHALLENGE);
                    this->ini.context_ask(AUTHID_PASSWORD);
                    this->mod = new FlatDialogMod(
                                                  this->ini,
                                                  this->front,
                                                  this->front.client_info.width,
                                                  this->front.client_info.height,
                                                  caption,
                                                  message,
                                                  button,
                                                  now,
                                                  challenge);
                    LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Challenge' ready");
                }

                break;
            case MODULE_INTERNAL_WIDGET2_LOGIN:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                if (this->ini.context_is_asked(AUTHID_TARGET_USER)
                    ||  this->ini.context_is_asked(AUTHID_TARGET_DEVICE)){
                    if (this->ini.context_is_asked(AUTHID_AUTH_USER)){
                        this->ini.account.username[0] = 0;
                    }
                    else {
                        strncpy(this->ini.account.username,
                                this->ini.globals.auth_user.get_cstr(),
                                // this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0),
                                sizeof(this->ini.account.username));
                        this->ini.account.username[sizeof(this->ini.account.username) - 1] = 0;
                    }
                }
                else if (this->ini.context_is_asked(AUTHID_AUTH_USER)) {
                    this->ini.account.username[0] = 0;
                }
                else {
                    TODO("check this! Assembling parts to get user login with target is not obvious"
                         "method used below il likely to show @: if target fields are empty")
                    char buffer[255];
                    snprintf( buffer, sizeof(buffer), "%s@%s:%s%s%s"
                            , this->ini.globals.target_user.get_cstr()
                            , this->ini.globals.target_device.get_cstr()
                            , this->ini.context.target_protocol.get_cstr()
                            , (!this->ini.context.target_protocol.is_empty() ? ":" : "")
                            , this->ini.globals.auth_user.get_cstr()
                            );
                    strcpy(this->ini.account.username, buffer);
                }

                this->mod = new FlatLoginMod(
                            // new LoginMod(
                                         this->ini,
                                         this->front,
                                         this->front.client_info.width,
                                         this->front.client_info.height);
                LOG(LOG_INFO, "ModuleManager::internal module Login ready");
                break;
            case MODULE_INTERNAL_WIDGET2_RWL:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                this->mod = new RwlMod(
                                       this->ini,
                                       this->front,
                                       this->front.client_info.width,
                                       this->front.client_info.height);
                LOG(LOG_INFO, "ModuleManager::internal module Login ready");
                break;

            case MODULE_XUP:
                {
                    const char * name = "XUP Target";
                    if (this->verbose){
                        LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP'\n");
                    }

                    int client_sck = ip_connect(this->ini.globals.target_device.get_cstr(),
                                                this->ini.context.target_port.get(),
                                                4, 1000,
                                                this->ini.debug.mod_xup);

                    if (client_sck == -1){
                        this->ini.context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                    }

                    SocketTransport * t = new SocketTransport(name
                                                              , client_sck
                                                              , this->ini.globals.target_device.get_cstr()
                                                              , this->ini.context.target_port.get()
                                                              , this->ini.debug.mod_xup);
                    this->mod_transport = t;

                    this->ini.context.auth_error_message.copy_c_str("failed authentification on remote X host");
                    this->mod = new xup_mod( t
                                             , this->front
                                             , this->front.client_info.width
                                             , this->front.client_info.height
                                             , this->ini.context.opt_width.get()
                                             , this->ini.context.opt_height.get()
                                             , this->ini.context.opt_bpp.get()
                                             );
                    this->mod->event.st = t;
                    this->ini.context.auth_error_message.empty();
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP' suceeded\n");
                    this->connected = true;
                }
                break;

            case MODULE_RDP:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP'");

                    // REDOC("hostname is the name of the RDP host ('windows' hostname) it is **not** used to get an ip address.");
                    // char hostname[255];

                    // TODO("as we now provide a client_info copy, we could extract hostname from in in mod_rdp, no need to use a separate field any more")
                    // hostname[0] = 0;
                    // if (this->front.client_info.hostname[0]){
                    //     memcpy(hostname, this->front.client_info.hostname, 31);
                    //     hostname[31] = 0;
                    // }

                    ClientInfo client_info = this->front.client_info;

                    if (strcmp(ini.context.mode_console.get_cstr(), "force") == 0) {
                        client_info.console_session = true;
                        LOG(LOG_INFO, "Session::mode console : force");
                    }
                    else if (strcmp(ini.context.mode_console.get_cstr(), "forbid") == 0) {
                        client_info.console_session = false;
                        LOG(LOG_INFO, "Session::mode console : forbid");
                    }
                    else {
                        // default is "allow", do nothing special
                    }

                    static const char * name = "RDP Target";

                    int client_sck = ip_connect(this->ini.globals.target_device.get_cstr(),
                                                this->ini.context.target_port.get(),
                                                3, 1000,
                                                this->ini.debug.mod_rdp);

                    if (client_sck == -1){
                        this->ini.context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                    }

                    TODO("RZ: We need find a better way to give access of STRAUTHID_AUTH_ERROR_MESSAGE to SocketTransport")
                    SocketTransport * t = new SocketTransport(
                                                              name
                                                              , client_sck
                                                              , this->ini.globals.target_device.get_cstr()
                                                              , this->ini.context.target_port.get()
                                                              , this->ini.debug.mod_rdp
                                                              , &this->ini.context.auth_error_message
                                                              );
                    this->mod_transport = t;

                    this->ini.context.auth_error_message.copy_c_str("failed authentification on remote RDP host");

                    ModRDPParams mod_rdp_params( this->ini.globals.target_user.get_cstr()
                                               , this->ini.context.target_password.get_cstr()
                                               , this->ini.globals.target_device.get_cstr()
                                               , "0.0.0.0"   // client ip is silenced
                                               , this->front.keymap.key_flags
                                               , this->ini.debug.mod_rdp
                                               );
                    //mod_rdp_params.enable_tls                          = true;
                    if (!mod_rdp_params.target_password[0]) {
                        mod_rdp_params.enable_nla                      = false;
                    }
                    else {
                        mod_rdp_params.enable_nla                      = this->ini.mod_rdp.enable_nla;
                    }
                    mod_rdp_params.enable_krb                          = this->ini.mod_rdp.enable_kerberos;
                    mod_rdp_params.enable_clipboard                    = this->ini.client.clipboard.get();
                    //mod_rdp_params.enable_fastpath                     = true;
                    //mod_rdp_params.enable_mem3blt                      = true;
                    mod_rdp_params.enable_bitmap_update                = this->ini.globals.enable_bitmap_update;
                    //mod_rdp_params.enable_new_pointer                  = true;
                    mod_rdp_params.acl                                 = acl;
                    mod_rdp_params.auth_channel                        = this->ini.globals.auth_channel;
                    mod_rdp_params.alternate_shell                     = this->ini.globals.alternate_shell.get_cstr();
                    mod_rdp_params.shell_working_directory             = this->ini.globals.shell_working_directory.get_cstr();
                    mod_rdp_params.rdp_compression                     = this->ini.mod_rdp.rdp_compression;
                    mod_rdp_params.error_message                       = &this->ini.context.auth_error_message;
                    mod_rdp_params.disconnect_on_logon_user_change     = this->ini.mod_rdp.disconnect_on_logon_user_change;
                    mod_rdp_params.open_session_timeout                = this->ini.mod_rdp.open_session_timeout;
                    mod_rdp_params.certificate_change_action           = this->ini.mod_rdp.certificate_change_action;
                    mod_rdp_params.enable_persistent_disk_bitmap_cache = this->ini.mod_rdp.persistent_disk_bitmap_cache;
                    mod_rdp_params.enable_cache_waiting_list           = this->ini.mod_rdp.cache_waiting_list;
                    mod_rdp_params.password_printing_mode              = this->ini.debug.password;

                    mod_rdp_params.extra_orders                    = this->ini.mod_rdp.extra_orders.c_str();

                    UdevRandom gen;
                    this->mod = new mod_rdp(t, this->front, client_info, gen, mod_rdp_params);
                    this->mod->event.st = t;

                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front.client_info.width, this->front.client_info.height));
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded\n");
                    this->ini.context.auth_error_message.empty();
                    this->connected = true;
                }
                break;

            case MODULE_VNC:
                {
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'\n");
                    static const char * name = "VNC Target";


                    int client_sck = ip_connect(this->ini.globals.target_device.get_cstr(),
                                                //this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                                this->ini.context.target_port.get(),
                                                3, 1000,
                                                this->ini.debug.mod_vnc);

                    if (client_sck == -1){
                        this->ini.context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                    }

                    SocketTransport * t = new SocketTransport(name
                                                              , client_sck
                                                              , this->ini.globals.target_device.get_cstr()
                                                              , this->ini.context.target_port.get()
                                                              , this->ini.debug.mod_vnc);
                    this->mod_transport = t;

                    this->ini.context.auth_error_message.copy_c_str("failed authentification on remote VNC host");

                    this->mod = new mod_vnc(t
                                            , this->ini
                                            , this->ini.globals.target_user.get_cstr()
                                            , this->ini.context.target_password.get_cstr()
                                            , this->front
                                            , this->front.client_info.width
                                            , this->front.client_info.height
                                            , this->front.client_info.keylayout
                                            , this->front.keymap.key_flags
                                            , this->ini.client.clipboard.get()
                                            , this->ini.mod_vnc.encodings.c_str()
                                            , this->ini.mod_vnc.allow_authentification_retries
                                            , this->ini.debug.mod_vnc);
                    this->mod->event.st = t;

                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded\n");
                    this->ini.context.auth_error_message.empty();
                    this->connected = true;
                }
                break;

            default:
                {
                    LOG(LOG_INFO, "ModuleManager::Unknown backend exception\n");
                    throw Error(ERR_SESSION_UNKNOWN_BACKEND);
                }
            }
        // if (this->connected) this->record(acl);
    }

    // Check movie start/stop/pause
    virtual void record(auth_api * acl)
    {
        if (this->ini.globals.movie.get()) {
            //TODO("Move start/stop capture management into module manager. It allows to remove front knwoledge from authentifier and module manager knows when video should or shouldn't be started (creating/closing external module mod_rdp or mod_vnc)") DONE ?
            if (this->front.capture_state == Front::CAPTURE_STATE_UNKNOWN) {
                this->front.start_capture(this->front.client_info.width,
                                          this->front.client_info.height,
                                          this->ini,
                                          acl);
                this->mod->rdp_input_invalidate(Rect( 0, 0, this->front.client_info.width, this->front.client_info.height));
            }
            else if (this->front.capture_state == Front::CAPTURE_STATE_PAUSED) {
                this->front.resume_capture();
                this->mod->rdp_input_invalidate(Rect( 0, 0, this->front.client_info.width, this->front.client_info.height));
            }
        }
        else if (this->front.capture_state == Front::CAPTURE_STATE_STARTED) {
            this->front.pause_capture();
        }
    }

};

#endif

