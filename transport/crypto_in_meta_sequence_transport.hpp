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

#ifndef REDEMPTION_TRANSPORT_CRYPTO_IN_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_CRYPTO_IN_META_SEQUENCE_TRANSPORT_HPP

#include "detail/meta_opener.hpp"
#include "buffer_transport.hpp"
#include "buffer/input_output_buf.hpp"
#include "buffer/crypto_filename_buf.hpp"

struct CryptoInMetaSequenceTransport
: InBufferTransport<
    transbuf::reopen_input<transbuf::icrypto_filename_base, detail::MetaOpener<transbuf::icrypto_filename_base> >,
    detail::in_meta_nexter
>
{
    CryptoInMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : CryptoInMetaSequenceTransport::TransportType(transbuf::buf_params(
        crypto_ctx, transbuf::buf_params(crypto_ctx, detail::temporary_concat(filename, extension).c_str())))
    {}

    CryptoInMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename)
    : CryptoInMetaSequenceTransport::TransportType(transbuf::buf_params(
        crypto_ctx, transbuf::buf_params(crypto_ctx, filename)))
    {}

    unsigned begin_chunk_time() const /*noexcept*/
    { return this->buffer().policy().get_begin_chunk_time(); }

    unsigned end_chunk_time() const /*noexcept*/
    { return this->buffer().policy().get_end_chunk_time(); }

    const char * path() const /*noexcept*/
    { return this->buffer().policy().get_path(); }

    unsigned get_seqno() const /*noexcept*/
    { return this->buffer().policy().get_seqno(); }
};

#endif
