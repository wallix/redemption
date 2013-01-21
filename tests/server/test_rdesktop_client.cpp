/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to check front-end behavior stays identical
   when connecting from rdesktop (mocked up)

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFrontRdesktopClient
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGPRINT

#include <errno.h>
#include <algorithm>
#include <sys/un.h>

#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#undef DEFAULT_FONT_NAME
#define DEFAULT_FONT_NAME "sans-10.fv1"

#include "constants.hpp"
#include "listen.hpp"
#include "ssl_calls.hpp"
#include "config.hpp"
#include "session.hpp"

BOOST_AUTO_TEST_CASE(TestIncomingConnection)
{
    // This server only support one incoming connection before closing listener
//    class ServerOnce : public Server
//    {
//        public:
//        int sck;
//        char ip_source[256];

//        ServerOnce() : sck(0)
//        {
//            ip_source[0] = 0;
//        }

//        virtual Server_status start(int incoming_sck)
//        {
//            struct sockaddr_in sin;
//            unsigned int sin_size = sizeof(struct sockaddr_in);
//            memset(&sin, 0, sin_size);
//            this->sck = accept(incoming_sck, (struct sockaddr*)&sin, &sin_size);
//            strcpy(ip_source, inet_ntoa(sin.sin_addr));
//            LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", sck, ip_source);
//            return START_WANT_STOP;
//        }
//    } one_shot_server;
//    Listen listener(one_shot_server, 3389, true, 5); // 25 seconds to connect, or timeout

    Inifile ini;
    int verbose = 511;
    ini.globals.debug.front = 511;

//    int nodelay = 1;
//    if (-1 == setsockopt(one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay))){
//        LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
//    }
//    wait_obj front_event(one_shot_server.sck);
//    SocketTransport front_trans("RDP Client", one_shot_server.sck, ini.globals.debug.front);

    LCGRandom gen(0);

    const char outdata[] =
    {
        #include "../tests/fixtures/trace_rdesktop_client_indata.hpp"
    };

    const char indata[] =
    {
        #include "../tests/fixtures/trace_rdesktop_client_outdata.hpp"
    };

    const char * name = "Test Front Transport";
    TestTransport front_trans(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    printf("test_rdesktop_client 1\n");
    Front front(&front_trans, &gen, &ini);
    printf("test_rdesktop_client 2\n");
    null_mod no_mod(front);

    printf("test_rdesktop_client 3\n");
    while (front.up_and_running == 0){
        front.incoming(no_mod);
    }
    BOOST_CHECK_EQUAL(1, front.up_and_running);
    printf("test_rdesktop_client 4\n");
    test_card_mod mod(front, front.client_info.width, front.client_info.height);
    printf("test_rdesktop_client 5\n");
    try {
        mod.draw_event();
    } catch (Error & e) {
        printf("Exception id=%u\n", e.id);
    }
    printf("test_rdesktop_client 6\n");

//    sleep(5);
//    shutdown(one_shot_server.sck, 2);
//    close(one_shot_server.sck);

//    LOG(LOG_INFO, "Listener closed\n");
//    LOG(LOG_INFO, "Incoming socket %d (ip=%s)\n", one_shot_server.sck, one_shot_server.ip_source);
}
