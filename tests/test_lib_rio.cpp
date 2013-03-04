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
#define BOOST_TEST_MODULE TestRedTransportLibrary
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "../libs/rio.h"

BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_generator(&status, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer+24, 1024)); // EOF
    
    rio_close(rt);
    rio_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestCheckTransport)
{
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_check(&status, "We read what we provide!", 24);
    // Check Transport it somewhat similar to Generator Transport,
    // but instead of allowing to read what is provided
    // it checks that was is written to it is the provided reference string

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(7, rio_send(rt, "We read", 7));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(8, rio_send(rt, " what we!!!", 11));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(-RIO_ERROR_DATA_MISMATCH, rio_send(rt, "xxx", 3));
}

BOOST_AUTO_TEST_CASE(TestCheckTransport2)
{
    RIO_ERROR status = RIO_ERROR_OK;
//    RIO * rt = rio_new_test(&status, "output", 6, "input", 5);
    RIO * rt = rio_new_check(&status, "output", 6);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rio_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rio_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rio_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(rt, "xxx", 3));
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
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_test(&status, "output", 6, "input", 5);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);
    // If both strings match, return length of send buffer
    BOOST_CHECK_EQUAL(2, rio_send(rt, "ou", 2));
    // if mismatch return the length of the common part (at least one)
    BOOST_CHECK_EQUAL(4, rio_send(rt, "tput", 4));
    // Now the next call mismatch
    BOOST_CHECK_EQUAL(0, rio_send(rt, "xxx", 3));
    BOOST_CHECK_EQUAL(-RIO_ERROR_TRAILING_DATA, rio_send(rt, "xxx", 3));
}

BOOST_AUTO_TEST_CASE(TestTestTransport2)
{
    // Test Transport behave as a generator when we perform only receives
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_test(&status, "output", 6, "We read what we provide!", 24);

    BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer+24, 1024));
    
    rio_close(rt);
    rio_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_outfile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

        char buffer[1024];
        strcpy(buffer, "We read what we provide!");
        BOOST_CHECK_EQUAL(5, rio_send(rt, buffer, 5));
        BOOST_CHECK_EQUAL(19, rio_send(rt, buffer + 5, 19));

        rio_close(rt);
        rio_delete(rt);

        ::close(fd);
    }

    {
        int fd = ::open(tmpname, O_RDONLY);

        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_infile(&status, fd);
        BOOST_CHECK(NULL != rt);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, status);

        uint8_t buffer[1024];
        
        BOOST_CHECK_EQUAL(3, rio_recv(rt, buffer, 3));
        BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
        BOOST_CHECK_EQUAL(21, rio_recv(rt, buffer + 3, 1024));
        BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
        BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 24, 1024));
        
        rio_close(rt);
        rio_delete(rt);
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

// The Outsequence transport use one inderection level to find out where data should be sent
// A sequence is a very simple object that expose 2 methods, 
// sq_get_trans() return the current transport to use
// sq_next() goes forward to the next transport tu use

// The simplest possible sequence is the "one" sequence implemented below : 
// - sq_get_trans() always return the same transport (the one the sequence was initialized with)
// - sq_next() does nothing
// In the test below, we just wrap a check transport in a one_sequence
// hence the resultant outseuence object behave exactly like a check sequence

BOOST_AUTO_TEST_CASE(TestOutSequenceTransport_OneSequence)
{
    RIO_ERROR status_trans = RIO_ERROR_OK;
    RIO * out = rio_new_check(&status_trans, "AAAAXBBBBXCCCCX", 15);

    RIO_ERROR status_seq = RIO_ERROR_OK;
    SQ * sequence = sq_new_one(&status_seq, out);

    RIO_ERROR status = RIO_ERROR_OK;
    RIO * rt = rio_new_outsequence(&status, sequence);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sequence));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));

    rio_close(rt);
    rio_delete(rt);
}


BOOST_AUTO_TEST_CASE(TestOutSequenceTransport_OutfilenameSequence)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS-000000.txt", "TESTOFS-000001.txt"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

// Second simplest sequence is "outfilename" sequence
// - sq_get_trans() open an outfile if necessary using the given name pattern 
//      and return it on subsequent calls until it is closed
// - sq_next() close the current outfile and step to the next filename wich will 
//    be used by the next sq_get_trans to create an outfile transport.

// The test below is very similar to the previous one except for the creation of the sequence
    {
        RIO_ERROR status_seq = RIO_ERROR_OK;
        struct timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        SQ * sequence = sq_new_outfilename(&status_seq, NULL, SQF_PREFIX_COUNT_EXTENSION, "TESTOFS", "txt", &tv);

        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_outsequence(&status, sequence);

        BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
        sq_timestamp(sequence, &tv);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(sequence));
        BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));

        rio_close(rt);
        rio_delete(rt);
        
        sq_delete(sequence);
    }

// 3rd simplest sequence is "intracker" sequence
// - Behavior is identical to infilename sequence except the input pattern is
// a Transport that contains the list of the input files.
// - sq_get_trans() open an infile if necessary using the name it got from tracker
//   and return it on subsequent calls until it is closed (reach EOF)
// - sq_next() close the current outfile and step to the next filename wich will 
//    be used by the next sq_get_trans to create an outfile transport.

    {
        RIO_ERROR status = RIO_ERROR_OK;
        const char trackdata[] = 
            "800 600\n"
            "\n"
            "\n"
            "TESTOFS-000000.txt 1352304810 1352304870\n"
            "TESTOFS-000001.txt 1352304870 1352304930\n";

        RIO * tracker = rio_new_generator(&status, trackdata, sizeof(trackdata)-1);

        status = RIO_ERROR_OK;
        SQ * sequence = sq_new_intracker(&status, tracker);

        status = RIO_ERROR_OK;
        RIO * rt = rio_new_insequence(&status, sequence);

        char buffer[1024] = {};
        BOOST_CHECK_EQUAL(10, rio_recv(rt, buffer, 10));
        BOOST_CHECK_EQUAL(0, buffer[10]);
        if (0 != memcmp(buffer, "AAAAXBBBBX", 10)){
            LOG(LOG_ERR, "expected \"AAAAXBBBBX\" got \"%s\"\n", buffer);
        }
        BOOST_CHECK_EQUAL(5, rio_recv(rt, buffer + 10, 1024));
        BOOST_CHECK_EQUAL(0, memcmp(buffer, "AAAAXBBBBXCCCCX", 15));
        BOOST_CHECK_EQUAL(0, buffer[15]);
        BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 15, 1024));
        rio_close(rt);
        rio_delete(rt);
        sq_delete(sequence);
    }

    const char * file[] = {
        "TESTOFS-000000.txt",
        "TESTOFS-000001.txt"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) < 0){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}


// 5th simplest sequence is "meta" sequence
// - Behavior is identical to intacker sequence except the input pattern is the name of the tracker file
// - sq_get_trans() open an infile if necessary using the name it got from tracker
//   and return it on subsequent calls until it is closed (reach EOF)
// - sq_next() close the current outfile and step to the next filename wich will 
//    be used by the next sq_get_trans to create an outfile transport.

BOOST_AUTO_TEST_CASE(TestSequenceMeta)
{
    RIO_ERROR status = RIO_ERROR_OK;
    SQ * sequence = sq_new_inmeta(&status, "./tests/fixtures/TESTOFS", "mwrm");

    status = RIO_ERROR_OK;
    RIO * rt = rio_new_insequence(&status, sequence);

    char buffer[1024] = {};
    BOOST_CHECK_EQUAL(10, rio_recv(rt, buffer, 10));
    BOOST_CHECK_EQUAL(0, buffer[10]);
    if (0 != memcmp(buffer, "AAAAXBBBBX", 10)){
        LOG(LOG_ERR, "expected \"AAAAXBBBBX\" got \"%s\"\n", buffer);
    }
    BOOST_CHECK_EQUAL(5, rio_recv(rt, buffer + 10, 1024));
    BOOST_CHECK_EQUAL(0, memcmp(buffer, "AAAAXBBBBXCCCCX", 15));
    BOOST_CHECK_EQUAL(0, buffer[15]);
    BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 15, 1024));
    rio_close(rt);
    rio_delete(rt);
    sq_delete(sequence);
}

// Outmeta is a transport that manage file opening and chunking by itself
// We provide a base filename and it creates an outfilename sequence based on it
// A trace of this sequence is kept in an independant journal file that will
// be used later to reopen the same sequence as an input transport.
// chunking is performed externally, using the independant seq object created by constructor.
// metadata can be attached to individual chunks through seq object.

// The seq object memory allocation is performed by Outmeta,
// hence returned seq *must not* be explicitely deleted
// deleting transport will take care of it.

BOOST_AUTO_TEST_CASE(TestOutMeta)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    RIO_ERROR status = RIO_ERROR_OK;
    SQ * seq  = NULL;
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = 1352304810;
    RIO * rt = rio_new_outmeta(&status, &seq, "TESTOFS", "mwrm", "800 600\n", "\n", "\n", &tv);

    BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
    BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
    tv.tv_sec += 100;
    sq_timestamp(seq, &tv);
    BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));

    rio_close(rt);
    rio_delete(rt);

    {
        RIO_ERROR status = RIO_ERROR_OK;
        SQ * sequence = sq_new_inmeta(&status, "./tests/fixtures/TESTOFS", "mwrm");

        status = RIO_ERROR_OK;
        RIO * rt = rio_new_insequence(&status, sequence);

        char buffer[1024] = {};
        BOOST_CHECK_EQUAL(10, rio_recv(rt, buffer, 10));
        BOOST_CHECK_EQUAL(0, buffer[10]);
        if (0 != memcmp(buffer, "AAAAXBBBBX", 10)){
            LOG(LOG_ERR, "expected \"AAAAXBBBBX\" got \"%s\"\n", buffer);
        }
        BOOST_CHECK_EQUAL(5, rio_recv(rt, buffer + 10, 1024));
        BOOST_CHECK_EQUAL(0, memcmp(buffer, "AAAAXBBBBXCCCCX", 15));
        BOOST_CHECK_EQUAL(0, buffer[15]);
        BOOST_CHECK_EQUAL(0, rio_recv(rt, buffer + 15, 1024));
        rio_close(rt);
        rio_delete(rt);
        sq_delete(sequence);
    }

    const char * file[] = {
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) < 0){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestInmeta)
{
    // cleanup of possible previous test files
    {
        const char * file[] = {"TESTOFS.mwrm", "TESTOFS-000000.wrm", "TESTOFS-000001.wrm"};
        for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
            ::unlink(file[i]);
        }
    }

    {
        RIO_ERROR status = RIO_ERROR_OK;
        SQ * seq  = NULL;
        struct timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        RIO * rt = rio_new_outmeta(&status, &seq, "TESTOFS", "mwrm", "800 600\n", "\n", "\n", &tv);

        BOOST_CHECK_EQUAL( 5, rio_send(rt, "AAAAX",  5));
        tv.tv_sec+= 100;
        sq_timestamp(seq, &tv);
        BOOST_CHECK_EQUAL(RIO_ERROR_OK, sq_next(seq));
        BOOST_CHECK_EQUAL(10, rio_send(rt, "BBBBXCCCCX", 10));
        tv.tv_sec+= 100;
        sq_timestamp(seq, &tv);

        rio_close(rt);
        rio_delete(rt);
    }
    
    {
        RIO_ERROR status = RIO_ERROR_OK;
        RIO * rt = rio_new_inmeta(&status, "TESTOFS", "mwrm");
        BOOST_CHECK( rt != NULL);

        char buffer[1024] = {};
        BOOST_CHECK_EQUAL(15, rio_recv(rt, buffer,  15));
        if (0 != memcmp(buffer, "AAAAXBBBBXCCCCX", 15)){
            BOOST_CHECK_EQUAL(0, buffer[15]); // this one should not have changed
            buffer[15] = 0;
            LOG(LOG_ERR, "expected \"AAAAXBBBBXCCCCX\" got \"%s\"", buffer);
            BOOST_CHECK(false);
        }
    }    
    
    const char * file[] = {
        "TESTOFS.mwrm",
        "TESTOFS-000000.wrm",
        "TESTOFS-000001.wrm"
    };
    for (size_t i = 0 ; i < sizeof(file)/sizeof(char*) ; ++i){
        if (::unlink(file[i]) < 0){
            BOOST_CHECK(false);
            LOG(LOG_ERR, "failed to unlink %s", file[i]);
        }
    }
}

