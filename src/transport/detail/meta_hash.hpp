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

struct MetaHashMaker
{
    MetaHashMaker(unsigned char (&hash)[MD_HASH_LENGTH*2])
    : MetaHashMaker(
        reinterpret_cast<unsigned char (&)[MD_HASH_LENGTH]>(hash[0]),
        reinterpret_cast<unsigned char (&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH])
    )
    {}

    MetaHashMaker(unsigned char (&hash1)[MD_HASH_LENGTH], unsigned char (&hash2)[MD_HASH_LENGTH]) {
        char * p = this->mes;
        auto write = [&p](unsigned char (&hash)[MD_HASH_LENGTH]){
            *p++ = ' ';                // 1 octet
            for (unsigned c : hash) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        write(hash1);
        write(hash2);
        *p = 0;
    }

    char const * c_str() const noexcept {
        return this->mes;
    }

private:
    char mes[(1 + MD_HASH_LENGTH*2) * 2 + 1];
};

template<class MetaBuf, class HashBuf>
int close_meta_hash(detail::MetaFilename hf, MetaBuf & meta_buf, HashBuf & crypto_hash, uint32_t verbose)
{
    if (!meta_buf.is_open()) {
        return 1;
    }

    unsigned const hash_len = MD_HASH_LENGTH * 2;

    unsigned char hash[hash_len + 1] = {0};
    hash[0] = ' ';

    if (meta_buf.close(reinterpret_cast<unsigned char(&)[hash_len]>(hash[1]))) {
        return 1;
    }

    char path[1024] = {};
    char basename[1024] = {};
    char extension[256] = {};
    char filename[2048] = {};

    canonical_path( hf.filename
                  , path, sizeof(path)
                  , basename, sizeof(basename)
                  , extension, sizeof(extension)
                  , verbose);
    snprintf(filename, sizeof(filename), "%s%s", basename, extension);

    if (crypto_hash.open(hf.filename) >= 0) {
        const size_t len = strlen(filename);
        if (crypto_hash.write(filename, len) != long(len)
         || crypto_hash.write(hash, hash_len+1) != hash_len+1
         || crypto_hash.close(/*hash*/) != 0) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [%u]\n", hf.filename, -hash_len);
            return 1;
        }

        if (chmod(hf.filename, S_IRUSR|S_IRGRP) == -1){
            LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", hf.filename, strerror(errno), errno);
        }
    }
    else {
        int e = errno;
        LOG(LOG_ERR, "Open to transport failed: code=%d", e);
        errno = e;
        return 1;
    }

    return 0;
}

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

