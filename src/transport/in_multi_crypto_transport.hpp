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

#pragma once

#include "transport/crypto_transport.hpp"

#include <string>
#include <vector>


struct InMultiCryptoTransport final : public Transport
{
    using EncryptionMode = InCryptoTransport::EncryptionMode;

    InMultiCryptoTransport(
        std::vector<std::string> filenames,
        CryptoContext & cctx,
        EncryptionMode encryption);

    bool disconnect() override;

    bool next() override;

private:
    Read do_atomic_read(uint8_t * data, size_t len) override;

    InCryptoTransport in_file;
    std::vector<std::string> filenames;
    std::size_t pos = 0;
};
