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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>

#include "transport.hpp"
#include "error.hpp"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv(&p, 1);
    BOOST_CHECK_EQUAL(p-buffer, 1);
    BOOST_CHECK_EQUAL(buffer[0], 'W');
    BOOST_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 2);
    BOOST_CHECK_EQUAL(p-buffer, 2);
    BOOST_CHECK_EQUAL(buffer[0], 'e');
    BOOST_CHECK_EQUAL(buffer[1], ' ');
    BOOST_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 9);
    BOOST_CHECK_EQUAL(p-buffer, 9);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    gt.recv(&p, 12);
    BOOST_CHECK_EQUAL(p-buffer, 12);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 12));

    p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    try {
        gt.recv(&p, 1);
        BOOST_CHECK_EQUAL(true, false);
    } catch (Error e) {
        BOOST_CHECK_EQUAL(p-buffer, 0);
        BOOST_CHECK_EQUAL(e.id, (int)ERR_TRANSPORT_NO_MORE_DATA);
    };
}

BOOST_AUTO_TEST_CASE(TestGeneratorTransport2)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv(&p, 0);
    BOOST_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv(&p, 1);
    BOOST_CHECK_EQUAL(p-buffer, 1);
    BOOST_CHECK_EQUAL(buffer[0], 'W');
    BOOST_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 2);
    BOOST_CHECK_EQUAL(p-buffer, 2);
    BOOST_CHECK_EQUAL(buffer[0], 'e');
    BOOST_CHECK_EQUAL(buffer[1], ' ');
    BOOST_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv(&p, 9);
    BOOST_CHECK_EQUAL(p-buffer, 9);
    BOOST_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    try {
        gt.recv(&p, 13);
        BOOST_CHECK(false);
    } catch (Error e) {
        BOOST_CHECK_EQUAL(p-buffer, 12);
        BOOST_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 12));
        BOOST_CHECK_EQUAL(e.id, (int)ERR_TRANSPORT_NO_MORE_DATA);
    };
}

BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    {
        char tmpname[128];
        sprintf(tmpname, "/tmp/test_transportXXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        {
            OutFileTransport ft(fd);
            ft.send("We write, ", 10);
            ft.send("and again, ", 11);
            ft.send("and so on.", 10);
        }
        ::close(fd);
        fd = ::open(tmpname, O_RDONLY);
        {
            char buf[128];
            char * pbuf = buf;
            InFileTransport ft(fd);
            ft.recv(&pbuf, 10);
            ft.recv(&pbuf, 11);
            ft.recv(&pbuf, 10);
            BOOST_CHECK_EQUAL(0, strncmp(buf, "We write, and again, and so on.", 31));
        }
        ::close(fd);
        ::unlink(tmpname);
    }
}

BOOST_AUTO_TEST_CASE(TestCheckTransport)
{
    CheckTransport gt("input", 5);
    BOOST_CHECK_EQUAL(gt.status, true);
    try{
        gt.send("in", 2);
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK_EQUAL(gt.status, true);
    try{
        gt.send("pot", 3);
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint16_t)ERR_TRANSPORT_DIFFERS, (uint16_t)e.id);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}


BOOST_AUTO_TEST_CASE(TestCheckTransportInputOverflow)
{
    CheckTransport gt("0123456789ABCDEF", 16);
    BOOST_CHECK_EQUAL(gt.status, true);
    try {
        gt.send("0123456789ABCDEFGHI", 19);
    } catch (const Error & e)
    {
        BOOST_CHECK_EQUAL((uint32_t)e.id, (uint32_t)ERR_TRANSPORT_NO_MORE_DATA);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}

BOOST_AUTO_TEST_CASE(TestTestTransport)
{
    // TestTransport is bidirectional
    // We provide both an output and an input source
    // when using a test Transport we read what we provide in input source
    // and we check that what we write to output is identical to output source
    // if send fails, the difference between expected and actual data is showed
    // and status is set to false (and will stay so) to allow tests to fail.
    // inside Transport, the difference is shown in trace logs.
    TestTransport gt("Test1", "OUTPUT", 6, "input", 5);
    BOOST_CHECK_EQUAL(gt.status, true);
    char buf[128] = {};
    char * p = buf;
    uint32_t sz = 3;
    gt.recv(&p, sz);
    BOOST_CHECK(0 == memcmp(p - sz, "OUT", sz));
    gt.send("in", 2);
    BOOST_CHECK_EQUAL(gt.status, true);
    sz = 3;
    gt.recv(&p, sz);
    BOOST_CHECK(0 == memcmp(p - sz, "PUT", sz));
    try {
        gt.send("pot", 3);
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint16_t)ERR_TRANSPORT_DIFFERS, (uint16_t)e.id);
    };
    BOOST_CHECK_EQUAL(gt.status, false);
}

#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "listen.hpp"
#include "config.hpp"
#include "ssl_calls.hpp"
#include "wait_obj.hpp"
#include "server.hpp"


// This test is somewhat tricky
// The goal is to check that SocketTransport objects are working as expected
// in order to achieve that we have to 
// - create a listening socket, 
// - connect a client socket to it
// - accept connected client socket
// - send data at one side
// - receive it at the other side
// The problem is that we have to do everything at once and never block or fail on IO
// this is done through one select loop listening on everybody and non blocking calls
// (non blocking calls should not even be necessary thanks to select)

BOOST_AUTO_TEST_CASE(TestSocketTransport)
{
    class ServerOnce : public Server
    {
        public:
        virtual Server_status start(int incoming_sck) { return START_WANT_STOP; }
    } dummy;
    
    Listen listener(dummy, 4444, true, 25); // 25 seconds to connect, or timeout

    int nb_inbuffer = 0;
    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    uint8_t * p = buffer;
    
    int nb_recv_sck = 0;
    // 10 should be enough for testing
    int recv_sck[10];
    SocketTransport * sck_trans[10];

    bool run = true;

    const char * ip = "127.0.0.1";
    LOG(LOG_INFO, "connecting to %s:%d", ip, 4444);
    int client_sck = socket(PF_INET, SOCK_STREAM, 0);
    union
    {
      struct sockaddr s;
      struct sockaddr_storage ss;
      struct sockaddr_in s4;
      struct sockaddr_in6 s6;
    } ucs;
    memset(&ucs, 0, sizeof(ucs));
    ucs.s4.sin_family = AF_INET;
    ucs.s4.sin_port = htons(4444);
    ucs.s4.sin_addr.s_addr = inet_addr(ip);
    if (ucs.s4.sin_addr.s_addr == INADDR_NONE) {
        struct hostent *h = gethostbyname(ip);
        if (!h) {
            LOG(LOG_ERR, "DNS resolution failed for %s with errno =%d (%s)\n", ip, errno, strerror(errno));
            run = false;
        }
        ucs.s4.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
    }
    fcntl(client_sck, F_SETFL, fcntl(client_sck, F_GETFL) | O_NONBLOCK);

    int res = -1;
    int data_sent = 0;
    SocketTransport * client_trans = NULL;

    while (run){
        fd_set rfds;
        fd_set wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        BOOST_CHECK_EQUAL(true, true);
        int max = listener.sck;
        FD_SET(max, &rfds);
        
        for (int i = 0 ; i < nb_recv_sck ; i++){
            if (recv_sck[i] > max){
                max = recv_sck[i];
            }
            FD_SET(recv_sck[i], &rfds);
        }

        if (((client_trans != NULL) && (data_sent == 0)) 
        || (res == -1))
        {
            FD_SET(client_sck, &wfds);
            if (client_sck > max){
                max = client_sck;
            }
        }

        int num = select(max + 1, &rfds, &wfds, 0, &timeout);
    
        switch (num) {
        case 0:
            LOG(LOG_INFO, "woke up on timeout\n");
        break;
        default:
        {
            if (FD_ISSET(client_sck, &wfds)){
                if (client_trans && (data_sent == 0)){
                    client_trans->send("AAAAXBBBBXCCCCXDDDDX", 20);
                    data_sent = 20;
                }
                else if (res == -1) {
                    res = ::connect(client_sck, &ucs.s, sizeof(ucs));
                    if (res != -1){
                        client_trans = new SocketTransport("Sender", client_sck, 511);
                    }
                }
            }

            for (int i = 0 ; i < nb_recv_sck ; i++){
                if (FD_ISSET(recv_sck[i], & rfds)){
                    LOG(LOG_INFO, "activity on %d", recv_sck[i]);
                    sck_trans[i]->recv(&p, 5);
                    nb_inbuffer += 5;
                    LOG(LOG_INFO, "received %*s\n", nb_inbuffer, buffer);
                    if (nb_inbuffer == 20){
                        run = false;
                    }
                }
            }
            
            if (FD_ISSET(listener.sck, &rfds)){
                char ip_source[128];
                union
                {
                  struct sockaddr s;
                  struct sockaddr_storage ss;
                  struct sockaddr_in s4;
                  struct sockaddr_in6 s6;
                } u;
                memset(&u, 0, sizeof(u));
                u.s4.sin_family = AF_INET;
                unsigned int sin_size = sizeof(u);
                memset(&u, 0, sin_size);
                int sck = accept(listener.sck, &u.s, &sin_size);
                strcpy(ip_source, inet_ntoa(u.s4.sin_addr));
                LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", sck, ip_source);
                if (sck > 0){
                    recv_sck[nb_recv_sck] = sck;
                    sck_trans[nb_recv_sck] = new SocketTransport("Reader", sck, 511);
                    nb_recv_sck++;

                }
            }
        }
        break;
        case -1:
            if ((errno == EINTR)||(errno==EAGAIN)) { continue; }
            LOG(LOG_INFO, "stopped on error [%d] %s\n", num, strerror(errno));
            run = false;
        }
    }
    
}
