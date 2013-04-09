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

   Template for new SQ_Intracker sequence class
*/

#ifndef _REDEMPTION_LIBS_SQ_INMETA_H_
#define _REDEMPTION_LIBS_SQ_INMETA_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio.h"
#include "sq_intracker.h"

extern "C" {
    struct SQInmeta {
        RIO_ERROR status;
        int fd;
        RIO * tracker;
        struct SQIntracker impl;
    };

    static inline RIO_ERROR sq_m_SQInmeta_constructor(SQInmeta * self, const char * prefix, const char * extension)
    {
        TODO("Manage all actual open error with more details")
        char tmpname[1024];
        if ((size_t)snprintf(tmpname, sizeof(tmpname), "%s%s", prefix, extension) >= sizeof(tmpname)){
            return RIO_ERROR_FILENAME_TOO_LONG;
        }
        self->fd = ::open(tmpname, O_RDONLY);
        if (self->fd < 0){
            return RIO_ERROR_OPEN;
        }
        self->status = RIO_ERROR_OK;
        RIO * rt = rio_new_infile(&self->status, self->fd);
        if (self->status != RIO_ERROR_OK){
            close(self->fd);
            return self->status;
        }
        self->tracker = rt;
        self->status = sq_m_SQIntracker_constructor(&self->impl, rt);
        if (self->status != RIO_ERROR_OK){
            close(self->fd);
            rio_delete(rt);
        }
        return self->status;
    }

    static inline RIO_ERROR sq_m_SQInmeta_destructor(SQInmeta * self)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        sq_m_SQIntracker_destructor(&self->impl);
        close(self->fd);
        rio_delete(self->tracker);
        self->tracker = NULL;
        self->status = RIO_ERROR_DESTRUCTED;
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQInmeta_get_trans(SQInmeta * self, RIO_ERROR * status)
    {
        if (self->status != RIO_ERROR_OK) {
            if (status) { *status = self->status; }
            return NULL;
        }
        RIO_ERROR status_res = RIO_ERROR_OK;
        RIO * res = sq_m_SQIntracker_get_trans(&self->impl, &status_res);
        if (!res){
            sq_m_SQInmeta_destructor(self);
            self->status = status_res;
        }
        return res;
    }

    static inline RIO_ERROR sq_m_SQInmeta_next(SQInmeta * self)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        RIO_ERROR res = sq_m_SQIntracker_next(&self->impl);
        if (res != RIO_ERROR_OK){
            sq_m_SQInmeta_destructor(self);
            self->status = res;
        }
        return res;
    }

    static inline RIO_ERROR sq_m_SQInmeta_timestamp(SQInmeta * self, timeval * tv)
    {
        return RIO_ERROR_OUT_SEQ_ONLY;
    }

    static inline RIO_ERROR sq_m_SQInmeta_get_chunk_info(SQInmeta * self,
                                                         unsigned * num_chunk,
                                                         char * path, size_t path_len,
                                                         timeval * begin,
                                                         timeval * end)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        RIO_ERROR res = sq_m_SQIntracker_get_chunk_info(&(self->impl), num_chunk, path, path_len, begin, end);
        if (res != RIO_ERROR_OK){
            sq_m_SQInmeta_destructor(self);
            self->status = res;
        }
        return res;
    }



    /*****************
    * SQCryptoInmeta *
    *****************/

    struct SQCryptoInmeta {
        RIO_ERROR status;
        RIO * tracker;
        struct SQCryptoIntracker impl;
    };

    static inline RIO_ERROR sq_m_SQCryptoInmeta_constructor(SQCryptoInmeta * self, const char * prefix, const char * extension)
    {
        TODO("Manage all actual open error with more details")
        char tmpname[1024];
        if ((size_t)snprintf(tmpname, sizeof(tmpname), "%s%s", prefix, extension) >= sizeof(tmpname)){
            return RIO_ERROR_FILENAME_TOO_LONG;
        }
        self->status = RIO_ERROR_OK;
        RIO * rt = rio_new_crypto(&self->status, tmpname, O_RDONLY);
        if (self->status != RIO_ERROR_OK){
            return self->status;
        }
        self->tracker = rt;
        self->status = sq_m_SQCryptoIntracker_constructor(&self->impl, rt);
        if (self->status != RIO_ERROR_OK){
            rio_delete(rt);
        }
        return self->status;
    }

    static inline RIO_ERROR sq_m_SQCryptoInmeta_destructor(SQCryptoInmeta * self)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        sq_m_SQCryptoIntracker_destructor(&self->impl);
        rio_delete(self->tracker);
        self->tracker = NULL;
        self->status = RIO_ERROR_DESTRUCTED;
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQCryptoInmeta_get_trans(SQCryptoInmeta * self, RIO_ERROR * status)
    {
        if (self->status != RIO_ERROR_OK) {
            if (status) { *status = self->status; }
            return NULL;
        }
        RIO_ERROR status_res = RIO_ERROR_OK;
        RIO * res = sq_m_SQCryptoIntracker_get_trans(&self->impl, &status_res);
        if (!res){
            sq_m_SQCryptoInmeta_destructor(self);
            self->status = status_res;
        }
        return res;
    }

    static inline RIO_ERROR sq_m_SQCryptoInmeta_next(SQCryptoInmeta * self)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        RIO_ERROR res = sq_m_SQCryptoIntracker_next(&self->impl);
        if (res != RIO_ERROR_OK){
            sq_m_SQCryptoInmeta_destructor(self);
            self->status = res;
        }
        return res;
    }

    static inline RIO_ERROR sq_m_SQCryptoInmeta_timestamp(SQCryptoInmeta * self, timeval * tv)
    {
        return RIO_ERROR_OUT_SEQ_ONLY;
    }

    static inline RIO_ERROR sq_m_SQCryptoInmeta_get_chunk_info(SQCryptoInmeta * self,
                                                         unsigned * num_chunk,
                                                         char * path, size_t path_len,
                                                         timeval * begin,
                                                         timeval * end)
    {
        if (self->status != RIO_ERROR_OK) { return self->status; }
        RIO_ERROR res = sq_m_SQCryptoIntracker_get_chunk_info(&(self->impl), num_chunk, path, path_len, begin, end);
        if (res != RIO_ERROR_OK){
            sq_m_SQCryptoInmeta_destructor(self);
            self->status = res;
        }
        return res;
    }
};

#endif

