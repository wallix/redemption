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

#if __has_include(<linux/limits.h>)
# include <linux/limits.h> // PATH_MAX
#else
# define PATH_MAX 4096
#endif

#include <sys/stat.h>
#include <sys/types.h>


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
    uint16_t witdh;
    uint16_t height;
    bool has_checksum;

    MetaHeader() = default;

    MetaHeader(WrmVersion version, bool has_checksum)
      : MetaHeader(version, 0, 0, has_checksum)
    {}

    MetaHeader(WrmVersion version, uint16_t witdh, uint16_t height, bool has_checksum)
      : version(version)
      , witdh(witdh)
      , height(height)
      , has_checksum(has_checksum)
    {}
};

struct MetaLine
{
    char    filename[PATH_MAX + 1] {};
    // always 0 with header.version = 1
    //@{
    off_t   size {};
    mode_t  mode {};
    uid_t   uid {};
    gid_t   gid {};
    dev_t   dev {};
    ino_t   ino {};
    time_t  mtime {};
    time_t  ctime {};
    //@}
    time_t  start_time {};
    time_t  stop_time {};
    // always true with header.version = 2 and header.has_checksum = true
    bool with_hash {};
    uint8_t hash1[MD_HASH::DIGEST_LENGTH] {};
    uint8_t hash2[MD_HASH::DIGEST_LENGTH] {};

    MetaLine() = default;
};

class MwrmLineReader
{
public:
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
    InTransport ibuf;

public:
    explicit MwrmLineReader(InTransport reader_buf) noexcept
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
    explicit MwrmReader(InTransport ibuf) noexcept;

    void read_meta_headers();

    void set_header(MetaHeader const & header);

    Transport::Read read_meta_line(MetaLine & meta_line);

    void read_meta_hash_line(MetaLine & meta_line);

    MetaHeader const& get_header() const
    {
        return this->header;
    }

private:
    MwrmLineReader line_reader;
    MetaHeader header;

    Transport::Read read_meta_hash_line_v1(MetaLine & meta_line);

    Transport::Read read_meta_line_v1(MetaLine & meta_line);

    enum class FileType : bool { Hash, Mwrm };
    Transport::Read read_meta_line_v2(MetaLine & meta_line, FileType file_type);
};


struct MwrmWriterBuf
{
    using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];

    void write_header(MetaHeader const & header) noexcept;
    void write_header(uint16_t width, uint16_t height, bool has_checksum) noexcept;

    void write_hash_file(MetaLine const & meta_line) noexcept;
    void write_hash_file(
        char const * filename, struct stat const & stat,
        bool with_hash, HashArray const & qhash, HashArray const & fhash) noexcept;

    // reset buf then write{filename, stat, start_and_stop, hashs};
    void write_line(MetaLine const & meta_line) noexcept;
    // reset buf then write{filename, stat, start_and_stop, hashs};
    void write_line(
        char const * filename, struct stat const & stat,
        time_t start_time, time_t stop_time,
        bool with_hash, HashArray const & qhash, HashArray const & fhash) noexcept;

    array_view_const_char buffer() const noexcept;
    char const * c_str() const noexcept;
    bool is_full() const noexcept;

    void reset_buf() noexcept
    {
        this->len = 0;
    }

private:
    friend class PrivateMwrmWriterBuf;

    static const std::size_t max_header_size = 42;
    char mes[MwrmLineReader::line_max + max_header_size + 1 /*null terminator*/];
    std::size_t len = 0;
};
