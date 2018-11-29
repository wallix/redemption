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

#include <cerrno>
#include <cstdio> // snprintf

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"


class InMetaSequenceTransport : public Transport
{
public:
    using EncryptionMode = InCryptoTransport::EncryptionMode;

    InMetaSequenceTransport(
        CryptoContext & cctx,
        const char * filename,
        const char * extension,
        EncryptionMode encryption,
        Fstat & fstat)
    : cfb(cctx, encryption, fstat)
    , begin_time(0)
    , buf_meta(cctx, encryption, fstat)
    , mwrm_reader(this->buf_meta)
    {
        char meta_filename[1024];
        // concat
        {
            int res = std::snprintf(meta_filename, sizeof(meta_filename), "%s%s", filename, extension);
            if (res >= int(sizeof(meta_filename))) {
                throw Error(ERR_TRANSPORT);
            }
        }
        this->buf_meta.open(meta_filename);

        this->mwrm_reader.read_meta_headers();

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

    MetaLine const & get_meta_line() const noexcept
    {
        return this->meta_line;
    }

    WrmVersion get_wrm_version()
    {
        return this->mwrm_reader.get_header().version;
    }

    void set_begin_time(time_t begin_time)
    {
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
    {
        return this->meta_line.filename;
    }

    bool next() override
    {
        if (!this->status) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        this->buf_close();

        if (Read::Eof == this->buf_next_line()) {
            this->status = false;
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

    Read buf_next_line()
    {
        if (Read::Eof == this->mwrm_reader.read_meta_line(this->meta_line)) {
            return Read::Eof;
        }

        if (!file_exist(this->meta_line.filename)) {
            char original_path[1024] = {};
            char basename[1024] = {};
            char extension[256] = {};
            char filename[3327] = {};

            canonical_path( this->meta_line.filename
                          , original_path, sizeof(original_path)
                          , basename, sizeof(basename)
                          , extension, sizeof(extension));
            std::snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

            if (file_exist(filename)) {
                utils::strlcpy(this->meta_line.filename, filename);
            }
        }

        return Read::Ok;
    }

    Read do_atomic_read(uint8_t * data, size_t len) override
    {
        for (;;) {
            if (!this->cfb.is_open()) {
                if (Read::Eof == this->buf_next_line()) {
                    return Read::Eof;
                }
                ++this->seqno;
                this->cfb.open(this->meta_line.filename);
            }

            if (Read::Ok == this->cfb.atomic_read(data, len)) {
                return Read::Ok;
            }

            this->cfb.close();
        }
    }

    InCryptoTransport cfb;
    time_t            begin_time;
    InCryptoTransport buf_meta;

    MwrmReader mwrm_reader;

    MetaLine meta_line;
    char meta_path[2048];
    bool status = true;
};
