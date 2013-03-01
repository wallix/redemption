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

   Main entry point file for RIO *Transport library

*/

#ifndef _REDEMPTION_LIBS_REDTRANS_H_
#define _REDEMPTION_LIBS_REDTRANS_H_

#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> // recv, send
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "sq_one.h"
#include "sq_outfilename.h"
//#include "sq_infilename.h"
#include "sq_intracker.h"
#include "sq_meta.h"

#include "rio_generator.h"
#include "rio_check.h"
#include "rio_test.h"
#include "rio_outfile.h"
#include "rio_infile.h"
#include "rio_socket.h"
#include "rio_socket_TLS.h"
#include "rio_outsequence.h"
#include "rio_insequence.h"
#include "rio_outmeta.h"
#include "rio_inmeta.h"
#include "rio_XXX.h"

TODO("finish coding of in/out meta classes (to add the 3 initial lines and timestamps after filename + possible keywords ?)")
TODO("add filter class sample")
TODO("Add versioning information for rio library (or use redemption version ?)")
TODO("replace Transport classes with calls to new methods")
TODO("convert PNG transport to new format")
TODO("write prototype python API for exposed methods")
TODO("check and enhance test coverage for lib")
TODO("check possible memory leaks (using valgrind on tests)")
TODO("check possible closes missing (especially when exiting on errors)")
TODO("check possible problems related to error management")
TODO("complete error lists with exhaustive possible system errors (and keep ERROR_ANY if we forget some)")
TODO("write filter classes to compress or encrypt/decrypt data on the fly")
TODO("write lib modification and tagging procedure")
TODO("create debian packager .deb (use git-builder as base sample) to build rio.deb package from github source")
TODO("extension: metadata files could be used to store non filename lines (meta lines could start with some reserved characters like ; ou #)"
     "This would be handy for large metadata that may not fit on one line")
TODO("C equivalent of LOG function (? PLAIN_C_LOG => PCLOG) : looks not necessary we can compile using C++ compiler we just have to provide C linkage entry points")

typedef enum {
    RIO_TYPE_GENERATOR,
    RIO_TYPE_CHECK,
    RIO_TYPE_TEST,
    RIO_TYPE_OUTFILE,
    RIO_TYPE_INFILE,
    RIO_TYPE_SOCKET,
    RIO_TYPE_SOCKET_TLS,
    RIO_TYPE_OUTSEQUENCE,
    RIO_TYPE_INSEQUENCE,
    RIO_TYPE_OUTMETA,
    RIO_TYPE_INMETA,
} RIO_TYPE;

typedef enum {
    SQ_TYPE_ONE,
    SQ_TYPE_OUTFILENAME,
    SQ_TYPE_INTRACKER,
    SQ_TYPE_META,
} SQ_TYPE;


struct SQ {
    unsigned sq_type;
    RIO_ERROR err;
    union {
      struct SQOne one;
      struct SQOutfilename outfilename;
      struct SQIntracker intracker;
      struct SQMeta meta;
    } u;
};

SQ * sq_new_one(RIO_ERROR * error, RIO * trans)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
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
    case RIO_ERROR_MALLOC:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_outfilename(RIO_ERROR * error, RIO * tracker, SQ_FORMAT format, const char * prefix, const char * extension)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_OUTFILENAME;
    res->err = sq_m_SQOutfilename_constructor(&(res->u.outfilename), tracker, format, prefix, extension);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_intracker(RIO_ERROR * error, RIO * tracker)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_INTRACKER;
    res->err = sq_m_SQIntracker_constructor(&(res->u.intracker), tracker);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

SQ * sq_new_meta(RIO_ERROR * error, const char * prefix, const char * extension)
{
    SQ * res = (SQ*)malloc(sizeof(SQ));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->sq_type = SQ_TYPE_META;
    res->err = sq_m_SQMeta_constructor(&(res->u.meta), prefix, extension);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO_ERROR sq_next(SQ * seq)
{
    RIO_ERROR res = RIO_ERROR_OK;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        res = sq_m_SQOne_next(&(seq->u.one));
        break;
    case SQ_TYPE_OUTFILENAME:
        res = sq_m_SQOutfilename_next(&(seq->u.outfilename));
        break;
    case SQ_TYPE_INTRACKER:
        res = sq_m_SQIntracker_next(&(seq->u.intracker));
        break;
    case SQ_TYPE_META:
        res = sq_m_SQMeta_next(&(seq->u.meta));
        break;
    default:
        res = RIO_ERROR_TYPE_MISMATCH;
    }
    return res;
}

RIO * sq_get_trans(SQ * seq, RIO_ERROR * error)
{
    RIO_ERROR status = RIO_ERROR_OK;
    RIO * trans = NULL;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        trans = sq_m_SQOne_get_trans(&(seq->u.one), &status);
        break;
    case SQ_TYPE_OUTFILENAME:
        trans = sq_m_SQOutfilename_get_trans(&(seq->u.outfilename), &status);
        break;
    case SQ_TYPE_INTRACKER:
        trans = sq_m_SQIntracker_get_trans(&(seq->u.intracker), &status);
        break;
    case SQ_TYPE_META:
        trans = sq_m_SQMeta_get_trans(&(seq->u.meta), &status);
        break;
    default:
        status = RIO_ERROR_TYPE_MISMATCH;
    }
    if (error) { *error = status; }
    return trans;
}

RIO_ERROR sq_delete(SQ * sq)
{
    RIO_ERROR status = RIO_ERROR_OK;
    switch(sq->sq_type){
        case SQ_TYPE_ONE:
            status = sq_m_SQOne_destructor(&(sq->u.one));
        break;
        case SQ_TYPE_OUTFILENAME:
            status = sq_m_SQOutfilename_destructor(&(sq->u.outfilename));
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


struct RIO {
    unsigned rt_type;
    RIO_ERROR err;
    union {

      struct RIOGenerator generator;
      struct RIOCheck check;
      struct RIOTest test;
      struct RIOOutfile outfile;
      struct RIOInfile infile;
      struct RIOSocket socket;
      struct RIOSocketTLS socket_tls;
      struct RIOOutsequence outsequence;
      struct RIOInsequence insequence;
      struct RIOOutmeta outmeta;
      struct RIOInmeta inmeta;
    } u;
};

RIO * rio_new_generator(RIO_ERROR * error, const void * data, size_t len)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_GENERATOR;
    res->err = rio_m_RIOGenerator_constructor(&(res->u.generator), data, len);
    if (*error) {*error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_check(RIO_ERROR * error, const void * data, size_t len)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_CHECK;
    res->err = rio_m_RIOCheck_constructor(&(res->u.check), data, len);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_test(RIO_ERROR * error, const void * data_check, size_t len_check, const void * data_gen, size_t len_gen)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_TEST;
    res->err = rio_m_RIOTest_constructor(&(res->u.test), data_check, len_check, data_gen, len_gen);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_outfile(RIO_ERROR * error, int fd)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_OUTFILE;
    res->err = rio_m_RIOOutfile_constructor(&(res->u.outfile), fd);
    if (error){ *error = res->err; }
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_infile(RIO_ERROR * error, int fd)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_INFILE;
    res->err = rio_m_RIOInfile_constructor(&(res->u.infile), fd);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_socket(RIO_ERROR * error, int sck)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_SOCKET;
    res->err = rio_m_RIOSocket_constructor(&(res->u.socket), sck);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_socket_tls(RIO_ERROR * error, int sck)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_SOCKET_TLS;
    res->err = rio_m_RIOSocketTLS_constructor(&(res->u.socket_tls), sck);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_outsequence(RIO_ERROR * error, SQ * seq)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_OUTSEQUENCE;
    res->err = rio_m_RIOOutsequence_constructor(&(res->u.outsequence), seq);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_insequence(RIO_ERROR * error, SQ * seq)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_INSEQUENCE;
    res->err = rio_m_RIOInsequence_constructor(&(res->u.insequence), seq);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_outmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension, 
                      const char * l1, const char * l2, const char * l3)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_OUTMETA;
    res->err = rio_m_RIOOutmeta_constructor(&(res->u.outmeta), seq, prefix, extension, l1, l2, l3);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}

RIO * rio_new_inmeta(RIO_ERROR * error, const char * prefix, const char * extension)
{
    RIO * res = (RIO *)malloc(sizeof(RIO));
    if (res == 0){ 
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    res->rt_type = RIO_TYPE_INMETA;
    res->err = rio_m_RIOInmeta_constructor(&(res->u.inmeta), prefix, extension);
    switch (res->err){
    default:
        free(res);
        return NULL;
    case RIO_ERROR_OK:
        break;
    }
    return res;
}


RIO_ERROR rio_delete(RIO * rt)
{
    RIO_ERROR status = RIO_ERROR_OK;
    switch(rt->rt_type){
        case RIO_TYPE_GENERATOR:
            status = rio_m_RIOGenerator_destructor(&(rt->u.generator));
        break;
        case RIO_TYPE_CHECK:
            status = rio_m_RIOCheck_destructor(&(rt->u.check));
        break;
        case RIO_TYPE_TEST:
            return rio_m_RIOTest_destructor(&(rt->u.test));
        break;
        case RIO_TYPE_INFILE:
            status = rio_m_RIOInfile_destructor(&(rt->u.infile));
        break;
        case RIO_TYPE_OUTFILE:
            status = rio_m_RIOOutfile_destructor(&(rt->u.outfile));
        break;
        case RIO_TYPE_SOCKET:
            status = rio_m_RIOSocket_destructor(&(rt->u.socket));
        break;
        case RIO_TYPE_SOCKET_TLS:
            status = rio_m_RIOSocketTLS_destructor(&(rt->u.socket_tls));
        break;
        case RIO_TYPE_OUTSEQUENCE:
            status = rio_m_RIOOutsequence_destructor(&(rt->u.outsequence));
        break;
        case RIO_TYPE_OUTMETA:
            status = rio_m_RIOOutmeta_destructor(&(rt->u.outmeta));
        break;
        case RIO_TYPE_INMETA:
            status = rio_m_RIOInmeta_destructor(&(rt->u.inmeta));
        break;
        default:
            ;
    }
    free(rt);
    return status;
}

RIO_ERROR rio_get_status(RIO * rt)
{
    return rt->err;
}


ssize_t rio_recv(RIO * rt, void * data, size_t len)
{
    if (rt->err != RIO_ERROR_OK){ return -rt->err; }
    switch (rt->rt_type){
    case RIO_TYPE_GENERATOR:{
        ssize_t res = rio_m_RIOGenerator_recv(&(rt->u.generator), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CHECK:{
        ssize_t res = rio_m_RIOCheck_recv(&(rt->u.check), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_TEST:{
        ssize_t res = rio_m_RIOTest_recv(&(rt->u.test), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_INFILE:{
        ssize_t res = rio_m_RIOInfile_recv(&(rt->u.infile), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTFILE:{
        ssize_t res = rio_m_RIOOutfile_recv(&(rt->u.outfile), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_SOCKET:{
        ssize_t res = rio_m_RIOSocket_recv(&(rt->u.socket), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_SOCKET_TLS:{
        ssize_t res = rio_m_RIOSocketTLS_recv(&(rt->u.socket_tls), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTSEQUENCE:{
        ssize_t res = rio_m_RIOOutsequence_recv(&(rt->u.outsequence), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_INSEQUENCE:{
        ssize_t res = rio_m_RIOInsequence_recv(&(rt->u.insequence), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTMETA:{
        ssize_t res = rio_m_RIOOutmeta_recv(&(rt->u.outmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_INMETA:{
        ssize_t res = rio_m_RIOInmeta_recv(&(rt->u.inmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    default:
        rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    return -rt->err;
}

ssize_t rio_send(RIO * rt, const void * data, size_t len)
{
    if (rt->err != RIO_ERROR_OK){ return -rt->err; }
    switch (rt->rt_type){
    case RIO_TYPE_GENERATOR: {
        ssize_t res = rio_m_RIOGenerator_send(&(rt->u.generator), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CHECK: {
        ssize_t res = rio_m_RIOCheck_send(&(rt->u.check), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_TEST: {
        ssize_t res = rio_m_RIOTest_send(&(rt->u.test), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_INFILE: {
        ssize_t res = rio_m_RIOInfile_send(&(rt->u.infile), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTFILE: {
        ssize_t res = rio_m_RIOOutfile_send(&(rt->u.outfile), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_SOCKET: {
        ssize_t res = rio_m_RIOSocket_send(&(rt->u.socket), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_SOCKET_TLS: {
        ssize_t res = rio_m_RIOSocketTLS_send(&(rt->u.socket_tls), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTSEQUENCE: {
        ssize_t res = rio_m_RIOOutsequence_send(&(rt->u.outsequence), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_OUTMETA: {
        ssize_t res = rio_m_RIOOutmeta_send(&(rt->u.outmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_INMETA: {
        ssize_t res = rio_m_RIOInmeta_send(&(rt->u.inmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    default:
        rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    return -rt->err;
}

void rio_close(RIO * rt)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (rt->err != RIO_ERROR_OK){ return; }
    switch(rt->rt_type){
        case RIO_TYPE_GENERATOR:
            rio_m_RIOGenerator_close(&(rt->u.generator));
        break;
        case RIO_TYPE_CHECK:
            rio_m_RIOCheck_close(&(rt->u.check));
        break;
        case RIO_TYPE_TEST:
            rio_m_RIOTest_close(&(rt->u.test));
        break;
        case RIO_TYPE_OUTFILE:
            rio_m_RIOOutfile_close(&(rt->u.outfile));
        break;
        case RIO_TYPE_INFILE:
            rio_m_RIOInfile_close(&(rt->u.infile));
        break;
        case RIO_TYPE_SOCKET:
            rio_m_RIOSocket_close(&(rt->u.socket));
        break;
        case RIO_TYPE_SOCKET_TLS:
            rio_m_RIOSocketTLS_close(&(rt->u.socket_tls));
        break;
        case RIO_TYPE_OUTMETA:
//            rio_m_RIOOutmeta_close(&(rt->u.outmeta));
        break;
        case RIO_TYPE_INMETA:
//            rio_m_RIOInmeta_close(&(rt->u.inmeta));
        break;
        default:
            ;
    }
    /* after a close any subsequent call to recv/send/etc. raise an error */
    rt->err = RIO_ERROR_CLOSED;
    return;
}

#endif
