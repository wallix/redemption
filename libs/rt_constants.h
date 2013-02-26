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

   Constants used by RT lib

*/

#ifndef _REDEMPTION_LIBS_RT_CONSTANTS_H_
#define _REDEMPTION_LIBS_RT_CONSTANTS_H_

extern "C" {

    typedef enum {
        RT_ERROR_OK,
        RT_ERROR_ANY,
        RT_ERROR_MALLOC,
        RT_ERROR_EOF,
        RT_ERROR_EAGAIN,
        RT_ERROR_EBADF,
        RT_ERROR_EDESTADDRREQ,
        RT_ERROR_EFAULT,
        RT_ERROR_EFBIG,
        RT_ERROR_EINVAL,
        RT_ERROR_EIO,
        RT_ERROR_ENOSPC,
        RT_ERROR_EPIPE,
        RT_ERROR_EISDIR,
        RT_ERROR_POSIX,
        RT_ERROR_RECV_ONLY,
        RT_ERROR_SEND_ONLY,
        RT_ERROR_DATA_MISMATCH,
        RT_ERROR_TYPE_MISMATCH,
        RT_ERROR_UNKNOWN_TYPE,
        RT_ERROR_TRAILING_DATA,
        RT_ERROR_CLOSED,
        RT_ERROR_CREAT,
        RT_ERROR_OPEN,
        RT_ERROR_NOT_IMPLEMENTED,
        RT_ERROR_STRING_PREFIX_TOO_LONG,
        RT_ERROR_STRING_EXTENSION_TOO_LONG,
    } RT_ERROR;

    typedef enum {
        SQF_PREFIX_PID_COUNT_EXTENSION,
        SQF_PREFIX_COUNT_EXTENSION,
        SQF_PREFIX_EXTENSION,
    } SQ_FORMAT;

    struct RT;
    struct SQ;

    // Forward headers for methods defined in redtrans 
    //(this allow to use them as an interface to individual transports for combining transports)
    RT * rt_new_generator(RT_ERROR * error, const void * data, size_t len);
    RT * rt_new_check(RT_ERROR * error, const void * data, size_t len);
    RT * rt_new_test(RT_ERROR * error,
                         const void * data_check, size_t len_check, 
                         const void * data_gen, size_t len_gen);
    RT * rt_new_outfile(RT_ERROR * error, int fd);
    RT * rt_new_infile(RT_ERROR * error, int fd);
    RT * rt_new_socket(RT_ERROR * error, int fd);
    RT * rt_new_outsequence(RT_ERROR * error, SQ * seq);
    RT * rt_new_insequence(RT_ERROR * error, SQ * seq);    
    RT * rt_new_inmeta(RT_ERROR * error, const char * prefix, const char * extension);

    RT_ERROR rt_delete(RT * rt);
    RT_ERROR sq_delete(SQ * rt);

    ssize_t rt_recv(RT * rt, void * data, size_t len);
    ssize_t rt_send(RT * rt, const void * data, size_t len);

    RT_ERROR rt_get_status(RT * rt);
    SQ * sq_new_one_RT(RT_ERROR * error, RT * trans);
    RT_ERROR sq_next(SQ * seq);
    RT * sq_get_trans(SQ * seq, RT_ERROR * error);

    SQ * sq_new_outfilename(RT_ERROR * error, RT * tracker, SQ_FORMAT format, const char * prefix, const char * extension);
    SQ * sq_new_intracker(RT_ERROR * error, RT * tracker);
}

#endif
