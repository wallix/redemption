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

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"


class InMetaSequenceTransport : public Transport
{
public:
    using EncryptionMode = InCryptoTransport::EncryptionMode;

    InMetaSequenceTransport(
        CryptoContext & cctx,
        const char * filename,
        const char * extension,
        EncryptionMode encryption,
        Fstat & fstat);

    ~InMetaSequenceTransport();

    [[nodiscard]] MetaLine const & get_meta_line() const noexcept
    {
        return this->meta_line;
    }

    WrmVersion get_wrm_version()
    {
        return this->mwrm_reader.get_header().version;
    }

    bool disconnect() override;

    [[nodiscard]] time_t begin_chunk_time() const noexcept
    {
        return this->meta_line.start_time;
    }

    [[nodiscard]] time_t end_chunk_time() const noexcept
    {
        return this->meta_line.stop_time;
    }

    [[nodiscard]] const char * path() const noexcept
    {
        return this->meta_line.filename;
    }

    bool next() override;

private:
    void buf_close();

    Read buf_next_line();

    Read do_atomic_read(uint8_t * data, size_t len) override;

    InCryptoTransport cfb;
    InCryptoTransport buf_meta;

    MwrmReader mwrm_reader;

    MetaLine meta_line;
    char meta_path[2048];
    bool status = true;
};
