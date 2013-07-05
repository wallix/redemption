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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Raphael Zhou, Meng Tan

   Manage Modules Life cycle : creation, destruction and chaining
   find out the next module to run from context reading
*/

#ifndef _REDEMPTION_ACL_MODULES_MANAGER_HPP_
#define _REDEMPTION_ACL_MODULES_MANAGER_HPP_


                // Check keep alive not reached
//                if (!this->acl->check_keep_alive(this->keep_alive_time, timestamp)){
//                }

                // Check inactivity not reached
//                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)){
//                }

                // Check movie start/stop/pause
//                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)){
//                }

//                        // Check if acl received an answer to auth_channel_target
//                        if (this->ini.globals.auth_channel[0]) {
//                            // Get acl answer to AUTHCHANNEL_TARGET
//                            if (!this->ini.context.authchannel_answer.is_empty()) {
//                                // If set, transmit to auth_channel channel
//                                this->mod->send_auth_channel_data(
//                                    this->ini.context.authchannel_answer.c_str());
//                                // Erase the context variable
//                                this->ini.context.authchannel_answer.empty();
//                            }
//                        }

                        // data incoming from server module
//                        if (this->mod->event.is_set(rfds)){
//                            this->mod->event.reset();
//                            if (this->verbose & 8){
//                                LOG(LOG_INFO, "Session::back_event fired");
//                            }
//                            BackEvent_t signal = this->mod->draw_event();
//                            switch (signal){
//                            case BACK_EVENT_NONE:
//                                // continue with same module
//                            break;
//                            case BACK_EVENT_STOP:
//                                // current module finished for some serious reason implying immediate exit
//                                // without going to close box.
//                                // the typical case (and only one used for now) is... we are coming from CLOSE_BOX
//                                this->internal_state = SESSION_STATE_STOP;
//                                break;
//                            case BACK_EVENT_REFRESH:
//                            {
//                                if (this->verbose & 8){
//                                    LOG(LOG_INFO, "Session::back event refresh");
//                                }
//                                bool record_video = false;
//                                bool keep_alive = false;
//                                int next_state = this->sesman->ask_next_module(
//                                                                    this->keep_alive_time,
//                                                                    record_video, keep_alive, this->nextmod);
//                                if (next_state != MODULE_WAITING){
//                                    this->internal_state = SESSION_STATE_RUNNING;
//                                }
//                                else {
//                                    this->internal_state = SESSION_STATE_WAITING_FOR_CONTEXT;
//                                }
//                            }
//                            break;
//                            case BACK_EVENT_NEXT:
//                            default:
//                            {
//                                if (this->verbose & 8){
//                                   LOG(LOG_INFO, "Session::back event end module");
//                                }
//                               // end the current module and switch to new one
//                                this->remove_mod();
//                                bool record_video = false;
//                                bool keep_alive = false;

//                                int next_state = this->sesman->ask_next_module(
//                                                                    this->keep_alive_time,
//                                                                    record_video, keep_alive,
//                                                                    this->nextmod);
//                                if (this->verbose & 8){
//                                    LOG(LOG_INFO, "session::next_state %u", next_state);
//                                }

//                                if (next_state != MODULE_WAITING){
//                                    this->internal_state = SESSION_STATE_STOP;
//                                    try {
//                                        this->session_setup_mod(next_state);
//                                        if (record_video) {
//                                            this->this->front.start_capture(
//                                                this->this->front.client_info.width,
//                                                this->this->front.client_info.height,
//                                                this->ini
//                                                );
//                                        }
//                                        else {
//                                            this->this->front.stop_capture();
//                                        }
//                                        if (this->sesman && keep_alive){
//                                            this->sesman->start_keep_alive(keep_alive_time);
//                                        }
//                                        this->internal_state = SESSION_STATE_RUNNING;
//                                    }
//                                    catch (const Error & e) {
//                                        LOG(LOG_INFO, "Session::connect failed Error=%u", e.id);
//                                        this->nextmod = INTERNAL_CLOSE;
//                                        this->session_setup_mod(MODULE_INTERNAL);
//                                        this->keep_alive_time = 0;
//                                        delete sesman;
//                                        this->sesman = NULL;
//                                        this->internal_state = SESSION_STATE_RUNNING;
//                                        this->this->front.stop_capture();
//                                        LOG(LOG_INFO, "Session::capture stopped, authentifier stopped");
//                                    }
//                                }
//                            }
//                            break;
//                            }
//                        }
//                    }
//                    break;
//                    case SESSION_STATE_CLOSE_CONNECTION:
//                    {
//                        if (this->mod->event.is_set(rfds)) {
//                            this->internal_state = SESSION_STATE_STOP;
//                        }
//                    }
//                    break;
//                }
//                if (this->internal_state == SESSION_STATE_STOP){
//                    break;
//                }


//    if (!this->acl) {
//        LOG(LOG_INFO, "Session::no authentifier available, closing");
//        this->ini.context.auth_error_message.copy_c_str("No authentifier available");
//        this->internal_state  = SESSION_STATE_CLOSE_CONNECTION;
//        this->nextmod         = INTERNAL_CLOSE;
//        this->session_setup_mod(MODULE_INTERNAL);
//        this->keep_alive_time = 0;
//        this->internal_state  = SESSION_STATE_RUNNING;
//        this->this->front.stop_capture();
//    }


//    if (strcmp(this->ini.context.mode_console.c_str(), "force") == 0){
//        this->this->front.set_console_session(true);
//        LOG(LOG_INFO, "Session::mode console : force");
//    }
//    else if (strcmp(this->ini.context.mode_console.c_str(), "forbid") == 0){
//        this->this->front.set_console_session(false);
//        LOG(LOG_INFO, "Session::mode console : forbid");
//    }
//    else {
//        // default is "allow", do nothing special
//    }

// Check movie start/stop/pause

#include "front.hpp"
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
#include "internal/widget2/bouncer2.hpp"
#include "internal/widget2/test_card_mod.hpp"
#include "internal/widget2/replay_mod.hpp"
#include "internal/widget2/selector_mod.hpp"
#include "internal/widget2/wab_close_mod.hpp"
#include "internal/widget2/dialog_mod.hpp"
#include "internal/widget2/login_mod.hpp"
#include "internal/widget2/rwl_mod.hpp"
#include "internal/widget2/rwl_login_mod.hpp"

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
    MODULE_INTERNAL_WIDGET2_CLOSE,
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

class ModuleManager
{
    public:
    Front & front;
    Inifile & ini;

    Transport * mod_transport;
    uint32_t verbose;
    
    ModuleManager(Front & front, Inifile & ini)
        : front(front)
        , ini(ini)
        , mod_transport(NULL)
        , verbose(0)
    {
    }
    
    virtual mod_api * new_mod(int target_module)
    {

//        if (strcmp(this->ini.context.mode_console.c_str(), "force") == 0){
//            this->front.set_console_session(true);
//            LOG(LOG_INFO, "ModuleManager::mode console : force");
//        }
//        else if (strcmp(this->ini.context.mode_console.c_str(), "forbid") == 0){
//            this->front.set_console_session(false);
//            LOG(LOG_INFO, "ModuleManager::mode console : forbid");
//        }
//        else {
//            // default is "allow", do nothing special
//        }

        LOG(LOG_INFO, "target_module=%u", target_module);

        mod_api * mod = NULL;

        switch (target_module)
        {
            case MODULE_INTERNAL_CLOSE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'INTERNAL::Close'");
                if (this->ini.context.auth_error_message.is_empty()) {
                    this->ini.context.auth_error_message.copy_c_str("Connection to server ended");
                }
                mod = new WabCloseMod(this->ini,
                                      this->front,
                                      this->front.client_info.width,
                                      this->front.client_info.height);
                this->front.init_pointers();
            }
            LOG(LOG_INFO, "ModuleManager::internal module Close ready");
            break;
            case MODULE_INTERNAL_BOUNCER2:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'bouncer2'");
                mod = new Bouncer2Mod(this->front,
                                      this->front.client_info.width,
                                      this->front.client_info.height
                                     );
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::internal module 'bouncer2' ready");
                }
            break;
            case MODULE_INTERNAL_TEST:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test'");
                mod = new ReplayMod(
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
                mod = new TestCardMod(this->front,
                                      this->front.client_info.width,
                                      this->front.client_info.height
                                     );
                LOG(LOG_INFO, "ModuleManager::internal module 'test_card' ready");
            break;
            case MODULE_INTERNAL_WIDGET2_SELECTOR:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'selector'");
                mod = new SelectorMod(this->ini,
                                      this->front,
                                      this->front.client_info.width,
                                      this->front.client_info.height
                                     );
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::internal module 'selector' ready");
                }
            break;
            case MODULE_INTERNAL_WIDGET2_CLOSE:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'CloseMod'");
                mod = new WabCloseMod(
                    this->ini,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height
                );
                LOG(LOG_INFO, "ModuleManager::internal module 'CloseMod' ready");
            break;
            case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
            case MODULE_INTERNAL_WIDGET2_DIALOG:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Accept Message'");
                const char * message = this->ini.context.message.get_cstr();
                const char * button = this->ini.translation.button_refused.get_cstr();
                const char * caption = "Information";
                mod = new DialogMod(
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
                mod = new DialogMod(
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
                mod = new LoginMod(
                    this->ini,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height);
                LOG(LOG_INFO, "ModuleManager::internal module Login ready");
                break;
            case MODULE_INTERNAL_WIDGET2_RWL:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                mod = new RwlMod(
                    this->ini,
                    this->front,
                    this->front.client_info.width,
                    this->front.client_info.height);
                LOG(LOG_INFO, "ModuleManager::internal module Login ready");
                break;
            case MODULE_INTERNAL_WIDGET2_RWL_LOGIN:
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
                mod = new RwlLoginMod(
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

                int client_sck = ip_connect(this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini.context.target_port.get(),
                                            4, 1000,
                                            this->ini.debug.mod_xup);

                if (client_sck == -1){
                    this->ini.context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                SocketTransport * t = new SocketTransport(
                      name
                    , client_sck
                    , this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini.context.target_port.get()
                    , this->ini.debug.mod_xup);
                mod_transport = t;

                this->ini.context.auth_error_message.copy_c_str("failed authentification on remote X host");
                mod = new xup_mod( t
                                   , this->front
                                   , this->front.client_info.width
                                   , this->front.client_info.height
                                   , this->ini.context.opt_width.get()
                                   , this->ini.context.opt_height.get()
                                   , this->ini.context.opt_bpp.get()
                                   );
                mod->event.obj = client_sck;
                this->ini.context.auth_error_message.empty();
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP' suceeded\n");
            }
            break;

            case MODULE_RDP:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP'");
                REDOC("hostname is the name of the RDP host ('windows' hostname) it is **not** used to get an ip address.")
                char hostname[255];
                hostname[0] = 0;
                if (this->front.client_info.hostname[0]){
                    memcpy(hostname, this->front.client_info.hostname, 31);
                    hostname[31] = 0;
                }
                static const char * name = "RDP Target";

                int client_sck = ip_connect(this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
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
                    , this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini.context.target_port.get()
                    , this->ini.debug.mod_rdp
                    , &this->ini.context.auth_error_message
                    );
                mod_transport = t;

                this->ini.context.auth_error_message.copy_c_str("failed authentification on remote RDP host");
                UdevRandom gen;
                mod = new mod_rdp( t
                                       , this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
                                       , this->ini.context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                                       , "0.0.0.0"  // client ip is silenced
                                       , this->front
                                       , hostname
                                       , true
                                       , this->front.client_info
                                       , &gen
                                       , this->front.keymap.key_flags
//                                       , this->acl   // we give mod_rdp a direct access to sesman for auth_channel channel
                                       , this->ini.globals.auth_channel
                                       , this->ini.globals.alternate_shell
                                       , this->ini.globals.shell_working_directory
                                       , this->ini.client.clipboard
                                       , true   // support fast-path
                                       , true   // support mem3blt
                                       , this->ini.globals.enable_bitmap_update
                                       , this->ini.debug.mod_rdp
                                       , true   // support new pointer
                                       );
                mod->event.obj = client_sck;

                mod->rdp_input_invalidate(Rect(0, 0, this->front.client_info.width, this->front.client_info.height));
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded\n");
                this->ini.context.auth_error_message.empty();
            }
            break;

            case MODULE_VNC:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'\n");
                static const char * name = "VNC Target";


                int client_sck = ip_connect(this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini.context.target_port.get(),
                                            3, 1000,
                                            this->ini.debug.mod_vnc);

                if (client_sck == -1){
                    this->ini.context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                SocketTransport * t = new SocketTransport(
                      name
                    , client_sck
                    , this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini.context.target_port.get()
                    , this->ini.debug.mod_vnc);
                mod_transport = t;

                this->ini.context.auth_error_message.copy_c_str("failed authentification on remote VNC host");

                mod = new mod_vnc(
                      t
                    , this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
                    , this->ini.context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                    , this->front
                    , this->front.client_info.width
                    , this->front.client_info.height
                    , this->front.client_info.keylayout
                    , this->front.keymap.key_flags
                    , this->ini.client.clipboard
                    , true /* RRE encoding */
                    , this->ini.debug.mod_vnc);
                mod->event.obj = client_sck;

                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded\n");
                this->ini.context.auth_error_message.empty();
            }
            break;

            default:
            {
                LOG(LOG_INFO, "ModuleManager::Unknown backend exception\n");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
        return mod;
    }

};

#endif

