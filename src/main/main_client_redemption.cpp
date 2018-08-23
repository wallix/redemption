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

#include <signal.h>

#include "utils/log.hpp"

#include "core/session_reactor.hpp"
#include "client_redemption/client_redemption.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include "client_redemption/client_headless_socket.hpp"

#include "client_redemption/client_channel_managers/fake_client_mod.hpp"

#pragma GCC diagnostic pop

int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response);


int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor session_reactor;

    RDPVerbose verbose = to_verbose_flags(0x0);      //to_verbose_flags(0x0);

    LOG(LOG_INFO, "ClientRedemption init");

    ClientHeadlessSocket headless_socket(session_reactor);
    ClientInputSocketAPI * headless_socket_api_obj = &headless_socket;

    {
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGPIPE);
        sa.sa_handler = [](int sig){
            std::cout << "got SIGPIPE(" << sig << ") : ignoring\n";
        };
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
        #if REDEMPTION_COMP_CLANG >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
        #endif
        sigaction(SIGPIPE, &sa, nullptr);
        REDEMPTION_DIAGNOSTIC_POP
    }

    ClientRedemption client( session_reactor, argv, argc, verbose
                           , nullptr
                           , nullptr
                           , nullptr
                           , headless_socket_api_obj
                           , nullptr
                           , nullptr);

    return run_mod(client, false, std::chrono::milliseconds(10000));
}


int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    if (front.mod) {
        auto & mod = *(front.mod);

        bool logged = false;

        while (true)
        {
            if (mod.logged_on == mod_api::CLIENT_LOGGED) {
                mod.logged_on = mod_api::CLIENT_UNLOGGED;

                std::cout << "RDP Session Log On.\n";
                if (quick_connection_test && !logged) {
                    logged = true;
                    std::cout << "quick_connection_test\n";
                    front.disconnect("", false);
                    return 0;
                }
            }

            if (time_stop < tvtime()) {
                std::cerr <<  " Exit timeout (timeout = " << time_out_response.count() << ")" << std::endl;
                front.disconnect("", false);
                return 8;
            }

            if (int err = front.wait_and_draw_event(time_mark)) {
                return err;
            }

            front.send_key_to_keep_alive();
        }
    }

    return 0;
}
