/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP

#include "detail/meta_opener.hpp"
#include "buffer_transport.hpp"
#include "buffer/file_buf.hpp"
#include "buffer/input_output_buf.hpp"

struct InMetaSequenceTransport
: InBufferTransport<
    transbuf::reopen_input<transbuf::ifile_base, detail::MetaOpener<transbuf::ifile_base> >,
    detail::in_meta_nexter
>
{
    InMetaTransport(const char * filename, const char * extension)
    : InMetaSequenceTransport::TransportType(detail::temporary_concat(filename, extension).str)
    {}

    unsigned begin_chunk_time() const /*noexcept*/
    { return this->impl().get_begin_chunk_time(); }

    unsigned end_chunk_time() const /*noexcept*/
    { return this->impl().get_end_chunk_time(); }

    const char * path() const /*noexcept*/
    { return this->impl().get_path(); }

    unsigned get_seqno() const /*noexcept*/
    { return this->impl().get_seqno(); }
};

#endif
