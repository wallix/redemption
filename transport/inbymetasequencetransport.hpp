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
#include "fdbub.hpp"
#include "rio/rio.h"
#include "rio/cryptofile.hpp"

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

    unsigned parse_sec(const char * first, const char * last)
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
}

class InByMetaSequenceTransport
: public Transport
{
    io::posix::fdbuf metabuf;

    int wrm_fd;

    struct ReaderBuf {
        io::posix::fdbuf & fdbuf;

        ReaderBuf(io::posix::fdbuf & fdbuf)
        : fdbuf(fdbuf)
        {}

        ssize_t operator()(char * buf, size_t len) const {
            return this->fdbuf.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderBuf> reader;
    char meta_path[1024];

    void open_next_wrm(int err)
    {
        ssize_t len = this->reader.read_line(this->path, sizeof(this->path) - 1, err);
        if (len < 0) {
            this->status = false;
            throw Error(-len, errno);
        }
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
        if (e1 - first == 64 && e2 != last) {
            first = e2 + 1;
            e1 = std::find(first, last, ' ');
            e2 = (e1 == last) ? e1 : std::find(e1+1, last, ' ');
        }

        this->end_chunk_time = detail::parse_sec(e1.base(), first.base());
        if (e1 != last) {
            ++e1;
        }
        this->begin_chunk_time = detail::parse_sec(e2.base(), e1.base());

        if (e2 != last) {
            *e2 = 0;
        }

        this->wrm_fd = ::open(this->path, O_RDONLY);

        if (this->wrm_fd < 0) {
            this->status = false;
            throw Error(err, errno);
        }

        ++this->chunk_num;
        Transport::next();
    }

public:
    //Line format "path sssss eeeee hhhhh HHHHH"
    char path[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;

    InByMetaSequenceTransport(const char * filename, const char * extension)
    : metabuf(detail::open_inmeta(filename, extension))
    , wrm_fd(-1)
    , reader(this->metabuf)
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
        if (this->reader.next_line()
         || this->reader.next_line()
         || this->reader.next_line()
        ) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
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
        this->open_next_wrm(ERR_TRANSPORT_NO_MORE_DATA);
        return true;
    }
};



/**********************************
* CryptoInByMetaSequenceTransport *
**********************************/

class CryptoInByMetaSequenceTransport
: public Transport
{
    io::posix::fdbuf metabuf;
    int wrm_fd;

    crypto_file crypto_wrm;
    crypto_file crypto_mwrm;

    CryptoContext * crypto_ctx;

    struct ReaderCrypto {
        crypto_file & crypto;

        ReaderCrypto(crypto_file & crypto)
        : crypto(crypto)
        {}

        int operator()(char * buf, size_t len) const {
            return this->crypto.read(buf, len);
        }
    };

    detail::ReaderLine<ReaderCrypto> reader_meta;

    void open_next_wrm(int err)
    {
        ssize_t len = this->reader_meta.read_line(this->path, sizeof(this->path) - 1, err);
        if (len < 0) {
            this->status = false;
            throw Error(-len, errno);
        }
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
        if (e1 - first == 64 && e2 != last) {
            first = e2 + 1;
            e1 = std::find(first, last, ' ');
            e2 = (e1 == last) ? e1 : std::find(e1+1, last, ' ');
        }

        this->end_chunk_time = detail::parse_sec(e1.base(), first.base());
        if (e1 != last) {
            ++e1;
        }
        this->begin_chunk_time = detail::parse_sec(e2.base(), e1.base());

        if (e2 != last) {
            *e2 = 0;
        }

        this->wrm_fd = ::open(this->path, O_RDONLY);

        if (this->wrm_fd < 0) {
            this->status = false;
            throw Error(err, errno);
        }

        ++this->chunk_num;
        Transport::next();
    }

public:
    //Line format "path sssss eeeee hhhhh HHHHH"
    char path[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;

    CryptoInByMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : Transport()
    , metabuf(detail::open_inmeta(filename, extension))
    , wrm_fd(-1)
    , crypto_ctx(crypto_ctx)
    , reader_meta(this->crypto_mwrm)
    , begin_chunk_time(0)
    , end_chunk_time(0)
    , chunk_num(0)
    {
        char meta_filename[1024];
        std::snprintf(meta_filename, sizeof(meta_filename), "%s%s", filename, extension);
        this->init_crypto_ctx(this->crypto_mwrm, meta_filename, this->metabuf.get_fd());

        // headers
        //@{
        if (this->reader_meta.next_line()
         || this->reader_meta.next_line()
         || this->reader_meta.next_line()
        ) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        //@}
    }

    virtual ~CryptoInByMetaSequenceTransport()
    {
        ::close(this->wrm_fd);
    }

    void next_chunk_info()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        this->open_next_wrm(ERR_TRANSPORT_READ_FAILED);

        this->init_crypto_ctx(this->crypto_wrm, this->path, this->wrm_fd);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        if (this->wrm_fd == -1) {
            this->next_chunk_info();
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
//                 unsigned char hash[HASH_LEN];
//                 this->crypto_wrm.close(hash, this->crypto_ctx->hmac_key);
                this->next_chunk_info();
                continue;
            }
            if (res == 0){
                if (zero_res) {
//                     unsigned char hash[HASH_LEN];
//                     this->crypto_wrm.close(hash, this->crypto_ctx->hmac_key);
                    this->next_chunk_info();
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
        this->open_next_wrm(ERR_TRANSPORT_NO_MORE_DATA);
        return true;
    }

private:
    void init_crypto_ctx(crypto_file & crypto, const char * filename, int fd)
    {
        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(filename, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (-1 == compute_hmac(trace_key, this->crypto_ctx->crypto_key, derivator)) {
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }

        if (-1 == crypto.open_read_init(fd, trace_key, this->crypto_ctx)) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
    }
};

#endif
