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

   Template for new SQ_One sequence class

*/

#ifndef _REDEMPTION_LIBS_SQ_OUTFILENAME_H_
#define _REDEMPTION_LIBS_SQ_OUTFILENAME_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rt_constants.h"


extern "C" {
    struct SQOutfilename {
        RT * trans;
        SQ_FORMAT format;
        char prefix[512];
        char extension[12];
        unsigned pid;
        unsigned count;
    };

    RT_ERROR sq_m_SQOutfilename_constructor(SQOutfilename * self, SQ_FORMAT format, const char * prefix, const char * extension)
    {
        self->trans = NULL;
        self->count = 0;
        self->format = format;
        self->pid = getpid();
        if (strlen(prefix) > sizeof(self->prefix) - 1){
            return RT_ERROR_STRING_PREFIX_TOO_LONG;
        }
        strcpy(self->prefix, prefix);
        if (strlen(extension) > sizeof(self->extension) - 1){
            return RT_ERROR_STRING_EXTENSION_TOO_LONG;
        }
        strcpy(self->extension, extension);
        return RT_ERROR_OK;
    }

    RT_ERROR sq_m_SQOutfilename_destructor(SQOutfilename * self)
    {
        if (self->trans){
            rt_delete(self->trans);
            self->trans = NULL;
        }
        return RT_ERROR_OK;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    void sq_im_SQOutfilename_get_name(SQOutfilename * self, char * buffer)
    {
        switch (self->format){
        default:
        case SQF_PREFIX_PID_COUNT_EXTENSION:
            sprintf(buffer, "%s-%06u-%06u.%s", self->prefix, self->pid, self->count, self->extension);
        break;
        case SQF_PREFIX_COUNT_EXTENSION:
            sprintf(buffer, "%s-%06u.%s", self->prefix, self->count, self->extension);
        break;
        }
    }

    RT * sq_m_SQOutfilename_get_trans(SQOutfilename * self, RT_ERROR * status)
    {
        if (status && (*status != RT_ERROR_OK)) { return self->trans; }
        if (!self->trans){
            char tmpname[1024];
            sq_im_SQOutfilename_get_name(self, tmpname);
            int fd = ::open(tmpname, O_WRONLY|O_CREAT, S_IRUSR|S_IRUSR);
            if (fd < 0){
                if (status) { *status = RT_ERROR_CREAT; }
                return self->trans;
            }
            self->trans = rt_new_outfile(status, fd);
        }
        return self->trans;
    }

    RT_ERROR sq_m_SQOutfilename_next(SQOutfilename * self)
    {
        if (self->trans){
            rt_delete(self->trans);
            self->trans = NULL;
        }
        self->count += 1;
        return RT_ERROR_OK;
    }
};

#endif

