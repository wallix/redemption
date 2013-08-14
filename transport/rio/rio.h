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

   Constants used by RIO *lib
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_RIO_H_
#define _REDEMPTION_TRANSPORT_RIO_RIO_H_

#include </usr/include/openssl/ssl.h>

extern "C" {
    typedef enum {
        RIO_ERROR_OK,                           // 0
        RIO_ERROR_ANY,
        RIO_ERROR_MALLOC,
        RIO_ERROR_EOF,
        RIO_ERROR_EAGAIN,
        RIO_ERROR_EBADF,
        RIO_ERROR_EDESTADDRREQ,
        RIO_ERROR_EFAULT,
        RIO_ERROR_EFBIG,
        RIO_ERROR_EINVAL,
        RIO_ERROR_EIO,                          // 10
        RIO_ERROR_ENOSPC,
        RIO_ERROR_EPIPE,
        RIO_ERROR_EISDIR,
        RIO_ERROR_POSIX,
        RIO_ERROR_RECV_ONLY,
        RIO_ERROR_SEND_ONLY,
        RIO_ERROR_DATA_MISMATCH,
        RIO_ERROR_TYPE_MISMATCH,
        RIO_ERROR_UNKNOWN_TYPE,
        RIO_ERROR_TRAILING_DATA,                // 20
        RIO_ERROR_CLOSE_FAILED,
        RIO_ERROR_CLOSED,
        RIO_ERROR_CREAT,
        RIO_ERROR_OPEN,
        RIO_ERROR_TLS_CONNECT_FAILED,
        RIO_ERROR_FILENAME_TOO_LONG,
        RIO_ERROR_NOT_IMPLEMENTED,
        RIO_ERROR_STRING_PREFIX_TOO_LONG,
        RIO_ERROR_STRING_PATH_TOO_LONG,
        RIO_ERROR_STRING_FILENAME_TOO_LONG,     // 30
        RIO_ERROR_STRING_EXTENSION_TOO_LONG,
        RIO_ERROR_INVALID_START_TIMESTAMP,
        RIO_ERROR_INVALID_STOP_TIMESTAMP,
        RIO_ERROR_OVERFLOW_START_TIMESTAMP,
        RIO_ERROR_OVERFLOW_STOP_TIMESTAMP,
        RIO_ERROR_OUT_ONLY,
        RIO_ERROR_OUT_SEQ_ONLY,
        RIO_ERROR_DESTRUCTED,
        RIO_ERROR_MORE_DATA,
        RIO_ERROR_SEEK_NOT_AVAILABLE,
        RIO_ERROR_RENAME,
    } RIO_ERROR;

    typedef enum {
        SQF_PATH_FILE_PID_COUNT_EXTENSION,
        SQF_PATH_FILE_COUNT_EXTENSION,
        SQF_PATH_FILE_PID_EXTENSION,
        SQF_PATH_FILE_EXTENSION,
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

    RIO * rio_new_outfilename(RIO_ERROR * error, const char * filename, const int groupid);
    RIO_ERROR rio_init_outfilename(RIO * self, const char * filename, const int groupid);

    RIO * rio_new_cryptooutfilename(RIO_ERROR * error, const char * filename, const int groupid);
    RIO_ERROR rio_init_cryptooutfilename(RIO * self, const char * filename, const int groupid);

    RIO * rio_new_infile(RIO_ERROR * error, int fd);
    RIO_ERROR rio_init_infile(RIO * self, int fd);

    RIO * rio_new_cryptoinfilename(RIO_ERROR * error, const char * filename);
    RIO_ERROR rio_init_cryptoinfilename(RIO * self, const char * filename);

    RIO * rio_new_socket(RIO_ERROR * error, int fd);
    RIO_ERROR rio_init_socket(RIO * self, int fd);

    RIO * rio_new_socket_tls(RIO_ERROR * error, SSL * ssl);
    RIO_ERROR rio_init_socket_tls(RIO * self, SSL * ssl);

    RIO * rio_new_outsequence(RIO_ERROR * error, SQ * seq);
    RIO_ERROR rio_init_outsequence(RIO * self, SQ * seq);

    RIO * rio_new_insequence(RIO_ERROR * error, SQ * seq);
    RIO_ERROR rio_init_insequence(RIO * self, SQ * seq);

    RIO * rio_new_inmeta(RIO_ERROR * error, SQ ** seq, const char * prefix, const char * extension);
    RIO_ERROR rio_init_inmeta(RIO * self, const char * prefix, const char * extension);

    RIO * rio_new_outmeta(RIO_ERROR * error, SQ ** seq, const char * path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid);
    RIO_ERROR rio_init_outmeta(RIO * self, SQ ** seq, const char * path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid);

    RIO * rio_new_cryptooutmeta(RIO_ERROR * error, SQ ** seq, const char * path, const char * hash_path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid);
    RIO_ERROR rio_init_cryptooutmeta(RIO * self, SQ ** seq, const char * path, const char * hash_path, const char * filename, const char * extension,
                      const char * l1, const char * l2, const char * l3, timeval * tv, const int groupid);

    void rio_delete(RIO * rt);
    void rio_clear(RIO * rt);

    RIO_ERROR rio_sign(RIO * rt, unsigned char * buf, size_t size, size_t * len);

    ssize_t rio_recv(RIO * rt, void * data, size_t len);
    ssize_t rio_send(RIO * rt, const void * data, size_t len);
    RIO_ERROR rio_seek(RIO * rt, int64_t offset, int whence);
    RIO_ERROR rio_get_status(RIO * rt);

    SQ * sq_new_one(RIO_ERROR * error, RIO * trans);
    RIO_ERROR sq_init_one(SQ * self, RIO * trans);

    SQ * sq_new_outfilename(RIO_ERROR * error,
        SQ_FORMAT format, const char * path, const char * filename, const char * extension,
        const int groupid);
    RIO_ERROR sq_init_outfilename(SQ * self,
        SQ_FORMAT format, const char * path, const char * filename, const char * extension,
        const int groupid);

    SQ * sq_new_cryptooutfilename(RIO_ERROR * error,
        SQ_FORMAT format, const char * path, const char * filename, const char * extension,
        const int groupid);
    RIO_ERROR sq_init_cryptooutfilename(SQ * self,
        SQ_FORMAT format, const char * path, const char * filename,
        const char * extension);

    SQ * sq_new_outtracker(RIO_ERROR * error, RIO * tracker,
                SQ_FORMAT format,
                const char * path, const char * filename, const char * extension,
                timeval * tv,
                const char * header1, const char * header2, const char * header3, const int groupid);
    RIO_ERROR sq_init_outtracker(SQ * self, RIO * tracker,
                SQ_FORMAT format,
                const char * path, const char * filename, const char * extension,
                timeval * tv,
                const char * header1, const char * header2, const char * header3, const int groupid);

    SQ * sq_new_cryptoouttracker(RIO_ERROR * error, RIO * tracker,
                SQ_FORMAT format,
                const char * path, const char * filename, const char * extension,
                timeval * tv,
                const char * header1, const char * header2, const char * header3, const int groupid);
    RIO_ERROR sq_init_cryptoouttracker(SQ * self, RIO * tracker,
                SQ_FORMAT format,
                const char * path, const char * filename, const char * extension,
                timeval * tv,
                const char * header1, const char * header2, const char * header3, const int groupid);

    SQ * sq_new_intracker(RIO_ERROR * error, RIO * tracker, const char * meta_path);
    RIO_ERROR sq_init_intracker(SQ * self, RIO * tracker, const char * meta_path);

    SQ * sq_new_cryptointracker(RIO_ERROR * error, RIO * tracker, const char * meta_path);
    RIO_ERROR sq_init_cryptointracker(SQ * self, RIO * tracker, const char * meta_path);

    SQ * sq_new_inmeta(RIO_ERROR * error, const char * prefix, const char * extension);
    RIO_ERROR sq_init_inmeta(SQ * self, const char * prefix, const char * extension);

    SQ * sq_new_cryptoinmeta(RIO_ERROR * error, const char * prefix, const char * extension);
    RIO_ERROR sq_init_cryptoinmeta(SQ * self, const char * prefix, const char * extension);

    void sq_delete(SQ * rt);
    void sq_clear(SQ * rt);
    RIO_ERROR sq_next(SQ * seq);
    RIO * sq_get_trans(SQ * seq, RIO_ERROR * error);
    RIO_ERROR sq_timestamp(SQ * seq, timeval * tv);
    RIO_ERROR sq_get_chunk_info(SQ * seq, unsigned & num_chunk, char * path, size_t path_len, timeval * begin, timeval * end);

    RIO * rio_new_crypto(RIO_ERROR * error, const char * file, int oflag);
    RIO_ERROR rio_init_crypto(RIO * self, const char * file, int oflag);
}

#endif

