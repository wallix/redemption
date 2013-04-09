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

#ifndef _REDEMPTION_LIBS_SQ_INTRACKER_H_
#define _REDEMPTION_LIBS_SQ_INTRACKER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio.h"

extern "C" {
    struct SQIntracker {
        int fd;
        RIO * trans;
        RIO * tracker;
//        TODO("CGR: check if name is used ?")
        char name[1024];
        char buffer[2048];
        int begin_line;
        int end_line;
        int eol;
        int eollen;
        RIO_ERROR rlstatus;
        char header1[1024];
        char header2[1024];
        char header3[1024];
        char line[1024];
        timeval start_tv;
        timeval stop_tv;
        int num_chunk;
    };

    // input: self->begin_line : beginning of available buffer (already read)
    // input: self->end_line :   end of available buffer (already read)
    // input: self->trans : IO data source

    // return self->eol : end of next line
    // self->begin_line and self->end_line may be modified (buffer recentered, more data from input)
    // self->eollen is 0 or 1 depending of the eol type (no eol = end of file reached or \n found)
    // (in other words : returns number of trailing eol characters at end of line)
    // return error code
    static inline RIO_ERROR sq_m_SQIntracker_readline(SQIntracker * self)
    {
        for (int i = self->begin_line; i < self->end_line; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                self->eollen = 1;
                return RIO_ERROR_OK;
            }
        }
        size_t trailing_room = sizeof(self->buffer) - self->end_line;
        // reframe buffer if no trailing room left
        if (trailing_room == 0){
            size_t used_len = self->end_line - self->begin_line;
            memmove(self->buffer, &(self->buffer[self->begin_line]), used_len);
            self->end_line = used_len;
            self->begin_line = 0;
        }
        ssize_t res = rio_recv(self->tracker, &(self->buffer[self->end_line]), sizeof(self->buffer) - self->end_line);
        if (res < 0){
            return (RIO_ERROR)-res;
        }
        self->end_line += res;
        if (self->begin_line == self->end_line) {
            return RIO_ERROR_EOF;
        }
        for (int i = self->begin_line; i < self->end_line; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                self->eollen = 1;
                return RIO_ERROR_OK;
            }
        }
        self->eol = self->end_line;
        self->eollen = 0;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_destructor(SQIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        if (self->trans){
            rio_delete(self->trans);
            self->trans = NULL;
        }
        return RIO_ERROR_CLOSED;
    }

    static inline RIO_ERROR sq_m_SQIntracker_next(SQIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){ return self->rlstatus; }
        if (self->trans){
            rio_delete(self->trans);
            close(self->fd);
            self->trans = NULL;
        }
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_constructor(SQIntracker * self, RIO * tracker)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->begin_line = self->eol = self->end_line = 0;
        self->line[0] = 0;
        self->eollen = 0;
        self->rlstatus = RIO_ERROR_OK;
        self->num_chunk = 0;

        // First header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header1, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header1[self->eol-self->begin_line] = 0;

        // Second header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header2, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header2[self->eol-self->begin_line] = 0;

        // 3rd header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header3, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header3[self->eol-self->begin_line] = 0;

        // First real filename line
        self->begin_line = self->eol;
        TODO("check what should happen if a file has no chunk")
        self->rlstatus = sq_m_SQIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_next_chunk(SQIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        size_t name_size = (unsigned)(self->eol - self->begin_line - self->eollen);
        if (self->begin_line == self->end_line) {
            return RIO_ERROR_EOF;
        }
        if (name_size >= sizeof(self->line)) {
            name_size = sizeof(self->line) - 1;
        }
        memcpy(self->line, &(self->buffer[self->begin_line]), name_size);
        self->line[name_size] = 0;
        ssize_t i = name_size - 1;
        unsigned stop_sec = 0;
        {
            unsigned old_stop_sec = 0;
            unsigned coef = 1;
            for ( ; i >= 0 ; i--){
                if (self->line[i] >= '0' && self->line[i] <= '9'){
                    if (coef > coef * 10){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    old_stop_sec = stop_sec;
                    stop_sec +=  coef * (self->line[i]-'0');
                    if (old_stop_sec > stop_sec){
                        return RIO_ERROR_OVERFLOW_STOP_TIMESTAMP;
                    }
                    coef *= 10;
                    continue;
                }
                if (self->line[i] == ' '){
                    self->stop_tv.tv_sec = stop_sec;
                    self->stop_tv.tv_usec = 0;
                    i--;
                    break;
                }
                return RIO_ERROR_INVALID_STOP_TIMESTAMP;
            }
        }
        unsigned start_sec = 0;
        {
            unsigned old_start_sec = 0;
            unsigned coef = 1;
            for ( ; i >= 0 ; i--){
                if (self->line[i] >= '0' && self->line[i] <= '9'){
                    if (coef > coef * 10){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    start_sec += coef * (self->line[i]-'0');
                    if (old_start_sec > start_sec){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    coef *= 10;
                    continue;
                }
                if (self->line[i] == ' '){
                    self->start_tv.tv_sec = start_sec;
                    self->start_tv.tv_usec = 0;
                    self->line[i] = 0;
                    break;
                }
                return RIO_ERROR_INVALID_START_TIMESTAMP;
            }
        }
        self->start_tv.tv_sec = start_sec;
        self->start_tv.tv_usec = 0;
        self->stop_tv.tv_sec = stop_sec;
        self->stop_tv.tv_usec = 0;
        self->num_chunk++;
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQIntracker_get_trans(SQIntracker * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return NULL; }
        if (self->rlstatus != RIO_ERROR_OK){
            if (status) { *status = self->rlstatus; }
            return NULL;
        }
        if (!self->trans){
            RIO_ERROR res = sq_m_SQIntracker_next_chunk(self);
            if (res != RIO_ERROR_OK){
                self->rlstatus = res;
                if (status) {*status = res;}
                return NULL;
            }
            TODO("add rights information to constructor")
            self->fd = ::open(self->line, O_RDONLY, S_IRUSR);
            if (self->fd < 0){
                self->rlstatus = RIO_ERROR_OPEN;
                if (status) { *status = self->rlstatus; }
                return self->trans; // self->trans is NULL
            }
            self->trans = rio_new_infile(&(self->rlstatus), self->fd);
            if (status){ *status = self->rlstatus; }
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQIntracker_timestamp(SQIntracker * self, timeval * tv)
    {
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQIntracker_get_chunk_info(SQIntracker * self,
                                unsigned * num_chunk,
                                char * path, size_t path_len,
                                timeval * begin,
                                timeval * end)
    {
        // ensure transport info is available and accurate (yes it goes all the way to opening trans)
        sq_m_SQIntracker_get_trans(self, NULL);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("CGR: check path_len")
        strcpy(path, self->line);
        if (begin) { *begin = self->start_tv; }
        if (end) { *end = self->stop_tv; }
        if (num_chunk) { *num_chunk = self->num_chunk; }
        return RIO_ERROR_OK;
    }



    /********************
    * SQCryptoIntracker *
    ********************/

    struct SQCryptoIntracker {
        RIO * trans;
        RIO * tracker;
//        TODO("CGR: check if name is used ?")
        char name[1024];
        char buffer[2048];
        int begin_line;
        int end_line;
        int eol;
        int eollen;
        RIO_ERROR rlstatus;
        char header1[1024];
        char header2[1024];
        char header3[1024];
        char line[1024];
        timeval start_tv;
        timeval stop_tv;
        int num_chunk;
    };

    // input: self->begin_line : beginning of available buffer (already read)
    // input: self->end_line :   end of available buffer (already read)
    // input: self->trans : IO data source

    // return self->eol : end of next line
    // self->begin_line and self->end_line may be modified (buffer recentered, more data from input)
    // self->eollen is 0 or 1 depending of the eol type (no eol = end of file reached or \n found)
    // (in other words : returns number of trailing eol characters at end of line)
    // return error code
    static inline RIO_ERROR sq_m_SQCryptoIntracker_readline(SQCryptoIntracker * self)
    {
        for (int i = self->begin_line; i < self->end_line; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                self->eollen = 1;
                return RIO_ERROR_OK;
            }
        }
        size_t trailing_room = sizeof(self->buffer) - self->end_line;
        // reframe buffer if no trailing room left
        if (trailing_room == 0){
            size_t used_len = self->end_line - self->begin_line;
            memmove(self->buffer, &(self->buffer[self->begin_line]), used_len);
            self->end_line = used_len;
            self->begin_line = 0;
        }
        ssize_t res = rio_recv(self->tracker, &(self->buffer[self->end_line]), sizeof(self->buffer) - self->end_line);
        if (res < 0){
            return (RIO_ERROR)-res;
        }
        self->end_line += res;
        if (self->begin_line == self->end_line) {
            return RIO_ERROR_EOF;
        }
        for (int i = self->begin_line; i < self->end_line; i++){
            if (self->buffer[i] == '\n'){
                self->eol = i+1;
                self->eollen = 1;
                return RIO_ERROR_OK;
            }
        }
        self->eol = self->end_line;
        self->eollen = 0;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_destructor(SQCryptoIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        if (self->trans){
            rio_delete(self->trans);
            self->trans = NULL;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_next(SQCryptoIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){ return self->rlstatus; }
        if (self->trans){
            rio_delete(self->trans);
            self->trans = NULL;
        }
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQCryptoIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_constructor(SQCryptoIntracker * self, RIO * tracker)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->begin_line = self->eol = self->end_line = 0;
        self->line[0] = 0;
        self->eollen = 0;
        self->rlstatus = RIO_ERROR_OK;
        self->num_chunk = 0;

        // First header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQCryptoIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header1, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header1[self->eol-self->begin_line] = 0;

        // Second header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQCryptoIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header2, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header2[self->eol-self->begin_line] = 0;

        // 3rd header line
        self->begin_line = self->eol;
        self->rlstatus = sq_m_SQCryptoIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("Add header sanity check")
        memcpy(self->header3, self->buffer + self->begin_line, self->end_line-self->begin_line);
        self->header3[self->eol-self->begin_line] = 0;

        // First real filename line
        self->begin_line = self->eol;
        TODO("check what should happen if a file has no chunk")
        self->rlstatus = sq_m_SQCryptoIntracker_readline(self);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_next_chunk(SQCryptoIntracker * self)
    {
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        size_t name_size = (unsigned)(self->eol - self->begin_line - self->eollen);
        if (self->begin_line == self->end_line) {
            return RIO_ERROR_EOF;
        }
        if (name_size >= sizeof(self->line)) {
            name_size = sizeof(self->line) - 1;
        }
        memcpy(self->line, &(self->buffer[self->begin_line]), name_size);
        self->line[name_size] = 0;
        ssize_t i = name_size - 1;
        unsigned stop_sec = 0;
        {
            unsigned old_stop_sec = 0;
            unsigned coef = 1;
            for ( ; i >= 0 ; i--){
                if (self->line[i] >= '0' && self->line[i] <= '9'){
                    if (coef > coef * 10){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    old_stop_sec = stop_sec;
                    stop_sec +=  coef * (self->line[i]-'0');
                    if (old_stop_sec > stop_sec){
                        return RIO_ERROR_OVERFLOW_STOP_TIMESTAMP;
                    }
                    coef *= 10;
                    continue;
                }
                if (self->line[i] == ' '){
                    self->stop_tv.tv_sec = stop_sec;
                    self->stop_tv.tv_usec = 0;
                    i--;
                    break;
                }
                return RIO_ERROR_INVALID_STOP_TIMESTAMP;
            }
        }
        unsigned start_sec = 0;
        {
            unsigned old_start_sec = 0;
            unsigned coef = 1;
            for ( ; i >= 0 ; i--){
                if (self->line[i] >= '0' && self->line[i] <= '9'){
                    if (coef > coef * 10){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    start_sec += coef * (self->line[i]-'0');
                    if (old_start_sec > start_sec){
                        return RIO_ERROR_OVERFLOW_START_TIMESTAMP;
                    }
                    coef *= 10;
                    continue;
                }
                if (self->line[i] == ' '){
                    self->start_tv.tv_sec = start_sec;
                    self->start_tv.tv_usec = 0;
                    self->line[i] = 0;
                    break;
                }
                return RIO_ERROR_INVALID_START_TIMESTAMP;
            }
        }
        self->start_tv.tv_sec = start_sec;
        self->start_tv.tv_usec = 0;
        self->stop_tv.tv_sec = stop_sec;
        self->stop_tv.tv_usec = 0;
        self->num_chunk++;
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQCryptoIntracker_get_trans(SQCryptoIntracker * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return NULL; }
        if (self->rlstatus != RIO_ERROR_OK){
            if (status) { *status = self->rlstatus; }
            return NULL;
        }
        if (!self->trans){
            RIO_ERROR res = sq_m_SQCryptoIntracker_next_chunk(self);
            if (res != RIO_ERROR_OK){
                self->rlstatus = res;
                if (status) {*status = res;}
                return NULL;
            }
            TODO("add rights information to constructor")
            self->trans = rio_new_crypto(&(self->rlstatus), self->line, O_RDONLY);
            if (status){ *status = self->rlstatus; }
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_timestamp(SQCryptoIntracker * self, timeval * tv)
    {
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoIntracker_get_chunk_info(SQCryptoIntracker * self,
                                unsigned * num_chunk,
                                char * path, size_t path_len,
                                timeval * begin,
                                timeval * end)
    {
        // ensure transport info is available and accurate (yes it goes all the way to opening trans)
        sq_m_SQCryptoIntracker_get_trans(self, NULL);
        if (self->rlstatus != RIO_ERROR_OK){
            return self->rlstatus;
        }
        TODO("CGR: check path_len")
        strcpy(path, self->line);
        if (begin) { *begin = self->start_tv; }
        if (end) { *end = self->stop_tv; }
        if (num_chunk) { *num_chunk = self->num_chunk; }
        return RIO_ERROR_OK;
    }
};

#endif

