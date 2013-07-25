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
#include "null/null.hpp"
#include "rdp/rdp.hpp"
#include "vnc/vnc.hpp"
#include "xup/xup.hpp"
#include "transitory/transitory.hpp"
#include "cli/cli_mod.hpp"
#include "internal/bouncer2_mod.hpp"
#include "internal/test_card_mod.hpp"
#include "internal/replay_mod.hpp"
#include "internal/selector_mod.hpp"
#include "internal/wab_close_mod.hpp"
#include "internal/dialog_mod.hpp"
#include "internal/login_mod.hpp"
#include "internal/rwl_mod.hpp"
#include "internal/rwl_login_mod.hpp"
#include "front.hpp"

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
    MODULE_INTERNAL_WIDGET2_RWL_LOGIN,
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
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

    MMApi() : last_module(false)
            , connected(false) {}
    ~MMApi() {}
    virtual void remove_mod() = 0;
    virtual void new_mod(int target_module, time_t now) = 0;
    virtual void record() = 0;
    virtual int next_module() = 0;
    virtual int get_mod_from_protocol() = 0;
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
//    virtual bool is_close_box() { return false; }

};

class MMIni : public MMApi {
public:
    Inifile & ini;
    uint32_t verbose;
    MMIni(Inifile & _ini) : ini(_ini)
                          , verbose(ini.debug.auth) {}
    virtual ~MMIni() {}
    virtual void remove_mod() {};
    virtual void new_mod(int target_module, time_t now) {
        printf("new mod %d at time: %d\n", (int)target_module, (int)now);
        switch(target_module) {
        case MODULE_VNC:
        case MODULE_XUP:
        case MODULE_RDP:
            this->connected = true;
            break;
        default:
            break;
        };
    };
    virtual void record() {};

    virtual void invoke_close_box(const char * auth_error_message,
                                  BackEvent_t & signal, time_t now) {
        this->last_module = true;
        if (auth_error_message) {
            this->ini.context.auth_error_message.copy_c_str(auth_error_message);
        }
        this->remove_mod();
        this->new_mod(MODULE_INTERNAL_CLOSE, now);
        signal = BACK_EVENT_NONE;
    }

    int get_mod_from_protocol() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::get_mod_from_protocol");
        }
        // Initialy, it no protocol known and get_value should provide "ASK".
        const char * protocol = this->ini.context.target_protocol.get_value();
        if (ini.globals.internal_domain) {
            const char * target = this->ini.globals.target_device.get_cstr();
            if (0 == strncmp(target, "autotest", 8)) {
                protocol = "INTERNAL";
            }
        }
        int res = MODULE_EXIT;
        if (!this->connected && 0 == strncasecmp(protocol, "RDP", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol RDP");
            }
            res = MODULE_RDP;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "APP", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol APPLICATION");
            }
            res = MODULE_RDP;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "VNC", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol VNC");
            }
            res = MODULE_VNC;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "XUP", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol XUP");
            }
            res = MODULE_XUP;
        }
        else if (strncasecmp(protocol, "INTERNAL", 8) == 0) {
            const char * target = this->ini.globals.target_device.get_cstr();
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL");
            }
            if (0 == strcmp(target, "bouncer2")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL bouncer2");
                }
                res = MODULE_INTERNAL_BOUNCER2;
            }
            else if (0 == strncmp(target, "autotest", 8)) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL test");
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
            else if (0 == strcmp(target, "selector")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL selector");
                }
                res = MODULE_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (0 == strcmp(target, "login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "rwl_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else if (0 == strcmp(target, "rwl")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "close") || 0 == strcmp(target, "widget2_close")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL close");
                }
                res = MODULE_INTERNAL_CLOSE;
            }
            else if (0 == strcmp(target, "widget2_dialog")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_dialog");
                }
                res = MODULE_INTERNAL_WIDGET2_DIALOG;
            }
            else if (0 == strcmp(target, "widget2_message")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MODULE_INTERNAL_WIDGET2_MESSAGE;
            }
            else if (0 == strcmp(target, "widget2_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "widget2_rwl")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "widget2_rwl_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL card");
                }
                res = MODULE_INTERNAL_CARD;
            }
        }
        else if (this->connected) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_close");
            }
            res = MODULE_INTERNAL_CLOSE;
        }
        else {
            LOG(LOG_WARNING, "Unsupported target protocol %c%c%c%c",
                protocol[0], protocol[1], protocol[2], protocol[3]);
            res = MODULE_EXIT;
        }
        return res;
    }

    int next_module() {
        LOG(LOG_INFO, "----------> ACL next_module <--------");
        if (!this->ini.context.rejected.get().is_empty()) {
            this->ini.context.auth_error_message.copy_str(this->ini.context.rejected.get());
            this->ini.context.rejected.set_empty();
            LOG(LOG_INFO, "MODULE_INTERNAL_CLOSE");
            return MODULE_INTERNAL_CLOSE;
        }
        else if (this->ini.context_is_asked(AUTHID_AUTH_USER)
            ||  this->ini.context_is_asked(AUTHID_PASSWORD)) {
            LOG(LOG_INFO, "===========> MODULE_LOGIN");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // Selector Target device and Target user contains list or possible targets
        else if (!this->ini.context_is_asked(AUTHID_SELECTOR)
                 &&   this->ini.context_get_bool(AUTHID_SELECTOR)
                 &&  !this->ini.context_is_asked(AUTHID_TARGET_DEVICE)
                 &&  !this->ini.context_is_asked(AUTHID_TARGET_USER)) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR");
            return MODULE_INTERNAL_WIDGET2_SELECTOR;
        }
        // Target User or Device asked and not in selector : back to login
        else if (this->ini.context_is_asked(AUTHID_TARGET_DEVICE)
                 ||  this->ini.context_is_asked(AUTHID_TARGET_USER)) {
            LOG(LOG_INFO, "===============> MODULE_LOGIN (2)");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show message
        else if (this->ini.context_is_asked(AUTHID_DISPLAY_MESSAGE)) {
            LOG(LOG_INFO, "==================> MODULE_DISPLAY");
            return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show confirmation message
        else if (this->ini.context_is_asked(AUTHID_ACCEPT_MESSAGE)) {
            LOG(LOG_INFO, "=================> MODULE_ACCEPT");
            return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
        }
        // Authenticated = true, means we have : AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER, TARGET_PASSWORD
        // proceed with connection.
        else if (this->ini.context.authenticated.get()) {
            //                record_video = this->ini.globals.movie.get();
            //                keep_alive = true;
            if (this->ini.context.auth_error_message.is_empty()) {
                this->ini.context.auth_error_message.copy_c_str("End of connection");
            }// seems strange ?
            LOG(LOG_INFO, "=================> MODULE_FROM_PROTOCOL");
            // this->ini.context.selector.set(false);
            return this->get_mod_from_protocol();
        }
        // User authentication rejected : close message
        else {
            if (!this->ini.context.rejected.get().is_empty()) {
                this->ini.context.auth_error_message.copy_str(this->ini.context.rejected.get());
            }
            if (this->ini.context.auth_error_message.is_empty()) {
                this->ini.context.auth_error_message.copy_c_str("Authentifier service failed");
            }
            LOG(LOG_INFO, "MODULE_INTERNAL_CLOSE");
            return MODULE_INTERNAL_CLOSE;
        }
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

    // Check movie start/stop/pause
    void record()
    {
        if (this->ini.globals.movie.get()) {
        TODO("Move start/stop capture management into module manager. It allows to remove front knwoledge from authentifier and module manager knows when video should or shouldn't be started (creating/closing external module mod_rdp or mod_vnc)")
            if (this->front.capture_state == Front::CAPTURE_STATE_UNKNOWN) {
                this->front.start_capture(this->front.client_info.width
                                   , this->front.client_info.height
                                   , this->ini
                                   );
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

    virtual void new_mod(int target_module, time_t now)
    {
        LOG(LOG_INFO, "target_module=%u", target_module);

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
                this->mod = new SelectorMod(this->ini,
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
                    this->mod = new WabCloseMod(this->ini,
                                                this->front,
                                                this->front.client_info.width,
                                                this->front.client_info.height,
                                                now);
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
                    this->mod = new DialogMod(
                                              this->ini,
                                              this->front,
                                              this->front.client_info.width,
                                              this->front.client_info.height,
                                              caption,
                                              message,
                                              button
                                              );
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
                    this->mod = new DialogMod(
                                              this->ini,
                                              this->front,
                                              this->front.client_info.width,
                                              this->front.client_info.height,
                                              caption,
                                              message,
                                              button
                                              );
                    LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Display Message' ready");
                }
                break;
            case MODULE_INTERNAL_WIDGET2_LOGIN:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                this->mod = new LoginMod(
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
            case MODULE_INTERNAL_WIDGET2_RWL_LOGIN:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                this->mod = new RwlLoginMod(
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
                                                //this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
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
                                                              //, this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
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
                    this->mod->event.obj = client_sck;
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
                                                //this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
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
                                                              //, this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                                                              , this->ini.context.target_port.get()
                                                              , this->ini.debug.mod_rdp
                                                              , &this->ini.context.auth_error_message
                                                              );
                    this->mod_transport = t;

                    this->ini.context.auth_error_message.copy_c_str("failed authentification on remote RDP host");
                    UdevRandom gen;
                    this->mod = new mod_rdp(t
                                            , this->ini.globals.target_user.get_cstr()
                                            , this->ini.context.target_password.get_cstr()
                                            //, this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
                                            //, this->ini.context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                                            , "0.0.0.0"  // client ip is silenced
                                            , this->front
                                            // , hostname
                                            , true
                                            , client_info
                                            , &gen
                                            , this->front.keymap.key_flags
//                                            , this->acl   // we give mod_rdp a direct access to sesman for auth_channel channel
                                            , &this->ini.context.authchannel_target
                                            , &this->ini.context.authchannel_result
                                            , this->ini.globals.auth_channel
                                            , this->ini.globals.alternate_shell.get_cstr()
                                            , this->ini.globals.shell_working_directory.get_cstr()
                                            , this->ini.client.clipboard.get()
                                            , true          // support fast-path
                                            , true          // support mem3blt
                                            , this->ini.globals.enable_bitmap_update
                                            , this->ini.debug.mod_rdp
                                            , true          // support new pointer
                                            , true          // support RDP 5.0 bulk compression
                                            );
                    this->mod->event.obj = client_sck;

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
                                                              //, this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                                                              , this->ini.context.target_port.get()
                                                              , this->ini.debug.mod_vnc);
                    this->mod_transport = t;

                    this->ini.context.auth_error_message.copy_c_str("failed authentification on remote VNC host");

                    this->mod = new mod_vnc(t
                                            , this->ini.globals.target_user.get_cstr()
                                            , this->ini.context.target_password.get_cstr()
                                            // , this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
                                            // , this->ini.context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                                            , this->front
                                            , this->front.client_info.width
                                            , this->front.client_info.height
                                            , this->front.client_info.keylayout
                                            , this->front.keymap.key_flags
                                            , this->ini.client.clipboard.get()
                                            , true /* RRE encoding */
                                            , this->ini.debug.mod_vnc);
                    this->mod->event.obj = client_sck;

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
    }



};

#endif

