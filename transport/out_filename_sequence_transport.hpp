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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_OUT_FILENAME_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_OUT_FILENAME_SEQUENCE_TRANSPORT_HPP

#include "detail/filename_sequence_policy.hpp"

#include "buffer_transport.hpp"
#include "fdbuf.hpp"

struct OutFilenameSequenceTransport
: OutBufferTransport<transbuf::output_buf<io::posix::fdbuf, detail::FilenameSequencePolicy> >
{
    OutFilenameSequenceTransport(FilenameGenerator::Format format,
                                 const char * const prefix,
                                 const char * const filename,
                                 const char * const extension,
                                 const int groupid,
                                 auth_api * authentifier = NULL,
                                 unsigned verbose = 0)
    : OutFilenameSequenceTransport::TransportType(
        detail::FilenameSequencePolicyParams(format, prefix, filename, extension, groupid))
    {
        (void)verbose;
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    { return &this->impl().seqgen(); }

    virtual void request_full_cleaning()
    {
        this->impl().request_full_cleaning();
    }
};

#endif
