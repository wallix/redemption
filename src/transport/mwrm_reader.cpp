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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan, Clément Moroldo
 */

#include "transport/mwrm_reader.hpp"
#include "transport/crypto_transport.hpp"
#include "utils/sugar/std_stream_proto.hpp"
#include "utils/chex_to_int.hpp"

#include <algorithm>

#include <cerrno>

// buffer must be able to contain line
// if no line at end of buffer apply some memmove
/// \exception Error : ERR_TRANSPORT_READ_FAILED
Transport::Read LineReader::next_line()
{
    if (this->eol > this->eof) {
        return Transport::Read::Eof;
    }
    this->cur = this->eol;
    char * pos = std::find(this->cur, this->eof, '\n');
    if (pos == this->eof) {
        // move remaining data to beginning of buffer
        size_t len = this->eof - this->cur;
        ::memmove(this->buf, this->cur, len);
        this->cur = this->buf;
        this->eof = this->cur + len;

        do { // read and append to buffer
            size_t ret = this->ibuf.partial_read(this->eof, std::end(this->buf)-2-this->eof);
            if (ret == 0) {
                break;
            }
            pos = std::find(this->eof, this->eof + ret, '\n');
            this->eof += ret;
            this->eof[0] = 0;
        } while (pos == this->eof);
    }

    this->eol = (pos == this->eof) ? pos : pos + 1;

    // end of file
    if (this->buf == this->eof) {
        return Transport::Read::Eof;
    }

    // line without \n is a error
    if (pos == this->eof) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    return Transport::Read::Ok;
}

namespace
{
    void init_stat_v1(MetaLine & meta_line)
    {
        meta_line.size = 0;
        meta_line.mode = 0;
        meta_line.uid = 0;
        meta_line.gid = 0;
        meta_line.dev = 0;
        meta_line.ino = 0;
        meta_line.mtime = 0;
        meta_line.ctime = 0;
    }

    char * extract_hash(uint8_t (&hash)[MD_HASH::DIGEST_LENGTH], char * p, int & err)
    {
        for (uint8_t & chash : hash) {
            chash  = chex_to_int(*p++, err) << 4;
            chash |= chex_to_int(*p++, err);
        }
        return p;
    }

    char * buf_sread_filename(char * p, char const * e, char * pline)
    {
        e -= 1;
        for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
            *p = *pline;
        }
        *p = 0;
        return pline;
    }
}

MwrmReader::MwrmReader(InTransport ibuf) noexcept
: line_reader(ibuf)
{
    this->header.version = WrmVersion::v1;
    this->header.has_checksum = false;
}

void MwrmReader::read_meta_headers()
{
    auto next_line = [this]{
        if (Transport::Read::Eof == this->line_reader.next_line()) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
    };
    next_line();
    this->header.has_checksum = false;
    this->header.version = WrmVersion::v1;
    if (this->line_reader.get_buf()[0] == 'v') {
        next_line(); // 800 600
        next_line(); // checksum or nochecksum
        this->header.version = WrmVersion::v2;
        this->header.has_checksum = (this->line_reader.get_buf()[0] == 'c');
    }
    // else v1
    next_line(); // blank
    next_line(); // blank
}

void MwrmReader::set_header(MetaHeader const & header)
{
    this->header = header;
}

Transport::Read MwrmReader::read_meta_line(MetaLine & meta_line)
{
    switch (this->header.version) {
    case WrmVersion::v1:
        init_stat_v1(meta_line);
        return this->read_meta_line_v1(meta_line);
    case WrmVersion::v2:
        return this->read_meta_line_v2(meta_line, FileType::Mwrm);
    }
    assert(false);
    throw Error(ERR_TRANSPORT_READ_FAILED);
}

void MwrmReader::read_meta_hash_line(MetaLine & meta_line)
{
    Transport::Read r = Transport::Read::Eof;
    switch (this->header.version) {
    case WrmVersion::v1:
        init_stat_v1(meta_line);
        r = this->read_meta_hash_line_v1(meta_line);
        break;
    case WrmVersion::v2:
        bool is_eof = false;
        // skip header: "v2\n\n\n"
        is_eof |= Transport::Read::Eof == this->line_reader.next_line();
        is_eof |= Transport::Read::Eof == this->line_reader.next_line();
        is_eof |= Transport::Read::Eof == this->line_reader.next_line();
        if (is_eof) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        r = this->read_meta_line_v2(meta_line, FileType::Hash);
        break;
    }

    if (r != Transport::Read::Ok || Transport::Read::Eof != this->line_reader.next_line()) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
}

Transport::Read MwrmReader::read_meta_hash_line_v1(MetaLine & meta_line)
{
    // don't use line_reader.next_line() because '\n' is a valid hash character :/
    auto line_buf = [this]{
        auto * const buf = std::begin(this->line_reader.buf);
        auto * first = buf;
        auto * last = std::end(this->line_reader.buf);
        do { // read and append to buffer
            size_t ret = this->line_reader.ibuf.partial_read(first, last-first);
            if (ret == 0) {
                break;
            }
            first += ret;
        } while (first != last);
        return make_array_view(buf, first);
    }();

    meta_line.with_hash = true;

    // Filename HASH_64_BINARY_BYTES
    //         ^
    //         |
    //     separator

    constexpr std::size_t hash_size = 32u;
    constexpr std::size_t size_after_filename = hash_size * 2 + 1;

    static_assert(hash_size == sizeof(meta_line.hash1), "");
    static_assert(hash_size == sizeof(meta_line.hash2), "");

    if (line_buf.size() <= size_after_filename) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
    if (line_buf.size() - size_after_filename >= sizeof(meta_line.filename)-1) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    auto const path_len = line_buf.size() - size_after_filename;
    memcpy(meta_line.filename, line_buf.begin(), path_len);
    meta_line.filename[path_len] = 0;
    memcpy(meta_line.hash1, line_buf.end() - 64, sizeof(meta_line.hash1));
    memcpy(meta_line.hash2, line_buf.end() - 32, sizeof(meta_line.hash1));

    return Transport::Read::Ok;
}

Transport::Read MwrmReader::read_meta_line_v1(MetaLine & meta_line)
{
    if (Transport::Read::Eof == this->line_reader.next_line()) {
        return Transport::Read::Eof;
    }

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

    auto line_buf = this->line_reader.get_buf();

    reverse_iterator last(line_buf.begin());
    reverse_iterator first(line_buf.end());
    reverse_iterator e1 = std::find(first, last, ' ');
    int err = 0;

    meta_line.with_hash = (e1 - first == 65);
    if (meta_line.with_hash) {
        extract_hash(meta_line.hash2, e1.base(), err);
    }

    reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
    if (e2 - e1 == 65) {
        extract_hash(meta_line.hash1, e2.base(), err);
    }
    else if (meta_line.with_hash) {
        LOG(LOG_ERR, "mwrm read line v1: fhash without qhash");
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    if (err) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    bool const has_hash = (e1 - first == 65 && e2 != last);

    if (has_hash) {
        first = e2 + 1;
        e1 = std::find(first, last, ' ');
        e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
    }

    char * pend;
    meta_line.stop_time  = strtoll(e1.base(), &pend, 10);
    err |= (*pend != (has_hash ? ' ' : '\n'));
    if (e1 != last) {
        ++e1;
    }
    meta_line.start_time = strtoll(e2.base(), &pend, 10);
    err |= (*pend != ' ');
    if (e2 != last) {
        *e2 = 0;
    }

    if (err) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    auto const path_len = std::min(size_t(e2.base() - line_buf.begin()), sizeof(meta_line.filename)-1);
    memcpy(meta_line.filename, line_buf.begin(), path_len);
    meta_line.filename[path_len] = 0;

    return Transport::Read::Ok;
}

Transport::Read MwrmReader::read_meta_line_v2(MetaLine & meta_line, FileType file_type)
{
    if (Transport::Read::Eof == this->line_reader.next_line()) {
        return Transport::Read::Eof;
    }

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

    auto line_buf = this->line_reader.get_buf();
    auto const line = line_buf.data();

    using std::begin;
    using std::end;

    char * pline = buf_sread_filename(begin(meta_line.filename), end(meta_line.filename), line);

    int err = 0;
    auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
    err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);

    if (file_type == FileType::Mwrm) {
        err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);
    }
    else {
        meta_line.start_time = 0;
        meta_line.stop_time  = 0;
    }

    meta_line.with_hash = (file_type == FileType::Mwrm)
      ? this->header.has_checksum
      : (this->header.has_checksum || (*pend != '\n' && *pend != '\0'));
    if (meta_line.with_hash) {
        // ' ' hash ' ' hash '\n'
        err |= line_buf.size() - (pend - line) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 3;
        if (!err)
        {
            err |= (*pend != ' '); pend = extract_hash(meta_line.hash1, ++pend, err);
            err |= (*pend != ' '); pend = extract_hash(meta_line.hash2, ++pend, err);
        }
    }
    err |= (*pend != '\n' && *pend != '\0');

    if (err) {
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    return Transport::Read::Ok;
}
