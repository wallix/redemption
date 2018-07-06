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

#include "utils/log.hpp"

#ifndef Q_MOC_RUN

#include "keyboard/keymap2.hpp"


#include "client_redemption/client_redemption_config.hpp"

#endif



class ClientRedemptionController : public ClientRedemptionConfig
{
private:
    Keymap2           keymap;
    StaticOutStream<256> decoded_data;    // currently not initialised
    int                  _timer;


public:
    ClientRedemptionController(SessionReactor& session_reactor, char* argv[], int argc, RDPVerbose verbose)
      : ClientRedemptionConfig(session_reactor, argv, argc, verbose)
      , keymap()
      , _timer(0)
        {
            this->keymap.init_layout(this->info.keylayout);
        }

    void init_layout(int lcid) {
        this->keymap.init_layout(lcid);
    }

    void refreshPressed() override {
        if (this->mod != nullptr) {
            Rect rect(0, 0, this->info.width, this->info.height);
            this->mod->rdp_input_invalidate(rect);
        }
    }

    void CtrlAltDelPressed() override {
        int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void CtrlAltDelReleased() override {
        int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void mouseButtonEvent(int x, int y, int flag) override {
        if (this->mod != nullptr) {
            this->mod->rdp_input_mouse(flag, x, y, &(this->keymap));
        }
    }

    void wheelEvent(int  /*unused*/,  int  /*unused*/, int delta) override {
        int flag(MOUSE_FLAG_HWHEEL);
        if (delta < 0) {
            flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
        }
        if (this->mod != nullptr) {
            //this->mod->rdp_input_mouse(flag, e->x(), e->y(), &(this->keymap));
        }
    }

    bool mouseMouveEvent(int x, int y) override {

        if (this->mod != nullptr && y < this->info.height) {
            this->mouse_data.x = x;
            this->mouse_data.y = y;
            this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y, &(this->keymap));
        }

        return false;
    }

    void send_rdp_scanCode(int keyCode, int flag) override {
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

    void send_rdp_unicode(uint16_t unicode, uint16_t flag) override {
        this->mod->rdp_input_unicode(unicode, flag);
    }

};
