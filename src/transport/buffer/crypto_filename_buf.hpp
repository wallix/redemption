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

#ifndef REDEMPTION_TRANSPORT_BUFFER_CRYPTO_FILENAME_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_CRYPTO_FILENAME_BUF_HPP

#include "transport/buffer/file_buf.hpp"
#include "transport/filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace transbuf {
    namespace detail {
        template<class Buf>
        int init_trace_key(Buf & buf, CryptoContext * cctx, const char * filename, mode_t mode, unsigned char * trace_key)
        {
            unsigned char derivator[DERIVATOR_LENGTH];
//            printf("init_trace_key: cctx.get_derivator\n");

            cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == cctx->compute_hmac(trace_key, derivator)) {
                return -1;
            }

            return buf.open(filename, mode);
        }
    }
}

#endif
