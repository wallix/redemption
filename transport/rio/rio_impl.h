/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Main entry point file for RIO *Transport library
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_IMPL_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_IMPL_H_

#define RIOVERSION "0.1"

#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> // recv, send
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "fileutils.hpp"

#include "rio/rio.h"

#include "rio/sq_one.h"
#include "rio/sq_outfilename.h"
#include "rio/sq_cryptooutfilename.h"
#include "rio/sq_outtracker.h"
#include "rio/sq_cryptoouttracker.h"
#include "rio/sq_intracker.h"
#include "rio/sq_inmeta.h"

#include "rio/rio_generator.h"
#include "rio/rio_check.h"
#include "rio/rio_test.h"
#include "rio/rio_outfile.h"
#include "rio/rio_infile.h"
#include "rio/rio_socket.h"
#include "rio/rio_socket_TLS.h"
#include "rio/rio_outsequence.h"
#include "rio/rio_insequence.h"
#include "rio/rio_outmeta.h"
#include "rio/rio_crypto.h"
#include "rio/rio_cryptooutmeta.h"
#include "rio/rio_inmeta.h"
#include "rio/rio_outfilename.h"
#include "rio/rio_cryptooutfilename.h"
#include "rio/rio_infilename.h"
#include "rio/rio_cryptoinfilename.h"

TODO("add filter class sample")
TODO("convert PNG transport to new format")
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

extern "C" {
    const char * rio_version(){
        return RIOVERSION;
    }
};

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
    RIO_TYPE_CRYPTO,
    RIO_TYPE_CRYPTOOUTMETA,
    RIO_TYPE_CRYPTOINMETA,
    RIO_TYPE_OUTFILENAME,
    RIO_TYPE_CRYPTOOUTFILENAME,
    RIO_TYPE_CRYPTOINFILENAME,
} RIO_TYPE;

typedef enum {
    SQ_TYPE_ONE,
    SQ_TYPE_OUTFILENAME,
    SQ_TYPE_OUTTRACKER,
    SQ_TYPE_INTRACKER,
    SQ_TYPE_INMETA,
    SQ_TYPE_CRYPTOOUTFILENAME,
    SQ_TYPE_CRYPTOOUTTRACKER,
    SQ_TYPE_CRYPTOINTRACKER,
    SQ_TYPE_CRYPTOINMETA,
} SQ_TYPE;


struct SQ {
    unsigned sq_type;
    RIO_ERROR err;
    union {
      struct SQOne               one;
      struct SQOutfilename       outfilename;
      struct SQCryptoOutfilename cryptooutfilename;
      struct SQOuttracker        outtracker;
      struct SQCryptoOuttracker  cryptoouttracker;
      struct SQIntracker         intracker;
      struct SQCryptoIntracker   cryptointracker;
      struct SQInmeta            inmeta;
      struct SQCryptoInmeta      cryptoinmeta;
    } u;
};

struct RIO {
    unsigned rt_type;
    RIO_ERROR err;
    union {
      struct RIOGenerator         generator;
      struct RIOCheck             check;
      struct RIOTest              test;
      struct RIOOutfile           outfile;
      struct RIOInfile            infile;
      struct RIOSocket            socket;
      struct RIOSocketTLS         socket_tls;
      struct RIOOutsequence       outsequence;
      struct RIOInsequence        insequence;
      struct RIOOutmeta           outmeta;
      struct RIOInmeta            inmeta;
      struct RIOCrypto            crypto;
      struct RIOCryptoOutmeta     cryptooutmeta;
      struct RIOCryptoInmeta      cryptoinmeta;
      struct RIOOutfilename       outfilename;
      struct RIOInfilename        infilename;
      struct RIOCryptoOutfilename cryptooutfilename;
      struct RIOCryptoInfilename  cryptoinfilename;
    } u;
};


RIO_ERROR sq_init_one(SQ * self, RIO * trans)
{
    self->sq_type = SQ_TYPE_ONE;
    self->err = sq_m_SQOne_constructor(&(self->u.one), trans);
    return self->err;
}

SQ * sq_new_one(RIO_ERROR * error, RIO * trans)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_one(self, trans);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_outfilename(SQ * self, SQ_FORMAT format, const char * path, const char * filename, const char * extension, const int groupid)
{
    self->sq_type = SQ_TYPE_OUTFILENAME;
    self->err = sq_m_SQOutfilename_constructor(&(self->u.outfilename), format, path, filename, extension, groupid);
    return self->err;
}

SQ * sq_new_outfilename(RIO_ERROR * error, SQ_FORMAT format, const char * path, const char * filename, const char * extension, const int groupid)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_outfilename(self, format, path, filename, extension, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_cryptooutfilename(SQ * self, SQ_FORMAT format, const char * path, const char * filename, const char * extension, const int groupid)
{
    self->sq_type = SQ_TYPE_CRYPTOOUTFILENAME;
    self->err = sq_m_SQCryptoOutfilename_constructor(&(self->u.cryptooutfilename), format, path, filename, extension, groupid);
    return self->err;
}

SQ * sq_new_cryptooutfilename(RIO_ERROR * error, SQ_FORMAT format, const char * path, const char * filename, const char * extension, const int groupid)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_cryptooutfilename(self, format, path, filename, extension, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_outtracker(SQ * self, RIO * tracker,
    SQ_FORMAT format,
    const char * path, const char * filename, const char * extension,
    timeval * tv,
    const char * header1, const char * header2, const char * header3, const int groupid)
{
    self->sq_type = SQ_TYPE_OUTTRACKER;
    self->err = sq_m_SQOuttracker_constructor( &(self->u.outtracker)
                                             , tracker, format, path, filename
                                             , extension, tv, header1, header2
                                             , header3, groupid);
    return self->err;
}

SQ * sq_new_outtracker(RIO_ERROR * error, RIO * tracker,
        SQ_FORMAT format,
        const char * path, const char * filename, const char * extension,
        timeval * tv,
        const char * header1, const char * header2, const char * header3,
        const int groupid)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_outtracker(self, tracker, format, path, filename, extension, tv, header1, header2, header3, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_cryptoouttracker(SQ * self, RIO * tracker,
    SQ_FORMAT format,
    const char * path, const char * filename, const char * extension,
    timeval * tv,
    const char * header1, const char * header2, const char * header3,
    const int groupid)
{
    self->sq_type = SQ_TYPE_CRYPTOOUTTRACKER;
    self->err = sq_m_SQCryptoOuttracker_constructor(&(self->u.cryptoouttracker), tracker,
                                            format,
                                            path, filename, extension,
                                            tv, header1, header2, header3, groupid);
    return self->err;
}

SQ * sq_new_cryptoouttracker(RIO_ERROR * error, RIO * tracker,
        SQ_FORMAT format,
        const char * path, const char * filename, const char * extension,
        timeval * tv,
        const char * header1, const char * header2, const char * header3,
        const int groupid)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_cryptoouttracker(self, tracker, format, path, filename, extension, tv, header1, header2, header3, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_intracker(SQ * self, RIO * tracker, const char * meta_path)
{
    self->sq_type = SQ_TYPE_INTRACKER;
    self->err = sq_m_SQIntracker_constructor(&(self->u.intracker), tracker, meta_path);
    return self->err;
}

SQ * sq_new_intracker(RIO_ERROR * error, RIO * tracker, const char * meta_path)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_intracker(self, tracker, meta_path);
    if (res != RIO_ERROR_OK){
        if (error) { *error = res; }
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_cryptointracker(SQ * self, RIO * tracker, const char * meta_path)
{
    self->sq_type = SQ_TYPE_CRYPTOINTRACKER;
    self->err = sq_m_SQCryptoIntracker_constructor(&(self->u.cryptointracker), tracker,
        meta_path);
    return self->err;
}

SQ * sq_new_cryptointracker(RIO_ERROR * error, RIO * tracker, const char * meta_path)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_cryptointracker(self, tracker, meta_path);
    if (res != RIO_ERROR_OK){
        if (error) { *error = res; }
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_inmeta(SQ * self, const char * prefix, const char * extension)
{
    self->sq_type = SQ_TYPE_INMETA;
    self->err = sq_m_SQInmeta_constructor(&(self->u.inmeta), prefix, extension);
    return self->err;
}

SQ * sq_new_inmeta(RIO_ERROR * error, const char * prefix, const char * extension)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
//    self->sq_type = SQ_TYPE_INMETA;
    RIO_ERROR res = sq_init_inmeta(self, prefix, extension);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_init_cryptoinmeta(SQ * self, const char * prefix, const char * extension)
{
    self->sq_type = SQ_TYPE_CRYPTOINMETA;
    self->err = sq_m_SQCryptoInmeta_constructor(&(self->u.cryptoinmeta), prefix, extension);
    return self->err;
}

SQ * sq_new_cryptoinmeta(RIO_ERROR * error, const char * prefix, const char * extension)
{
    SQ * self = (SQ*)malloc(sizeof(SQ));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = sq_init_cryptoinmeta(self, prefix, extension);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR sq_get_chunk_info(SQ * seq, unsigned * num_chunk, char * path, size_t path_len, timeval * begin, timeval * end)
{
    RIO_ERROR res = RIO_ERROR_OK;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        res = sq_m_SQOne_get_chunk_info(&(seq->u.one), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_OUTFILENAME:
        res = sq_m_SQOutfilename_get_chunk_info(&(seq->u.outfilename), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_CRYPTOOUTFILENAME:
        res = sq_m_SQCryptoOutfilename_get_chunk_info(&(seq->u.cryptooutfilename), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_OUTTRACKER:
        res = sq_m_SQOuttracker_get_chunk_info(&(seq->u.outtracker), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_CRYPTOOUTTRACKER:
        res = sq_m_SQCryptoOuttracker_get_chunk_info(&(seq->u.cryptoouttracker), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_INTRACKER:
        res = sq_m_SQIntracker_get_chunk_info(&(seq->u.intracker), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_CRYPTOINTRACKER:
        res = sq_m_SQCryptoIntracker_get_chunk_info(&(seq->u.cryptointracker), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_INMETA:
        res = sq_m_SQInmeta_get_chunk_info(&(seq->u.inmeta), num_chunk, path, path_len, begin, end);
        break;
    case SQ_TYPE_CRYPTOINMETA:
        res = sq_m_SQCryptoInmeta_get_chunk_info(&(seq->u.cryptoinmeta), num_chunk, path, path_len, begin, end);
        break;
    default:
        res = RIO_ERROR_TYPE_MISMATCH;
    }
    return res;
}

RIO_ERROR sq_timestamp(SQ * seq, timeval * tv)
{
    RIO_ERROR res = RIO_ERROR_OK;
    switch (seq->sq_type){
    case SQ_TYPE_ONE:
        res = sq_m_SQOne_timestamp(&(seq->u.one), tv);
        break;
    case SQ_TYPE_OUTFILENAME:
        res = sq_m_SQOutfilename_timestamp(&(seq->u.outfilename), tv);
        break;
    case SQ_TYPE_CRYPTOOUTFILENAME:
        res = sq_m_SQCryptoOutfilename_timestamp(&(seq->u.cryptooutfilename), tv);
        break;
    case SQ_TYPE_OUTTRACKER:
        res = sq_m_SQOuttracker_timestamp(&(seq->u.outtracker), tv);
        break;
    case SQ_TYPE_CRYPTOOUTTRACKER:
        res = sq_m_SQCryptoOuttracker_timestamp(&(seq->u.cryptoouttracker), tv);
        break;
    case SQ_TYPE_INTRACKER:
        res = sq_m_SQIntracker_timestamp(&(seq->u.intracker), tv);
        break;
    case SQ_TYPE_CRYPTOINTRACKER:
        res = sq_m_SQCryptoIntracker_timestamp(&(seq->u.cryptointracker), tv);
        break;
    case SQ_TYPE_INMETA:
        res = sq_m_SQInmeta_timestamp(&(seq->u.inmeta), tv);
        break;
    case SQ_TYPE_CRYPTOINMETA:
        res = sq_m_SQCryptoInmeta_timestamp(&(seq->u.cryptoinmeta), tv);
        break;
    default:
        res = RIO_ERROR_TYPE_MISMATCH;
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
    case SQ_TYPE_CRYPTOOUTFILENAME:
        res = sq_m_SQCryptoOutfilename_next(&(seq->u.cryptooutfilename));
        break;
    case SQ_TYPE_OUTTRACKER:
        res = sq_m_SQOuttracker_next(&(seq->u.outtracker));
        break;
    case SQ_TYPE_CRYPTOOUTTRACKER:
        res = sq_m_SQCryptoOuttracker_next(&(seq->u.cryptoouttracker));
        break;
    case SQ_TYPE_INTRACKER:
        res = sq_m_SQIntracker_next(&(seq->u.intracker));
        break;
    case SQ_TYPE_CRYPTOINTRACKER:
        res = sq_m_SQCryptoIntracker_next(&(seq->u.cryptointracker));
        break;
    case SQ_TYPE_INMETA:
        res = sq_m_SQInmeta_next(&(seq->u.inmeta));
        break;
    case SQ_TYPE_CRYPTOINMETA:
        res = sq_m_SQCryptoInmeta_next(&(seq->u.cryptoinmeta));
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
    case SQ_TYPE_CRYPTOOUTFILENAME:
        trans = sq_m_SQCryptoOutfilename_get_trans(&(seq->u.cryptooutfilename), &status);
        break;
    case SQ_TYPE_OUTTRACKER:
        trans = sq_m_SQOuttracker_get_trans(&(seq->u.outtracker), &status);
        break;
    case SQ_TYPE_CRYPTOOUTTRACKER:
        trans = sq_m_SQCryptoOuttracker_get_trans(&(seq->u.cryptoouttracker), &status);
        break;
    case SQ_TYPE_INTRACKER:
        trans = sq_m_SQIntracker_get_trans(&(seq->u.intracker), &status);
        break;
    case SQ_TYPE_CRYPTOINTRACKER:
        trans = sq_m_SQCryptoIntracker_get_trans(&(seq->u.cryptointracker), &status);
        break;
    case SQ_TYPE_INMETA:
        trans = sq_m_SQInmeta_get_trans(&(seq->u.inmeta), &status);
        break;
    case SQ_TYPE_CRYPTOINMETA:
        trans = sq_m_SQCryptoInmeta_get_trans(&(seq->u.cryptoinmeta), &status);
        break;
    default:
        status = RIO_ERROR_TYPE_MISMATCH;
    }
    if (error) { *error = status; }
    return trans;
}

void sq_clear(SQ * sq)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (sq->err != RIO_ERROR_OK){ return; }
    switch(sq->sq_type){
        case SQ_TYPE_ONE:
            sq_m_SQOne_destructor(&(sq->u.one));
        break;
        case SQ_TYPE_OUTFILENAME:
            sq_m_SQOutfilename_destructor(&(sq->u.outfilename));
        break;
        case SQ_TYPE_CRYPTOOUTFILENAME:
            sq_m_SQCryptoOutfilename_destructor(&(sq->u.cryptooutfilename));
        break;
        case SQ_TYPE_OUTTRACKER:
            sq_m_SQOuttracker_destructor(&(sq->u.outtracker));
        break;
        case SQ_TYPE_CRYPTOOUTTRACKER:
            sq_m_SQCryptoOuttracker_destructor(&(sq->u.cryptoouttracker));
        break;
        case SQ_TYPE_INTRACKER:
            sq_m_SQIntracker_destructor(&(sq->u.intracker));
        break;
        case SQ_TYPE_CRYPTOINTRACKER:
            sq_m_SQCryptoIntracker_destructor(&(sq->u.cryptointracker));
        break;
        case SQ_TYPE_INMETA:
            sq_m_SQInmeta_destructor(&(sq->u.inmeta));
        break;
        case SQ_TYPE_CRYPTOINMETA:
            sq_m_SQCryptoInmeta_destructor(&(sq->u.cryptoinmeta));
        break;
        default:
            ;
    }
    sq->err = RIO_ERROR_CLOSED;
    return;
}

void sq_delete(SQ * self)
{
    if (!self) { return; }
    sq_clear(self);
    free(self);
}


RIO_ERROR rio_init_outfile(RIO * self, int fd)
{
    self->rt_type = RIO_TYPE_OUTFILE;
    self->err = rio_m_RIOOutfile_constructor(&(self->u.outfile), fd);
    return self->err;
}

RIO * rio_new_outfile(RIO_ERROR * error, int fd)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_outfile(self, fd);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_cryptooutfilename(RIO * self, const char * filename, const int groupid)
{
    self->rt_type = RIO_TYPE_CRYPTOOUTFILENAME;
    self->err = rio_m_RIOCryptoOutfilename_constructor(&(self->u.cryptooutfilename), filename, groupid);
    return self->err;
}

RIO * rio_new_cryptooutfilename(RIO_ERROR * error, const char * filename, const int groupid)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_cryptooutfilename(self, filename, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_infile(RIO * self, int fd)
{
    self->rt_type = RIO_TYPE_INFILE;
    self->err = rio_m_RIOInfile_constructor(&(self->u.infile), fd);
    return self->err;
}

RIO * rio_new_infile(RIO_ERROR * error, int fd)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_infile(self, fd);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_cryptoinfilename(RIO * self, const char * filename)
{
    self->rt_type = RIO_TYPE_CRYPTOINFILENAME;
    self->err = rio_m_RIOCryptoInfilename_constructor(&(self->u.cryptoinfilename), filename);
    return self->err;
}

RIO * rio_new_cryptoinfilename(RIO_ERROR * error, const char * filename)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_cryptoinfilename(self, filename);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_generator(RIO * self, const void * data, size_t len)
{
    self->rt_type = RIO_TYPE_GENERATOR;
    self->err = rio_m_RIOGenerator_constructor(&(self->u.generator), data, len);
    return self->err;
}

RIO * rio_new_generator(RIO_ERROR * error, const void * data, size_t len)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_generator(self, data, len);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_check(RIO * self, const void * data, size_t len)
{
    self->rt_type = RIO_TYPE_CHECK;
    self->err = rio_m_RIOCheck_constructor(&(self->u.check), data, len);
    return self->err;
}

RIO * rio_new_check(RIO_ERROR * error, const void * data, size_t len)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_check(self, data, len);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_test(RIO * self, const void * data_check, size_t len_check, const void * data_gen, size_t len_gen)
{
    self->rt_type = RIO_TYPE_TEST;
    self->err = rio_m_RIOTest_constructor(&(self->u.test), data_check, len_check, data_gen, len_gen);
    return self->err;
}

RIO * rio_new_test(RIO_ERROR * error, const void * data_check, size_t len_check, const void * data_gen, size_t len_gen)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_test(self, data_check, len_check, data_gen, len_gen);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_socket(RIO * self, int sck)
{
    self->rt_type = RIO_TYPE_SOCKET;
    self->err = rio_m_RIOSocket_constructor(&(self->u.socket), sck);
    return self->err;
}

RIO * rio_new_socket(RIO_ERROR * error, int sck)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_socket(self, sck);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_socket_tls(RIO * self, SSL * ssl)
{
    self->rt_type = RIO_TYPE_SOCKET_TLS;
    self->err = rio_m_RIOSocketTLS_constructor(&(self->u.socket_tls), ssl);
    return self->err;
}

RIO * rio_new_socket_tls(RIO_ERROR * error, SSL * ssl)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_socket_tls(self, ssl);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_outsequence(RIO * self, SQ * seq)
{
    self->rt_type = RIO_TYPE_OUTSEQUENCE;
    self->err = rio_m_RIOOutsequence_constructor(&(self->u.outsequence), seq);
    return self->err;
}

RIO * rio_new_outsequence(RIO_ERROR * error, SQ * seq)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_outsequence(self, seq);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_insequence(RIO * self, SQ * seq)
{
    self->rt_type = RIO_TYPE_INSEQUENCE;
    self->err = rio_m_RIOInsequence_constructor(&(self->u.insequence), seq);
    return self->err;
}

RIO * rio_new_insequence(RIO_ERROR * error, SQ * seq)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_insequence(self, seq);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_outmeta(RIO * self, SQ ** seq, const char * path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid)
{
    self->rt_type = RIO_TYPE_OUTMETA;
    self->err = rio_m_RIOOutmeta_constructor(&(self->u.outmeta), seq, path, filename, extension, l1, l2, l3, tv, groupid);
    return self->err;
}

RIO * rio_new_outmeta(RIO_ERROR * error, SQ ** seq, const char * path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_outmeta(self, seq, path, filename, extension, l1, l2, l3, tv, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_cryptooutmeta(RIO * self, SQ ** seq, const char * path, const char * hash_path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid)
{
    self->rt_type = RIO_TYPE_CRYPTOOUTMETA;
    self->err = rio_m_RIOCryptoOutmeta_constructor(&(self->u.cryptooutmeta), seq, path, hash_path, filename, extension, l1, l2, l3, tv, groupid);
    return self->err;
}

RIO * rio_new_cryptooutmeta(RIO_ERROR * error, SQ ** seq, const char * path, const char * hash_path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_cryptooutmeta(self, seq, path, hash_path, filename, extension, l1, l2, l3, tv, groupid);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_inmeta(RIO * self, SQ ** seq, const char * prefix, const char * extension)
{
    self->rt_type = RIO_TYPE_INMETA;
    self->err = rio_m_RIOInmeta_constructor(&(self->u.inmeta), seq, prefix, extension);
    return self->err;
}

RIO * rio_new_inmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_inmeta(self, seq, prefix, extension);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_init_cryptoinmeta(RIO * self, SQ ** seq, const char * prefix, const char * extension)
{
    self->rt_type = RIO_TYPE_CRYPTOINMETA;
    self->err = rio_m_RIOCryptoInmeta_constructor(&(self->u.cryptoinmeta), seq, prefix, extension);
    return self->err;
}

RIO * rio_new_cryptoinmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_cryptoinmeta(self, seq, prefix, extension);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}


RIO_ERROR rio_get_status(RIO * rt)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (rt->err != RIO_ERROR_OK){
        return rt->err;
    }
    switch(rt->rt_type){
        case RIO_TYPE_GENERATOR:
            rt->err = rio_m_RIOGenerator_get_status(&(rt->u.generator));
        break;
        case RIO_TYPE_CHECK:
            rt->err = rio_m_RIOCheck_get_status(&(rt->u.check));
        break;
        case RIO_TYPE_TEST:
            rt->err = rio_m_RIOTest_get_status(&(rt->u.test));
        break;
        case RIO_TYPE_OUTFILE:
            rt->err = rio_m_RIOOutfile_get_status(&(rt->u.outfile));
        break;
        case RIO_TYPE_INFILE:
            rt->err = rio_m_RIOInfile_get_status(&(rt->u.infile));
        break;
        case RIO_TYPE_SOCKET:
            rt->err = rio_m_RIOSocket_get_status(&(rt->u.socket));
        break;
        case RIO_TYPE_SOCKET_TLS:
            rt->err = rio_m_RIOSocketTLS_get_status(&(rt->u.socket_tls));
        break;
        case RIO_TYPE_OUTSEQUENCE:
            rt->err = rio_m_RIOOutsequence_get_status(&(rt->u.outsequence));
        break;
        case RIO_TYPE_INSEQUENCE:
            rt->err = rio_m_RIOInsequence_get_status(&(rt->u.insequence));
        break;
        case RIO_TYPE_OUTMETA:
            rt->err = rio_m_RIOOutmeta_get_status(&(rt->u.outmeta));
        break;
        case RIO_TYPE_INMETA:
            rt->err = rio_m_RIOInmeta_get_status(&(rt->u.inmeta));
        break;
        case RIO_TYPE_CRYPTO:
            rt->err = rio_m_RIOCrypto_get_status(&(rt->u.crypto));
        break;
        case RIO_TYPE_CRYPTOOUTMETA:
            rt->err = rio_m_RIOCryptoOutmeta_get_status(&(rt->u.cryptooutmeta));
        break;
        case RIO_TYPE_CRYPTOINMETA:
            rt->err = rio_m_RIOCryptoInmeta_get_status(&(rt->u.cryptoinmeta));
        break;
        case RIO_TYPE_CRYPTOOUTFILENAME:
            rt->err = rio_m_RIOCryptoOutfilename_get_status(&(rt->u.cryptooutfilename));
        break;
        case RIO_TYPE_CRYPTOINFILENAME:
            rt->err = rio_m_RIOCryptoInfilename_get_status(&(rt->u.cryptoinfilename));
        break;
        default:
            rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    return rt->err;
}

ssize_t rio_recv(RIO * rt, void * data, size_t len)
{
    if (rt->err != RIO_ERROR_OK){
        if (rt->err == RIO_ERROR_EOF){
            return 0;
        }
        return -rt->err;
    }
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
    case RIO_TYPE_CRYPTO:{
        ssize_t res = rio_m_RIOCrypto_recv(&(rt->u.crypto), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOOUTMETA:{
        ssize_t res = rio_m_RIOCryptoOutmeta_recv(&(rt->u.cryptooutmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOINMETA:{
        ssize_t res = rio_m_RIOCryptoInmeta_recv(&(rt->u.cryptoinmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOOUTFILENAME:{
        ssize_t res = rio_m_RIOCryptoOutfilename_recv(&(rt->u.cryptooutfilename), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOINFILENAME:{
        ssize_t res = rio_m_RIOCryptoInfilename_recv(&(rt->u.cryptoinfilename), data, len);
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
    case RIO_TYPE_INSEQUENCE: {
        ssize_t res = rio_m_RIOInsequence_send(&(rt->u.insequence), data, len);
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
    case RIO_TYPE_CRYPTO: {
        ssize_t res = rio_m_RIOCrypto_send(&(rt->u.crypto), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOOUTMETA: {
        ssize_t res = rio_m_RIOCryptoOutmeta_send(&(rt->u.cryptooutmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOINMETA: {
        ssize_t res = rio_m_RIOCryptoInmeta_send(&(rt->u.cryptoinmeta), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOOUTFILENAME: {
        ssize_t res = rio_m_RIOCryptoOutfilename_send(&(rt->u.cryptooutfilename), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    case RIO_TYPE_CRYPTOINFILENAME: {
        ssize_t res = rio_m_RIOCryptoInfilename_send(&(rt->u.cryptoinfilename), data, len);
        if (res < 0){ rt->err = (RIO_ERROR)-res; }
        return res;
    }
    default:
        rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    return -rt->err;
}

void rio_clear(RIO * rt)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (rt->err != RIO_ERROR_OK){
        return;
    }
    switch(rt->rt_type){
        case RIO_TYPE_GENERATOR:
            rt->err = rio_m_RIOGenerator_destructor(&(rt->u.generator));
        break;
        case RIO_TYPE_CHECK:
            rt->err = rio_m_RIOCheck_destructor(&(rt->u.check));
        break;
        case RIO_TYPE_TEST:
            rt->err = rio_m_RIOTest_destructor(&(rt->u.test));
        break;
        case RIO_TYPE_OUTFILE:
            rt->err = rio_m_RIOOutfile_destructor(&(rt->u.outfile));
        break;
        case RIO_TYPE_INFILE:
            rt->err = rio_m_RIOInfile_destructor(&(rt->u.infile));
        break;
        case RIO_TYPE_SOCKET:
            rt->err = rio_m_RIOSocket_destructor(&(rt->u.socket));
        break;
        case RIO_TYPE_SOCKET_TLS:
            rt->err = rio_m_RIOSocketTLS_destructor(&(rt->u.socket_tls));
        break;
        case RIO_TYPE_OUTSEQUENCE:
            rt->err = rio_m_RIOOutsequence_destructor(&(rt->u.outsequence));
        break;
        case RIO_TYPE_INSEQUENCE:
            rt->err = rio_m_RIOInsequence_destructor(&(rt->u.insequence));
        break;
        case RIO_TYPE_OUTMETA:
            rt->err = rio_m_RIOOutmeta_destructor(&(rt->u.outmeta));
        break;
        case RIO_TYPE_INMETA:
            rt->err = rio_m_RIOInmeta_destructor(&(rt->u.inmeta));
        break;
        case RIO_TYPE_CRYPTO:
            rt->err = rio_m_RIOCrypto_destructor(&(rt->u.crypto));
        break;
        case RIO_TYPE_CRYPTOOUTMETA:
            rio_m_RIOCryptoOutmeta_destructor(&(rt->u.cryptooutmeta));
        break;
        case RIO_TYPE_CRYPTOINMETA:
            rio_m_RIOCryptoInmeta_destructor(&(rt->u.cryptoinmeta));
        break;
        case RIO_TYPE_CRYPTOOUTFILENAME:
            rt->err = rio_m_RIOCryptoOutfilename_destructor(&(rt->u.cryptooutfilename));
        break;
        case RIO_TYPE_CRYPTOINFILENAME:
            rt->err = rio_m_RIOCryptoInfilename_destructor(&(rt->u.cryptoinfilename));
        break;
        default:
            rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    /* after a close any subsequent call to recv/send/etc. raise an error */
}

RIO_ERROR rio_sign(RIO * rt, unsigned char * buf, size_t size, size_t * len)
{
    /* if transport goes into error state it should be immediately flushed and closed (if it means something)
       hence no need to close it again calling close
    */
    if (rt->err != RIO_ERROR_OK){
        return rt->err;
    }
    switch(rt->rt_type){
        case RIO_TYPE_GENERATOR:
            rt->err = rio_m_RIOGenerator_sign(&(rt->u.generator), buf, size, len);
        break;
        case RIO_TYPE_CHECK:
            rt->err = rio_m_RIOCheck_sign(&(rt->u.check), buf, size, len);
        break;
        case RIO_TYPE_TEST:
            rt->err = rio_m_RIOTest_sign(&(rt->u.test), buf, size, len);
        break;
        case RIO_TYPE_OUTFILE:
            rt->err = rio_m_RIOOutfile_sign(&(rt->u.outfile), buf, size, len);
        break;
        case RIO_TYPE_INFILE:
            rt->err = rio_m_RIOInfile_sign(&(rt->u.infile), buf, size, len);
        break;
        case RIO_TYPE_SOCKET:
            rt->err = rio_m_RIOSocket_sign(&(rt->u.socket), buf, size, len);
        break;
        case RIO_TYPE_SOCKET_TLS:
            rt->err = rio_m_RIOSocketTLS_sign(&(rt->u.socket_tls), buf, size, len);
        break;
        case RIO_TYPE_OUTSEQUENCE:
            rt->err = rio_m_RIOOutsequence_sign(&(rt->u.outsequence), buf, size, len);
        break;
        case RIO_TYPE_INSEQUENCE:
            rt->err = rio_m_RIOInsequence_sign(&(rt->u.insequence), buf, size, len);
        break;
        case RIO_TYPE_OUTMETA:
            rt->err = rio_m_RIOOutmeta_sign(&(rt->u.outmeta), buf, size, len);
        break;
        case RIO_TYPE_INMETA:
            rt->err = rio_m_RIOInmeta_sign(&(rt->u.inmeta), buf, size, len);
        break;
        case RIO_TYPE_CRYPTO:
            rt->err = rio_m_RIOCrypto_sign(&(rt->u.crypto), buf, size, len);
        break;
        case RIO_TYPE_CRYPTOOUTMETA:
            rio_m_RIOCryptoOutmeta_sign(&(rt->u.cryptooutmeta), buf, size, len);
        break;
        case RIO_TYPE_CRYPTOINMETA:
            rio_m_RIOCryptoInmeta_sign(&(rt->u.cryptoinmeta), buf, size, len);
        break;
        case RIO_TYPE_CRYPTOOUTFILENAME:
            rt->err = rio_m_RIOCryptoOutfilename_sign(&(rt->u.cryptooutfilename), buf, size, len);
        break;
        case RIO_TYPE_CRYPTOINFILENAME:
            rt->err = rio_m_RIOCryptoInfilename_sign(&(rt->u.cryptoinfilename), buf, size, len);
        break;
        default:
            rt->err = RIO_ERROR_UNKNOWN_TYPE;
    }
    return rt->err;
}

void rio_delete(RIO * self)
{
    if (!self) { return; }
    rio_clear(self);
    free(self);
}


inline ssize_t sq_outfilename_filesize(const SQ * seq, uint32_t count)
{
    char filename[1024];
    sq_im_SQOutfilename_get_name(&(seq->u.outfilename), filename, sizeof(filename), count);
    return ::filesize(filename);
}

inline ssize_t sq_outfilename_unlink(const SQ * seq, uint32_t count)
{
    char filename[1024];
    sq_im_SQOutfilename_get_name(&(seq->u.outfilename), filename, sizeof(filename), count);
    int status = ::unlink(filename);
    if (status < 0){
        LOG(LOG_INFO, "removing file %s failed. Error [%u] : %s\n", filename, errno, strerror(errno));
    }
    return status;
}

inline void sq_outfilename_get_name(const SQ * seq, char * path, size_t len, uint32_t count)
{
    sq_im_SQOutfilename_get_name(&(seq->u.outfilename), path, len, count);
}

inline ssize_t sq_cryptooutfilename_filesize(const SQ * seq, uint32_t count)
{
    char filename[1024];
    sq_im_SQCryptoOutfilename_get_name(&(seq->u.cryptooutfilename), filename, sizeof(filename), count);
    return ::filesize(filename);
}

inline ssize_t sq_cryptooutfilename_unlink(const SQ * seq, uint32_t count)
{
    char filename[1024];
    sq_im_SQCryptoOutfilename_get_name(&(seq->u.cryptooutfilename), filename, sizeof(filename), count);
    int status = ::unlink(filename);
    if (status < 0){
        LOG(LOG_INFO, "removing file %s failed. Error [%u] : %s\n", filename, errno, strerror(errno));
    }
    return status;
}

inline void sq_cryptooutfilename_get_name(const SQ * seq, char * path, size_t len, uint32_t count)
{
    sq_im_SQCryptoOutfilename_get_name(&(seq->u.cryptooutfilename), path, len, count);
}


RIO_ERROR rio_init_crypto(RIO * self, const char * file, int oflag)
{
    self->rt_type = RIO_TYPE_CRYPTO;
    self->err = rio_m_RIOCrypto_constructor(&(self->u.crypto), file, oflag);
    return self->err;
}

RIO * rio_new_crypto(RIO_ERROR * error, const char * file, int oflag)
{
    RIO * self = (RIO *)malloc(sizeof(RIO));
    if (self == 0){
        if (error){ *error = RIO_ERROR_MALLOC; }
        return NULL;
    }
    RIO_ERROR res = rio_init_crypto(self, file, oflag);
    if (error) { *error = res; }
    if (res != RIO_ERROR_OK){
        free(self);
        return NULL;
    }
    return self;
}

#endif
