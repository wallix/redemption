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
   Copyright (C) Wallix 2017-2018
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "utils/log.hpp"

#include "core/session_reactor.hpp"
#include "client_redemption/client_redemption.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include "client_redemption/client_headless_socket.hpp"

#pragma GCC diagnostic pop

int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test);


int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor session_reactor;

    RDPVerbose verbose = to_verbose_flags(0x0);

    LOG(LOG_INFO, "ClientRedemption init");

    ClientHeadlessSocket headless_socket;
    ClientInputSocketAPI * headless_socket_api_obj = &headless_socket;


    ClientRedemption client( session_reactor, argv, argc, verbose
                           , nullptr
                           , nullptr
                           , nullptr
                           , headless_socket_api_obj
                           , nullptr
                           , nullptr);

/*    int i = 0;
    if (client.mod) {
        std::cout << "init conn 2" << std::endl;
        while (!client.mod->is_up_and_running()) {

            if (int err = client.wait_and_draw_event({ 0, 50000 })) {
                std::cout << "init conn error " <<  err << std::endl;
                return err;
            }
            std::cout << "init conn step " << i << std::endl;
            i++;
        }
    }

    std::cout << "init conn32" << std::endl*/;

    return run_mod(client, true, std::chrono::milliseconds(6000), false);
}


int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    if (front.mod) {

        auto & mod = *(front.mod);

        bool running = true;
        bool connected = false;

        while (running)
        {
            if (mod.logged_on == mod_api::CLIENT_LOGGED) {
                mod.logged_on = mod_api::CLIENT_UNLOGGED;

                LOG(LOG_INFO, "RDP Session Log On.");
                if (quick_connection_test) {
                    LOG(LOG_INFO, "quick_connection_test");
                    return 0;
                }
                break;
            }

            if (time_set_connection_test) {
                if (time_stop > tvtime()) {
                    std::cerr <<  " Exit timeout (timeout = " << time_out_response.count() << std::endl;
                    return 8;
                }
            }

            if (int err = front.wait_and_draw_event(time_mark)) {
                return err;
            }

            front.send_key_to_keep_alive();

            if (connected) {
                running = mod.is_up_and_running();
            }

            if (mod.is_up_and_running()) {
                connected = true;
            }
        }
    }

    return 0;
}
