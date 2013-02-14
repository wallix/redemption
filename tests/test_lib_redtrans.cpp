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

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRedTransportLibrary
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> // recv, send
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "../libs/redtrans.h"

typedef enum {
    RT_TYPE_GENERATOR,
    RT_TYPE_CHECK,
    RT_TYPE_TEST,
    RT_TYPE_OUTFILE,
    RT_TYPE_INFILE,
    RT_TYPE_SOCKET,
    RT_TYPE_CLIENTSOCKET,
    RT_TYPE_OUTBYFILENAME, // based on OUTFILE
    RT_TYPE_INBYFILENAME, // based on INFILE
    RT_TYPE_OUTBYFILENAMESEQUENCE, // based on OUTFILE
    RT_TYPE_OUTBYFILENAMESEQUENCEWITHMETA, // based on OUTFILE
    RT_TYPE_INBYFILENAMESEQUENCE, // based on INFILE
    RT_TYPE_INBYMETASEQUENCE, // based on INFILE
    
} RT_TYPE;

typedef enum {
    RT_ERROR_OK,
    RT_ERROR_TYPE_MISMATCH,
    RT_ERROR_UNKNOWN_TYPE,
} RT_ERROR;

TODO("These classes are needing a large cleanup")

struct RT {
    unsigned rt_type;
    union {

      struct Generator {
        size_t current;
        uint8_t * data;
        size_t len;
      } generator;

      struct Check {
      } check;

      struct Test {
      } test;

      struct Outfile {
        int fd;
      } outfile;

      struct Infile {
        int fd;
      } infile;

      struct Socket {
      } socket;

      struct ClientSocket {
      } clientsocket;

      struct OutByFilename {
        int fd;
      } outbyfilename;

      struct InByFilename {
        int fd;
      } inbyfilename;

      struct OutByFilenameSequence {
        int fd;
      } outbyfilenamesequence;

      struct OutByFilenameSequenceWithMeta {
        int fd;
      } outbyfilenamesequencewithmeta;

      struct InByFilenameSequence {
        int fd;
      } inbyfilenamesequence;

      struct InByMetaSequence {
        int fd;
      } inbymetasequence;
    } u;
};

RT * rt_new(RT_TYPE t)
{
    RT * res = (RT*)malloc(sizeof(RT));
    res->rt_type = t;
    return res;
}

RT_ERROR rt_init_generator(RT * rt, const void * data, size_t len)
{
    if (rt->rt_type != RT_TYPE_GENERATOR){
        return RT_ERROR_TYPE_MISMATCH;
    }
    rt->u.generator.data = (uint8_t *)malloc(len);
    rt->u.generator.len = len;
    rt->u.generator.current = 0;
    memcpy(rt->u.generator.data, data, len);
    return RT_ERROR_OK;
}

RT_ERROR rt_init_outfile_writer(RT * rt, int fd)
{
    if (rt->rt_type != RT_TYPE_OUTFILE){
        return RT_ERROR_TYPE_MISMATCH;
    }
    rt->u.outfile.fd = fd;
    return RT_ERROR_OK;
}

RT_ERROR rt_init_infile_reader(RT * rt, int fd)
{
    if (rt->rt_type != RT_TYPE_INFILE){
        return RT_ERROR_TYPE_MISMATCH;
    }
    rt->u.infile.fd = fd;
    return RT_ERROR_OK;
}

ssize_t rt_internal_recv_t_generator(RT * rt, void * data, size_t len)
{
    if (rt->u.generator.current + len > rt->u.generator.len){
        size_t available_len = rt->u.generator.len - rt->u.generator.current;
        memcpy(data, (char*)rt->u.generator.data + rt->u.generator.current, available_len);
        rt->u.generator.current += available_len;
        return available_len;
    }
    memcpy(data, (char*)rt->u.generator.data + rt->u.generator.current, len);
    rt->u.generator.current += len;
    return len;
}

ssize_t rt_internal_recv_t_infile(RT * rt, void * data, size_t len)
{
    size_t ret = 0;
    size_t remaining_len = len;
    size_t total_len = 0;
    while (remaining_len) {
        ret = ::read(rt->u.infile.fd, (uint8_t*)data + total_len, remaining_len);
        if (ret < 0){
            if (errno == EINTR){
                continue;
            }
            return -1;
        }
        if (ret == 0){
            break;
        }
        remaining_len -= ret;
        total_len += ret;
    }
    return total_len;
}

ssize_t rt_recv(RT * rt, void * data, size_t len)
{
    switch (rt->rt_type){
    case RT_TYPE_GENERATOR:
        return rt_internal_recv_t_generator(rt, data, len);
    break;
    case RT_TYPE_INFILE:
        return rt_internal_recv_t_infile(rt, data, len);
    break;
    case RT_TYPE_OUTFILE:
        return -1;
    break;
    default:
        ;
    }
    return -1;
}

ssize_t rt_internal_send_t_outfile(RT * rt, const void * data, size_t len)
{
    ssize_t ret = 0;
    size_t remaining_len = len;
    size_t total_sent = 0;
    while (remaining_len) {
        ret = ::write(rt->u.outfile.fd, (uint8_t*)data + total_sent, remaining_len);
        if (ret <= 0){
            if (errno == EINTR){
                continue;
            }
            return -1;
        }
        remaining_len -= ret;
        total_sent += ret;
    }
    return total_sent;
}

ssize_t rt_send(RT * rt, void * data, size_t len)
{
    switch (rt->rt_type){
    case RT_TYPE_GENERATOR:
        return -1;
        break;
    case RT_TYPE_INFILE:
        return -1;
    break;
    case RT_TYPE_OUTFILE:
        return rt_internal_send_t_outfile(rt, data, len);
    break;
    default:
        ;
    }
    return -1;
}

void rt_close(RT * rt)
{
    switch(rt->rt_type){
        case RT_TYPE_GENERATOR:
        break;
        case RT_TYPE_OUTFILE:
            close(rt->u.outfile.fd);
        break;
        case RT_TYPE_INFILE:
            close(rt->u.infile.fd);
        break;
        default:
            ;
    }
    return;
}

void rt_delete(RT * rt)
{
    switch(rt->rt_type){
        case RT_TYPE_GENERATOR:
            free(rt->u.generator.data);
        break;
        case RT_TYPE_INFILE:
        break;
        case RT_TYPE_OUTFILE:
        break;
        default:
            ;
    }
    free(rt);
    return;
}


BOOST_AUTO_TEST_CASE(TestGeneratorTransport)
{
    RT * rt = rt_new(RT_TYPE_GENERATOR);
    BOOST_CHECK(NULL != rt);

    unsigned status = rt_init_generator(rt, "We read what we provide!", 24);
    BOOST_CHECK_EQUAL(0, status);

    uint8_t buffer[1024];
    
    BOOST_CHECK_EQUAL(3, rt_recv(rt, buffer, 3));
    BOOST_CHECK_EQUAL(0, memcmp("We ", buffer, 3));
    BOOST_CHECK_EQUAL(21, rt_recv(rt, buffer+3, 1024));
    BOOST_CHECK_EQUAL(0, memcmp("We read what we provide!", buffer, 24));
    BOOST_CHECK_EQUAL(0, rt_recv(rt, buffer+24, 1024));
    
    rt_close(rt);
    rt_delete(rt);
}

BOOST_AUTO_TEST_CASE(TestFileTransport)
{
    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");

    {
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        RT * rt = rt_new(RT_TYPE_OUTFILE);
        BOOST_CHECK(NULL != rt);

        unsigned status = rt_init_outfile_writer(rt, fd);
        BOOST_CHECK_EQUAL(0, status);

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

        RT * rt = rt_new(RT_TYPE_INFILE);
        BOOST_CHECK(NULL != rt);

        unsigned status = rt_init_infile_reader(rt, fd);
        BOOST_CHECK_EQUAL(0, status);

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

