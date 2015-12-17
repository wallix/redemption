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

#ifndef REDEMPTION_TRANSPORT_CRYPTO_IN_FILENAME_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_CRYPTO_IN_FILENAME_TRANSPORT_HPP

#include "buffer/crypto_filename_buf.hpp"
#include "mixin_transport.hpp"

struct CryptoInFilenameTransport
: InputTransport<transbuf::icrypto_filename_buf>
{
    CryptoInFilenameTransport(CryptoContext * crypto_ctx, const char * filename)
    : CryptoInFilenameTransport::TransportType(crypto_ctx)
    {
        if (this->buffer().open(filename, 0600) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};

#endif
