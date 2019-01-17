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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "core/callback.hpp"
#include "keyboard/keymap2.hpp"
#include "client_redemption/client_redemption_api.hpp"
#include "client_redemption/client_input_output_api/client_keymap_api.hpp"
#include "mod/internal/replay_mod.hpp"


class ClientCallback
{
private:
    Keymap2           keymap;
    StaticOutStream<256> decoded_data;    // currently not initialised
    int                  _timer;

    mod_api            * mod = nullptr;
    ClientRedemptionAPI * client;

    ClientKeyLayoutAPI * rdp_keyLayout_api;
    ReplayMod * replay_mod = nullptr;

public:
    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    ClientCallback(ClientRedemptionAPI * client)
    :  _timer(0)
    , client(client)
    , rdp_keyLayout_api(nullptr)
    {}

    void set_rdp_keyLayout_api(ClientKeyLayoutAPI * rdp_keyLayout_api) {
        this->rdp_keyLayout_api = rdp_keyLayout_api;
    }

    mod_api * get_mod() {
        return this->mod;
    }

    void set_mod(mod_api * mod) {
        this->mod = mod;
    }

    void init() {
        this->mod = nullptr;
    }

    // REPLAY
    void delete_replay_mod() {
        this->client->delete_replay_mod();
    }

    void replay(const std::string & movie_path) {
        this->client->replay(movie_path);
    }

    timeval reload_replay_mod(int begin, timeval now_stop) {
        return this->client->reload_replay_mod(begin, now_stop);
    }

    bool load_replay_mod(timeval time_1, timeval time_2) {
        return this->client->load_replay_mod(time_1, time_2);
    }



    void set_replay(ReplayMod * replay_mod) {
        this->replay_mod = replay_mod;
    }

    void set_sync() {
        this->replay_mod->set_sync();
    }

    bool is_replay_on() {
        if (!this->replay_mod->get_break_privplay_client()) {
            if (!this->replay_mod->play_client()) {
                return true;
            }
        }
        return false;
    }

    time_t get_real_time_movie_begin() {
        return this->replay_mod->get_real_time_movie_begin();
    }

    void set_pause(timeval time) {
        this->replay_mod->set_pause(time);
    }

    void instant_play_client(std::chrono::microseconds time) {
        this->replay_mod->instant_play_client(time);
    }


    // Controller
    void init_layout(int lcid) {
        this->keymap.init_layout(lcid);
    }
    // TODO string_view
    void keyPressEvent(const int key, std::string const& text) {
        this->rdp_keyLayout_api->init(0, key, text);
        int keyCode = this->rdp_keyLayout_api->get_scancode();
        if (keyCode != 0) {
            this->send_rdp_scanCode(keyCode, this->rdp_keyLayout_api->get_flag());
        }
    }

    // TODO string_view
    void keyReleaseEvent(const int key, std::string const& text) {
        this->rdp_keyLayout_api->init(KBD_FLAG_UP, key, text);
        int keyCode = this->rdp_keyLayout_api->get_scancode();
        if (keyCode != 0) {
            this->send_rdp_scanCode(keyCode, this->rdp_keyLayout_api->get_flag());
        }
    }

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) {
        this->client->connect(ip, name, pwd, port);
    }

    void close() {
        if (this->get_mod() != nullptr) {
            time_t now;
            time(&now);

            this->mod->disconnect(now);
            this->mod = nullptr;
        }
        this->client->close();
    }

    void disconnect(bool pipe_broken) {
        if (this->get_mod() != nullptr) {
            if (!pipe_broken) {
                time_t now;
                time(&now);

                this->mod->disconnect(now);
            }
            this->mod = nullptr;
        }
        this->client->disconnect("", false);
    }

    void refreshPressed(const int width, const int height) {
        if (this->mod != nullptr) {
            Rect rect(0, 0, width, height);
            this->mod->rdp_input_invalidate(rect);
        }
    }

    void CtrlAltDelPressed() {
        const int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void CtrlAltDelReleased() {
        const int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP; /*NOLINT*/

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void mouseButtonEvent(int x, int y, int flag) {
        if (this->mod != nullptr && flag != MOUSE_FLAG_BUTTON4) {

            this->mod->rdp_input_mouse(flag, x, y, &(this->keymap));
        }
    }

    void wheelEvent(int delta) {
        int flag(0);
        if (delta < 0) {
            flag = MOUSE_FLAG_WHEEL | MOUSE_FLAG_WHEEL_NEGATIVE;
        } else {
            flag = MOUSE_FLAG_BUTTON4;
        }

        if (this->mod != nullptr) {
            this->mod->rdp_input_mouse(flag, 0, 0, &(this->keymap));
        }
    }

    bool mouseMouveEvent(int x, int y) {

        if (this->mod != nullptr /*&& y < this->config.info.height*/) {
            this->mouse_data.x = x;
            this->mouse_data.y = y;
            this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y, &(this->keymap));
        }

        return false;
    }

    void send_rdp_scanCode(int keyCode, int flag) {
        bool tsk_switch_shortcuts = false;
        Keymap2::DecodedKeys decoded_keys = this->keymap.event(flag, keyCode, tsk_switch_shortcuts);
        switch (decoded_keys.count)
        {
        case 2:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[1]);
            }
            break;
        case 1:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            break;
        default:
        case 0:
            break;
        }
        if (this->mod != nullptr) {
            this->mod->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->keymap));
        }
    }

    void send_rdp_unicode(uint16_t unicode, uint16_t flag) {
        this->mod->rdp_input_unicode(unicode, flag);
    }

    KeyCustomDefinition get_key_info(int keyCode, std::string const& text) {
        return this->rdp_keyLayout_api->get_key_info(keyCode, text);
    }

};


