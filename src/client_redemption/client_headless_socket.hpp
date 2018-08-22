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
   Copyright (C) Wallix 2018
   Author(s): Clément Moroldo
*/

#pragma once

#include "utils/log.hpp"


#include "client_redemption/client_input_output_api/client_socket_api.hpp"
#include "client_redemption/client_input_output_api/client_mouse_keyboard_api.hpp"
//#include "client_redemption/client_input_output_api/client"


class ClientHeadlessSocket : public ClientInputSocketAPI {

public:

    SessionReactor& session_reactor;

    ClientHeadlessSocket(SessionReactor& session_reactor)
      : session_reactor(session_reactor)
    {}

    virtual bool start_to_listen(int client_sck, mod_api * mod) override {
        (void) client_sck;
        (void) mod;

//         LOG(LOG_INFO, "start_to_listen()");

        try {
            while (!mod->is_up_and_running()) {
                std::cout << " Early negociations...\n";
                if (int err = this->client->wait_and_draw_event({3, 0})) {
                    std::cout << " Error: wait_and_draw_event() fail during negociation (" << err << ").\n";
                }
            }

        } catch (const Error & e) {
            std::cout << " Error: Failed during RDP early negociations step. " << e.errmsg() << "\n";
            return false;
        }
//                                 try {

// //             this->primary_connection_finished = true;
// //             this->start_wab_session_time = tvtime();
//
//         } catch (const Error & e) {
//             std::cout << " Error: Failed during RDP early negociations step. " << e.errmsg() << "\n";
//             return false;
//         }
        LOG(LOG_INFO, " Early negociations completes.\n");


//         timeval now = tvtime();
//         this->session_reactor.set_current_time(now);

        return true;
    }

    virtual void disconnect() override {}
};


class ClientHeadlessInput : public ClientInputMouseKeyboardAPI
{
    virtual void update_keylayout() override {}

    virtual void init_form() override {}


    // TODO string_view
    void virtual keyPressEvent(const int /*key*/, std::string const& /*text*/) override {}

    // TODO string_view
    void virtual keyReleaseEvent(const int /*key*/, std::string const& /*text*/) override {}
};


