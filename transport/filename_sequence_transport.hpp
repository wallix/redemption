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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_FILENAME_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_FILENAME_SEQUENCE_TRANSPORT_HPP

#include "buffer/filename_sequence_buf.hpp"
#include "buffer_transport.hpp"
#include "sequence_generator.hpp"

// struct InFilenameSequenceTransport
// : InBufferTransport<transbuf::ifile_buf>
// {
//     InFilenameTransport(const char * filename)
//     {
//         if (this->open(filename) < 0) {
//             LOG(LOG_ERR, "failed opening=%s\n", filename);
//             throw Error(ERR_TRANSPORT_OPEN_FAILED);
//         }
//     }
// };


struct OutFilenameSequenceTransport
: OutBufferTransport<transbuf::filename_sequence_buf>
{
    OutFilenameSequenceTransport(FilenameGenerator::Format format,
                                 const char * const prefix,
                                 const char * const filename,
                                 const char * const extension,
                                 const int groupid,
                                 auth_api * authentifier = NULL,
                                 unsigned verbose = 0)
    : OutFilenameSequenceTransport::TransportType(
        transbuf::filename_sequence_params(format, prefix, filename, extension, groupid))
    {
        (void)verbose;
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    virtual bool next()
    {
        return !this->close();
    }

    using OutFilenameSequenceTransport::seqgen;
};

#endif
