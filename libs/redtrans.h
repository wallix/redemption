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

   Main entry point file for RT Transport library

*/

#ifndef _REDEMPTION_LIBS_REDTRANS_H_
#define _REDEMPTION_LIBS_REDTRANS_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> // recv, send
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "rt_generator.h"
#include "rt_check.h"
#include "rt_test.h"
#include "rt_outfile.h"
#include "rt_infile.h"
#include "rt_inbyfilename.h"
#include "rt_outbyfilename.h"
#include "rt_inbyfilenamesequence.h"
#include "rt_outbyfilenamesequence.h"
#include "rt_outbyfilenamesequencewithmeta.h"
#include "rt_inbymetasequence.h"
#include "rt_socket.h"
#include "rt_clientsocket.h"
#include "rt_XXX.h"

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

struct RT {
    unsigned rt_type;
    union {

      struct RTGenerator generator;

      struct RTCheck check;

      struct RTTest test;

      struct RTOutfile outfile;

      struct RTInfile infile;

      struct RTSocket socket;

      struct RTClientSocket client_socket;

      struct RTOutByFilename out_by_filename;

      struct RTInByFilename in_by_filename;

      struct RTOutByFilenameSequence out_by_filename_sequence;

      struct RTOutByFilenameSequenceWithMeta out_by_filename_sequence_with_meta;

      struct RTInByFilenameSequence in_by_filename_sequence;

      struct RTInByMetaSequence in_by_meta_sequence;
    } u;
};

RT * rt_new_generator(RT_ERROR * error, const void * data, size_t len)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_OK; }
        return NULL;
    }
    res->rt_type = RT_TYPE_GENERATOR;
    RT_ERROR status = rt_m_RTGenerator_constructor(&(res->u.generator), data, len);
    switch (status){
    default:
        rt_m_RTGenerator_destructor(&(res->u.generator));
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_OK:
        if (error){ *error = RT_ERROR_OK; }
        break;
    }
    return res;
}


RT * rt_new_outfile(RT_ERROR * error, int fd)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_OK; }
        return NULL;
    }
    res->rt_type = RT_TYPE_OUTFILE;
    RT_ERROR status = rt_m_RTOutfile_constructor(&(res->u.outfile), fd);
    switch (status){
    default:
        rt_m_RTOutfile_destructor(&(res->u.outfile));
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_OK:
        if (error){ *error = RT_ERROR_OK; }
        break;
    }
    return res;
}

RT * rt_new_infile(RT_ERROR * error, int fd)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_OK; }
        return NULL;
    }
    res->rt_type = RT_TYPE_INFILE;
    RT_ERROR status = rt_m_RTInfile_constructor(&(res->u.infile), fd);
    switch (status){
    default:
        rt_m_RTInfile_destructor(&(res->u.infile));
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        if (error){ *error = status; }
        return NULL;
    case RT_ERROR_OK:
        if (error){ *error = RT_ERROR_OK; }
        break;
    }
    return res;
}

RT_ERROR rt_delete(RT * rt)
{
    RT_ERROR status = RT_ERROR_OK;
    switch(rt->rt_type){
        case RT_TYPE_GENERATOR:
            status = rt_m_RTGenerator_destructor(&(rt->u.generator));
        break;
        case RT_TYPE_INFILE:
            status = rt_m_RTInfile_destructor(&(rt->u.infile));
        break;
        case RT_TYPE_OUTFILE:
            status = rt_m_RTOutfile_destructor(&(rt->u.outfile));
        break;
        default:
            ;
    }
    free(rt);
    return status;
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
        return rt_m_RTGenerator_recv(&(rt->u.generator), data, len);
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

#endif
