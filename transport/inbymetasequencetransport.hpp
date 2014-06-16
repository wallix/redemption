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

#include "transport.hpp"
#include "error.hpp"
#include "fileutils.hpp"
#include "fdbuf.hpp"
#include "crypto_transport.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <limits>

namespace detail {
    int open_inmeta(const char * prefix, const char * extension)
    {
        char filename[1024];
        if (std::snprintf(filename, sizeof(filename), "%s%s", prefix, extension) >= int(sizeof(filename))) {
            throw Error(ERR_TRANSPORT);
        }

        //char meta_path[1024];
        //char cp_basename[1024];
        //char cp_extension[128];
        //
        //meta_path[0] = cp_basename[0] = cp_extension[0] = 0;
        //canonical_path(filename,
        //               meta_path, sizeof(meta_path),
        //               cp_basename, sizeof(cp_basename),
        //               cp_extension, sizeof(cp_extension));
        //LOG(LOG_INFO, "%s", meta_path);

        const int fd = ::open(filename, O_RDONLY);
        if (fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        return fd;
    }

    template<class Reader>
    class ReaderLine
    {
        char buf[1024];
        char * eof;
        char * cur;
        Reader reader;

        int read(int err) /*noexcept*/
        {
            ssize_t ret = this->reader(this->buf, sizeof(this->buf));
            if (ret < 0 && errno != EINTR) {
                return -ERR_TRANSPORT_READ_FAILED;
            }
            if (ret == 0) {
                return -err;
            }
            this->eof = this->buf + ret;
            this->cur = this->buf;
            return 0;
        }

    public:
        ReaderLine(Reader reader) /*noexcept*/
        : eof(buf)
        , cur(buf)
        , reader(reader)
        {}

        ssize_t read_line(char * dest, size_t len, int err) throw(Error) /*noexcept*/
        {
            ssize_t total_read = 0;
            while (1) {
                char * pos = std::find(this->cur, this->eof, '\n');
                if (len < size_t(pos - this->cur)) {
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
                if (int e = this->read(err)) {
                    return e;
                }
            }
            return total_read;
        }

        int next_line() /*noexcept*/
        {
            char * pos;
            while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {
                if (int e = this->read(ERR_TRANSPORT_READ_FAILED)) {
                    return e;
                }
            }
            this->cur = pos+1;
            return 0;
        }
    };

    template<class Reader>
    struct MetaReader
    {
        char path[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
        detail::ReaderLine<Reader> reader;
        int wrm_fd;
        unsigned begin_chunk_time;
        unsigned end_chunk_time;

        template<class T>
        MetaReader(T & arg_reader)
        : reader(arg_reader)
        , wrm_fd(-1)
        , begin_chunk_time(0)
        , end_chunk_time(0)
        {
            // headers
            //@{
            if (this->reader.next_line()
             || this->reader.next_line()
             || this->reader.next_line()
            ) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            //@}

            this->path[0] = 0;
        }

        ~MetaReader()
        {
            ::close(this->wrm_fd);
        }

        ///\return 0 if success
        int next()
        {
            ssize_t len = reader.read_line(path, sizeof(path) - 1, ERR_TRANSPORT_NO_MORE_DATA);
            if (len < 0) {
                return -len;
            }
            path[len] = 0;

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

            reverse_iterator last(path);
            reverse_iterator first(path + len);
            reverse_iterator e1 = std::find(first, last, ' ');
            reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1+1, last, ' ');
            if (e1 - first == 64 && e2 != last) {
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

            ::close(this->wrm_fd);
            this->wrm_fd = ::open(path, O_RDONLY);

            if (this->wrm_fd < 0) {
                return ERR_TRANSPORT_NO_MORE_DATA;
            }

            return 0;
        }

    private:
        static unsigned parse_sec(const char * first, const char * last)
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
    };
}

class InByMetaSequenceTransport
: public Transport
{
    io::posix::fdbuf metabuf;

    struct ReaderBuf {
        io::posix::fdbuf & fdbuf;

        ReaderBuf(io::posix::fdbuf & fdbuf)
        : fdbuf(fdbuf)
        {}

        ssize_t operator()(char * buf, size_t len) const {
            return this->fdbuf.read(buf, len);
        }
    };

    detail::MetaReader<ReaderBuf> meta_reader;

    void open_next_wrm()
    {
        if (int e = this->meta_reader.next()) {
            this->status = false;
            throw Error(e, errno);
        }
        this->begin_chunk_time = this->meta_reader.begin_chunk_time;
        this->end_chunk_time = this->meta_reader.end_chunk_time;
        Transport::next();
    }

public:
    unsigned begin_chunk_time;
    unsigned end_chunk_time;

    InByMetaSequenceTransport(const char * filename, const char * extension)
    : metabuf(detail::open_inmeta(filename, extension))
    , meta_reader(this->metabuf)
    , begin_chunk_time(0)
    , end_chunk_time(0)
    {}

    const char * path() const
    {
        return this->meta_reader.path;
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = io::posix::read_all(this->meta_reader.wrm_fd, *pbuffer, len);
        if (res < 0) {
            if (this->meta_reader.wrm_fd == -1) {
                this->open_next_wrm();
                res = io::posix::read_all(this->meta_reader.wrm_fd, *pbuffer, len);
            }
            else {
                this->status = false;
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
                    ::close(this->meta_reader.wrm_fd);
                    this->open_next_wrm();
                    if (this->meta_reader.wrm_fd < 0) {
                        *pbuffer = tmppbuffer;
                        res = -1;
                        break;
                    }
                }
                res = io::posix::read_all(this->meta_reader.wrm_fd, *pbuffer, len);
                if (res < 0) {
                    *pbuffer = tmppbuffer;
                    break;
                }
                *pbuffer += res;
                len -= res;
            } while (len);
        }
        if (res < 0) {
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        this->open_next_wrm();
        return true;
    }

    void next_chunk_info()
    {
        this->next();
    }
};



/**********************************
* CryptoInByMetaSequenceTransport *
**********************************/

class CryptoInByMetaSequenceTransport
: public Transport
{
    io::posix::fdbuf metabuf;
    bool crypto_wrm_is_init;

    crypto_file crypto_wrm;
    crypto_file crypto_mwrm;

    CryptoContext & crypto_ctx;

    struct ReaderCrypto {
        crypto_file & crypto;

        ReaderCrypto(crypto_file & crypto)
        : crypto(crypto)
        {}

        int operator()(char * buf, size_t len) const {
            return this->crypto.read(buf, len);
        }
    };

    detail::MetaReader<ReaderCrypto> meta_reader;

    void open_next_wrm()
    {
        if (int e = this->meta_reader.next()) {
            this->status = false;
            throw Error(e, errno);
        }
        this->begin_chunk_time = this->meta_reader.begin_chunk_time;
        this->end_chunk_time = this->meta_reader.end_chunk_time;
        Transport::next();
    }

    crypto_file & init_crypto_mwrm(const char * filename, const char * extension)
    {
        char meta_filename[1024];
        std::snprintf(meta_filename, sizeof(meta_filename), "%s%s", filename, extension);
        init_crypto_read(this->crypto_mwrm, this->crypto_ctx, this->metabuf.get_fd(),
                         meta_filename, ERR_TRANSPORT_OPEN_FAILED);
        return this->crypto_mwrm;
    }

public:
    unsigned begin_chunk_time;
    unsigned end_chunk_time;

    CryptoInByMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : Transport()
    , metabuf(detail::open_inmeta(filename, extension))
    , crypto_wrm_is_init(false)
    , crypto_ctx(*crypto_ctx)
    , meta_reader(this->init_crypto_mwrm(filename, extension))
    , begin_chunk_time(0)
    , end_chunk_time(0)
    {}

    const char * path() const
    {
        return this->meta_reader.path;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        if (!this->crypto_wrm_is_init) {
            if (this->meta_reader.wrm_fd == -1) {
                this->next_chunk_info();
            }
            init_crypto_read(this->crypto_wrm, this->crypto_ctx, this->meta_reader.wrm_fd,
                             this->meta_reader.path, ERR_TRANSPORT_WRITE_FAILED);
            this->crypto_wrm_is_init = true;
        }
        int res;
        size_t remaining_len = len;
        bool zero_res = false;
        while (remaining_len) {
            res = this->crypto_wrm.read(*pbuffer, remaining_len);
            if (res < 0) {
                if (errno != EINTR) {
                    break;
                }
                this->next_crypto_wrm();
                continue;
            }
            if (res == 0){
                if (zero_res) {
                    this->next_crypto_wrm();
                    zero_res = false;
                }
                else {
                    zero_res = true;
                }
            }
            *pbuffer += res;
            remaining_len -= res;
        }
        if (remaining_len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error)
    {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        this->open_next_wrm();
        return true;
    }

    void next_chunk_info()
    {
        this->next();
    }

private:
    void next_crypto_wrm()
    {
        this->next_chunk_info();
        new (&this->crypto_wrm) crypto_file;
        init_crypto_read(this->crypto_wrm, this->crypto_ctx, this->meta_reader.wrm_fd,
                         this->meta_reader.path, ERR_TRANSPORT_WRITE_FAILED);
    }
};

#endif
