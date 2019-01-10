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

//#include "utils/log.hpp"
#include "client_redemption/client_config/client_redemption_config.hpp"
#include "client_redemption/client_input_output_api/client_keymap_api.hpp"

#include "qt_scancode_keymap.hpp"



class QtClientRDPKeyLayout : public ClientKeyLayoutAPI {

public:
    Qt_ScanCode_KeyMap qtRDPKeymap;

    QtClientRDPKeyLayout()
    : ClientKeyLayoutAPI()
    , qtRDPKeymap()
    {}

    ~QtClientRDPKeyLayout() = default;


    virtual void update_keylayout(const int LCID) override {
        this->qtRDPKeymap.setKeyboardLayout(LCID);
    }

    void init(const int flag, const int key, std::string const& text) override {
        this->qtRDPKeymap.keyEvent(flag, key, text);
    }

    int get_scancode() override {
        return this->qtRDPKeymap.scanCode;
    }

    int get_flag() override {
        return this->qtRDPKeymap.flag;
    }

    void clearCustomKeyCode() override {
        this->qtRDPKeymap.clearCustomKeyCode();
    }

    void setCustomKeyCode(const int qtKeyID, const int scanCode, const std::string & ASCII8, const int extended) override {
        this->qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);
    }

    KeyCustomDefinition get_key_info(int keycode, std::string const& text) override {
        this->qtRDPKeymap.keyEvent(0, keycode, text);
        KeyCustomDefinition key_info(
            this->qtRDPKeymap.qKeyCode,
            this->qtRDPKeymap.scanCode,
            this->qtRDPKeymap.ascii,
            this->qtRDPKeymap.flag &0x0100 ? 0x0100: 0,
            this->qtRDPKeymap.qKeyName
          );

        return key_info;
    }

};
