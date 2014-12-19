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

#include "socket_transport.hpp"
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
#include "front.hpp"
#include "translation.hpp"

#include "internal/flat_login_mod.hpp"
#include "internal/flat_selector2_mod.hpp"
#include "internal/flat_wab_close_mod.hpp"
#include "internal/flat_dialog_mod.hpp"
#include "internal/flat_wait_mod.hpp"
#include "internal/interactive_target_mod.hpp"
#include "internal/widget_test_mod.hpp"

#include "mod_osd.hpp"
#include "mm_api.hpp"

#define STRMODULE_LOGIN            "login"
#define STRMODULE_SELECTOR         "selector"
#define STRMODULE_SELECTOR_LEGACY  "selector_legacy"
#define STRMODULE_CONFIRM          "confirm"
#define STRMODULE_CHALLENGE        "challenge"
#define STRMODULE_VALID            "valid"
#define STRMODULE_TRANSITORY       "transitory"
#define STRMODULE_CLOSE            "close"
#define STRMODULE_CONNECTION       "connection"
#define STRMODULE_TARGET           "interactive_target"
#define STRMODULE_MESSAGE          "message"
#define STRMODULE_RDP              "RDP"
#define STRMODULE_VNC              "VNC"
#define STRMODULE_INTERNAL         "INTERNAL"
#define STRMODULE_WAITINFO         "waitinfo"

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
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
    MODULE_INTERNAL_DIALOG_CHALLENGE,
    MODULE_INTERNAL_TARGET,
    MODULE_INTERNAL_BOUNCER2,
    MODULE_INTERNAL_TEST,
    MODULE_INTERNAL_WIDGET2_SELECTOR,
    MODULE_INTERNAL_WIDGET2_SELECTOR_LEGACY,
    MODULE_INTERNAL_WIDGETTEST,
    MODULE_INTERNAL_WAIT_INFO,
    MODULE_EXIT_INTERNAL_CLOSE,
    MODULE_TRANSITORY,
    MODULE_AUTH,
    MODULE_CLI
};

class MMIni : public MMApi {
public:
    Inifile & ini;
    uint32_t verbose;
    MMIni(Inifile & _ini) : ini(_ini)
                          , verbose(ini.debug.auth)
    {}
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
            this->ini.context.auth_error_message = auth_error_message;
        }
        if (this->mod) {
            this->mod->disconnect();
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
            if (this->ini.context.auth_error_message.empty()) {
                this->ini.context.auth_error_message = TR("end_connection", this->ini);
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
        else if (!strcmp(module_cstr, STRMODULE_SELECTOR_LEGACY)) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR_LEGACY");
            return MODULE_INTERNAL_WIDGET2_SELECTOR_LEGACY;
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
        else if (!strcmp(module_cstr, STRMODULE_WAITINFO)) {
            LOG(LOG_INFO, "===========> MODULE_WAITINFO");
            return MODULE_INTERNAL_WAIT_INFO;
        }
        else if (!strcmp(module_cstr, STRMODULE_TARGET)) {
            LOG(LOG_INFO, "===========> MODULE_INTERACTIVE_TARGET");
            return MODULE_INTERNAL_TARGET;
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
            const char * target = this->ini.context.target_host.get_cstr();
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
                this->ini.context.movie = user;
                const size_t len_user = strlen(user);
                if (0 != strcmp(".mwrm", user + len_user - 5) && len_user + 5 < this->ini.context.movie.max_size()) {
                    strcpy(this->ini.context.movie.data() + len_user, ".mwrm");
                }
                res = MODULE_INTERNAL_TEST;
            }
            else if (0 == strcmp(target, "widget2_message")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MODULE_INTERNAL_WIDGET2_MESSAGE;
            }
            else if (0 == strcmp(target, "widgettest")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widgettest");
                }
                res = MODULE_INTERNAL_WIDGETTEST;
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

    virtual void check_module() {
        if (this->ini.context.forcemodule.get() &&
            !this->is_connected()) {
            this->mod->get_event().signal = BACK_EVENT_NEXT;
            this->mod->get_event().set();
            this->ini.context.forcemodule.set(false);
            // Do not send back the value to sesman.
        }
    }
};


class ModuleManager : public MMIni
{
    struct module_osd
    : public mod_osd
    {
        module_osd(
            ModuleManager & manager, const Rect & rect,
            std::function<void(mod_api & mod, const Rect & rect, const Rect & clip)> f)
        : mod_osd(manager.front, *manager.mod, rect, std::move(f))
        , manager(manager)
        , old_mod(manager.mod)
        {
            manager.osd = this;
        }

        ~module_osd()
        {
            this->manager.mod = this->old_mod;
            this->manager.osd = nullptr;
            // disable draw hidden
            if (this->is_active()) {
                this->set_gd(*this, nullptr);
            }
        }

        virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
        {
            if (this->fg().contains_pt(x, y)) {
                if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
                    this->delete_self();
                }
            }
            else {
                mod_osd::rdp_input_mouse(device_flags, x, y, keymap);
            }
        }

        virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
        {
            if (keymap->nb_kevent_available() > 0){
                if (!(param3 & SlowPath::KBDFLAGS_DOWN)
                 && keymap->top_kevent() == Keymap2::KEVENT_ESC
                 && keymap->is_ctrl_pressed()) {
                    keymap->get_kevent();
                    this->delete_self();
                }
                else {
                    mod_osd::rdp_input_scancode(param1, param2, param3, param4, keymap);
                }
            }
        }

        void delete_self()
        {
            this->swap_active();
            delete this;
        }
        ModuleManager & manager;
        mod_api * old_mod;
    };
    module_osd * osd = nullptr;

    static const int padw = 16;
    static const int padh = 16;

    struct sock_mod_barrier {};
    template<class Mod>
    struct ModWithSocket : private SocketTransport, Mod
    {
        template<class... Args>
        ModWithSocket( ModuleManager & mm, const char * name, int sck, uint32_t verbose
                     , std::string * error_message, sock_mod_barrier, Args && ... mod_args)
        : SocketTransport( name, sck
                         , mm.ini.context.target_host.get_cstr()
                         , mm.ini.context.target_port.get()
                         , verbose, error_message)
        , Mod(*this, std::forward<Args>(mod_args)...)
        {
            mm.mod_transport = this;
        }
    };

public:
    void clear_osd_message() {
        if (this->osd) {
            this->osd->delete_self();
        }
    }

    void osd_message(std::string message) {
        this->clear_osd_message();
        int w, h;
        message += "  ";
        message += TR("disable_osd", this->ini);
        this->front.text_metrics(message.c_str(), w, h);
        w += padw * 2;
        h += padh * 2;
        uint32_t color = BLACK;
        uint32_t background_color = LIGHT_YELLOW;
        if (24 != this->front.client_info.bpp) {
            color = color_encode(color, this->front.client_info.bpp);
            background_color = color_encode(background_color, this->front.client_info.bpp);
        }
        this->mod = new module_osd(
            *this, Rect(this->front.client_info.width < w ? 0 : (this->front.client_info.width - w) / 2, 0, w, h),
            [this, message, color, background_color](mod_api & mod, const Rect & rect, const Rect & clip) {
                const Rect r = rect.intersect(clip);
                this->front.begin_update();
                this->front.draw(RDPOpaqueRect(r, background_color), r);
                this->front.server_draw_text(clip.x + padw, padh, message.c_str(), color, background_color, r);
                this->front.end_update();
            }
        );
    }


    Front & front;
    null_mod no_mod;
    SocketTransport * mod_transport = nullptr;

    ModuleManager(Front & front, Inifile & ini)
        : MMIni(ini)
        , front(front)
        , no_mod(this->front)
    {
        this->no_mod.get_event().reset();
        this->mod = &this->no_mod;
    }

    virtual void remove_mod()
    {
        delete this->osd;

        if (this->mod != &this->no_mod){
            delete this->mod;
            this->mod = &this->no_mod;
            this->mod_transport = nullptr;
        }
    }

    virtual ~ModuleManager()
    {
        this->remove_mod();
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
                                      , this->ini
                                      );
            if (this->verbose){
                LOG(LOG_INFO, "ModuleManager::internal module 'test' ready");
            }
            break;
        case MODULE_INTERNAL_WIDGETTEST:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'widgettest'");
            this->mod = new WidgetTestMod(this->ini,
                                          this->front,
                                          this->front.client_info.width,
                                          this->front.client_info.height
                                          );
            LOG(LOG_INFO, "ModuleManager::internal module 'widgettest' ready");
            break;
        case MODULE_INTERNAL_CARD:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test_card'");
            this->mod = new TestCardMod(this->front,
                                        this->front.client_info.width,
                                        this->front.client_info.height,
                                        false
                                        );
            LOG(LOG_INFO, "ModuleManager::internal module 'test_card' ready");
            break;
        case MODULE_INTERNAL_WIDGET2_SELECTOR:
        case MODULE_INTERNAL_WIDGET2_SELECTOR_LEGACY:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'selector'");
            this->mod = new FlatSelector2Mod(this->ini,
                        // new FlatSelectorMod(this->ini,
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
                if (this->ini.context.auth_error_message.empty()) {
                    this->ini.context.auth_error_message = "Connection to server ended";
                }
                this->mod = new FlatWabCloseMod(this->ini,
                            // new WabCloseMod(this->ini,
                                            this->front,
                                            this->front.client_info.width,
                                            this->front.client_info.height,
                                            now,
                                            true
                                            );
            }
            LOG(LOG_INFO, "ModuleManager::internal module Close ready");
            break;
        case MODULE_INTERNAL_TARGET:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Interactive Target'");
                this->mod = new InteractiveTargetMod(this->ini,
                                                     this->front,
                                                     this->front.client_info.width,
                                                     this->front.client_info.height);
                LOG(LOG_INFO, "ModuleManager::internal module 'Interactive Target' ready");
            }
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
        case MODULE_INTERNAL_WAIT_INFO:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
                const char * message = this->ini.context.message.get_cstr();
                const char * caption = TR("information", this->ini);
                bool showform = this->ini.context.showform.get();
                uint flag = this->ini.context.formflag.get();
                this->mod = new FlatWaitMod(
                                            this->ini,
                                            this->front,
                                            this->front.client_info.width,
                                            this->front.client_info.height,
                                            caption,
                                            message,
                                            now,
                                            showform,
                                            flag);
                LOG(LOG_INFO, "ModuleManager::internal module 'Wait Info Message' ready");
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

        case MODULE_XUP:
            {
                const char * name = "XUP Target";
                if (this->verbose){
                    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP'\n");
                }

                int client_sck = ip_connect(this->ini.context.target_host.get_cstr(),
                                            this->ini.context.target_port.get(),
                                            4, 1000,
                                            this->ini.debug.mod_xup);

                if (client_sck == -1){
                    this->ini.context.auth_error_message = "failed to connect to remote TCP host";
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                this->ini.context.auth_error_message = "failed authentification on remote X host";

                this->mod = new ModWithSocket<xup_mod>( *this
                                                      , name
                                                      , client_sck
                                                      , this->ini.debug.mod_xup
                                                      , nullptr
                                                      , sock_mod_barrier()
                                                      , this->front
                                                      , this->front.client_info.width
                                                      , this->front.client_info.height
                                                      , this->ini.context.opt_width.get()
                                                      , this->ini.context.opt_height.get()
                                                      , this->ini.context.opt_bpp.get()
                );

                this->ini.context.auth_error_message.clear();
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

                int client_sck = ip_connect(this->ini.context.target_host.get_cstr(),
                                            this->ini.context.target_port.get(),
                                            3, 1000,
                                            this->ini.debug.mod_rdp);

                if (client_sck == -1){
                    this->ini.context.auth_error_message = "failed to connect to remote TCP host";
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                this->ini.context.auth_error_message = "failed authentification on remote RDP host";

                // BEGIN READ PROXY_OPT
                if (!this->ini.globals.disable_proxy_opt) {
                    update_authorized_channels(this->ini.mod_rdp.allow_channels,
                                               this->ini.mod_rdp.deny_channels,
                                               this->ini.context.proxy_opt.get());
                }
                // END READ PROXY_OPT

                ModRDPParams mod_rdp_params( this->ini.globals.target_user.get_cstr()
                                           , this->ini.context.target_password.get_cstr()
                                           , this->ini.context.target_host.get_cstr()
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
                //mod_rdp_params.enable_fastpath                     = true;
                //mod_rdp_params.enable_mem3blt                      = true;
                mod_rdp_params.enable_bitmap_update                = this->ini.globals.enable_bitmap_update;
                //mod_rdp_params.enable_new_pointer                  = true;
                mod_rdp_params.enable_glyph_cache                  = this->ini.globals.glyph_cache;
                mod_rdp_params.acl                                 = acl;
                mod_rdp_params.auth_channel                        = this->ini.globals.auth_channel;
                mod_rdp_params.alternate_shell                     = this->ini.globals.alternate_shell.get_cstr();
                mod_rdp_params.shell_working_directory             = this->ini.globals.shell_working_directory.get_cstr();
                mod_rdp_params.target_application_account          = this->ini.globals.target_application_account.get_cstr();
                mod_rdp_params.target_application_password         = this->ini.globals.target_application_password.get_cstr();
                mod_rdp_params.rdp_compression                     = this->ini.mod_rdp.rdp_compression;
                mod_rdp_params.error_message                       = &this->ini.context.auth_error_message;
                mod_rdp_params.disconnect_on_logon_user_change     = this->ini.mod_rdp.disconnect_on_logon_user_change;
                mod_rdp_params.open_session_timeout                = this->ini.mod_rdp.open_session_timeout;
                mod_rdp_params.certificate_change_action           = this->ini.mod_rdp.certificate_change_action;
                mod_rdp_params.enable_persistent_disk_bitmap_cache = this->ini.mod_rdp.persistent_disk_bitmap_cache;
                mod_rdp_params.enable_cache_waiting_list           = this->ini.mod_rdp.cache_waiting_list;
                mod_rdp_params.persist_bitmap_cache_on_disk        = this->ini.mod_rdp.persist_bitmap_cache_on_disk;
                mod_rdp_params.password_printing_mode              = this->ini.debug.password;
                mod_rdp_params.cache_verbose                       = this->ini.debug.cache;

                mod_rdp_params.extra_orders                        = this->ini.mod_rdp.extra_orders.c_str();

                mod_rdp_params.allow_channels                      = &(this->ini.mod_rdp.allow_channels);
                mod_rdp_params.deny_channels                       = &(this->ini.mod_rdp.deny_channels);

                UdevRandom gen;

                TODO("RZ: We need find a better way to give access of STRAUTHID_AUTH_ERROR_MESSAGE to SocketTransport")
                this->mod = new ModWithSocket<mod_rdp>( *this
                                                      , name
                                                      , client_sck
                                                      , this->ini.debug.mod_rdp
                                                      , &this->ini.context.auth_error_message
                                                      , sock_mod_barrier()
                                                      , this->front
                                                      , client_info
                                                      , gen
                                                      , mod_rdp_params
                                                      );

                // DArray<Rect> rects(1);
                // rects[0] = Rect(0, 0, this->front.client_info.width, this->front.client_info.height);
                // this->mod->rdp_input_invalidate2(rects);
                this->mod->rdp_input_invalidate(Rect(0, 0, this->front.client_info.width, this->front.client_info.height));
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded\n");
                this->ini.context.auth_error_message.clear();
                this->connected = true;
            }
            break;

        case MODULE_VNC:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'\n");
                static const char * name = "VNC Target";


                int client_sck = ip_connect(this->ini.context.target_host.get_cstr(),
                                            //this->ini.context_get_value(AUTHID_TARGET_HOST, NULL, 0),
                                            this->ini.context.target_port.get(),
                                            3, 1000,
                                            this->ini.debug.mod_vnc);

                if (client_sck == -1){
                    this->ini.context.auth_error_message = "failed to connect to remote TCP host";
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                this->ini.context.auth_error_message = "failed authentification on remote VNC host";

                this->mod = new ModWithSocket<mod_vnc>( *this
                                                      , name
                                                      , client_sck
                                                      , this->ini.debug.mod_vnc
                                                      , nullptr
                                                      , sock_mod_barrier()
                                                      , this->ini
                                                      , this->ini.globals.target_user.get_cstr()
                                                      , this->ini.context.target_password.get_cstr()
                                                      , this->front
                                                      , this->front.client_info.width
                                                      , this->front.client_info.height
                                                      , this->front.client_info.keylayout
                                                      , this->front.keymap.key_flags
                                                      , this->ini.mod_vnc.clipboard_up.get()
                                                      , this->ini.mod_vnc.clipboard_down.get()
                                                      , this->ini.mod_vnc.encodings.c_str()
                                                      , this->ini.mod_vnc.allow_authentification_retries
                                                      , true
                                                      , this->ini.debug.mod_vnc
                );

                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded\n");
                this->ini.context.auth_error_message.clear();
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

    // Check movie start/stop/pause
    virtual void record(auth_api * acl)
    {
        if (this->ini.globals.movie.get() || !ini.context.pattern_kill.is_empty() || !ini.context.pattern_notify.is_empty()) {
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

