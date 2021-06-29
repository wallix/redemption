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

#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "client_redemption/client_redemption_api.hpp"
#include "mod/internal/replay_mod.hpp"

// TODO Keymap::KeyCode
enum: uint8_t {
    KBD_SCANCODE_ALTGR  = 0x38,
    KBD_SCANCODE_SHIFT  = 0x36,
    KBD_SCANCODE_ENTER  = 0x1C,
    KBD_SCANCODE_BK_SPC = 0x0E,
    KBD_SCANCODE_CTRL   = 0x1D,
    KBD_SCANCODE_DELETE = 0x53
};

class ClientCallback
{
private:
    Keymap keymap;

    mod_api * mod = nullptr;
    ClientRedemptionAPI * client;

    ReplayMod * replay_mod = nullptr;

public:
    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    ClientCallback(ClientRedemptionAPI * client, KeyLayout layout)
    : keymap(layout)
    , client(client)
    {}

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

    bool load_replay_mod(MonotonicTimePoint begin, MonotonicTimePoint end) {
        return this->client->load_replay_mod(begin, end);
    }

    void set_replay(ReplayMod * replay_mod) {
        this->replay_mod = replay_mod;
    }

    void init_layout(int kbdid) {
        if (auto* layout = find_layout_by_id(KeyLayout::KbdId(kbdid))) {
            this->keymap.set_layout(*layout);
        }
    }

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) {
        this->client->connect(ip, name, pwd, port);
    }

    void close() {
        if (this->get_mod() != nullptr) {
            this->mod->disconnect();
            this->mod = nullptr;
        }
        this->client->close();
    }

    void disconnect(bool pipe_broken) {
        if (this->get_mod() != nullptr) {
            if (!pipe_broken) {
                this->mod->disconnect();
            }
            this->mod = nullptr;
        }
        this->client->disconnect("", false);
    }

    void refreshPressed(const uint16_t width, const uint16_t height) {
        if (this->mod != nullptr) {
            Rect rect(0, 0, width, height);
            this->mod->rdp_input_invalidate(rect);
        }
    }

    void CtrlAltDelPressed() {
        const auto flags = Keymap::KbdFlags::Extended;

        this->send_rdp_scanCode(flags, Keymap::Scancode::LCtrl);
        this->send_rdp_scanCode(flags, Keymap::Scancode::LAlt);
        this->send_rdp_scanCode(flags, Keymap::Scancode::Delete);
    }

    void CtrlAltDelReleased() {
        const auto flags = Keymap::KbdFlags::Extended | Keymap::KbdFlags::Release;

        this->send_rdp_scanCode(flags, Keymap::Scancode::LCtrl);
        this->send_rdp_scanCode(flags, Keymap::Scancode::LAlt);
        this->send_rdp_scanCode(flags, Keymap::Scancode::Delete);
    }

    void mouseButtonEvent(int x, int y, int flag) {
        if (this->mod != nullptr && flag != MOUSE_FLAG_BUTTON4) {
            this->mod->rdp_input_mouse(flag, x, y);
        }
    }

    void wheelEvent(int delta) {
        int flag(0);
        if (delta < 0) {
            flag = MOUSE_FLAG_WHEEL | MOUSE_FLAG_WHEEL_NEGATIVE;
        }
        else {
            flag = MOUSE_FLAG_BUTTON4;
        }

        if (this->mod != nullptr) {
            this->mod->rdp_input_mouse(flag, 0, 0);
        }
    }

    bool mouseMouveEvent(int x, int y) {

        if (this->mod != nullptr /*&& y < this->config.info.height*/) {
            this->mouse_data.x = x;
            this->mouse_data.y = y;
            this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y);
        }

        return false;
    }

    void send_rdp_scanCode(kbdtypes::KbdFlags flags, kbdtypes::Scancode scancode) {
        if (this->mod != nullptr) {
            this->mod->rdp_input_scancode(flags, scancode, /*timer=*/0, this->keymap);
        }
    }

    void send_rdp_unicode(kbdtypes::KbdFlags flag, uint16_t unicode) {
        this->mod->rdp_input_unicode(flag, unicode);
    }
};
