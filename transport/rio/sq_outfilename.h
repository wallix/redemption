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
   Author(s): Christophe Grosjean

   Template for new SQ_Outfilename sequence class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_SQ_OUTFILENAME_H_
#define _REDEMPTION_TRANSPORT_RIO_SQ_OUTFILENAME_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio.h"

extern "C" {
    struct SQOutfilename {
        int fd;
        RIO * trans;
        timeval start_tv;
        timeval stop_tv;
        RIO * tracker;
        SQ_FORMAT format;
        char path[1024];
        char filename[1024];
        char extension[12];
        unsigned pid;
        unsigned count;
        int groupid;
    };

    static inline RIO_ERROR sq_m_SQOutfilename_constructor(SQOutfilename * self,
                SQ_FORMAT format,
                const char * path, const char * filename, const char * extension, const int groupid)
    {
        self->trans = NULL;
        self->count = 0;
        self->format = format;
        self->pid = getpid();
        if (strlen(path) > sizeof(self->path) - 1){
            return RIO_ERROR_STRING_PATH_TOO_LONG;
        }
        strcpy(self->path, path);
        if (strlen(filename) > sizeof(self->filename) - 1){
            return RIO_ERROR_STRING_FILENAME_TOO_LONG;
        }
        strcpy(self->filename, filename);

        if (strlen(extension) > sizeof(self->extension) - 1){
            return RIO_ERROR_STRING_EXTENSION_TOO_LONG;
        }
        strcpy(self->extension, extension);
        self->groupid = groupid;
        return RIO_ERROR_OK;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    static inline size_t sq_im_SQOutfilename_get_name(const SQOutfilename * self, char * buffer, size_t size, int count)
    {
        size_t res = 0;
        switch (self->format){
        default:
        case SQF_PATH_FILE_PID_COUNT_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u-%06u%s", self->path, self->filename, self->pid, count, self->extension);
        break;
        case SQF_PATH_FILE_COUNT_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u%s", self->path, self->filename, count, self->extension);
        break;
        case SQF_PATH_FILE_PID_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u%s", self->path, self->filename, self->pid, self->extension);
        break;
        case SQF_PATH_FILE_EXTENSION:
            res = snprintf(buffer, size, "%s%s%s", self->path, self->filename, self->extension);
        break;
        }
        return res;
    }

    static RIO_ERROR sq_m_SQOutfilename_timestamp(SQOutfilename * self, timeval * tv)
    {
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_destructor(SQOutfilename * self)
    {
        if (self->trans){
            char tmpname[1024];
            sq_im_SQOutfilename_get_name(self, tmpname, sizeof(tmpname), self->count);
            rio_delete(self->trans);
            int res = close(self->fd);
            if (res < 0){
                LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
                return RIO_ERROR_CLOSE_FAILED;
            }
            self->trans = NULL;
        }
        return RIO_ERROR_CLOSED;
    }

    static inline RIO * sq_m_SQOutfilename_get_trans(SQOutfilename * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return self->trans; }
        if (!self->trans){
            char tmpname[1024];
            sq_im_SQOutfilename_get_name(self, tmpname, sizeof(tmpname), self->count);
            TODO("add rights information to constructor")
            self->fd = ::open(tmpname, O_WRONLY|O_CREAT, S_IRUSR);
            if (self->fd < 0){
                if (status) { *status = RIO_ERROR_CREAT; }
                return self->trans;
            }
            if (self->groupid){
//                if (chown(tmpname, (uid_t)-1, self->groupid) < 0){
//                    LOG(LOG_ERR, "can't set file %s group to %u : %s [%u]", tmpname, self->groupid, strerror(errno), errno);
//                }
                if (chmod(tmpname, S_IRUSR|S_IRGRP) == -1){
                    LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", tmpname, strerror(errno), errno);
                }
            }
            self->trans = rio_new_outfile(status, self->fd);
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_next(SQOutfilename * self)
    {
        sq_m_SQOutfilename_destructor(self);
        self->count += 1;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQOutfilename_get_chunk_info(SQOutfilename * self, unsigned * num_chunk, char * path, size_t path_len, timeval * begin, timeval * end)
    {
        return RIO_ERROR_OK;
    }
};

#endif

