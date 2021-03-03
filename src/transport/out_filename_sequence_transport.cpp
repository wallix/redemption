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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Christophe Grosjean, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#include "transport/out_filename_sequence_transport.hpp"
#include "utils/strutils.hpp"
#include "utils/log.hpp"

#include <cstring>


OutFilenameSequenceTransport::FilenameGenerator::FilenameGenerator(
    const char * const prefix,
    const char * const filename,
    const char * const extension)
: last_filename(nullptr)
, last_num(-1u)
{
    int len = snprintf(this->filename_gen, sizeof(this->filename_gen), "%s%s-", prefix, filename);
    if (len <= 0
     || len >= int(sizeof(this->filename_gen) - 1)
     || !utils::strbcpy(this->extension, extension)
    ) {
        LOG(LOG_ERR, "Filename too long");
        throw Error(ERR_TRANSPORT);
    }

    this->filename_suffix_pos = std::size_t(len);
}

const char * OutFilenameSequenceTransport::FilenameGenerator::get(unsigned count) const
{
    if (count == this->last_num && this->last_filename) {
        return this->last_filename;
    }

    snprintf( this->filename_gen + this->filename_suffix_pos
            , sizeof(this->filename_gen) - this->filename_suffix_pos
            , "%06u%s", count, this->extension);
    return this->filename_gen;
}

void OutFilenameSequenceTransport::FilenameGenerator::set_last_filename(unsigned num, const char * name)
{
    this->last_num = num;
    this->last_filename = name;
}


OutFilenameSequenceTransport::OutFilenameSequenceTransport(
    const char * const prefix,
    const char * const filename,
    const char * const extension,
    const int groupid,
    std::function<void(const Error & error)> notify_error)
: filegen_(prefix, filename, extension)
, buf_(invalid_fd(), notify_error)
, groupid_(groupid)
{
    this->current_filename_[0] = 0;
}

char const* OutFilenameSequenceTransport::seqgen(unsigned i) const noexcept
{
    return this->filegen_.get(i);
}

bool OutFilenameSequenceTransport::next()
{
    if (!this->status) {
        LOG(LOG_ERR, "OutFilenameSequenceTransport::next: Invalid status!");
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    if (this->do_next()) {
        this->status = false;
        LOG(LOG_ERR, "OutFilenameSequenceTransport::next: Create next file failed!");
        throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
    }

    ++this->seqno_;
    return true;
}

bool OutFilenameSequenceTransport::disconnect()
{
    return this->do_next();
}

OutFilenameSequenceTransport::~OutFilenameSequenceTransport()
{
    this->do_next();
}

void OutFilenameSequenceTransport::do_send(const uint8_t * data, size_t len)
{
    if (!this->buf_.is_open()) {
        this->open_filename(this->filegen_.get(this->num_file_));
    }
    this->buf_.send(data, len);
}

/// \return 0 if success
int OutFilenameSequenceTransport::do_next()
{
    if (this->buf_.is_open()) {
        this->buf_.close();
        // LOG(LOG_INFO, "pngcapture: \"%s\" -> \"%s\".", this->current_filename_, this->rename_to);
        return this->rename_filename() ? 0 : 1;
    }

    return 1;
}

void OutFilenameSequenceTransport::open_filename(const char * filename)
{
    snprintf(this->current_filename_, sizeof(this->current_filename_),
                "%sred-XXXXXX.tmp", filename);
    const int fd = ::mkostemps(this->current_filename_, 4, O_WRONLY | O_CREAT);
    if (fd < 0) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }
    // LOG(LOG_INFO, "pngcapture=%s", this->current_filename_);
    // TODO PERF used fchmod
    if (chmod(this->current_filename_, this->groupid_ ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
        LOG( LOG_ERR, "can't set file %s mod to %s : %s [%d]"
            , this->current_filename_
            , this->groupid_ ? "u+r, g+r" : "u+r"
            , strerror(errno), errno);
    }
    this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
    this->buf_.open(unique_fd{fd});
}

const char * OutFilenameSequenceTransport::rename_filename()
{
    this->filegen_.set_last_filename(-1u, "");
    const char * filename = this->filegen_.get(this->num_file_);
    this->filegen_.set_last_filename(this->num_file_, this->current_filename_);

    const int res = ::rename(this->current_filename_, filename);
    // LOG( LOG_INFO, "renaming file \"%s\" to \"%s\""
    //    , this->current_filename_, filename);
    if (res < 0) {
        LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed error=%d : %s"
            , this->current_filename_, filename, errno, strerror(errno));
        return nullptr;
    }

    this->current_filename_[0] = 0;
    ++this->num_file_;
    this->filegen_.set_last_filename(-1u, "");

    return filename;
}
