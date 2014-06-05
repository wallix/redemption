/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Raphael Zhou

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_OUTMETATRANSPORT_HPP
#define REDEMPTION_TRANSPORT_OUTMETATRANSPORT_HPP

#include "error.hpp"
#include "fdbuf.hpp"
#include "transport.hpp"
#include "fileutils.hpp"
#include "urandom_read.hpp"
#include "outfilenametransport.hpp"
#include "outfilenametransport.hpp"
#include "crypto_transport.hpp"

#include <limits>

#include <sys/time.h>


namespace detail
{
    struct MetaFilename
    {
        char filename[2048];

        MetaFilename(const char * path, const char * basename,
                     FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
        {
            int res = format == (
               FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
            || format == FilenameGenerator::PATH_FILE_PID_EXTENSION)
            ? snprintf(this->filename, sizeof(this->filename)-1, "%s%s-%06u.mwrm", path, basename, getpid())
            : snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
            if (res > int(sizeof(this->filename) - 6) || res < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }
    };

    template<class Writer>
    void write_meta_headers(Writer & writer, const char * path,
                            uint16_t width, uint16_t height, auth_api * authentifier)
    {
        char header1[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2];
        const int len = sprintf(header1, "%u %u", width, height);
        ssize_t res = writer.write(header1, len);
        if (res > 0) {
            res = writer.write("\n\n\n", 3);
        }

        if (res < 0) {
            int err = errno;
            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
        }
    }
}

class OutmetaTransport
: public Transport
{
    detail::OutFilenameCreator filename_creator;
    io::posix::fdbuf fdbuf;
    time_t start_sec;
    time_t stop_sec;

public:
    OutmetaTransport(const char * path, const char * basename, timeval now,
                     uint16_t width, uint16_t height, const int groupid, auth_api * authentifier = NULL,
                     unsigned verbose = 0,
                     FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : filename_creator(format, path, basename, ".wrm", groupid)
    , start_sec(now.tv_sec)
    , stop_sec(now.tv_sec)
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        if (this->fdbuf.open(detail::MetaFilename(path, basename, format).filename, O_WRONLY|O_CREAT, S_IRUSR) < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        detail::write_meta_headers(this->fdbuf, path, width, height, this->authentifier);
    }

    virtual ~OutmetaTransport()
    {
        if (this->filename_creator.is_open()) {
            this->filename_creator.rename_tmp(this->seqno);
            this->write_wrm(false);
        }
    }

    const FilenameGenerator * seqgen() const
    {
        return &this->filename_creator.seqgen();
    }

    using Transport::send;
    virtual void send(const char * buffer, size_t len) throw(Error)
    {
        this->filename_creator.send(buffer, len, this->authentifier, this->seqno);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw(Error)
    {
        LOG(LOG_INFO, "OutmetaTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw(Error)
    {
        this->filename_creator.seek(offset, whence);
    }

    virtual bool next()
    {
        this->filename_creator.rename_tmp(this->seqno);
        this->write_wrm(true);
        return Transport::next();
    }

    virtual void timestamp(timeval now)
    {
        this->stop_sec = now.tv_sec;
    }

private:
    void write_wrm(bool send_exception)
    {
        const char * filename = this->filename_creator.seqgen().get(this->seqno);
        size_t len = strlen(filename);
        ssize_t res = this->fdbuf.write(filename, len);
        if (res > 0) {
            char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 5];
            len = snprintf(mes, sizeof(mes), " %u %u\n",
                           (unsigned)this->start_sec,
                           (unsigned)this->stop_sec+1);
            res = this->fdbuf.write(mes, len);
            this->start_sec = this->stop_sec;
        }
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
            if (send_exception) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
    }
};


/*************************
* CryptoOutmetaTransport *
*************************/

class CryptoOutmetaTransport
: public Transport
{
    detail::OutFilenameCreator filename_creator;
    io::posix::fdbuf fdbuf;
    detail::MetaFilename hf;
    crypto_file crypto_meta;
    crypto_file crypto_wrm;
    time_t start_sec;
    time_t stop_sec;
    CryptoContext & crypto_ctx;


public:
    CryptoOutmetaTransport(CryptoContext * crypto_ctx, const char * path, const char * hash_path,
        const char * basename, timeval now, uint16_t width, uint16_t height,
        const int groupid, auth_api * authentifier = NULL, unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : filename_creator(format, path, basename, ".wrm", groupid)
    , hf(hash_path, basename, format)
    , start_sec(now.tv_sec)
    , stop_sec(now.tv_sec)
    , crypto_ctx(*crypto_ctx)
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::MetaFilename mf(path, basename, format);

        if (this->fdbuf.open(mf.filename, O_WRONLY|O_CREAT, S_IRUSR) < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        init_crypto_write(this->crypto_meta, this->crypto_ctx, this->fdbuf.get_fd(),
                          mf.filename, ERR_TRANSPORT_OPEN_FAILED);
        detail::write_meta_headers(this->crypto_meta, path, width, height, this->authentifier);
    }

    ~CryptoOutmetaTransport()
    {
        if (this->filename_creator.is_open()) {
            this->write_wrm(false);
        }
        char          path[1024] = {};
        char          basename[1024] = {};
        char          extension[256] = {};
        char          filename[2048] = {};

        canonical_path(hf.filename,
                       path, sizeof(path),
                       basename, sizeof(basename),
                       extension, sizeof(extension));
        std::snprintf(filename, sizeof(filename), "%s%s", basename, extension);

        unsigned char hash[HASH_LEN] = {0};

        TODO("check if sign returns some error");
        this->crypto_meta.close(hash, this->crypto_ctx.hmac_key);

        crypto_file crypto_hash;
        TODO("check errors when storing hash");
        int hash_fd = ::open(hf.filename, O_WRONLY|O_CREAT, S_IRUSR);
        if (hash_fd > 0) {
            if (noexcept_init_crypto_write(crypto_hash, this->crypto_ctx, hash_fd, hf.filename)) {
                const size_t len = strlen(filename);
                if (crypto_hash.write(filename, len) != long(len)
                 || crypto_hash.write(" ", 1) != 1
                 || crypto_hash.write(reinterpret_cast<const char*>(hash), HASH_LEN) != HASH_LEN) {
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [%u]\n", hf.filename, -HASH_LEN);
                }
                else {
                    crypto_hash.close(hash, this->crypto_ctx.hmac_key);
                }
            }

            if (chmod(hf.filename, S_IRUSR|S_IRGRP) == -1){
                LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", hf.filename, strerror(errno), errno);
            }
            ::close(hash_fd);
        }
        else {
            LOG(LOG_ERR, "Open to transport failed: code=%d", errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw(Error)
    {
        if (!this->filename_creator.is_open()) {
            const char * filename = this->filename_creator.seqgen().get(this->seqno);
            this->filename_creator.open_if_not_open(ERR_TRANSPORT_WRITE_FAILED, this->seqno);
            new (&this->crypto_wrm) crypto_file;
            init_crypto_write(this->crypto_wrm, this->crypto_ctx, this->filename_creator.get_fd(),
                              filename, ERR_TRANSPORT_WRITE_FAILED);
        }
        int res = this->crypto_wrm.write(buffer, len);
        if (res < 0) {
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", this->filename_creator.get_path());
                this->authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_INFO, "Write to transport failed (CM): code=%d", errno);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw(Error)
    {
        LOG(LOG_INFO, "CryptoOutmetaTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw(Error)
    {
        LOG(LOG_INFO, "Set position within transport failed: code=%d", errno);
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool next()
    {
        this->write_wrm(true);
        return Transport::next();
    }

    virtual void timestamp(timeval now)
    {
        this->stop_sec = now.tv_sec;
    }

private:
    void write_wrm(bool send_exception)
    {
        unsigned char hash[HASH_LEN];
        this->crypto_wrm.close(hash, this->crypto_ctx.hmac_key);

        this->filename_creator.rename_tmp(this->seqno);

        const char * filename = this->filename_creator.seqgen().get(this->seqno);
        size_t len = strlen(filename);
        ssize_t res = this->crypto_meta.write(filename, len);
        if (res > 0) {
            char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + HASH_LEN*2 + 5];
            len = snprintf(mes, sizeof(mes) - 3, " %u %u",
                           (unsigned)this->start_sec,
                           (unsigned)this->stop_sec+1);
            char * p = mes + len;
            *p++ = ' ';                           //     1 octet
            for (int i = 0; i < HASH_LEN / 2; i++, p += 2) {
                sprintf(p, "%02x", hash[i]);      //    64 octets (hash1)
            }
            *p++ = ' ';                           //     1 octet
            for (int i = HASH_LEN / 2; i < HASH_LEN; i++, p += 2) {
                sprintf(p, "%02x", hash[i]);      //    64 octets (hash2)
            }
            *p++ = '\n';                          //     1 octet
            res = this->crypto_meta.write(mes, p-mes);
            this->start_sec = this->stop_sec;
        }
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
            if (send_exception) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
    }
};

#endif
