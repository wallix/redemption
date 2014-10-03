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

#ifndef REDEMPTION_TRANSPORT_DETAIL_META_OPENER_HPP
#define REDEMPTION_TRANSPORT_DETAIL_META_OPENER_HPP

#include "log.hpp"
#include "error.hpp"
#include "no_param.hpp"
#include "fileutils.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>

namespace detail
{
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

        ssize_t read_line(char * dest, size_t len, int err) /*noexcept*/
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


    template<class BufParam = no_param, class BufMetaParam = no_param>
    struct in_meta_sequence_buf_param
    {
        const char * meta_filename;
        BufParam buf_param;
        BufMetaParam meta_param;
        uint32_t verbose;

        in_meta_sequence_buf_param(
            const char * meta_filename,
            uint32_t verbose = 0,
            const BufParam & buf_param = BufParam(),
            const BufMetaParam & meta_param = BufMetaParam())
        : meta_filename(meta_filename)
        , buf_param(buf_param)
        , meta_param(meta_param)
        , verbose(verbose)
        {}
    };


    template<class Buf, class BufMeta>
    class in_meta_sequence_buf
    : public Buf
    {
        struct ReaderBuf
        {
            BufMeta & buf;

            ReaderBuf(BufMeta & buf)
            : buf(buf)
            {}

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        char path[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
        BufMeta buf_meta;
        ReaderLine<ReaderBuf> reader;
        unsigned begin_chunk_time;
        unsigned end_chunk_time;
        char meta_path[2048];
        uint32_t verbose;

        static BufMeta & open_and_return(const char * filename, BufMeta & buf)
        {
            if (buf.open(filename) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            return buf;
        }

    public:
        template<class BufParam, class BufMetaParam>
        in_meta_sequence_buf(const in_meta_sequence_buf_param<BufParam, BufMetaParam> & params)
        : Buf(params.buf_param)
        , buf_meta(params.meta_param)
        , reader(open_and_return(params.meta_filename, this->buf_meta))
        , begin_chunk_time(0)
        , end_chunk_time(0)
        , verbose(params.verbose)
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
            this->meta_path[0] = 0;

            char basename[1024] = {};
            char extension[256] = {};

            canonical_path( params.meta_filename, this->meta_path, sizeof(this->meta_path), basename, sizeof(basename), extension
                          , sizeof(extension), this->verbose);
        }

        ssize_t read(void * data, size_t len) /*noexcept*/
        {
            if (!this->is_open()) {
                if (const int e = this->open_next()) {
                    return e;
                }
            }

            ssize_t res = this->Buf::read(data, len);
            if (res < 0) {
                return res;
            }
            if (size_t(res) != len) {
                ssize_t res2 = res;
                do {
                    if (const ssize_t err = this->Buf::close()) {
                        return res;
                    }
                    data = static_cast<char*>(data) + res2;
                    if (const int e = this->open_next()) {
                        return res;
                    }
                    len -= res2;
                    res2 = this->Buf::read(data, len);
                    if (res2 < 0) {
                        return res;
                    }
                    res += res2;
                } while (size_t(res2) != len);
            }
            return res;
        }

        /// \return 0 if success
        int next()
        {
            if (this->is_open()) {
                this->close();
            }

            return this->next_line();
        }

    private:
        int open_next() {
            if (const int e = this->next_line()) {
                return e < 0 ? e : -1;
            }
            const int e = this->Buf::open(this->path);
            return (e < 0) ? e : 0;
        }

        int next_line()
        {
            ssize_t len = reader.read_line(this->path, sizeof(this->path) - 1, ERR_TRANSPORT_NO_MORE_DATA);
            if (len < 0) {
                return -len;
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
            reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
            if (e1 - first == 64 && e2 != last) {
                first = e2 + 1;
                e1 = std::find(first, last, ' ');
                e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
            }

            this->end_chunk_time = this->parse_sec(e1.base(), first.base());
            if (e1 != last) {
                ++e1;
            }
            this->begin_chunk_time = this->parse_sec(e2.base(), e1.base());

            if (e2 != last) {
                *e2 = 0;
            }

            if (!file_exist(this->path)) {
                char original_path[1024] = {};
                char basename[1024] = {};
                char extension[256] = {};
                char filename[2048] = {};

                canonical_path( this->path, original_path, sizeof(original_path), basename, sizeof(basename), extension
                              , sizeof(extension), this->verbose);
                snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

                if (file_exist(filename)) {
                    strcpy(this->path, filename);
                }
            }
            return 0;
        }

    public:
        const char * current_path() const /*noexcept*/
        { return this->path; }

        unsigned get_begin_chunk_time() const /*noexcept*/
        { return this->begin_chunk_time; }

        unsigned get_end_chunk_time() const /*noexcept*/
        { return this->end_chunk_time; }

    private:
        static unsigned parse_sec(const char * first, const char * last)
        {
            unsigned sec = 0;
            for (; first != last; ++first) {
                if (*first < '0' || '9' < *first) {
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                unsigned old_sec = sec;
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

#endif
