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

   Template for new SQ_Outtracker sequence class
*/

#ifndef _REDEMPTION_TRANSPORT_RIO_SQ_CRYPTOOUTTRACKER_H_
#define _REDEMPTION_TRANSPORT_RIO_SQ_CRYPTOOUTTRACKER_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rio.h"

#define HASH_LEN 64

extern "C" {

    /*********************
    * SQCryptoOuttracker *
    *********************/

    struct SQCryptoOuttracker {
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
        CryptoContext * crypto_ctx;
    };

    static inline RIO_ERROR sq_m_SQCryptoOuttracker_constructor(SQCryptoOuttracker * self, RIO * tracker,
                    CryptoContext * crypto_ctx,
                    SQ_FORMAT format,
                    const char * path, const char * filename, const char * extension,
                    struct timeval * tv, const char * header1, const char * header2, const char * header3,
                    const int groupid)
    {
        self->trans = NULL;
        self->tracker = tracker;
        self->count = 0;
        self->format = format;
        self->pid = getpid();
        self->start_tv.tv_sec = self->stop_tv.tv_sec = tv->tv_sec;
        self->start_tv.tv_usec = self->stop_tv.tv_usec = tv->tv_usec;
        self->crypto_ctx = crypto_ctx;
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
        if (tracker){
            TODO("Add error management, add sanity check for headers (no CR, not too long)");
            rio_send(tracker, header1, strlen(header1));
            rio_send(tracker, "\n", 1);
            rio_send(tracker, header2, strlen(header2));
            rio_send(tracker, "\n", 1);
            rio_send(tracker, header3, strlen(header3));
            rio_send(tracker, "\n", 1);
        }
        self->groupid = groupid;
        return RIO_ERROR_OK;
    }

    // internal utility method, used to get name of files used for target transports
    // it is called internally, but actual goal is to enable tests to check and remove the created files afterward.
    // not a part of external sequence API
    static inline size_t sq_im_SQCryptoOuttracker_get_name(SQCryptoOuttracker * self, char * buffer, size_t size, int count)
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

    static inline size_t sq_im_SQCryptoOuttracker_get_line(SQCryptoOuttracker * self, char * buffer, size_t size, int count)
    {
        size_t res = 0;
        switch (self->format){
        default:
        case SQF_PATH_FILE_PID_COUNT_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u-%06u%s %u %u", self->path, self->filename, self->pid, count, self->extension,
                (unsigned)self->start_tv.tv_sec, (unsigned)self->stop_tv.tv_sec+1);
        break;
        case SQF_PATH_FILE_COUNT_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u%s %u %u", self->path, self->filename, count, self->extension,
                (unsigned)self->start_tv.tv_sec, (unsigned)self->stop_tv.tv_sec+1);
        break;
        case SQF_PATH_FILE_PID_EXTENSION:
            res = snprintf(buffer, size, "%s%s-%06u%s %u %u", self->path, self->filename, self->pid, self->extension,
                (unsigned)self->start_tv.tv_sec, (unsigned)self->stop_tv.tv_sec+1);
        break;
        case SQF_PATH_FILE_EXTENSION:
            res = snprintf(buffer, size, "%s%s%s %u %u", self->path, self->filename, self->extension,
                (unsigned)self->start_tv.tv_sec, (unsigned)self->stop_tv.tv_sec+1);
        break;
        }
        return res;
    }

    static RIO_ERROR sq_m_SQCryptoOuttracker_timestamp(SQCryptoOuttracker * self, timeval * tv)
    {
        self->stop_tv.tv_sec = tv->tv_sec;
        self->stop_tv.tv_usec = tv->tv_usec;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoOuttracker_destructor(SQCryptoOuttracker * self)
    {
        if (self->trans){
            unsigned char hash[HASH_LEN];
            size_t        res_len;

            TODO("check if sign returns some error");
            rio_sign(self->trans, hash, sizeof(hash), &res_len);
            if (self->tracker) {
                char buffer[1024];
                size_t len = sq_im_SQCryptoOuttracker_get_line(self, buffer, sizeof(buffer)-1, self->count);
                rio_send(self->tracker, buffer, len);

                char *p = buffer;

                *p++ = ' ';                           //     1 octet
                for (int i = 0; i < HASH_LEN / 2; i++, p += 2)
                    sprintf(p, "%02x", hash[i]);      //    64 octets (hash1)

                *p++ = ' ';                           //     1 octet
                for (int i = HASH_LEN / 2; i < HASH_LEN; i++, p += 2)
                    sprintf(p, "%02x", hash[i]);      //    64 octets (hash2)
                *p++ = '\n';                          //     1 octet

                rio_send(self->tracker, buffer, 131);  // = 131 octets

                self->start_tv.tv_sec = self->stop_tv.tv_sec;
                self->start_tv.tv_usec = self->stop_tv.tv_usec;
            }
            TODO("check if close returns some error");
            rio_delete(self->trans);
            self->trans = NULL;
        }
        return RIO_ERROR_OK;
    }

    static inline RIO * sq_m_SQCryptoOuttracker_get_trans(SQCryptoOuttracker * self, RIO_ERROR * status)
    {
        if (status && (*status != RIO_ERROR_OK)) { return self->trans; }
        if (!self->trans){
            char tmpname[1024];
            sq_im_SQCryptoOuttracker_get_name(self, tmpname, sizeof(tmpname), self->count);
            TODO("add rights information to constructor");
            self->trans = rio_new_crypto(status, self->crypto_ctx, tmpname, O_WRONLY);
            if (self->groupid){
//                if (chown(tmpname, (uid_t)-1, self->groupid) < 0){
//                    LOG(LOG_ERR, "can't set file %s group to %u : %s [%u]", tmpname, self->groupid, strerror(errno), errno);
//                }
                if (chmod(tmpname, S_IRUSR|S_IRGRP) == -1){
                    LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", tmpname, strerror(errno), errno);
                }
            }
        }
        return self->trans;
    }

    static inline RIO_ERROR sq_m_SQCryptoOuttracker_next(SQCryptoOuttracker * self)
    {
        sq_m_SQCryptoOuttracker_destructor(self);
        self->count += 1;
        return RIO_ERROR_OK;
    }

    static inline RIO_ERROR sq_m_SQCryptoOuttracker_get_chunk_info(SQCryptoOuttracker * self, unsigned * num_chunk, char * path, size_t path_len, timeval * begin, timeval * end)
    {
        return RIO_ERROR_OK;
    }
};

#endif

