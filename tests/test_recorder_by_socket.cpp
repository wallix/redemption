/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSocket
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>

// #define LOGPRINT

#include <netinet/tcp.h>
#include "transport.hpp"
#include "wrm_recorder.hpp"
#include "GraphicToFile.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "staticcapture.hpp"

#include "listen.hpp"
#include "socket_capture.hpp"

#include "check_sig.hpp"

TODO("This test does not behave properly if another process is already using the socket asked for. Change it so that it fails nicely. (change to be done is probably inside Server class")

class SessionRecorderTest : public Server
{
    virtual Server_status start(int incoming_sck)
    {
        struct sockaddr_in sin;
        unsigned int sin_size = sizeof(struct sockaddr_in);
        memset(&sin, 0, sin_size);
        TODO("We should manage accept errors")
        int sck = accept(incoming_sck, (struct sockaddr*)&sin, &sin_size);
        char ip_source[256];
        strcpy(ip_source, inet_ntoa(sin.sin_addr));
        close(incoming_sck);

        //printf("Setting new session socket to %d\n", sck);

        int nodelay = 1;
        if (0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay)))
        {
            WRMRecorder recorder(FIXTURES_PATH "/test_w2008_2-880.mwrm", FIXTURES_PATH);
            SocketCapture consumer("test", sck);
            recorder.consumer(&consumer);

            while (recorder.selected_next_order())
            {
                switch (recorder.chunk_type())
                {
                    case WRMChunk::BREAKPOINT:
                        recorder.ignore_breakpoint();
                        break;
                    case WRMChunk::META_FILE:
                    case WRMChunk::TIME_START:
                    case WRMChunk::TIMESTAMP:
                        recorder.ignore_chunks();
                        break;
                    default:
                        recorder.interpret_order();
                        break;
                }
            }
        }
        else
        {
            BOOST_CHECK_MESSAGE(false, "Failed to set socket TCP_NODELAY option on client socket");
            return START_FAILED;
        }

        //printf("Session::end of Session(%u)", sck);

        shutdown(sck, 2);
        close(sck);
        return START_WANT_STOP;
    }
};

BOOST_AUTO_TEST_CASE(TestSocket)
{
    pid_t pid = fork();
    switch (pid)
    {
        case 0: /* child */
        {
            SessionRecorderTest ss;
            //Inifile ini(CFG_PATH "/" RDPPROXY_INI);
            int port = 13389;
            Listen listener(ss, port);
        }
        break;
        default: /* father */
        {
            sleep(1);
            ClientSocketTransport t("test", "127.0.0.1", 13389);
            if (t.connect())
            {
                StaticCapture consumer(800, 600, "/tmp/socket_test.png", 0, 0, true);
                RDPUnserializer unserializer(&t, &consumer, Rect(0,0,800,600));
                while (unserializer.next())
                    ;

                char mess[1024];
                if (!check_sig(consumer.drawable, mess,
                    "\xd0\x8a\xe3\x69\x7c\x88\x91\xf8\xc4\xf5"
                    "\xd8\x90\xaa\xaa\xec\x13\xd0\xde\x1c\xe1"))
                {
                    BOOST_CHECK_MESSAGE(false, mess);
                }
            }
            else
            {
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
        }
        break;
        case -1:
            // error forking
            fprintf(stderr, "Error creating process for new session : %s\n", strerror(errno));
            BOOST_CHECK(false);
            break;
    }

//     BOOST_CHECK(false);
}
