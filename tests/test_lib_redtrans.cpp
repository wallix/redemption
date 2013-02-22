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
#define BOOST_TEST_MODULE TestRedTransportLibrary
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/redtrans.h"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_generator(&status, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(-RT_ERROR_EOF, rt_recv(rt, buffer+24, 1024));
    
    rt_close(rt);
    rt_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestCheckTransport)
{
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_check(&status, "We read what we provide!", 24);
    // Check Transport it somewhat similar to Generator Transport,
    // but instead of allowing to read what is provided
    // it checks that was is written to it is the provided reference string

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(7, rt_send(rt, "We read", 7));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(8, rt_send(rt, " what we!!!", 11));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(-RT_ERROR_DATA_MISMATCH, rt_send(rt, "xxx", 3));
}

BOOST_AUTO_TEST_CASE(TestCheckTransport2)
{
    RT_ERROR status = RT_ERROR_OK;
//    RT * rt = rt_new_test(&status, "output", 6, "input", 5);
    RT * rt = rt_new_check(&status, "output", 6);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rt_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rt_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rt_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RT_ERROR_TRAILING_DATA, rt_send(rt, "xxx", 3));
}

// TestTransport is basically (and internally) a Generator comined with a Check Transport
// It is designed fot testing purpose of bidirectional transports
// ie:
// - for testing an input file behavior, we will use a Generator
// - for testing an output file behavior, we wll use a Check
// - for testing a socket performing alternated sending and receiving, we use Test Transport
// (obviously a Test transport without input data will behave like Check,
// and a Test Transport without output data will behave like Generator)

BOOST_AUTO_TEST_CASE(TestTestTransport)
{
    // Test Transport behave as a Check when we perform only send
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_test(&status, "output", 6, "input", 5);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rt_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rt_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rt_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RT_ERROR_TRAILING_DATA, rt_send(rt, "xxx", 3));
}

BOOST_AUTO_TEST_CASE(TestTestTransport2)
{
    // Test Transport behave as a generator when we perform only receives
    RT_ERROR status = RT_ERROR_OK;
    RT * rt = rt_new_test(&status, "output", 6, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(-RT_ERROR_EOF, rt_recv(rt, buffer+24, 1024));
    
    rt_close(rt);
    rt_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        RT_ERROR status = RT_ERROR_OK;
        RT * rt = rt_new_outfile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

        char buffer[1024];
        strcpy(buffer, "We read what we provide!");
        BOOST_CHECK_EQUAL(5, rt_send(rt, buffer, 5));
        BOOST_CHECK_EQUAL(19, rt_send(rt, buffer + 5, 19));

        rt_close(rt);
        rt_delete(rt);

        ::close(fd);
    }

    {
        int fd = ::open(tmpname, O_RDONLY);

        RT_ERROR status = RT_ERROR_OK;
        RT * rt = rt_new_infile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);

        uint8_t buffer[1024];
        
        BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
        BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
        BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer + 3, 1024));
        BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
        BOOST_CHECK_EQUAL(0, rt_recv(rt, buffer + 24, 1024));
        
        rt_close(rt);
        rt_delete(rt);
    }
}


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
    RT * sck_rt[10];

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

    int res = -1;
    int data_sent = 0;
    RT * client_rt = NULL;

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
            if (recv_sck[i] > max){
                max = recv_sck[i];
            }
            FD_SET(recv_sck[i], &rfds);
        }

        if (((client_rt != NULL) && (data_sent == 0)) 
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
                if (client_rt && (data_sent == 0)){
                    int len = rt_send(client_rt, "AAAAXBBBBXCCCCXDDDDX", 20);
                    if (res < 0){
                        BOOST_CHECK_EQUAL(RT_ERROR_OK, (RT_ERROR)(-len));
                        return;
                    }
                    data_sent = 20;
                }
                else if (res == -1) {
                    res = ::connect(client_sck, &ucs.s, sizeof(ucs));
                    if (res != -1){
                        RT_ERROR status = RT_ERROR_OK;
                        client_rt = rt_new_socket(&status, client_sck);
                        BOOST_CHECK(NULL != client_rt);
                        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
                        if ((client_rt == NULL) || (status != RT_ERROR_OK)){
                            return;
                        }
                    }
                }
            }

            for (int i = 0 ; i < nb_recv_sck ; i++){
                if (FD_ISSET(recv_sck[i], & rfds)){
                    LOG(LOG_INFO, "activity on %d", recv_sck[i]);
                    int len = rt_recv(sck_rt[i], &(((char*)p)[nb_inbuffer]), 5);
                    if (len < 0){
                        BOOST_CHECK_EQUAL(RT_ERROR_OK, (RT_ERROR)(-len));
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
                if (sck > 0){
                    recv_sck[nb_recv_sck] = sck;
                    RT_ERROR status = RT_ERROR_OK;
                    RT * server_rt = rt_new_socket(&status, sck);
                    if ((server_rt == NULL) || (status != RT_ERROR_OK)){
                        BOOST_CHECK(NULL != server_rt);
                        BOOST_CHECK_EQUAL(RT_ERROR_OK, status);
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
            LOG(LOG_INFO, "stopped on error [%d] %s\n", num, strerror(errno));
            run = false;
        }
    }

}
