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

#include <cerrno>
#include <cstdio> // snprintf

#include "transport/in_meta_sequence_transport.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"


InMetaSequenceTransport::InMetaSequenceTransport(
    CryptoContext & cctx,
    const char * filename,
    const char * extension,
    EncryptionMode encryption)
: cfb(cctx, encryption)
, buf_meta(cctx, encryption)
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

InMetaSequenceTransport::~InMetaSequenceTransport()
{
    this->buf_close();
    if (this->buf_meta.is_open()) {
        this->buf_meta.close();
    }
}

bool InMetaSequenceTransport::disconnect()
{
    this->buf_close();
    return true;
}

bool InMetaSequenceTransport::next()
{
    if (!this->status) {
        LOG(LOG_ERR, "InMetaSequenceTransport::next: Invalid status!");
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    this->buf_close();

    if (Read::Eof == this->buf_next_line()) {
        this->status = false;
        LOG(LOG_ERR, "InMetaSequenceTransport::next: No more line!");
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
    }

    ++this->seqno;
    this->cfb.open(this->meta_line.filename);
    return true;
}

void InMetaSequenceTransport::buf_close()
{
    if (this->cfb.is_open()) {
        this->cfb.close();
    }
}

Transport::Read InMetaSequenceTransport::buf_next_line()
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

Transport::Read InMetaSequenceTransport::do_atomic_read(uint8_t * data, size_t len)
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
