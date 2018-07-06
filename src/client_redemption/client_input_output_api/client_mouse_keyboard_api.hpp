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


#include "client_redemption/client_redemption_api.hpp"




class ClientInputMouseKeyboardAPI : public ClientIO {


public:

    ClientInputMouseKeyboardAPI() = default;

    virtual ~ClientInputMouseKeyboardAPI() = default;


    virtual ClientRedemptionAPI * get_client() {
        return this->client;
    }

    virtual void update_keylayout() = 0;

    virtual void init_form() = 0;

    virtual void pre_load_movie() {}


    // CONTROLLER
    virtual void connexionReleased() {
        this->client->connect();

    }

    virtual void disconnexionReleased() {
        this->client->disconnexionReleased();
    }

    void CtrlAltDelPressed() {
        this->client->CtrlAltDelPressed();
    }

    void CtrlAltDelReleased() {
        this->client->CtrlAltDelReleased();
    }

    virtual void mouseButtonEvent(int x, int y, int flag) {
        this->client->mouseButtonEvent(x, y, flag);
    }

    virtual void wheelEvent(int x,  int y, int delta) {
        this->client->wheelEvent(x, y, delta);
    }

    virtual bool mouseMouveEvent(int x, int y) {
        return this->client->mouseMouveEvent(x, y);
    }

    // TODO string_view
    void virtual keyPressEvent(const int key, std::string const& text)  = 0;

    // TODO string_view
    void virtual keyReleaseEvent(const int key, std::string const& text)  = 0;

    void virtual refreshPressed() {
        this->client->refreshPressed();
    }

    virtual void open_options() {}

    // TODO string_view
    virtual ClientRedemptionAPI::KeyCustomDefinition get_key_info(int /*unused*/, std::string const& /*unused*/) {
        return ClientRedemptionAPI::KeyCustomDefinition(0, 0, "", 0, "");
    }

};
