/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
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
#define BOOST_TEST_MODULE TestSocketRIO
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include"log.hpp"

#include"rio/rio.h"
#include"rio/rio_impl.h"

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
BOOST_AUTO_TEST_CASE(TestSocket)
{
    int port = 5555;
    int listener_sck = socket(PF_INET, SOCK_STREAM, 0);

    /* reuse same port if a previous daemon was stopped */
    unsigned int option_len;
    int allow_reuse = 1;
    setsockopt(listener_sck, SOL_SOCKET, SO_REUSEADDR, (char*)&allow_reuse, sizeof(allow_reuse));

    /* set snd buffer to at least 32 Kbytes */
    int snd_buffer_size;
    option_len = sizeof(snd_buffer_size);
    if (0 == getsockopt(listener_sck, SOL_SOCKET, SO_SNDBUF,
        (char*)&snd_buffer_size, &option_len)) {
        if (snd_buffer_size < 32768) {
            snd_buffer_size = 32768;
            setsockopt(listener_sck, SOL_SOCKET, SO_SNDBUF,
                (char*)&snd_buffer_size, sizeof(snd_buffer_size));
        }
    }

    /* set non blocking */
    fcntl(listener_sck, F_SETFL, fcntl(listener_sck, F_GETFL) | O_NONBLOCK);

    union
    {
      struct sockaddr s;
      struct sockaddr_storage ss;
      struct sockaddr_in s4;
      struct sockaddr_in6 s6;
    } lu;
    memset(&lu, 0, sizeof(lu));
    lu.s4.sin_family = AF_INET;
    lu.s4.sin_port = htons(port);
    lu.s4.sin_addr.s_addr = INADDR_ANY;

    LOG(LOG_INFO, "Listen: binding socket %d on port %d", listener_sck, port);
    if (0 != bind(listener_sck, &lu.s, sizeof(lu))) {
        LOG(LOG_ERR, "Listen: error binding socket [errno=%u] %s", errno, strerror(errno));
        ((listener_sck) && (shutdown(listener_sck, 2), close(listener_sck)));
        if (listener_sck){
            shutdown(listener_sck, 2);
            close(listener_sck);
            BOOST_CHECK(false);
            return;
        }
    }

    LOG(LOG_INFO, "Listen: listening on socket %d", listener_sck);
    if (0 != listen(listener_sck, 2)) {
        LOG(LOG_ERR, "Listen: error listening on socket\n");
        if (listener_sck){
            shutdown(listener_sck, 2);
            close(listener_sck);
            BOOST_CHECK(false);
            return;
        }
    }

    int nb_inbuffer = 0;
    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    uint8_t * p = buffer;
    
    int nb_recv_sck = 0;
    // 10 should be enough for testing
    int recv_sck[10];
    RIO * sck_rt[10];

    bool run = true;

    const char * ip = "127.0.0.1";
    LOG(LOG_INFO, "connecting to %s:%d", ip, port);
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
    ucs.s4.sin_port = htons(port);
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

    int data_sent = 0;
    RIO * client_rt = NULL;

    while (run){
        fd_set rfds;
        fd_set wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        BOOST_CHECK_EQUAL(true, true);
        int max = listener_sck;
        FD_SET(max, &rfds);
        
        for (int i = 0 ; i < nb_recv_sck ; i++){
            FD_SET(recv_sck[i], &rfds);
            if (recv_sck[i] > max){ max = recv_sck[i]; }
        }

        FD_SET(client_sck, &wfds);
        if (client_sck > max){ max = client_sck; }

        int num = select(max + 1, &rfds, &wfds, 0, &timeout);
    
        switch (num) {
        case 0: // this is timeout : as everything is automated it should never happen
            BOOST_CHECK(false);
            return;
        break;
        default:
        {
            if (FD_ISSET(client_sck, &wfds)){
                 // connected client
                if (client_rt == NULL){
                    int res = ::connect(client_sck, &ucs.s, sizeof(ucs));
                    if (res < 0){
                        if (!try_again(errno)){
                            LOG(LOG_ERR, "conection failed with error %s", strerror(errno));
                            BOOST_CHECK(false);
                            return;
                        }
                    }
                    else {
                        RIO_ERROR status = RIO_ERROR_OK;
                        client_rt = rio_new_socket(&status, client_sck);
                        if ((client_rt == NULL) || (status != RIO_ERROR_OK)){
                            BOOST_CHECK(NULL != client_rt);
                            BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
                            return;
                        }
                    }
                }
                else {
                    // send data on client socket
                    if (data_sent < 20){
                        int res = rio_send(client_rt, "AAAAXBBBBXCCCCXDDDDX" + data_sent, 20 - data_sent);
                        if (res < 0){
                            BOOST_CHECK_EQUAL(RIO_ERROR_OK, (RIO_ERROR)(-res));
                            return;
                        }
                        data_sent += res;
                    }
                }
            }

            for (int i = 0 ; i < nb_recv_sck ; i++){
                // received data on connected socket (server side)
                if (FD_ISSET(recv_sck[i], & rfds)){
                    LOG(LOG_INFO, "received data activity on %d", recv_sck[i]);
                    int len = rio_recv(sck_rt[i], &(((char*)p)[nb_inbuffer]), 5);
                    if (len < 0){
                        BOOST_CHECK_EQUAL(RIO_ERROR_OK, (RIO_ERROR)(-len));
                        return;
                    }
                    nb_inbuffer += len;
                    LOG(LOG_INFO, "received %*s\n", nb_inbuffer, buffer);
                    if (0 != memcmp("AAAAXBBBBXCCCCXDDDDX", buffer, nb_inbuffer)){
                        BOOST_CHECK(false);
                        return;
                    }
                    if (nb_inbuffer >= 20){
                        run = false;
                    }
                }
            }
            // accept new connection on server socket
            if (FD_ISSET(listener_sck, &rfds)){
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
                int sck = accept(listener_sck, &u.s, &sin_size);
                strcpy(ip_source, inet_ntoa(u.s4.sin_addr));
                LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", sck, ip_source);
                if (sck < 0){
                    if (!try_again(errno)){
                        LOG(LOG_ERR, "accept failed with error %s", strerror(errno));
                        BOOST_CHECK(false);
                        return;
                    }
                }
                else {
                    recv_sck[nb_recv_sck] = sck;
                    RIO_ERROR status = RIO_ERROR_OK;
                    RIO * server_rt = rio_new_socket(&status, sck);
                    if ((server_rt == NULL) || (status != RIO_ERROR_OK)){
                        BOOST_CHECK(NULL != server_rt);
                        BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
                        return;
                    }
                    sck_rt[nb_recv_sck] = server_rt;
                    nb_recv_sck++;
                }
            }
        }
        break;
        case -1:
            if ((errno == EINTR)||(errno==EAGAIN)) { continue; }
            LOG(LOG_INFO, "select stopped on error [%d] %s\n", num, strerror(errno));
            run = false;
        }
    }
}
