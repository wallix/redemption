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

#include "sq_one.h"
#include "sq_outfilename.h"
#include "sq_infilename.h"
#include "sq_intracker.h"
#include "sq_meta.h"

#include "rt_generator.h"
#include "rt_check.h"
#include "rt_test.h"
#include "rt_outfile.h"
#include "rt_infile.h"
#include "rt_socket.h"
#include "rt_outsequence.h"
#include "rt_insequence.h"
#include "rt_outmeta.h"
#include "rt_inmeta.h"
#include "rt_inbyfilenamesequence.h"
#include "rt_outbyfilenamesequence.h"
#include "rt_outbyfilenamesequencewithmeta.h"
#include "rt_inbymetasequence.h"
#include "rt_XXX.h"

typedef enum {
    RT_TYPE_GENERATOR,
    RT_TYPE_CHECK,
    RT_TYPE_TEST,
    RT_TYPE_OUTFILE,
    RT_TYPE_INFILE,
    RT_TYPE_SOCKET,
    RT_TYPE_OUTSEQUENCE,
    RT_TYPE_OUTMETA,
    RT_TYPE_INMETA,
    RT_TYPE_INSEQUENCE,
} RT_TYPE;

typedef enum {
    SQ_TYPE_ONE,
    SQ_TYPE_OUTFILENAME,
    SQ_TYPE_INFILENAME,
    SQ_TYPE_INTRACKER,
    SQ_TYPE_META,
} SQ_TYPE;


struct SQ {
    unsigned sq_type;
    RT_ERROR err;
    union {
      struct SQOne one;
      struct SQOutfilename outfilename;
      struct SQInfilename infilename;
      struct SQIntracker intracker;
      struct SQMeta meta;
    } u;
};

SQ * sq_new_one_RT(RT_ERROR * error, RT * trans)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_ONE;
    res->err = sq_m_SQOne_constructor(&(res->u.one), trans);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        sq_m_SQOne_destructor(&(res->u.one));
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

SQ * sq_new_outfilename(RT_ERROR * error, RT * tracker, SQ_FORMAT format, const char * prefix, const char * extension)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_OUTFILENAME;
    res->err = sq_m_SQOutfilename_constructor(&(res->u.outfilename), tracker, format, prefix, extension);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_infilename(RT_ERROR * error, SQ_FORMAT format, const char * prefix, const char * extension)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_INFILENAME;
    res->err = sq_m_SQInfilename_constructor(&(res->u.infilename), format, prefix, extension);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_intracker(RT_ERROR * error, RT * tracker)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_INTRACKER;
    res->err = sq_m_SQIntracker_constructor(&(res->u.intracker), tracker);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_meta(RT_ERROR * error, const char * prefix, const char * extension)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_META;
    res->err = sq_m_SQMeta_constructor(&(res->u.meta), prefix, extension);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT_ERROR sq_next(SQ * seq)
{
    RT_ERROR res = RT_ERROR_OK;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        res = sq_m_SQOne_next(&(seq->u.one));
        break;
    case SQ_TYPE_OUTFILENAME:
        res = sq_m_SQOutfilename_next(&(seq->u.outfilename));
        break;
    case SQ_TYPE_INFILENAME:
        res = sq_m_SQInfilename_next(&(seq->u.infilename));
        break;
    case SQ_TYPE_INTRACKER:
        res = sq_m_SQIntracker_next(&(seq->u.intracker));
        break;
    case SQ_TYPE_META:
        res = sq_m_SQMeta_next(&(seq->u.meta));
        break;
    default:
        res = RT_ERROR_TYPE_MISMATCH;
    }
    return res;
}

RT * sq_get_trans(SQ * seq, RT_ERROR * error)
{
    RT_ERROR status = RT_ERROR_OK;
    RT * trans = NULL;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        trans = sq_m_SQOne_get_trans(&(seq->u.one), &status);
        break;
    case SQ_TYPE_OUTFILENAME:
        trans = sq_m_SQOutfilename_get_trans(&(seq->u.outfilename), &status);
        break;
    case SQ_TYPE_INFILENAME:
        trans = sq_m_SQInfilename_get_trans(&(seq->u.infilename), &status);
        break;
    case SQ_TYPE_INTRACKER:
        trans = sq_m_SQIntracker_get_trans(&(seq->u.intracker), &status);
        break;
    case SQ_TYPE_META:
        trans = sq_m_SQMeta_get_trans(&(seq->u.meta), &status);
        break;
    default:
        status = RT_ERROR_TYPE_MISMATCH;
    }
    if (error) { *error = status; }
    return trans;
}

RT_ERROR sq_delete(SQ * sq)
{
    RT_ERROR status = RT_ERROR_OK;
    switch(sq->sq_type){
        case SQ_TYPE_ONE:
            status = sq_m_SQOne_destructor(&(sq->u.one));
        break;
        case SQ_TYPE_OUTFILENAME:
            status = sq_m_SQOutfilename_destructor(&(sq->u.outfilename));
        break;
        case SQ_TYPE_INFILENAME:
            status = sq_m_SQInfilename_destructor(&(sq->u.infilename));
        break;
        case SQ_TYPE_INTRACKER:
            status = sq_m_SQIntracker_destructor(&(sq->u.intracker));
        break;
        case SQ_TYPE_META:
            status = sq_m_SQMeta_destructor(&(sq->u.meta));
        break;
        default:
            ;
    }
    free(sq);
    return status;
}


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
      struct RTOutsequence outsequence;
      struct RTInsequence insequence;
      struct RTOutmeta outmeta;
      struct RTInmeta inmeta;
//      struct RTInsequence insequence;
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
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_socket(RT_ERROR * error, int sck)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_SOCKET;
    res->err = rt_m_RTSocket_constructor(&(res->u.socket), sck);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_outsequence(RT_ERROR * error, SQ * seq)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_OUTSEQUENCE;
    res->err = rt_m_RTOutsequence_constructor(&(res->u.outsequence), seq);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_insequence(RT_ERROR * error, SQ * seq)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_INSEQUENCE;
    res->err = rt_m_RTInsequence_constructor(&(res->u.insequence), seq);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_outmeta(RT_ERROR * error, SQ ** seq, const char * prefix, const char * extension)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_OUTMETA;
    res->err = rt_m_RTOutmeta_constructor(&(res->u.outmeta), seq, prefix, extension);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RT_ERROR_OK:
        break;
    }
    return res;
}

RT * rt_new_inmeta(RT_ERROR * error, const char * prefix, const char * extension)
{
    RT * res = (RT*)malloc(sizeof(RT));
    if (res == 0){ 
        if (error){ *error = RT_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RT_TYPE_INMETA;
    res->err = rt_m_RTInmeta_constructor(&(res->u.inmeta), prefix, extension);
    switch (res->err){
    default:
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
        case RT_TYPE_SOCKET:
            status = rt_m_RTSocket_destructor(&(rt->u.socket));
        break;
        case RT_TYPE_OUTSEQUENCE:
            status = rt_m_RTOutsequence_destructor(&(rt->u.outsequence));
        break;
        case RT_TYPE_OUTMETA:
            status = rt_m_RTOutmeta_destructor(&(rt->u.outmeta));
        break;
        case RT_TYPE_INMETA:
            status = rt_m_RTInmeta_destructor(&(rt->u.inmeta));
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
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_CHECK:{
        ssize_t res = rt_m_RTCheck_recv(&(rt->u.check), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_TEST:{
        ssize_t res = rt_m_RTTest_recv(&(rt->u.test), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_INFILE:{
        ssize_t res = rt_m_RTInfile_recv(&(rt->u.infile), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTFILE:{
        ssize_t res = rt_m_RTOutfile_recv(&(rt->u.outfile), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_SOCKET:{
        ssize_t res = rt_m_RTSocket_recv(&(rt->u.socket), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTSEQUENCE:{
        ssize_t res = rt_m_RTOutsequence_recv(&(rt->u.outsequence), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_INSEQUENCE:{
        ssize_t res = rt_m_RTInsequence_recv(&(rt->u.insequence), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTMETA:{
        ssize_t res = rt_m_RTOutmeta_recv(&(rt->u.outmeta), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_INMETA:{
        ssize_t res = rt_m_RTInmeta_recv(&(rt->u.inmeta), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
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
    case RT_TYPE_SOCKET: {
        ssize_t res = rt_m_RTSocket_send(&(rt->u.socket), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTSEQUENCE: {
        ssize_t res = rt_m_RTOutsequence_send(&(rt->u.outsequence), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_OUTMETA: {
        ssize_t res = rt_m_RTOutmeta_send(&(rt->u.outmeta), data, len);
        if (res < 0){ rt->err = (RT_ERROR)-res; }
        return res;
    }
    case RT_TYPE_INMETA: {
        ssize_t res = rt_m_RTInmeta_send(&(rt->u.inmeta), data, len);
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
            rt_m_RTGenerator_close(&(rt->u.generator));
        break;
        case RT_TYPE_CHECK:
            rt_m_RTCheck_close(&(rt->u.check));
        break;
        case RT_TYPE_TEST:
            rt_m_RTTest_close(&(rt->u.test));
        break;
        case RT_TYPE_OUTFILE:
            rt_m_RTOutfile_close(&(rt->u.outfile));
        break;
        case RT_TYPE_INFILE:
            rt_m_RTInfile_close(&(rt->u.infile));
        break;
        case RT_TYPE_SOCKET:
            rt_m_RTSocket_close(&(rt->u.socket));
        break;
        case RT_TYPE_OUTMETA:
//            rt_m_RTOutmeta_close(&(rt->u.outmeta));
        break;
        case RT_TYPE_INMETA:
//            rt_m_RTInmeta_close(&(rt->u.inmeta));
        break;
        default:
            ;
    }
    /* after a close any subsequent call to recv/send/etc. raise an error */
    rt->err = RT_ERROR_CLOSED;
    return;
}

#endif
