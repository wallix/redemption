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

   Constants used by RIO *lib

*/

#ifndef _REDEMPTION_LIBS_RIO_CONSTANTS_H_
#define _REDEMPTION_LIBS_RIO_CONSTANTS_H_

extern "C" {

    typedef enum {
        RIO_ERROR_OK,
        RIO_ERROR_ANY,
        RIO_ERROR_MALLOC,
        RIO_ERROR_EOF,
        RIO_ERROR_EAGAIN,
        RIO_ERROR_EBADF,
        RIO_ERROR_EDESTADDRREQ,
        RIO_ERROR_EFAULT,
        RIO_ERROR_EFBIG,
        RIO_ERROR_EINVAL,
        RIO_ERROR_EIO,
        RIO_ERROR_ENOSPC,
        RIO_ERROR_EPIPE,
        RIO_ERROR_EISDIR,
        RIO_ERROR_POSIX,
        RIO_ERROR_RECV_ONLY,
        RIO_ERROR_SEND_ONLY,
        RIO_ERROR_DATA_MISMATCH,
        RIO_ERROR_TYPE_MISMATCH,
        RIO_ERROR_UNKNOWN_TYPE,
        RIO_ERROR_TRAILING_DATA,
        RIO_ERROR_CLOSED,
        RIO_ERROR_CREAT,
        RIO_ERROR_OPEN,
        RIO_ERROR_TLS_CONNECT_FAILED,
        RIO_ERROR_FILENAME_TOO_LONG,
        RIO_ERROR_NOT_IMPLEMENTED,
        RIO_ERROR_STRING_PREFIX_TOO_LONG,
        RIO_ERROR_STRING_EXTENSION_TOO_LONG,
        RIO_ERROR_INVALID_START_TIMESTAMP,
        RIO_ERROR_INVALID_STOP_TIMESTAMP,
        RIO_ERROR_OUT_ONLY,
    } RIO_ERROR;

    typedef enum {
        SQF_PREFIX_PID_COUNT_EXTENSION,
        SQF_PREFIX_COUNT_EXTENSION,
        SQF_PREFIX_EXTENSION,
    } SQ_FORMAT;

    struct RIO;
    struct SQ;

    // Forward headers for methods defined in redtrans 
    //(this allow to use them as an interface to individual transports for combining transports)

    RIO * rio_new_generator(RIO_ERROR * error, const void * data, size_t len);
    RIO_ERROR rio_init_generator(RIO * self, const void * data, size_t len);

    RIO * rio_new_check(RIO_ERROR * error, const void * data, size_t len);
    RIO_ERROR rio_init_check(RIO * self, const void * data, size_t len);

    RIO * rio_new_test(RIO_ERROR * error,
                         const void * data_check, size_t len_check, 
                         const void * data_gen, size_t len_gen);
    RIO_ERROR rio_init_test(RIO * self,
                         const void * data_check, size_t len_check, 
                         const void * data_gen, size_t len_gen);

    RIO * rio_new_outfile(RIO_ERROR * error, int fd);
    RIO_ERROR rio_init_outfile(RIO * self, int fd);

    RIO * rio_new_infile(RIO_ERROR * error, int fd);
    RIO_ERROR rio_init_infile(RIO * self, int fd);

    RIO * rio_new_socket(RIO_ERROR * error, int fd);
    RIO_ERROR rio_init_socket(RIO * self, int fd);

    RIO * rio_new_outsequence(RIO_ERROR * error, SQ * seq);
    RIO_ERROR rio_init_outsequence(RIO * self, SQ * seq);

    RIO * rio_new_insequence(RIO_ERROR * error, SQ * seq);    
    RIO_ERROR rio_init_insequence(RIO * self, SQ * seq);    

    RIO * rio_new_inmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension);
    RIO_ERROR rio_init_inmeta(RIO * self, const char * prefix, const char * extension);

    RIO * rio_new_outmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension, 
                      const char * l1, const char * l2, const char * l3, timeval * tv);
    RIO_ERROR rio_init_outmeta(RIO * self, SQ ** seq, const char * prefix, const char * extension, 
                      const char * l1, const char * l2, const char * l3, timeval * tv);

    void rio_delete(RIO * rt);
    void rio_clear(RIO * rt);
    ssize_t rio_recv(RIO * rt, void * data, size_t len);
    ssize_t rio_send(RIO * rt, const void * data, size_t len);

    RIO_ERROR rio_get_status(RIO * rt);

    SQ * sq_new_one(RIO_ERROR * error, RIO * trans);
    RIO_ERROR sq_init_one(SQ * self, RIO * trans);

    SQ * sq_new_outfilename(RIO_ERROR * error, RIO * tracker, SQ_FORMAT format, const char * prefix, const char * extension, timeval * tv);
    RIO_ERROR sq_init_outfilename(SQ * self, RIO * tracker, SQ_FORMAT format, const char * prefix, const char * extension, timeval * tv);

    SQ * sq_new_intracker(RIO_ERROR * error, RIO * tracker);
    RIO_ERROR sq_init_intracker(SQ * self, RIO * tracker);

    SQ * sq_new_inmeta(RIO_ERROR * error, const char * prefix, const char * extension);
    RIO_ERROR sq_init_inmeta(SQ * self, const char * prefix, const char * extension);

    void sq_delete(SQ * rt);
    void sq_clear(SQ * rt);
    RIO_ERROR sq_next(SQ * seq);
    RIO * sq_get_trans(SQ * seq, RIO_ERROR * error);


}

#endif
