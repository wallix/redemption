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

    Qt_ScanCode_KeyMap qtRDPKeymap;

public:
    QtClientRDPKeyLayout()
    : ClientKeyLayoutAPI()
    , qtRDPKeymap()
    {}

    ~QtClientRDPKeyLayout() = default;

    void update_keylayout(int LCID) override {
        this->qtRDPKeymap.setKeyboardLayout(LCID);
    }

    void key_event(int flag, int key, std::string_view text) override {
        this->qtRDPKeymap.keyEvent(flag, key, text);
    }

    uint16_t get_scancode() override {
        return this->qtRDPKeymap.scanCode;
    }

    uint16_t get_flag() override {
        return this->qtRDPKeymap.flag;
    }
};
