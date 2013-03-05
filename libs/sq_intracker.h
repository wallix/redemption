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
        timeval start_tv;
        timeval stop_tv;
    };

    // input: self->begin : beginning of available buffer (already read)
    // input: self->end :   end of available buffer (already read)
    // input: self->trans : IO data source

    // return self->eol : end of next line
    // self->begin and self->end may be modified (buffer recentered, more data from input)
    // return 0 if no eol (end of file reached)
    // return 1 if \n found
    // (in other words : returns number of trailing eol characters at end of line)
    // return error codes as negative number
    static inline int sq_m_SQIntracker_readline(SQIntracker * self)
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
            return (RIO_ERROR)-res;
        }
        self->end += res;
        if (self->begin == self->end) {
            return (RIO_ERROR)-RIO_ERROR_EOF;
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

    static inline RIO_ERROR sq_m_SQIntracker_destructor(SQIntracker * self)
    {
        if (self->trans){
            rio_delete(self->trans);
            self->trans = NULL;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_next(SQIntracker * self)
    {
        sq_m_SQIntracker_destructor(self);
        self->begin = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus < 0){
            return (RIO_ERROR)-self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_constructor(SQIntracker * self, RIO * tracker)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->begin = self->eol = self->end = 0;
        
        // First header line
        RIO_ERROR status = sq_m_SQIntracker_next(self);
        if (status != RIO_ERROR_OK){ return (RIO_ERROR)-status; }
        // Second header line
        status = sq_m_SQIntracker_next(self);
        if (status != RIO_ERROR_OK){ return (RIO_ERROR)-status; }
        // 3rd header line
        status = sq_m_SQIntracker_next(self);
        if (status != RIO_ERROR_OK){ return (RIO_ERROR)-status; }
        // First real filename line
        status = sq_m_SQIntracker_next(self);
        if (status != RIO_ERROR_OK){ return (RIO_ERROR)-status; }
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
            TODO("really this is a line, not a filename, we have to manage timestamps after filename");
            char line[1024];
            size_t name_size = (unsigned)(self->eol - self->begin - self->rlstatus);
            if (self->begin == self->end) {
                if (status) { *status = RIO_ERROR_EOF; }
                return self->trans; // self->trans is NULL                
            }
            if (name_size >= sizeof(line)) {
                name_size = sizeof(line) - 1;
            }
            memcpy(line, &(self->buffer[self->begin]), name_size);
            line[name_size] = 0;
            unsigned stop_sec = 0;
            ssize_t i = name_size - 1;
            for ( ; i >= 0 ; i--){
                if (line[i] >= '0' && line[i] <= '9'){
                    stop_sec = stop_sec * 10 + (line[i]-'0');
                    TODO("add check for tv overflow");
                    continue;
                }
                if (line[i] == ' '){
                    self->stop_tv.tv_sec = stop_sec;
                    self->stop_tv.tv_usec = 0;
                    i--;
                    break;
                }
                if (status) {
                    TODO("close trans");
                    *status = (RIO_ERROR)-RIO_ERROR_INVALID_STOP_TIMESTAMP;
                }
                return NULL;
            }
            unsigned start_sec = 0;
            for ( ; i >= 0 ; i--){
                if (line[i] >= '0' && line[i] <= '9'){
                    start_sec = start_sec * 10 + (line[i]-'0');
                    TODO("add check for tv overflow");
                    continue;
                }
                if (line[i] == ' '){
                    self->start_tv.tv_sec = start_sec;
                    self->start_tv.tv_usec = 0;
                    line[i] = 0;
                    break;
                }
                if (status) {
                    TODO("close trans");
                    *status = (RIO_ERROR)-RIO_ERROR_INVALID_START_TIMESTAMP;
                }
                return NULL;
            }
            int fd = ::open(line, O_RDONLY, S_IRUSR|S_IRUSR);
            if (fd < 0){
                if (status) { *status = RIO_ERROR_OPEN; }
                return self->trans; // self->trans is NULL
            }
            self->trans = rio_new_infile(status, fd);
        }
        return self->trans;
    }
    
    static inline RIO_ERROR sq_m_SQIntracker_timestamp(SQIntracker * self, timeval * tv)
    {
        return RIO_ERROR_OK;
    }

};

#endif

