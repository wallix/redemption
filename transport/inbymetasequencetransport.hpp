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
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_INBYMETASEQUENCETRANSPORT_HPP
#define REDEMPTION_TRANSPORT_INBYMETASEQUENCETRANSPORT_HPP

#include "infiletransport.hpp"
#include "transport.hpp"
#include "error.hpp"
#include "fileutils.hpp"
#include "unique_ptr.hpp"
#include "rio/rio.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <limits>

class InByMetaSequenceTransport
: public Transport
{
    struct close_fd {
        typedef int pointer;
        void operator()(int fd) const {
            ::close(fd);
        }
    };

    unique_ptr<int, close_fd> meta_fd;

    int wrm_fd;


    static int open(const char * prefix, const char * extension) {
        char filename[1024];
        if (snprintf(filename, sizeof(filename), "%s%s", prefix, extension) >= int(sizeof(filename))) {
            throw Error(ERR_TRANSPORT);
        }

        const int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        return fd;
    }

    char buf[1024];
    char * eof;
    char * cur;

    void read(int err)
    {
        ssize_t ret = ::read(this->meta_fd.get(), this->buf, sizeof(this->buf));
        if (ret < 0 && errno != EINTR) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        if (ret == 0) {
            throw Error(err);
        }
        this->eof = this->buf + ret;
        this->cur = this->buf;
    }

    size_t read_line(char * dest, size_t len, int err) throw(Error)
    {
        size_t total_read = 0;
        while (1) {
            char * pos = std::find(this->cur, this->eof, '\n');
            if (len < pos - this->cur) {
                total_read += len;
                memcpy(dest, this->cur, len);
                this->cur += len;
                break;
            }
            total_read += pos - this->cur;
            memcpy(dest, this->cur, pos - this->cur);
            this->cur = pos+1;
            if (pos != this->eof) {
                break;
            }
            this->read(err);
        }
        return total_read;
    }

    void next_line()
    {
        char * pos;
        while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {
            this->read(ERR_TRANSPORT_READ_FAILED);
        }
        this->cur = pos+1;
    }

    char meta_path[1024];

    static unsigned parse_sec(char * first, char * last)
    {
        unsigned sec = 0;
        unsigned old_sec;
        for (; first != last; ++first) {
            if (*first < '0' || '9' < *first) {
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            old_sec = sec;
            sec *= 10;
            sec += *first - '0';
            if (old_sec > sec) {
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }
        return sec;
    }

    void open_next_wrm(int err)
    {
        size_t len = this->read_line(this->path, sizeof(this->path) - 1, err);
        this->path[len] = 0;

        // Line format "fffff sssss eeeee hhhhh HHHHH"
        //                               ^  ^  ^  ^
        //                               |  |  |  |
        //                               |hash1|  |
        //                               |     |  |
        //                           space3    |hash2
        //                                     |
        //                                   space4
        //
        // filename(1 or >) + space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135
        typedef std::reverse_iterator<char*> reverse_iterator;

        reverse_iterator last(this->path);
        reverse_iterator first(this->path + len);
        reverse_iterator e1 = std::find(first, last, ' ');
        reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1+1, last, ' ');
        if (last - e1 == 64 && e2 != last) {
            first = e2 + 1;
            e1 = std::find(first, last, ' ');
            e2 = (e1 == last) ? e1 : std::find(e1+1, last, ' ');
        }

        this->end_chunk_time = this->parse_sec(e1.base(), first.base());
        if (e1 != last) {
            ++e1;
        }
        this->begin_chunk_time = this->parse_sec(e2.base(), e1.base());

        if (e2 != last) {
            *e2 = 0;
        }

        this->wrm_fd = ::open(this->path, O_RDONLY);

        this->chunk_num++;
    }

public:
    char path[1024 + (std::numeric_limits<uint16_t>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;

    InByMetaSequenceTransport(const char * filename, const char * extension)
    : meta_fd(this->open(filename, extension))
    , wrm_fd(-1)
    , eof(buf)
    , cur(buf)
    , begin_chunk_time(0)
    , end_chunk_time(0)
    , chunk_num(0)
    {
        char cp_basename[1024];
        char cp_extension[128];

        this->meta_path[0] = cp_basename[0] = cp_extension[0] = 0;
        canonical_path(filename,
                       this->meta_path, sizeof(this->meta_path),
                       cp_basename, sizeof(cp_basename),
                       cp_extension, sizeof(cp_extension));
        LOG(LOG_INFO, "%s", this->meta_path);

        // headers
        //@{
        this->next_line();
        this->next_line();
        this->next_line();
        //@}

        this->path[0] = 0;
    }

    virtual ~InByMetaSequenceTransport()
    {
        ::close(this->wrm_fd);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    void next_chunk_info()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        this->open_next_wrm(ERR_TRANSPORT_READ_FAILED);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = io::posix::read_all(this->wrm_fd, *pbuffer, len);
        if (res < 0) {
            if (this->wrm_fd == -1) {
                this->open_next_wrm(ERR_TRANSPORT_NO_MORE_DATA);
                res = io::posix::read_all(this->wrm_fd, *pbuffer, len);
            }
            else {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
        }
        if (res > 0) {
            *pbuffer += res;
        }
        if (size_t(res) < len && res >= 0) {
            char * tmppbuffer = *pbuffer - res;
            len -= res;
            do {
                if (res == 0) {
                    ::close(this->wrm_fd);
                    this->open_next_wrm(ERR_TRANSPORT_NO_MORE_DATA);
                    if (this->wrm_fd < 0) {
                        *pbuffer = tmppbuffer;
                        res = -1;
                        break;
                    }
                }
                res = io::posix::read_all(this->wrm_fd, *pbuffer, len);
                if (res < 0) {
                    *pbuffer = tmppbuffer;
                    break;
                }
                *pbuffer += res;
                len -= res;
            } while (len);
        }
        if (res < 0) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }
};



/**********************************
* CryptoInByMetaSequenceTransport *
**********************************/

class CryptoInByMetaSequenceTransport : public Transport {
public:
    char path[1024];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;

    RIO * rio;
    SQ * seq;

    CryptoInByMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : Transport()
    {
        memset(this->path, 0, sizeof(path));
        this->begin_chunk_time = 0;
        this->end_chunk_time = 0;
        this->chunk_num = 0;

        RIO_ERROR status = RIO_ERROR_OK;
        SQ * seq = NULL;
        this->rio = rio_new_cryptoinmeta(&status, &seq, crypto_ctx, filename, extension);
        if (status != RIO_ERROR_OK){
            throw Error(ERR_TRANSPORT);
        }
        this->seq = seq;
    }

    ~CryptoInByMetaSequenceTransport()
    {
        if (this->rio){
            rio_delete(this->rio);
        }
    }

    void next_chunk_info()
    {
        {
            timeval tv_begin = {};
            timeval tv_end = {};
            RIO_ERROR status = sq_get_chunk_info(this->seq, &this->chunk_num, this->path, sizeof(this->path), &tv_begin, &tv_end);
            if (status != RIO_ERROR_OK){
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            this->begin_chunk_time = tv_begin.tv_sec;
            this->end_chunk_time = tv_end.tv_sec;
        }
        // if some error occurs calling sq_next
        // it will be took care of when opening next chunk, not now
        sq_next(this->seq);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = rio_recv(this->rio, *pbuffer, len);
        if (res <= 0){
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        *pbuffer += res;
        if (res != (ssize_t)len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) { throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE); }

};

#endif
