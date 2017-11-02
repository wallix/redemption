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

#pragma once

#include "capture/cryptofile.hpp"
#include "transport/transport.hpp"
#include "utils/sugar/std_stream_proto.hpp"

#include <linux/limits.h> // PATH_MAX
#include <sys/types.h>

class InCryptoTransport;


enum class WrmVersion : uint8_t
{
    //unknown,
    v1 = 1,
    v2 = 2
};

REDEMPTION_OSTREAM(out, WrmVersion version)
{
    return out << 'v' << static_cast<int>(version);
}

struct MetaHeader
{
    WrmVersion version;
    //unsigned witdh;
    //unsigned height;
    bool has_checksum;
};

struct MetaLine
{
    char    filename[PATH_MAX + 1];
    // always 0 with header.version = 1
    //@{
    off_t   size;
    mode_t  mode;
    uid_t   uid;
    gid_t   gid;
    dev_t   dev;
    ino_t   ino;
    time_t  mtime;
    time_t  ctime;
    //@}
    time_t  start_time;
    time_t  stop_time;
    // always true with header.version = 2 and header.has_checksum = true
    bool with_hash;
    uint8_t hash1[MD_HASH::DIGEST_LENGTH];
    uint8_t hash2[MD_HASH::DIGEST_LENGTH];

    MetaLine()
    : filename{}
    , size(0)
    , mode(0)
    , uid(0)
    , gid(0)
    , dev(0)
    , ino(0)
    , mtime{}
    , ctime{}
    , start_time{}
    , stop_time{}
    , hash1{}
    , hash2{}
    {}
};

class LineReader
{
    constexpr static std::size_t line_max =
        PATH_MAX + 1 + 1 +
        (std::numeric_limits<long long>::digits10 + 1 + 1) * 8 +
        (std::numeric_limits<unsigned long long>::digits10 + 1 + 1) * 2 +
        (1 + MD_HASH::DIGEST_LENGTH*2) * 2 +
        2;

    char buf[line_max + 1]; // This is to avoid for the bug to be too visible
    char * eof;
    char * eol;
    char * cur;
    InCryptoTransport & ibuf;

    friend class MwrmReader; // for LineReader::read_meta_hash_line()

public:
    LineReader(InCryptoTransport & reader_buf) noexcept
    : buf{}
    , eof(buf)
    , eol(buf)
    , cur(buf)
    , ibuf(reader_buf)
    {}

    // buffer must be able to contain line
    // if no line at end of buffer apply some memmove
    /// \exception Error : ERR_TRANSPORT_READ_FAILED
    Transport::Read next_line();

    array_view_char get_buf() const
    { return {this->cur, this->eol}; }
};

struct MwrmReader
{
    MwrmReader(InCryptoTransport & ibuf) noexcept;

    void read_meta_headers();

    void set_header(MetaHeader const & header);

    Transport::Read read_meta_line(MetaLine & meta_line);

    void read_meta_hash_line(MetaLine & meta_line);

    MetaHeader const& get_header() const
    {
        return this->header;
    }

private:
    LineReader line_reader;
    MetaHeader header;

    Transport::Read read_meta_hash_line_v1(MetaLine & meta_line);

    Transport::Read read_meta_line_v1(MetaLine & meta_line);

    Transport::Read read_meta_line_v2(MetaLine & meta_line, bool has_start_and_stop_time);
};
