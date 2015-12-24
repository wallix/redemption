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

#ifndef REDEMPTION_TRANSPORT_CRYPTO_OUT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_CRYPTO_OUT_META_SEQUENCE_TRANSPORT_HPP

#include "out_meta_sequence_transport_with_sum.hpp"
#include "buffer/crypto_filename_buf.hpp"
#include "utils/genrandom.hpp"
#include "fileutils.hpp"

namespace detail
{
    struct ocrypto_filter
    : transfil::encrypt_filter
    {
        ocrypto_filter(CryptoContext &)
        {}

        template<class Buf>
        int open(Buf & buf, CryptoContext & cctx, char const * filename) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];
            get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == compute_hmac(trace_key, cctx.crypto_key, derivator)) {
                return -1;
            }

            unsigned char iv[32];
            cctx.random(iv, 32);
            return transfil::encrypt_filter::open(buf, trace_key, &cctx, iv);
        }
    };
}

using CryptoOutMetaSequenceTransport = detail::OutHashedMetaSequenceTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        detail::ocrypto_filter,
        transbuf::ocrypto_filename_buf,
        transbuf::ocrypto_filename_buf
    >,
    detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>
>;

#endif
