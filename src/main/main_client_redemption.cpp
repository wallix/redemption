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
   Copyright (C) Wallix 2010-2016
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "utils/log.hpp"

#include "client_redemption/client_redemption.hpp"

#pragma GCC diagnostic pop

int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test);


int main(int argc, char** argv)
{
    SessionReactor session_reactor;

    RDPVerbose verbose = to_verbose_flags(0xffffffff);

    LOG(LOG_INFO, "ClientRedemption 1");

    ClientRedemption client(session_reactor, argv, argc, verbose
                           , nullptr
                           , nullptr
                           , nullptr
                           , nullptr
                           , nullptr
                           , nullptr);



    return run_mod(client, true, std::chrono::milliseconds(6000), true);
}


int run_mod(ClientRedemption & front, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    LOG(LOG_INFO, "run mod 1");

    if (front.connected && front.mod) {

        LOG(LOG_INFO, "run mod 2");

        auto & mod = *(front.mod);

        while (mod.is_up_and_running())
        {
            LOG(LOG_INFO, "run mod 3");
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
                    //std::cerr <<  " Exit timeout (timeout = " << time_out_response.tv_sec << " sec " <<  time_out_response.tv_usec << " µsec)" << std::endl;
                    return 8;
                }
            }

            if (int err = front.wait_and_draw_event(time_mark)) {
                return err;
            }

            front.send_key_to_keep_alive();
        }
    }

    return 0;
}