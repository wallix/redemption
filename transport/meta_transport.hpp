/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_PUBLIC_TRANSPORT_META_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_META_TRANSPORT_HPP

#include "filename_sequence_transport.hpp"
#include "buffer/file_buf.hpp"

#include <limits>
#include <ctime>

template<class Buf/*, class FilenameAccessTraits*/>
struct out_meta_nexter
: protected Buf
{
    out_meta_nexter(time_t sec) /*noexcept*/
    : start_sec(sec)
    , stop_sec(sec)
    {}

    template<class Transport, class TransportBuf>
    int next(Transport & /*trans*/, TransportBuf & buf) /*noexcept*/
    {
        if (buf.is_open()) {
            buf.close();
            const char * filename = buf.impl().seqgen().get(buf.impl().seqnum());
            size_t len = strlen(filename);
            ssize_t res = this->write(filename, len);
            if (res >= 0 && size_t(res) == len) {
                char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 5];
                len = snprintf(mes, sizeof(mes), " %u %u\n",
                               (unsigned)this->start_sec,
                               (unsigned)this->stop_sec+1);
                res = this->write(mes, len);
                this->start_sec = this->stop_sec;
            }
            if (res < 0) {
                int err = errno;
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
                return res;
            }
            return 0;
        }
        return 1;
    }

    void update_sec(time_t sec) /*noexcept*/
    { this->stop_sec = sec; }

private:
    time_t start_sec;
    time_t stop_sec;
};

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


struct OutMetaTransport
: OutBufferTransport<
    transbuf::output_buf<io::posix::fdbuf, detail::FilenameSequencePolicy>,
    out_meta_nexter<transbuf::ofile_base>
>
{
    OutMetaTransport(const char * path,
                     const char * basename,
                     timeval now,
                     uint16_t width,
                     uint16_t height,
                     const int groupid,
                     auth_api * authentifier = NULL,
                     unsigned verbose = 0,
                     FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : OutMetaTransport::TransportType(
        detail::FilenameSequencePolicyParams(format, path, basename, ".wrm", groupid),
        now.tv_sec
    )
    {
        (void)verbose;

        if (this->OutMetaTransport::NexterType::open(detail::MetaFilename(path, basename, format).filename, S_IRUSR) < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(static_cast<transbuf::ofile_base&>(*this), path, width, height, this->authentifier);
    }

    virtual void timestamp(timeval now)
    {
        this->update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    { return &this->impl().seqgen(); }
};


namespace detail {
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

    template<class Buf>
    class MetaOpener
    : public transbuf::open_close_base
    {
        struct ReaderBuf
        {
            Buf & buf;

            ReaderBuf(Buf & buf)
            : buf(buf)
            {}

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        char path[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
        Buf buf;
        ReaderLine<ReaderBuf> reader;
        unsigned begin_chunk_time;
        unsigned end_chunk_time;
        unsigned seqnum;

        static Buf & open_and_return(const char * filename, Buf & buf)
        {
            if (buf.open(filename) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            return buf;
        }

        void read_header()
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

    public:
        MetaOpener(const char * filename)
        : reader(this->open_and_return(filename, this->buf))
        , begin_chunk_time(0)
        , end_chunk_time(0)
        , seqnum(0)
        {
            this->read_header();
        }

        template<class T>
        MetaOpener(const transbuf::two_params<T, const char *> & params)
        : buf(params.buf_params)
        , reader(this->open_and_return(params.other_params, this->buf))
        , begin_chunk_time(0)
        , end_chunk_time(0)
        , seqnum(0)
        {
            this->read_header();
        }

        template<class UBuf>
        int init(UBuf & buf) /*noexcept*/
        {
            if (int e = this->next()) {
                return e;
            }
            return buf.open(this->path);
        }

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
            ++this->seqnum;
            return 0;
        }

        unsigned get_seqno() const /*noexcept*/
        { return this->seqnum; }

        const char * get_path() const /*noexcept*/
        { return this->path; }

        unsigned get_begin_chunk_time() const /*noexcept*/
        { return this->begin_chunk_time; }

        unsigned get_end_chunk_time() const /*noexcept*/
        { return this->end_chunk_time; }

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

    struct temporary_concat
    {
        char str[1024];

        temporary_concat(const char * a, const char * b)
        {
            if (std::snprintf(this->str, sizeof(this->str), "%s%s", a, b) >= int(sizeof(this->str))) {
                throw Error(ERR_TRANSPORT);
            }
        }

        const char * c_str() const /*noexcept*/
        { return this->str; }
    };
}

struct in_meta_nexter
{
    in_meta_nexter() /*noexcept*/
    {}

    template<class Transport, class TransportBuf>
    int next(Transport & /*trans*/, TransportBuf & buf) /*noexcept*/
    {
        if (buf.is_open()) {
            buf.close();
        }
        return buf.impl().next();
    }
};

struct InMetaTransport
: InBufferTransport<
    transbuf::reopen_input<transbuf::ifile_base, detail::MetaOpener<transbuf::ifile_base> >,
    in_meta_nexter
>
{
    InMetaTransport(const char * filename, const char * extension)
    : InMetaTransport::TransportType(detail::temporary_concat(filename, extension).str)
    {}

    unsigned begin_chunk_time() const /*noexcept*/
    { return this->impl().get_begin_chunk_time(); }

    unsigned end_chunk_time() const /*noexcept*/
    { return this->impl().get_end_chunk_time(); }

    const char * path() const /*noexcept*/
    { return this->impl().get_path(); }

    unsigned get_seqno() const /*noexcept*/
    { return this->impl().get_seqno(); }
};

#endif
