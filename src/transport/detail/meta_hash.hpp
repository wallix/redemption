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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_TRANSPORT_DETAIL_META_HASH_HPP
#define REDEMPTION_TRANSPORT_DETAIL_META_HASH_HPP


#include <cstdio>

#include "cryptofile.h"
#include "meta_opener.hpp"
#include "meta_writer.hpp"
#include "fileutils.hpp"

namespace detail {

template<class FilterParams = no_param>
struct out_hash_meta_sequence_filename_buf_param
{
    out_meta_sequence_filename_buf_param<CryptoContext&> meta_sq_params;
    FilterParams filter_params;
    CryptoContext & cctx;

    out_hash_meta_sequence_filename_buf_param(
        CryptoContext & cctx,
        time_t start_sec,
        FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        FilterParams const & filter_params = FilterParams(),
        uint32_t verbose = 0)
    : meta_sq_params(start_sec, format, hash_prefix, prefix, filename, extension, groupid, verbose, cctx)
    , filter_params(filter_params)
    , cctx(cctx)
    {}
};

template<class Buf, class BufFilter, class BufMeta, class BufHash>
class out_hash_meta_sequence_filename_buf_impl
: public out_meta_sequence_filename_buf_impl<Buf, BufMeta>
{
    CryptoContext & cctx;
    BufFilter wrm_filter;

    using sequence_base_type = out_meta_sequence_filename_buf_impl<Buf, BufMeta>;

public:
    template<class FilterParams>
    explicit out_hash_meta_sequence_filename_buf_impl(
        out_hash_meta_sequence_filename_buf_param<FilterParams> const & params
    )
    : sequence_base_type(params.meta_sq_params)
    , cctx(params.cctx)
    , wrm_filter(params.filter_params)
    {}

    ssize_t write(const void * data, size_t len)
    {
        if (!this->buf().is_open()) {
            const char * filename = this->get_filename_generate();
            const int res = this->open_filename(filename);
            if (res < 0) {
                return res;
            }
            if (int err = this->wrm_filter.open(this->buf(), this->cctx, filename)) {
                return err;
            }
        }
        return this->wrm_filter.write(this->buf(), data, len);
    }

    int close()
    {
        if (this->buf().is_open()) {
            if (this->next()) {
                return 1;
            }
        }

        BufHash hash_buf(this->cctx);

        if (!this->meta_buf().is_open()) {
            return 1;
        }

        hash_type hash;

        if (this->meta_buf().close(hash)) {
            return 1;
        }

        return write_meta_hash(this->hash_filename(), this->meta_filename(), hash_buf, &hash, this->verbose);
    }

    int next()
    {
        if (this->buf().is_open()) {
            hash_type hash;
            {
                const int res1 = this->wrm_filter.close(this->buf(), hash, this->cctx.hmac_key);
                const int res2 = this->buf().close();
                if (res1) {
                    return res1;
                }
                if (res2) {
                    return res2;
                }
            }

            return this->next_meta_file(&hash);
        }
        return 1;
    }
};


struct HashHeader {
    unsigned version;
};

template<class Reader>
HashHeader read_hash_headers(ReaderLine<Reader> & reader)
{
    HashHeader header{1};

    char line[32];
    auto sz = reader.read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
    if (sz < 0) {
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }

    // v2
    REDASSERT(line[0] == 'v');
    header.version = 2;

    if (reader.next_line()
     || reader.next_line()
    ) {
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }

    return header;
}

template<class Reader>
int read_hash_file_v2(ReaderLine<Reader> & reader, HashHeader const & hash_header,  MetaLine & hash_line) {
    char line[
        PATH_MAX + 1 + 1 +
        (std::numeric_limits<long long>::digits10 + 1 + 1 + 1) * 8 +
        (1 + MD_HASH_LENGTH * 2) * 2 +
        2
    ];
    ssize_t len = reader.read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
    if (len < 0) {
        return -len;
    }
    line[len] = 0;

    // Line format "fffff
    // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
    //             hhhhh HHHHH"
    //            ^  ^  ^  ^
    //            |  |  |  |
    //            |hash1|  |
    //            |     |  |
    //          space   |hash2
    //                  |
    //                space
    //
    // filename(1 or >) + ... + space(1) + hash1(64) + space(1) + hash2(64) >= 135

    using std::begin;
    using std::end;

    // filename
    auto pline = line;
    {
        auto p = begin(hash_line.filename);
        auto e = end(hash_line.filename) - 1;
        for (; p != e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
            *p = *pline;
        }
        *p = 0;
    }

    int err = 0;
    auto pend = pline;                   hash_line.size       = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.mode       = strtoull(pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.uid        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.gid        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.dev        = strtoull(pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.ino        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.mtime      = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; hash_line.ctime      = strtoll (pline, &pend, 10);

    if (!(err |= (len - (pend - line) != (sizeof(hash_line.hash1) + sizeof(hash_line.hash2)) * 2 + 2))
    ) {
        auto phash = begin(hash_line.hash1);
        for (auto e = ++pend + sizeof(hash_line.hash1) * 2u; pend != e; ++pend, ++phash) {
            *phash = (chex_to_int(*pend, err) << 4);
            *phash |= chex_to_int(*++pend, err);
        }
        err |= (*pend != ' ');
        phash = begin(hash_line.hash2);
        for (auto e = ++pend + sizeof(hash_line.hash2) * 2u; pend != e; ++pend, ++phash) {
            *phash = (chex_to_int(*pend, err) << 4);
            *phash |= chex_to_int(*++pend, err);
        }
    }

    err |= bool(*pend);

    if (err) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    return 0;
}

}

#endif

