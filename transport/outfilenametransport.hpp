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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_OUTFILENAMETRANSPORT_HPP
#define REDEMPTION_TRANSPORT_OUTFILENAMETRANSPORT_HPP

#include "transport.hpp"
#include "read_and_write.hpp"
#include "urandom_read.hpp"
#include "crypto_transport.hpp"
#include "sequence_generator.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

namespace detail
{
    class OutFilenameCreator
    {
        FilenameGenerator filegen;
        char        current_filename[1024];
        int         fd;

    public:
        OutFilenameCreator(FilenameGenerator::Format format,
                           const char * const prefix,
                           const char * const filename,
                           const char * const extension,
                           const int groupid)
        : filegen(format, prefix, filename, extension, groupid)
        , fd(-1)
        {
            this->current_filename[0] = 0;
        }

        const FilenameGenerator & seqgen() const
        {
            return this->filegen;
        }

        const char * get_path() const
        {
            return this->filegen.path;
        }

        int get_fd() const
        {
            return this->fd;
        }

        void open_if_not_open(int err_id, int seqno)
        {
            if (!this->is_open()) {
                snprintf(this->current_filename, sizeof(this->current_filename), "%sred-XXXXXX.tmp", this->filegen.path);
                TODO("add rights information to constructor");
                this->fd = ::mkostemps(this->current_filename, 4, O_WRONLY | O_CREAT);
                if (this->fd < 0) {
                    throw Error(err_id, errno);
                }
                if (chmod( this->current_filename, (this->filegen.groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR)) == -1) {
                    LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                       , this->current_filename, strerror(errno), errno
                       , (this->filegen.groupid ? "u+r, g+r" : "u+r"));
                }
                this->filegen.set_last_filename(seqno, this->current_filename);
            }
        }

        bool is_open() const
        {
            return -1 != this->fd;
        }

        void rename_tmp(int seqno)
        {
            if (this->is_open()) {
                int res = close(this->fd);
                this->fd = -1;
                if (res < 0) {
                    LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
                }
                this->filegen.set_last_filename(0, 0);

                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                const char * filename = this->filegen.get(seqno);
                res = ::rename(this->current_filename, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                    , this->current_filename, filename, errno, strerror(errno));
                }

                this->current_filename[0] = 0;
            }
        }

        void send(const char * const buffer, size_t len, auth_api * authentifier, int seqno) throw (Error)
        {
            this->open_if_not_open(ERR_TRANSPORT_WRITE_FAILED, seqno);
            ssize_t res = io::posix::write_all(this->fd, buffer, len);
            if (res < 0){
                if (errno == ENOSPC) {
                    char message[1024];
                    snprintf(message, sizeof(message), "100|%s", this->filegen.path);
                    authentifier->report("FILESYSTEM_FULL", message);
                }
                LOG(LOG_ERR, "Write to transport failed (F): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }

        void seek(int64_t offset, int whence) throw (Error)
        {
            if (::lseek(this->fd, offset, whence) < 0) {
                throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
            }
        }
    };
}


class OutFilenameTransport
: public Transport
{
    detail::OutFilenameCreator filename_creator;

public:
    OutFilenameTransport(FilenameGenerator::Format format,
                         const char * const prefix,
                         const char * const filename,
                         const char * const extension,
                         const int groupid,
                         auth_api * authentifier = NULL,
                         unsigned verbose = 0)
    : filename_creator(format, prefix, filename, extension, groupid)
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    virtual ~OutFilenameTransport()
    {
        this->filename_creator.rename_tmp(this->seqno);
    }

    const FilenameGenerator * seqgen() const
    {
        return &this->filename_creator.seqgen();
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        this->filename_creator.send(buffer, len, this->authentifier, this->seqno);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {
        LOG(LOG_ERR, "OutFilenameTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error)
    {
        this->filename_creator.seek(offset, whence);
    }

    virtual bool next()
    {
        this->filename_creator.rename_tmp(this->seqno);
        return Transport::next();
    }
};



/*****************************
* CryptoOutFilenameTransport *
*****************************/

class CryptoOutFilenameTransport
: public Transport
{
    CryptoContext & crypto_ctx;
    crypto_file cf;
    int fd;

public:
    CryptoOutFilenameTransport(CryptoContext * crypto_ctx, const char * filename,
                               auth_api * authentifier = NULL, unsigned verbose = 0)
    : crypto_ctx(*crypto_ctx)
    , fd(::open(filename, O_WRONLY | O_CREAT, 0600))
    {
        if (this->fd == -1) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        init_crypto_write(this->cf, this->crypto_ctx, this->fd, filename, ERR_TRANSPORT_OPEN_FAILED);
    }

    ~CryptoOutFilenameTransport()
    {
        unsigned char hash[HASH_LEN];
        this->cf.close(hash, this->crypto_ctx.hmac_key);
        close(this->fd);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t res = this->cf.write(buffer, len);
        if (res <= 0) {
            this->status = false;
            LOG(LOG_ERR, "Write to transport failed (CF): code=%d", errno);
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        if (res != (ssize_t)len) {
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {
        LOG(LOG_INFO, "OutFilenameTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE, errno);
    }
};

#endif
