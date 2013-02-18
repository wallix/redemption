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
    RT_ERROR err;
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
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_GENERATOR;
    res->err = rt_m_RTGenerator_constructor(&(res->u.generator), data, len);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        rt_m_RTGenerator_destructor(&(res->u.generator));
        free(res);
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_check(RT_ERROR * error, const void * data, size_t len)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_CHECK;
    res->err = rt_m_RTCheck_constructor(&(res->u.check), data, len);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        rt_m_RTCheck_destructor(&(res->u.check));
        free(res);
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_test(RT_ERROR * error, const void * data_check, size_t len_check, const void * data_gen, size_t len_gen)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_TEST;
    res->err = rt_m_RTTest_constructor(&(res->u.test), data_check, len_check, data_gen, len_gen);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        rt_m_RTTest_destructor(&(res->u.test));
        free(res);
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_outfile(RT_ERROR * error, int fd)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_OUTFILE;
    res->err = rt_m_RTOutfile_constructor(&(res->u.outfile), fd);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        rt_m_RTOutfile_destructor(&(res->u.outfile));
        free(res);
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_infile(RT_ERROR * error, int fd)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_INFILE;
    res->err = rt_m_RTInfile_constructor(&(res->u.infile), fd);
    switch (res->err){
    default:
        rt_m_RTInfile_destructor(&(res->u.infile));
        free(res);
        return NULL;
    case RT_ERROR_MALLOC:
        free(res);
        return NULL;
    case RT_ERROR_OK:
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
        case RT_TYPE_CHECK:
            status = rt_m_RTCheck_destructor(&(rt->u.check));
        break;
        case RT_TYPE_TEST:
            return rt_m_RTTest_destructor(&(rt->u.test));
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

RT_ERROR rt_get_status(RT * rt)
{
    return rt->err;
}


ssize_t rt_recv(RT * rt, void * data, size_t len)
{
    if (rt->err != RT_ERROR_OK){ return -rt->err; }
    switch (rt->rt_type){
    case RT_TYPE_GENERATOR:{
        ssize_t res = rt_m_RTGenerator_recv(&(rt->u.generator), data, len);
        if (res < 0){
            rt->err = (RT_ERROR)-res;
        }
        return res;
    }
    case RT_TYPE_CHECK:{
        ssize_t res = rt_m_RTCheck_recv(&(rt->u.check), data, len);
        if (res < 0){
            rt->err = (RT_ERROR)-res;
        }
        return res;
    }
    case RT_TYPE_TEST:{
        ssize_t res = rt_m_RTTest_recv(&(rt->u.test), data, len);
        if (res < 0){
            rt->err = (RT_ERROR)-res;
        }
        return res;
    }
    case RT_TYPE_INFILE:{
        ssize_t res = rt_m_RTInfile_recv(&(rt->u.infile), data, len);
        if (res < 0){
            rt->err = (RT_ERROR)-res;
        }
        return res;
    }
    case RT_TYPE_OUTFILE:{
        ssize_t res = rt_m_RTOutfile_recv(&(rt->u.outfile), data, len);
        if (res < 0){
            rt->err = (RT_ERROR)-res;
        }
        return res;
    }
    default:
        rt->err = RT_ERROR_UNKNOWN_TYPE;
    }
    return -rt->err;
}

ssize_t rt_send(RT * rt, const void * data, size_t len)
{
    if (rt->err != RT_ERROR_OK){ return -rt->err; }
    switch (rt->rt_type){
    case RT_TYPE_GENERATOR: {
        ssize_t res = rt_m_RTGenerator_send(&(rt->u.generator), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_CHECK: {
        ssize_t res = rt_m_RTCheck_send(&(rt->u.check), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_TEST: {
        ssize_t res = rt_m_RTTest_send(&(rt->u.test), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_INFILE: {
        ssize_t res = rt_m_RTInfile_send(&(rt->u.infile), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTFILE: {
        ssize_t res = rt_m_RTOutfile_send(&(rt->u.outfile), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    default:
        rt->err = RT_ERROR_UNKNOWN_TYPE;
    }
    return -rt->err;
}

void rt_close(RT * rt)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (rt->err != RT_ERROR_OK){ return; }
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
