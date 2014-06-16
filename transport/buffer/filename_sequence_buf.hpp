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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILENAME_SEQUENCE_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_FILENAME_SEQUENCE_BUF_HPP

#include "log.hpp"
#include "fdbuf.hpp"
#include "dispatch_read_and_write.hpp"
#include "sequence_generator.hpp"

#include <cstdio>
#include <cstdlib>

namespace transbuf {

struct filename_sequence_params
{
    FilenameGenerator::Format format;
    const char * const prefix;
    const char * const filename;
    const char * const extension;
    const int groupid;

    filename_sequence_params(FilenameGenerator::Format format,
                             const char * const prefix,
                             const char * const filename,
                             const char * const extension,
                             const int groupid) /*noexcept*/
    : format(format)
    , prefix(prefix)
    , filename(filename)
    , extension(extension)
    , groupid(groupid)
    {}
};


class ofilename_sequence_buf
: public dispatch_write<ofilename_sequence_buf>
{
    friend class dispatch_core;

    FilenameGenerator filegen;
    char current_filename[1024];
    io::posix::fdbuf file;
    int seqno;

public:
    ofilename_sequence_buf(filename_sequence_params const & params)
    : filegen(params.format, params.prefix, params.filename, params.extension, params.groupid)
    {
        this->current_filename[0] = 0;
    }

    ~ofilename_sequence_buf() /*noexcept*/
    {
        this->close();
    }

    int close() /*noexcept*/
    {
        if (this->ready()) {
            if (this->file.close() < 0) {
                LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
            }
            this->filegen.set_last_filename(0, 0);

            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            const char * filename = this->filegen.get(this->seqno);
            const int res = ::rename(this->current_filename, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                , this->current_filename, filename, errno, strerror(errno));
            }

            this->current_filename[0] = 0;
        }
        return 0;
    }

    bool ready() const /*noexcept*/
    { return this->file.is_open(); }

    const FilenameGenerator & seqgen() const /*noexcept*/
    { return this->filegen; }

protected:
    io::posix::fdbuf & dispatch()
    { return this->file; }

    int init() /*noexcept*/
    {
        if (!this->ready()) {
            std::snprintf(this->current_filename, sizeof(this->current_filename), "%sred-XXXXXX.tmp", this->filegen.path);
            TODO("add rights information to constructor");
            const int res = this->file.open(::mkostemps(this->current_filename, 4, O_WRONLY | O_CREAT));
            if (res < 0) {
                return res;
            }
            if (chmod( this->current_filename, (this->filegen.groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR)) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->current_filename, strerror(errno), errno
                   , (this->filegen.groupid ? "u+r, g+r" : "u+r"));
            }
            this->filegen.set_last_filename(this->seqno, this->current_filename);
            ++this->seqno;
        }
        return 0;
    }
};

}

#endif
