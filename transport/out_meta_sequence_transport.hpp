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

#ifndef REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP

#include "detail/meta_writer.hpp"
#include "mixin_transport.hpp"
#include "buffer/file_buf.hpp"
#include "fdbuf.hpp"

struct OutMetaSequenceTransport
: //SeekableTransport<
    OutputNextTransport<detail::out_meta_sequence_filename_buf<
        detail::empty_ctor<io::posix::fdbuf>,
        detail::empty_ctor<transbuf::ofile_base>
    >, detail::GetCurrentPath >
// >
{
    OutMetaSequenceTransport(
        const char * path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = NULL,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : OutMetaSequenceTransport::TransportType(
        detail::out_meta_sequence_filename_buf_param<>(now.tv_sec, format, path, basename, ".wrm", groupid))
    {
        (void)verbose;

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().mwrm(), path, width, height, this->authentifier);
    }

    virtual void timestamp(timeval now) /*noexcept*/
    {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    {
        return &(this->buffer().seqgen());
    }
};

#endif
