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


#pragma once

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <cstdio>

#include <memory>


#include "utils/log.hpp"
#include "core/error.hpp"
#include "no_param.hpp"
#include "utils/fileutils.hpp"
#include "openssl_crypto.hpp"
#include "transport/cryptofile.hpp"

namespace detail
{
    inline char chex_to_int(char c, int & err) {
        return
            '0' <= c && c <= '9' ? c-'0'
          : 'a' <= c && c <= 'f' ? c-'a' + 10
          : 'A' <= c && c <= 'F' ? c-'A' + 10
          : ((err |= 1), '\0');
    }

    template<class Reader>
    class ReaderLine
    {
        char buf[1024];
        char * eof;
        char * cur;
        Reader reader;

        int read(int err)
        {
            ssize_t ret = this->reader.reader_read(this->buf, sizeof(this->buf));
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
        explicit ReaderLine(Reader reader) noexcept
        : eof(buf)
        , cur(buf)
        , reader(reader)
        {}

        ssize_t read_line(char * dest, size_t len, int err)
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
                dest += pos - this->cur;
                this->cur = pos + 1;
                if (pos != this->eof) {
                    break;
                }
                if (int e = this->read(err)) {
                    return e;
                }
            }
            return total_read;
        }

        int next_line()
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

    struct MetaHeader {
        unsigned version;
        //unsigned witdh;
        //unsigned height;
        bool has_checksum;
    };


    template<class Reader>
    MetaHeader read_meta_headers(ReaderLine<Reader> & reader)
    {
        MetaHeader header{1, false};

        char line[32];
        auto sz = reader.read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
        if (sz < 0) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        // v2
        if (line[0] == 'v') {
            if (reader.next_line()
             || (sz = reader.read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED)) < 0
            ) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            header.version = 2;
            header.has_checksum = (line[0] == 'c');
        }
        // else v1

        if (reader.next_line()
         || reader.next_line()
        ) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        return header;
    }

    struct MetaLine
    {
        char    filename[PATH_MAX + 1];
        off_t   size;
        mode_t  mode;
        uid_t   uid;
        gid_t   gid;
        dev_t   dev;
        ino_t   ino;
        time_t  mtime;
        time_t  ctime;
        time_t  start_time;
        time_t  stop_time;
        unsigned char hash1[MD_HASH_LENGTH];
        unsigned char hash2[MD_HASH_LENGTH];
    };

    inline time_t meta_parse_sec(const char * first, const char * last)
    {
        time_t sec = 0;
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

    static inline char const * sread_filename(char * p, char const * e, char const * pline)
    {
        e -= 1;
        for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
            *p = *pline;
        }
        *p = 0;
        return pline;
    }

    struct temporary_concat
    {
        char str[1024];

        temporary_concat(const char * a, const char * b)
        {
            if (std::snprintf(this->str, sizeof(this->str), "%s%s", a, b) >= int(sizeof(this->str))) {
                throw Error(ERR_TRANSPORT);
            }
        }

        const char * c_str() const noexcept
        { return this->str; }
    };
}

