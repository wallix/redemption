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

   Template for new SQ_Intracker sequence class

*/

#ifndef _REDEMPTION_LIBS_SQ_INTRACKER_H_
#define _REDEMPTION_LIBS_SQ_INTRACKER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio_constants.h"


extern "C" {
    struct SQIntracker {
        RIO * trans;
        RIO * tracker;
        char name[1024];
        char buffer[2048];
        int begin;
        int end;
        int eol;
        int rlstatus;
    };

    // input: begin : beginning of available buffer (already read)
    // input: end :   end of available buffer (already read)

    // return eol : end of next line
    // begin and end may be modified (buffer recentered, more data from input)
    // return 0 if no eol (end of file reached)
    // return 1 if \n found
    // (in other words : returns number of trailing eol characters at end of line)
    // return error codes as negative number
    static inline bool sq_m_SQIntracker_readline(SQIntracker * self)
    {
        for (int i = self->begin; i < self->end; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                return 1;
            }
        }
        size_t trailing_space = sizeof(self->buffer) - self->end;
        // reframe buffer if no trailing space left
        if (trailing_space == 0){
            size_t used_len = self->end - self->begin;
            memmove(self->buffer, &(self->buffer[self->begin]), used_len);
            self->end = used_len;
            self->begin = 0;
        }
        ssize_t res = rio_recv(self->tracker, &(self->buffer[self->end]), sizeof(self->buffer) - self->end);
        if (res < 0){
            return -res;
        }
        self->end += res;
        if (self->begin == self->end) {
            return -RIO_ERROR_EOF;
        }
        for (int i = self->begin; i < self->end; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                return 1;
            }
        }
        self->eol = self->end;
        return 0;
    }


    static inline RIO_ERROR sq_m_SQIntracker_constructor(SQIntracker * self, RIO * tracker)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->begin = self->eol = self->end = 0;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus < 0){
            return (RIO_ERROR)-self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    static inline size_t sq_im_SQIntracker_get_name(SQIntracker * self, char * buffer, size_t size)
    {
        size_t name_size = (unsigned)(self->eol - self->begin - self->rlstatus);
        if (self->begin == self->end) { return 0; }
        if (name_size >= size) {
            name_size = size - 1;
        }
        memcpy(buffer, &(self->buffer[self->begin]), name_size);
        buffer[name_size] = 0;
        return name_size;
    }


    static inline RIO_ERROR sq_m_SQIntracker_destructor(SQIntracker * self)
    {
        if (self->trans){
            rio_delete(self->trans);
            self->trans = NULL;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQIntracker_get_trans(SQIntracker * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return NULL; }
        if (!self->trans){
            if (self->rlstatus < 0){
                if (status) { *status = (RIO_ERROR)-self->rlstatus; }
                return NULL; // self->trans is NULL
            }
            char tmpname[1024];
            if (sq_im_SQIntracker_get_name(self, tmpname, sizeof(tmpname)) <= 0){
                if (status) { *status = RIO_ERROR_EOF; }
                return self->trans; // self->trans is NULL                
            }
            int fd = ::open(tmpname, O_RDONLY, S_IRUSR|S_IRUSR);
            if (fd < 0){
                if (status) { *status = RIO_ERROR_OPEN; }
                return self->trans; // self->trans is NULL
            }
            self->trans = rio_new_infile(status, fd);
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQIntracker_next(SQIntracker * self)
    {
        sq_m_SQIntracker_destructor(self);
        self->begin = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        return RIO_ERROR_OK;
    }
};

#endif

