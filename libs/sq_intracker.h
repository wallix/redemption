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

   Template for new SQ_Intracker sequence class

*/

#ifndef _REDEMPTION_LIBS_SQ_INTRACKER_H_
#define _REDEMPTION_LIBS_SQ_INTRACKER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rt_constants.h"


extern "C" {
    struct SQIntracker {
        RT * trans;
        RT * tracker;
        char name[1024];
        char buffer[2048];
        char * begin;
        char * end;
        char * eol;
        int rlstatus;
    };

    static inline bool sq_m_SQIntracker_readline(RT * trans, char ** begin, char **end, char **eol, char buffer[], size_t len)
    {
        for (char * p = *begin; p < *end; p++){
            if (*p == '\n'){
                *eol = p+1;
                return 1;
            }
        }
        size_t trailing_space = buffer + len - *end;
        // reframe buffer if no trailing space left
        if (trailing_space == 0){
            size_t used_len = *end - *begin;
            memmove(buffer, *begin, used_len);
            *end = buffer + used_len;
            *begin = buffer;
        }
        ssize_t res = 0;
        do {
            res = rt_recv(trans, *end, buffer + len - *end);
        } while ((res < 0) && (errno == EINTR));
        if (res < 0){
            if (res != RT_ERROR_EOF){
                return -res;
            }
        }
        if (res == 0){
            if (*begin != *end) {
                *eol = *end;
                return 0;
            }
            return -RT_ERROR_EOF;
        }
        *end += res;
        for (char * p = *begin; p < *end; p++){
            if (*p == '\n'){
                *eol = p+1;
                return 1;
            }
        }
        *eol = *end;
        return 0;
    }


    RT_ERROR sq_m_SQIntracker_constructor(SQIntracker * self, RT * tracker)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->begin = self->end = self->buffer;
        self->eol = NULL;
        self->rlstatus = sq_m_SQIntracker_readline(self->tracker, &self->begin, &self->end, &self->eol, self->buffer, sizeof(self->buffer));
        if (self->rlstatus < 0){
            return (RT_ERROR)-self->rlstatus;
        }
        return RT_ERROR_OK;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    size_t sq_im_SQIntracker_get_name(SQIntracker * self, char * buffer, size_t size)
    {
        size_t res = snprintf(buffer, size, "%s", self->begin, self->eol - self->begin);
        return res;
    }


    RT_ERROR sq_m_SQIntracker_destructor(SQIntracker * self)
    {
        if (self->trans){
            rt_delete(self->trans);
            self->trans = NULL;
        }
        return RT_ERROR_OK;
    }

    RT * sq_m_SQIntracker_get_trans(SQIntracker * self, RT_ERROR * status)
    {
        if (status && (*status != RT_ERROR_OK)) { return self->trans; }
        if (!self->trans){
            char tmpname[1024];
            sq_im_SQIntracker_get_name(self, tmpname, sizeof(tmpname));
            int fd = ::open(tmpname, O_RDONLY, S_IRUSR|S_IRUSR);
            if (fd < 0){
                if (status) { *status = RT_ERROR_OPEN; }
                return self->trans; // self->trans is NULL
            }
            self->trans = rt_new_infile(status, fd);
        }
        return self->trans;
    }

    RT_ERROR sq_m_SQIntracker_next(SQIntracker * self)
    {
        sq_m_SQIntracker_destructor(self);
        self->rlstatus = sq_m_SQIntracker_readline(self->tracker, &self->begin, &self->end, &self->eol, self->buffer, sizeof(self->buffer));
        if (self->rlstatus < 0){
            return (RT_ERROR)-self->rlstatus;
        }
        return RT_ERROR_OK;
    }
};

#endif

