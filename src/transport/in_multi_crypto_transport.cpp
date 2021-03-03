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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "utils/log.hpp"
#include "core/error.hpp"
#include "transport/in_multi_crypto_transport.hpp"


InMultiCryptoTransport::InMultiCryptoTransport(
    std::vector<std::string> filenames,
    CryptoContext& cctx,
    InMultiCryptoTransport::EncryptionMode encryption)
: in_file(cctx, encryption)
, filenames(std::move(filenames))
{}

bool InMultiCryptoTransport::disconnect()
{
    if (this->in_file.is_open()) {
        this->in_file.close();
    }
    return true;
}

bool InMultiCryptoTransport::next()
{
    if (this->pos >= this->filenames.size()) {
        LOG(LOG_ERR, "WrmsTransport::next: No more line!");
        throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
    }

    this->in_file.open(this->filenames[this->pos].c_str());
    ++this->pos;
    return true;
}

Transport::Read InMultiCryptoTransport::do_atomic_read(uint8_t* data, std::size_t len)
{
    for (;;) {
        if (!this->in_file.is_open()) {
            if (this->pos >= this->filenames.size()) {
                return Read::Eof;
            }
            this->in_file.open(this->filenames[this->pos].c_str());
            ++this->pos;
        }

        if (Read::Ok == this->in_file.atomic_read(data, len)) {
            return Read::Ok;
        }

        this->in_file.close();
    }
}
