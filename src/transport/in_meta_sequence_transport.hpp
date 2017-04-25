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
#include <cstdio> // snprintf

#include "utils/log.hpp"
#include "utils/chex_to_int.hpp"
#include "capture/cryptofile.hpp"
#include "transport/in_crypto_transport.hpp"


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
    unsigned char hash1[MD_HASH::DIGEST_LENGTH];
    unsigned char hash2[MD_HASH::DIGEST_LENGTH];
};

enum class WrmVersion : uint8_t
{
    unknown,
    v1 = 1,
    v2 = 2
};

class InMetaSequenceTransport : public Transport
{
    InCryptoTransport cfb;
    time_t            begin_time;
    InCryptoTransport buf_meta;

    struct rl_t {
        char buf[1024];
        char * eof;
        char * cur;
        rl_t() : eof(this->buf), cur(this->buf) {}
    } rl;

    WrmVersion meta_header_version;
    bool meta_header_has_checksum;

    MetaLine meta_line;
    char meta_path[2048];

public:
    MetaLine const & get_meta_line() const noexcept
    {
        return this->meta_line;
    }

    ssize_t buf_reader_read_line(char * dest, size_t len, int err)
    {
        ssize_t total_read = 0;
        while (1) {
            if (this->rl.cur > this->rl.eof) {
                LOG(LOG_INFO, "InMetaSequenceTransport::ERR_TRANSPORT_READ_FAILED");
                return -ERR_TRANSPORT_READ_FAILED;
            }
            char * pos = std::find(this->rl.cur, this->rl.eof, '\n');
            if (len < static_cast<size_t>(pos - this->rl.cur)) {
                total_read += len;
                memcpy(dest, this->rl.cur, len);
                this->rl.cur += len;
                break;
            }
            total_read += pos - this->rl.cur;
            memcpy(dest, this->rl.cur, pos - this->rl.cur);
            dest += pos - this->rl.cur;
            this->rl.cur = pos + 1;
            if (pos != this->rl.eof) {
                break;
            }

            ssize_t ret = this->buf_meta.partial_read(this->rl.buf, sizeof(this->rl.buf));
            if (ret == 0) {
                return -err;
            }
            this->rl.eof = this->rl.buf + ret;
            this->rl.cur = this->rl.buf;
        }
        return total_read;
    }

    WrmVersion get_wrm_version() {
        return this->meta_header_version;
    }

    int buf_reader_next_line()
    {
        char * pos;
        while ((pos = std::find(this->rl.cur, this->rl.eof, '\n')) == this->rl.eof) {
            ssize_t ret = this->buf_meta.partial_read(this->rl.buf, sizeof(this->rl.buf));
            if (ret == 0) {
                return -ERR_TRANSPORT_READ_FAILED;
            }
            this->rl.eof = this->rl.buf + ret;
            this->rl.cur = this->rl.buf;
        }
        this->rl.cur = pos+1;
        return 0;
    }

    bool get_status() const override
    {
        return this->status ? this->cfb.get_status() : false;
    }

public:
    int buf_read_meta_file_v1(MetaLine & meta_line) {
        char line[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + (1+MD_HASH::DIGEST_LENGTH*2) * 2 + 2];
        ssize_t len = this->buf_reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
        if (len < 0) {
            return -len;
        }
        line[len] = 0;

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

        // TODO Code below looks much too complicated for what it's doing

        typedef std::reverse_iterator<char*> reverse_iterator;

        using std::begin;

        reverse_iterator last(line);
        reverse_iterator first(line + len);
        reverse_iterator e1 = std::find(first, last, ' ');
        if (e1 - first == 64) {
            int err = 0;
            auto phash = begin(meta_line.hash2);
            for (char * b = e1.base(), * e = b + 64; e != b; ++b, ++phash) {
                *phash = (chex_to_int(*b, err) << 4);
                *phash |= chex_to_int(*++b, err);
            }
            REDASSERT(!err);
        }

        reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
        if (e2 - (e1 + 1) == 64) {
            int err = 0;
            auto phash = begin(meta_line.hash1);
            for (char * b = e2.base(), * e = b + 64; e != b; ++b, ++phash) {
                *phash = (chex_to_int(*b, err) << 4);
                *phash |= chex_to_int(*++b, err);
            }
            REDASSERT(!err);
        }

        if (e1 - first == 64 && e2 != last) {
            first = e2 + 1;
            e1 = std::find(first, last, ' ');
            e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
        }

        meta_line.stop_time = meta_parse_sec(e1.base(), first.base());
        if (e1 != last) {
            ++e1;
        }
        meta_line.start_time = meta_parse_sec(e2.base(), e1.base());

        if (e2 != last) {
            *e2 = 0;
        }

        auto path_len = std::min(int(e2.base() - line), PATH_MAX);
        memcpy(meta_line.filename, line, path_len);
        meta_line.filename[path_len] = 0;

        return 0;
    }

    char const * buf_sread_filename(char * p, char const * e, char const * pline)
    {
        e -= 1;
        for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
            *p = *pline;
        }
        *p = 0;
        return pline;
    }

    template<bool read_start_stop_time>
    int buf_read_meta_file_v2_impl(bool has_checksum, MetaLine & meta_line) {
        char line[
            PATH_MAX + 1 + 1 +
            (std::numeric_limits<long long>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<unsigned long long>::digits10 + 1 + 1) * 2 +
            (1 + MD_HASH::DIGEST_LENGTH*2) * 2 +
            2
        ];
        ssize_t len = this->buf_reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
        if (len < 0) {
            return -len;
        }
        line[len] = 0;

        // Line format "fffff
        // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
        // sssss eeeee hhhhh HHHHH"
        //            ^  ^  ^  ^
        //            |  |  |  |
        //            |hash1|  |
        //            |     |  |
        //        space3    |hash2
        //                  |
        //                space4
        //
        // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
        //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

        auto pline = line + (this->buf_sread_filename(std::begin(meta_line.filename), std::end(meta_line.filename), line) - line);


        int err = 0;
        auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);
        if (read_start_stop_time) {
            err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);

            if (meta_line.stop_time < this->begin_time && this->meta_header_version == WrmVersion::v2) {
                ssize_t len = this->buf_reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
                if (len < 0) {
                    return -len;
                }
                line[len] = 0;

                return buf_read_meta_file_v2_impl<true>(has_checksum, meta_line);

            }
        }



        // TODO Why do this with lambda ? Is it so important to avoid typing 3 lines of code two times ?
        if (has_checksum){
            err |= len - (pend - line) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 2;
            if (!err)
            {
                {
                    auto phash = std::begin(meta_line.hash1);
                    for (auto e = ++pend + sizeof(meta_line.hash1) * 2u; pend != e; ++pend, ++phash) {
                        *phash = (chex_to_int(*pend, err) << 4);
                        *phash |= chex_to_int(*++pend, err);
                    }
                }
                err |= (*pend != ' ');
                {
                    auto phash = std::begin(meta_line.hash2);
                    for (auto e = ++pend + sizeof(meta_line.hash2) * 2u; pend != e; ++pend, ++phash) {
                        *phash = (chex_to_int(*pend, err) << 4);
                        *phash |= chex_to_int(*++pend, err);
                    }
                }
            }
        }
        err |= bool(*pend);

        if (err) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        return 0;
    }

    int buf_read_meta_file_v2(MetaLine & meta_line) {
        return buf_read_meta_file_v2_impl<true>(this->meta_header_has_checksum, meta_line);
    }

    int buf_read_meta_file(MetaLine & meta_line)
    {
        if (this->meta_header_version == WrmVersion::v1) {
            return this->buf_read_meta_file_v1(meta_line);
        }
        else {
            return this->buf_read_meta_file_v2(meta_line);
        }
    }

private:
    int buf_open_next() {
        if (const int e = this->buf_reader_next_line()) {
            return e < 0 ? e : -1;
        }
        this->cfb.open(this->meta_line.filename);
        return 0;
    }

    int buf_next_line()
    {
        if (auto err = this->buf_read_meta_file(this->meta_line)) {
            return err;
        }

        if (!file_exist(this->meta_line.filename)) {
            char original_path[1024] = {};
            char basename[1024] = {};
            char extension[256] = {};
            char filename[2048] = {};

            canonical_path( this->meta_line.filename
                          , original_path, sizeof(original_path)
                          , basename, sizeof(basename)
                          , extension, sizeof(extension));
            std::snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

            if (file_exist(filename)) {
                strcpy(this->meta_line.filename, filename);
            }
        }

        return 0;
    }

public:
    using EncryptionMode = InCryptoTransport::EncryptionMode;

    InMetaSequenceTransport(
        CryptoContext & cctx,
        const char * filename,
        const char * extension,
        EncryptionMode encryption)
    : cfb(cctx, encryption)
    , begin_time(0)
    , buf_meta(cctx, encryption)
    , meta_header_version(WrmVersion::v1)
    , meta_header_has_checksum(false)
    {
        temporary_concat tmp(filename, extension);
        const char * meta_filename = tmp.c_str();
        this->buf_meta.open(meta_filename);

        char line[32];
        auto sz = this->buf_reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
        if (sz < 0) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        // v2
        if (line[0] == 'v') {
            if (this->buf_reader_next_line()
             || (sz = this->buf_reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED)) < 0
            ) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            this->meta_header_version = WrmVersion::v2;
            this->meta_header_has_checksum = (line[0] == 'c');
        }
        // else v1

        if (this->buf_reader_next_line()) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        if (this->buf_reader_next_line()) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        this->meta_line.start_time = 0;
        this->meta_line.stop_time = 0;

        this->meta_path[0] = 0;

        char basename[1024] = {};
        char extension2[256] = {};

        canonical_path( meta_filename
                      , this->meta_path, sizeof(this->meta_path)
                      , basename, sizeof(basename)
                      , extension2, sizeof(extension2));
    }

    ~InMetaSequenceTransport()
    {
        this->buf_close();
        if (this->buf_meta.is_open()) {
            this->buf_meta.close();
        }
    }

    void set_begin_time(time_t begin_time) {
        this->begin_time = begin_time;
    }


    bool disconnect() override
    {
        this->buf_close();
        return true;
    }

    time_t begin_chunk_time() const noexcept
    {
        return this->meta_line.start_time;
    }

    time_t end_chunk_time() const noexcept
    {
        return this->meta_line.stop_time;
    }

    const char * path() const noexcept
    { return this->meta_line.filename; }

    bool next() override
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        this->buf_close();

        const ssize_t res = this->buf_next_line();
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        ++this->seqno;
        this->cfb.open(this->meta_line.filename);
        return true;
    }

private:
    void buf_close()
    {
        if (this->cfb.is_open()) {
            this->cfb.close();
        }
    }

    bool buf_read(uint8_t * data, size_t len)
    {
        for (;;) {
            if (!this->cfb.is_open()) {
                if (const int e1 = this->buf_next_line()) {
                    if (e1 == ERR_TRANSPORT_NO_MORE_DATA) {
                        return false;
                    }
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, e1);
                }
                ++this->seqno;
                this->cfb.open(this->meta_line.filename);
            }

            if (this->cfb.atomic_read(data, len)) {
                return true;
            }

            this->cfb.close();
        }
    }

    bool do_atomic_read(uint8_t * buffer, size_t len) override
    {
        return this->buf_read(buffer, len);
    }
};
